# Reciclagem individual — v120, 03/11/2026

## Escopo e reprodução

O usuário aprovou o bug de nível como resolvido e pediu o segundo item. Confirmou que o botão defeituoso é o de **reciclar no detalhe de uma peça**, não o de lote. Não foram tratados heróis, tutorial ou seleção coletiva.

Criado package isolado `com.habby.kinjarun.offline.qa.equipbreak`, label/nome QA Reciclagem. Fixture artificial: conta nível2,1500 ouro/0 gemas/0 EXP, sem livros; três colares EquipId3002/qualidade2/parte3: UID1 nível5 não equipado, UID2 nível1 não equipado, UID3 nível3 equipado. Somente esses itens artificiais foram usados. Telefone, saves principais e acervo externo não foram lidos ou editados para fabricar o teste.

V118 usa a biblioteca normal v116 para reproduzir: o detalhe mostra Lv5/20, Livro de Forja de C e botão vermelho. A confirmação original prevê400 ouro/4 livros. Confirmar Desmontar às14:25:30 do relógio Android produz NRE em `ET.EquipHelper.<BreakEquip>d__1.MoveNext`, via `EquipManager.<BreakEquip>d__94` e `UIBreakEquipTips.OnClickTipsBreak`. Sem transação local instalada, peça/saldos permaneceram iguais.

## Contrato reaproveitado e limites da análise

Usados dump/índice/tabelas e imagem ARM64 já recuperados, sem repetir extração, recuperação ou mapeamento do APK. REA abriu a imagem exata (`A2A4510A6650A6BFC9C23949E466392A098F9A1C992E21194B531C1E45BABDBE`), mas `procedure_assembly` retornou provider_unavailable: Ghidra não configurado, Hopper sem suporte Windows, IDA não configurado. Sem instalar/configurar ferramentas. O decodificador existente `tools/metadata/trace_offline_player.py` do acervo foi reutilizado em memória para métodos conhecidos; saídas novas somente em `build/tests/equip-recycle/`.

Fluxo: `UIBreakEquipTips.OnClickTipsBreak`0x1559238 → `EquipManager.BreakEquip(int[])`0x2C3984C → `ET.EquipHelper.BreakEquip(int)`0x2C2D118 → MoveNext0xF3845C. A state machine requer Scene/Session, recebe `G2C_BreakEquip`, testa Error e chama `EquipManager.OnBreakEquipData`0x2C32EE0. Esse callback registra retorno/telemetria de **EquipLevelDown** e red point; o inventário precisa ser atualizado antes dele.

`GetBreakEquipReturn`0x2C37E0C é também chamado pelo preview original. Soma os custos das configurações de níveis anteriores e `EquipQualityCfg.extra_coin/extra_item` (zero nas qualidades1–15 do alvo). Material vem de `EquipCfg.upgrade_item`. Não derivar devolução dos custos atuais de melhoria nem inventar percentuais. O bridge passa a reduzir o nível para1, mantendo peça/UID/qualidade/parte/wear. Observação positiva cobre qualidade2, níveis5 e3; outras faixas não tiveram gameplay validado.

## Implementação

Hook verificado somente em `EquipHelper.BreakEquip`0x2C2D118, prólogo exato `FF4301D1 F44F03A9 FD7B04A9 FD030191`. Tarefa original `ETTask<bool>`, modelo/resposta `G2C_BreakEquip`, callback original `OnBreakEquipData`. UI/coroutine de confirmação permanecem originais.

Na thread Unity, valida UID único, registro completo, correspondência live/save da peça e wear, nível>1, saldos e limites de inteiro. Consulta a API original de devolução por parâmetros ref via IL2CPP. Prepara todos os modelos novos/antigos e os ObscuredInt antes de alterar dados. O payload de resposta contém EquipInfo anterior e RetItemIds/Nums; a alteração de inventário é feita por `OnEquipDataInitResp`/`OnItemInit`, ouro pelo setter original e `SyncValueToShow`, sem offsets gerenciados.

Transação usa o guard compartilhado de upgrades e journal/snapshots compatíveis com a recuperação existente no bootstrap. Snapshots, marcador e os três seeds são gravados com fsync/rename e sincronização do diretório. Readback confere peça/nível/qualidade/wear, ouro, livros e bytes dos seeds. Só depois chama o callback original e remove o marcador de commit. Falha tenta restaurar saves e managers; snapshots/journal conservados se a restauração falhar. Interrupção antes do commit será recuperada no bootstrap; **falha injetada/interrupção real ainda não testadas**. Nível1 é recusado antes de conceder retorno; a UI original também esconde o botão nesse estado. Double-tap/reentrada não foi provocado para aumentar cobertura artificialmente.

## Resultado no BlueStacks

V119 QA, mesma biblioteca ARM64 da v120 normal:

| Caso | Resultado observado |
|---|---|
| UID1, não equipado, nível5 | Preview400ouro/4livros; commit às14:30:30: nível5→1, ouro1500→1900, item20003:0→4; UID/qualidade/parte/wear mantidos |
| UID3, equipado, nível3 | Preview200ouro/2livros; commit às14:31:28: nível3→1, ouro1900→2100, livros4→6; continuou equipado |
| Restart PID13701→14568 | Home2100ouro/0gemas/0EXP; UIDs1/2/3 nível1/qualidade2/parte3, apenasUID3 equipado; initialized e bolsa confirmados |
| Detalhe equipado após restart | Lv1/20, ATQ15, botão Remover e livro6/1; botão vermelho de reciclar ausente. Página de equipamento recalculou ATQ total33→25 |

Nenhum item foi destruído, nenhuma gema/EXP concedida e nenhuma peça normal foi consumida. A transação foi única por peça e não se repetiu no restart. NRE preexistente de `UIHeroPage.HeroProperty` ainda ocorre ao abrir equipamento; não é corrigida nem atribuída à reciclagem.

Evidências locais em `build/tests/equip-recycle/`: `reproduce.txt`, `recycle-preview.png`, disassemblies focados, `v119-startup.txt`, `v119-preview.png`, `v119-once.txt/result.png`, `v119-worn-preview.png`, `v119-worn-result.txt/png`, `v119-restart.txt`, `v119-restart-bag.png`, `v119-level-one.png`.

## Build normal e reprodução

`offline/build.ps1 -EquipBreakFixture` é opt-in e exige o package exato QA; recusa audit1 antes de criar outputs e não aceita outro fixture junto. Seeds normais não são alterados. Fixture só inicializa instalação nova, sem repor após gameplay/restart. Guard de package exercitado com recusa esperada.

Biblioteca SHA256 `86644FB763BACB75145C402F8F94C91562E835C4CB319F75C70AB7E71FDA41DC`. Compilação ARM64, contrato JNI/D8/wrapper, hash original e assinatura aprovados; warning antigo de `equipment_rewards` sem uso.

- V119 `v119-local-equipment-recycle-qa`, package qa.equipbreak, APK SHA256 `AE23E343A6D36012ADA89E9E94B00E382E5089C7651B1AE88C71759130DB727B`.
- V120 `v120-local-equipment-recycle`, package audit1, **seeds normais**, APK SHA256 `568E74C12A54597D86C79DB94368B2A335F386D816A32C0D0E953F546646C60D`.

Prefixo completo dos packages: `com.habby.kinjarun.offline.`. Atualizações com `adb -s 127.0.0.1:6610 install -r`, mesmo certificado de laboratório. V120 instalada somente no BlueStacks, não no celular. Não desinstalar/limpar dados ao atualizar o perfil correspondente. BuildIds existentes não devem ser reutilizados.

**Identidade da instalação normal:** o registro `audit1-package-before.txt` já retornava `Unable to find package` antes da instalação da v120. Portanto `install -r` fez uma instalação nova: firstInstallTime/lastUpdateTime `2026-11-03 14:32:47`, não uma migração do audit1 anterior. Nenhuma desinstalação/limpeza foi executada pelo agente nesta rodada; a origem da ausência não foi determinada. PID15369 carregou initialized/1500ouro/0gemas/0EXP/bolsa vazia e hook instalado. Preservação de um save anterior na atualização da build normal não foi validada aqui; a atualização/restart com peças e recursos foi validada no package QA. QA reaberto ao concluir para inspeção.

```powershell
./offline/native/build-native.ps1
./offline/build.ps1 -BuildId NOVO -PackageName com.habby.kinjarun.offline.qa.equipbreak -EquipBreakFixture -NativeLibrary ./offline/build/arm64-v8a/libkinjaoffline.so -PackageApk
```

Resultado pronto para avaliação do usuário, sem passar ao bug de heróis até aprovação. Cobertura não inclui lote coletivo, outros EquipIds/qualidades, save real do celular ou fault injection.
