# Guia de manutenção do Kinja Run offline

## Reciclagem individual — v120 em avaliação

[Reprodução, contrato e testes](docs/testing/EQUIPMENT_RECYCLE_V120.md). Usuário aprovou nível como resolvido e confirmou falha no detalhe de uma peça. NRE online reproduzida em BreakEquip na v118 QA. V119 QA/v120 normal interceptam helper0x2C2D118 com prólogo conferido, usam GetBreakEquipReturn original, devolvem ouro/livros e retornam peça ao nível1 mantendo UID/qualidade/parte/wear. Modelos preparados, guard/journal/fsync, readback e rollback de saves/managers. Duas peças artificiais q2 foram validadas:5→1 não equipada (+400ouro/4livros) e3→1 equipada (+200/2); restart manteve2100ouro/6livros e as3peças. UI esconde reciclagem em nível1. V120 normal instalada somente no audit1 do BlueStacks, sem seeds QA; package já ausente antes de instalar, portanto instalação nova, não migração do perfil anterior. Nenhuma desinstalação/limpeza executada pelo agente. Lote/fault injection/outras faixas não validados; aguardar avaliação antes de heróis.

## Nível da conta — teste QA positivo e build normal v116

[Evidências completas](docs/testing/ACCOUNT_LEVEL_QA_V116.md). Usuário autorizou saves artificiais em packages separados. Migração 2/5838 → 4/838 confirmada em runtime e restart; coleta HangUp em outro QA passou 2/1990 +151 EXP → 3/141, ouro1500→1651, home/readback/restart concordantes. Teste revelou receiver boxed incorreto em `GetDecrypted` na v112: corrigido com `object_unbox` na v116 normal. V115 QA e v116 normal usam a mesma biblioteca, sem novas regras de EXP de combate. Audit1 preservado por update, sem importar save do celular. Fixtures em `build.ps1` são opt-in e restritos a packages QA exatos, sem reposição de recursos. Outros bugs aguardam avaliação do usuário.

## Nível da conta — correção v112 em avaliação

[Relatório e limites](docs/testing/ACCOUNT_LEVEL_V112.md). Imagem do usuário mostra nível 2 com 5838/2000 EXP. O crédito local salvava EXP sem avançar nível. V112 usa `AccountLevelCfg` original para consumir os custos, preservar excedente e atualizar Level/Exp juntos; também normaliza excesso de EXP no login, sem conceder recursos. Caso isolado 2/5838 → 4/838, idempotência e todos os limites passaram. ARM64 compilado; audit1 do BlueStacks atualizado e home preservada, sem mudança no celular/acervo. Passagem de nível em gameplay e migração do save avançado ainda não observadas. EXP de combate permanece sem fórmula implementada. Outros bugs aguardam aprovação do usuário após este item.

## BlueStacks — execução da v111 relatada pelo usuário

Coleta posterior em 03/11/2026: ADB conectado a127.0.0.1:6610, APK audit1 instalado confere com SHA256 da v111; abertura/home observadas e reinício carregou a baseline1500/0/0,inventário0. Android16/API36 com suporte ARM64 anunciado. Rede ativa por Ethernet virtual: execução sem rede e refresh pós-vitória permanecem pendentes. Evidências e limites em [BLUESTACKS.md](docs/testing/BLUESTACKS.md). O parágrafo seguinte registra o relato anterior à coleta instrumental.

Em 03/11/2026, o usuário informou que conseguiu executar normalmente o APK v111 no BlueStacks. Evidência por relato do usuário, sem captura/logs ou inspeção por este agente. Não promover esse resultado a validação do refresh pós-vitória, persistência, execução sem rede ou bateria completa. Detalhes e pendências em [docs/testing/BLUESTACKS.md](docs/testing/BLUESTACKS.md). Os registros anteriores de BlueStacks não testado referem-se ao momento da migração.


## Workspace leve após migração

Validação da migração em 03/11/2026: biblioteca AArch64 compilada e contrato Java/JNI/D8 aprovado na pasta nova; painel exercitado com referência externa e grafo. BuildId histórico v110 recusado antes de gerar saídas. Nenhum APK empacotado/instalado e nenhum gameplay ou BlueStacks validado nesta etapa. Detalhes em `docs/planning/migration-validation.json`.

A implementação de trabalho está em `kinja-run-local`; o acervo antigo permanece externo. Consulte [setup](docs/development/SETUP.md), [resultado da migração](docs/planning/MIGRATION_REPORT.md) e [BlueStacks](docs/testing/BLUESTACKS.md). `config/local.json` resolve APK/toolchains/assinatura sem copiá-los. O Atlas declara referências externas/ausentes; contagens do painel anterior pertencem ao snapshot do acervo, não ao checkout leve. Scripts de análise históricos podem exigir seus inputs locais. Nenhuma lógica de gameplay ou save foi modificada. Validação de compilação/migração não é validação Android ou BlueStacks.


## Exploração visual do projeto — Project Atlas

Abrir [project-atlas/index.html](project-atlas/index.html). O painel local integra sistemas, árvore física, filtros/busca, classes/métodos do índice recuperado, grafo por níveis, fluxos offline, marcos por BuildId e documentação integral com referências cruzadas. Instruções e escopo em [project-atlas/README.md](project-atlas/README.md); catálogo curado em `project-atlas/catalog.json`. Atualizar com `python project-atlas/build_inventory.py` na raiz e recarregar.

O painel é documentação somente leitura: não altera lógica, APKs, assinatura ou saves e não move arquivos do Android. Reaproveita os índices e evidências existentes, sem reiniciar engenharia reversa. Associações automáticas por nome/caminho são inferidas; citações documentais não são chamadas runtime. O inventário exclui dependências/toolchains/caches/chaves e registra as exclusões. A cobertura física não significa compreensão semântica integral de todos os assets/tipos.

**Precedência do checkpoint:** [OFFLINE_READINESS_AUDIT.md](analysis/native/OFFLINE_READINESS_AUDIT.md) registra v110 principal assinada e não instalada, lab6 ainda v107 e audit1 na v111. O refresh pós-vitória da v111 não teve vitória de gameplay validada. Esse checkpoint é posterior aos resumos v108/v109 abaixo; preservar a distinção de perfil/build e os limites de cada teste.

**Validação do painel:** geração do inventário e sintaxe JavaScript verificadas; navegação, árvore/filtros, seleção de arquivo, cópia, documento integral, método recuperado, grafo/seleção por classe, fluxos e busca global exercitados no navegador via servidor loopback. Isso não é nova validação de instalação ou gameplay Android. Abertura direta por `file://` é prevista por dados em script local, mas não foi exercitada pela automação de navegador, que permite somente HTTP/HTTPS.

## Auditoria de conta nova — v109 no perfil separado audit1

[Bateria de prontidão](analysis/native/OFFLINE_READINESS_AUDIT.md). Lab6/v107 permanece preservado; teste natural usa com.habby.kinjarun.offline.audit1, sem QA, seeds normais e mesmo keystore. V108 revelou login preso em67%: o cálculo dos menus exigia região3 ausente no seed inicial. `read_local_menu_progress` na v109 aceita esse caso apenas enquanto MaxRegion<3, mantendo capítulo/menus bloqueados; dados alcançados ausentes ou existentes malformados continuam erros. Não preencher capítulos artificialmente. Regressão isolada passou e conta nova chegou à home1500/0/0/Nv1 com bolsa vazia; v109 atualizada somente no audit1. Bateria continua em andamento, sem declarar progressão completa. Abertura offline do lab6 passou, com NRE de RecoverPower ainda presente.

## Unir tudo: adaptação offline autorizada, build atual v107

[Implementação e evidências](analysis/native/ONEKEY_OFFLINE_V107.md). O usuário autorizou expressamente política local independente da seleção remota desconhecida: trios iguais nas qualidades1–3, cascata até roxo, raiz de maior nível/UID, equipados excluídos. Empate da raiz pelo menorUID, materiais de menores níveis/maioresUIDs. Mantém sobras/ID/parte/nível, usa resposta/callback ET próprios, não faz sucessivos commits individuais. Journal/fsync, validação total save/live/receita/cache, modelos preparados, readback e rollback dos managers; snapshots/journal mantidos se a restauração falhar. Não inventar refunds/recursos. Loja preservada.

V107 instalada com adb install -r/lab keystore no lab6; APK `5F244C374A26EAA44558E8F61C93C3A88830E1EC403EB7BBD420043E3A0437F0`, .so `3973702C547BE9C709EF62EA0AA1E82CFBFD7C159D1FBEC05C462ADE068CEB2A`. Hook OneKeyCompoundEquip0x2C2D50C, prólogo `FF0301D1 F31300F9 FD7B03A9 FDC30091`. Sem QA/novas peças/wipe. O bootstrap calcula **dry-run sem consumo**, compara original195/live195 e desserializa respostas; projeção69 passos/138 materiais/31 resultados finais/57 sobreviventes. Os números projetados não significam que peças foram removidas/aplicadas.

Diagnósticos completos somente em filesDir privado: kinja_onekey_dryrun.json/preflight.json/last_commit.json, nunca usados para carregar itens. V105 que exportaria para armazenamento externo não foi instalada; revisão automática rejeitou essa ação e a implementação foi substituída por diagnóstico privado. Testes isolados do planejador/JSON/journal passaram. Preservação de195 registros e saldos após reinício v106 está em kinja-v106-validation.json. Gameplay/persistência de lote coletivo com consumo continua sem validação por incluir peças reais; não inferir permissão de consumo da autorização da política. Detalhes de evidências/reinício final no relatório.

Reinício v107/PIDs20496→22535 confirmado: preflight de195 itens e modelos/listas originais repetidos, saldos/projeção iguais, bolsa preservada visualmente. Logs capturaram195 registros antes/138 depois; todos os disponíveis iguais e57 sem cobertura externa, explicitados em kinja-v107-validation.json. Hash do APK instalado confere. Sem lote consumidor/callback de sucesso neste teste; não promover a gameplay validado. Fixtures sincronizadas/journal da fonte final passaram em kinja-v107-json-journal-regression.txt.

## Histórico: Unir tudo antes da autorização da política local

Investigação posterior à v102: [ONEKEY_1_9_1_INVESTIGATION.md](analysis/native/ONEKEY_1_9_1_INVESTIGATION.md). O helper ET0x2C2D50C aguarda resposta remota com EquipUids/CostEquips; o callback0x2C3372C usa inventário já atualizado. O legado ProtocolManager.HandleEquipOneKeyCompound não é essa rota e agrega todos os materiais retornados sem limitar à receita, renovando UID e aplicando devoluções. Não copiar esse handler como contrato ET nem anunciar coletiva funcional. Sem consumo/teste coletivo/build nova nesta investigação; v102 continua instalada. Falta evidência da seleção remota ou autorização explícita para uma política offline documentada. Há itens reais nos grupos; nenhum seeding/teste destrutivo foi feito.

Este documento é o mapa de trabalho para continuar a adaptação offline do Kinja Run 1.9.1. Foi escrito para que uma pessoa ou LLM possa receber um relato de erro, localizar o fluxo relevante, investigar antes de editar e distinguir o que foi observado do que ainda é hipótese.

## Objetivo e arquitetura

O projeto contém duas coisas diferentes:

1. `index.html`: protótipo independente para navegador. Ele não é o jogo original, não importa os bundles do APK e não compartilha o save do Android.
2. `offline/`: adaptador que empacota uma Activity JNI e uma biblioteca nativa sobre o APK Unity original. O combate, as telas, personagens e assets continuam sendo os do jogo original. O bridge substitui algumas respostas que antes vinham do servidor por respostas/saves locais.

O APK de entrada esperado é `kinja-run-1-9-1.apk`, SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. O adaptador foi feito para esse cliente ARM64 específico. Não aplicar RVAs, bytes de prólogo ou conclusões a outra versão sem nova análise.

O código IL2CPP original não é reescrito no APK. `offline/native/offline_bridge.cpp` instala redirecionamentos em memória durante a execução; `offline/build.ps1` troca o manifest/`level0` necessários ao wrapper e adiciona DEX, biblioteca e seeds.

## Mapa do repositório

### Fusão: estado atual v102 (14/10/2026)

A correção atual e as evidências supersedem as conclusões históricas de fusão v79–v84: [relatório 1.9.1/v102](analysis/native/FUSION_1_9_1_V102.md). Três causas foram confirmadas: alteração de Quality descartada por releitura da string antes do replace, cálculo de caches antes de abrir função102, e receita fixa de três cópias incompatível com faixas superiores. `EquipQualityCfg.cost` contém a receita explícita da qualidade **atual**; não criar EquipCompoundCfg ou tabela alternativa.

O bootstrap recalcula `UpdateEquipCompoundRedPoint` depois de `SetOpeningFunc`. O bridge individual conserva APIs `EquipCanCompound`/`EquipRelateCompound`, lê cost pelo array gerenciado, aceita um ou dois materiais e valida ID/parte/qualidade configurada, UID único, condição equipado e concordância do save com objetos vivos. Deserializa respostas antes do consumo, prepara journal/snapshots, grava qualidade da raiz e remove somente materiais, aplica `OnEquipDataInitResp`, verifica resultado e ausência dos materiais, então chama `OnGetCompoundEquip`. Em falha restaura save e managers. Moeda permanece igual. Não escrever offsets de objetos; tudo roda na thread Unity. A raiz conserva UID/EquipId/nível/parte/wear. Materiais equipados são recusados, e esse caso ainda requer contrato original de gameplay.

O observador de `UICompoundEquipTips.UpdateCompoundList` (`0x12F8530`, prólogo `FF4307D1 EAAB00FD E923166D FC6F17A9`) chama o original por trampoline e lê `_selected` via IL2CPP/List, emitindo `Fusion UI selected=[...]` antes da confirmação. Não identificar peças por posição: o comparador original não desempata pelo UID.

Build instalada `v102-fusion-atomic-recipes-selection-evidence`: APK `D4B367A3E54B7A30093F6237AA54ECBD36B3A61DBCDEE25DB121498FEB87E5FC`, `.so` `A7DED0DD84EC19309A7E58D3BD8E2537D0655F24E1FC56C0CDDC6492608DAD7D`. Mesmo lab6/certificado/firstInstallTime; compilação sem defines QA, atualização por `adb install -r`. BuildIds/APKs anteriores preservados, incluindo v57. v98 parcial ficou preservada; v99/v100/v101 são intermediárias e só v99/v102 foram instaladas.

Validação individual confirmada: uma única fusão das cópias QA existentes28/29/30, EquipId3002/q2/Nv.5/parte3/não equipado; raiz28 q3, materiais29/30 removidos, teto20→30/ATK31→43/novo atributo na UI original. Reinício carregou q3/Nv.5 e bolsa com azul Nv.5, sem reaparecimento dos materiais; saldo salvo7.490.720 ouro/938.307 gemas/1.000.640 EXP. Plano, logs e capturas `build/kinja-v102-*` especificados no relatório. Não repetir esses UIDs consumidos. Teste ARM64 do trecho real de JSON em `build/kinja-v99-json-regression.txt` reproduz o defeito antigo sem acessar save.

“Unir tudo” chama `ET.EquipHelper.OneKeyCompoundEquip`, não o helper individual; “detalhes” abre outra tela. A UI busca elegibilidade1–3, mas não estabelece a ordem/raízes/grupos da transação ET. Mantém-se sem hook até fechar esse contrato. Receitas superiores, material/raiz equipado, double tap, falha de callback e recuperação interrompida ainda não têm teste positivo de gameplay nesta correção. Não fabricar peças para ampliar cobertura. A loja ficou fora do escopo.

Para análise nativa, usar `libil2cpp.stripes.so` reproduzido do APK exato com XOR **e permutação** (`A2A4510A...`), não a antiga cópia `code-analysis.so` que pode apontar outro trecho num RVA protegido. Buffs e sufixos/receitas completos estão no relatório; há uma única qualidade azul e vermelho +4. As mensagens históricas de commit v79/v80 não provam qualidade persistida.

| Caminho | Para que serve |
|---|---|
| `AGENTS.md` | Instruções curtas que um agente deve ler automaticamente antes de editar. |
| `MAINTENANCE_GUIDE.md` | Este mapa de arquitetura, evidências, comandos e pendências. |
| `FUNCTIONAL_AUDIT.md` | Resultados por build de auditoria funcional no Android, com status pass/fail/não testado. |
| `REVERSE_ENGINEERING.md` | Registro inicial da inspeção estática. É histórico; trechos que dizem que não houve execução antecedem o bridge e os testes de dispositivo posteriores. |
| `README.md` | Visão geral e instruções do protótipo de navegador. |
| `offline/native/offline_bridge.cpp` | Hooks JNI/IL2CPP, sessão local, callbacks, recompensas e saves. Fonte principal para bugs da build Android offline. |
| `offline/native/build-native.ps1` | Compila a biblioteca ARM64 com o NDK. |
| `offline/build.ps1` | Prepara wrapper, aplica alterações ao pacote e, com `-PackageApk`, assina o APK. |
| `offline/prepare_wrapper.ps1` | Compila/valida a Activity JNI que chama o bridge antes de inicializar Unity. |
| `offline/assets/seed_*.json` | Estado inicial local de conta, regiões, itens, heróis e respostas ET. Não sobrescreve saves já criados no aparelho. |
| `analysis/il2cpp/dump.cs` | Tipos e assinaturas nomeados recuperados da metadata IL2CPP; não é código-fonte original. |
| `analysis/native/libil2cpp.code-analysis.so` | Imagem de análise usada para disassembly. Conferir a versão/hash do alvo antes de usar endereços. |
| `analysis/bundles/tables/clientdata.ht/*FunctionCfg*` | Tabela client-side de funcionalidades, IDs e requisitos de menu. |
| `build/` | APKs, bibliotecas e relatórios gerados. Não editar manualmente outputs; use um BuildId novo. |

## Fluxo de inicialização offline

1. `local.kinja.OfflineActivity` configura o bridge com o `filesDir` e `AssetManager` antes de chamar a Activity Unity original.
2. O worker espera `libil2cpp.so` e o domínio estarem prontos. A entrada de `AccountInit` é redirecionada somente depois de conferir os 16 bytes originais.
3. O hook `account_init_hook` roda em `AccountInit` na thread Unity. Ele lê ou cria `files/offline_save/seed_*.json`, repara saves anteriores e desserializa as respostas ET pelos tipos originais.
4. Os callbacks originais carregam itens, equipamento, talentos, heróis, regiões, tutorial, jogador e estado dos baús da Loja. Em seguida são definidos UID/PID locais, o relógio local e os status offline.
5. `FuncManager.SetOpeningFunc` recebe a lista de menus compatível com o progresso local. Por fim, o callback de jogador conclui a tarefa de login original.

Toda chamada a singleton, `runtime_invoke`, callback de gerente ou alteração de estado IL2CPP precisa permanecer no contexto/thread Unity. O worker é para aguardar o runtime e instalar hooks, não para manipular managers da Unity.

## Hooks atuais e onde investigar

Os endereços abaixo são offsets/RVAs usados pelo bridge sobre a base ASLR de `libil2cpp.so`. `patch_entry` exige que os primeiros 16 bytes coincidam com o prólogo conhecido e recusa instalar o hook se houver divergência.

| Método original | RVA usado | Comportamento offline |
|---|---:|---|
| `AccountInit` | `0x27972e0` | Substitui login remoto por callbacks locais. |
| `UIUnionPanel.DontHaveUnionPartAwake` | `0x186c74c` | Ignora warmup do painel de guilda online. |
| `UIUnionPanel.InitDontHaveUnionPartInfo` | `0x1870870` | Ignora carga de informação de guilda online. |
| `UIUnionPanel.UpdateDontHaveUnionPage` | `0x186e9b8` | Ignora atualização de página de guilda online. |
| `AdManager.ShowRewardAdWithCallback` | `0xF08120` | Completa localmente a callback de anúncio recompensado; não abre anúncio. |
| `UITreasureBoxPage.UpdateBuyDiamond` | `0x13DFB00` | Chama a atualização original e depois desativa a seção de ofertas pagas do prefab via APIs Unity/IL2CPP; mantém baús e seção Ouro. |
| `PurchaseManager.Purchase(int)` | `0x2AA0120` | Intercepta no modo offline e retorna sem iniciar fluxo de compra externa. Hook central instalado; clique real ainda não testado. |
| `FightHelper.EnterFight` | `0x29c3728` | Retorna `ETTask<bool>(true)` local e guarda o RegionId ativo. |
| `FightHelper.KillEnemy` | `0x29c4abc` | Retorna `G2C_KillEnemy` local; paga 5 ouro por inimigo, 25 por elite e 50 por boss. |
| `FightHelper.EndGame` | `0x29c3f44` | Determina vitória, persiste progresso/recompensas e devolve `G2C_EndGame` local. |
| `FightHelper.Resurgence` | `0x29c54c0` | Cria resposta local, chama `GameSceneManager.OnResurgenceSucceed` e retorna tarefa concluída para o fluxo original fechar o painel. |
| `FightDataManager.DeductEnemyHp` | `0x27C9D4C` | Só na região local 3 (Capítulo 2), dano positivo do jogador vira no mínimo 9999. Usa trampoline para chamar original. |
| `FightDataManager.DeductPlayerHp` | `0x27E027C` | Só na região local 3, dano recebido não forçado é reduzido a 10%, arredondado para cima. Kills forçados são preservados. Usa trampoline. |

Os endereços/prologues são específicos do APK. Não atualize um RVA por tentativa. Use o dump/metadata e disassembly do binário correto, verifique a assinatura ABI e mantenha a validação de bytes. Os trampolines atuais copiam 16 bytes e retomam em `entry + 16`; antes de reutilizar esse padrão, confirme que as instruções copiadas não dependem de PC-relative addressing e que a assinatura ARM64 está exata.

### Fluxo de reviver

O botão de anúncio percorre código original. A callback de anúncio é completada imediatamente; depois `FightHelper.Resurgence(bool)` entrega a resposta local. O handler `GameSceneManager.OnResurgenceSucceed` aplica a vida ao estado do jogo; a coroutine original ainda é responsável por fechar/tirar o overlay. Se o menu sumir mas o jogo ficar escuro/congelado, investigar essa ordem e o callback de gameplay — não adicionar um `CloseTween` isolado, pois isso pode fechar só a UI e deixar o herói morto ou input bloqueado.

### Fluxo de fim de partida e recompensa

`end_game_hook` consulta `FightDataManager.CurrentRegion` e contadores para inferir se venceu. `persist_local_clear` atualiza o save, `apply_local_reward_items` aplica as recompensas ao `ItemManager`, `refresh_local_regions` manda o save atualizado ao callback original de regiões e `make_end_response` monta a resposta ET. Cada camada deve ser analisada quando uma recompensa ou capítulo não aparecer; uma resposta de fim de jogo pode ser bem-sucedida mesmo se o item não foi persistido ou se a tela de progressão não foi atualizada.

## Menus e progressão local

`FuncManager.SetOpeningFunc(List<int>)` recebia do backend a lista de funcionalidades liberadas. Em modo offline, não havia resposta para preencher essa lista. Isso explicava menus bloqueados mesmo com nível/capítulo gravados corretamente.

`set_local_opening_functions` reconstruída no bridge consulta `seed_player.json` e `seed_regions.json`, e atualmente libera:

| ID | Menu | Regra local atual |
|---:|---|---|
| 101 | Talentos | Região local 2 concluída (Capítulo 1). |
| 102 | Equipamento | Conta no nível 2 ou superior. |
| 103 | Loja | Conta no nível 2 ou superior. |
| 116 | Recompensas de inatividade | Região local 3 concluída (Capítulo 2). |

Mapeamento observado: região 2 = Capítulo 1; região 3 = Capítulo 2. A configuração `FunctionCfg` está em `analysis/bundles/tables/clientdata.ht/8655918910526765208_FunctionCfgCfgData.json`; a enumeração está perto de `FuncType` em `analysis/il2cpp/dump.cs`.

A lista é montada no login. No estado atual, não se deve presumir que um menu novo se atualize imediatamente ao concluir capítulo dentro da mesma sessão; pode ser necessário voltar à tela inicial ou reiniciar até existir um refresh explícito. A integração de menus/atividades para capítulos posteriores e a tela de heróis usam regras/dados distintos e ainda precisam ser mapeadas. Na v31, logs confirmaram IDs `[101,102,103,116]` com nível 2 e capítulos 1/2 concluídos; auditoria posterior capturou a home no Capítulo 3 com alguns ícones visíveis, mas também encontrou exceções ao abrir Shop, HangUp e telas/ações de herói. Consulte `FUNCTIONAL_AUDIT.md` para cada evidência e lacuna.

No APK 1.9.1, `UITreasureBoxPage.UpdateTreasurePage` consulta `TreasureBoxCfg` para IDs 101, 115 e 108 e passa os objetos a `TreasureBoxItem.UpdateItem`. O crash v31–v34 vinha de `ShopManager.GetShopChestState(cfg.id)` retornar nulo porque `_shopChestState.List` não tinha os IDs pedidos. A v35 adicionou `seed_shop_chests.json` e entrega o modelo `ET.G2C_ShopChestState` por `ShopManager.OnGetChestState`, na thread Unity. Abertura do painel passou no dispositivo v35. Os registros locais usam `DayRemainNum=0`, `NextTime=0`, `TotalOpenTimes=0`, `GroupIds=[]` e `GroupNums=[]`; isso inicializa o estado, mas não implementa compra/abertura de baú. Os painéis e gasto de gemas continuam sem teste. O trace temporário v33 observou alguns widgets ausentes, mas o trace de v34 durante o crash mostrou que a exceção acontecia antes de `GetGameObject`; não atribuir o crash aos widgets.

No cliente 1.9.1, as ofertas pagas ficam em `UITreasureBoxPage/obj/ContentPanel/Top/BuyDiamond` no bundle `clientgameobjectsui48.ht`; o grupo contém `imgTitleBg`, seis `BuyDiamondItemN` e `diamondShopEffect`. A v38 tentou esconder cada `BuyDiamondItem`, mas o teste mostrou que `UpdateItem` não era chamado no catálogo vazio. A v39 chama a rotina original `UITreasureBoxPage.UpdateBuyDiamond` por trampoline validado e então localiza o caminho do prefab com `Transform.Find`, obtém o GameObject e chama `SetActive(false)` pelo runtime IL2CPP. No aparelho, o log confirma a desativação e a captura mostra os seis cartões `$0.99` ausentes enquanto os baús permanecem visíveis. A entrada central `PurchaseManager.Purchase(int)` em RVA `0x2AA0120` também tem hook de retorno sem chamar a loja externa; só a instalação foi confirmada, sem clique de ponta a ponta. Os botões de baú/oro que gastam gemas continuam em aberto porque podem depender do servidor. Os 16 bytes são específicos do APK 1.9.1 e verificados por `patch_entry`. Ver `FUNCTIONAL_AUDIT.md` para build/hash e evidências.

Ao testar a Loja pós-v35, não agrupe toda NullReference em `TreasureBoxItem`: o log v39 continha a exceção conhecida de `ET.HangUpHelper.GetHangUpData` chamada por `ActivityManager.RequestHangUpInfo` durante `Main.OpenMainPanel`. O modelo local de baús corrigiu o NRE original de `TreasureBoxItem.UpdateItem`. Em v41/v42, `GetHangUpData()` usa resposta local vazia e o painel abre sem NRE. A análise do `UITimeGainPanel.<OnClickReceive>d__45.MoveNext` (1.9.1) mostrou que a rotina retorna quando `_items` está vazia e só então chama `RewardHangUp(0)`; o seed HangUp vazio explica por que o hook type 0 não aparece. Não é evidência de falha de despacho do toque. A opção grátis de Volta Rápida chama `RewardHangUp(1)` e recebe resposta local vazia: o callback funciona, mas ainda não há cálculo/concessão de recompensa local. O botão Receber permanece visualmente habilitado com lista vazia; corrigir a regra/feedback sem suprimir exceções. Fluxos de gemas continuam sem teste.

Não incluir automaticamente IDs de compras, eventos remotos, guilda/competição ou serviços que exigem backend. Antes de liberar um menu, confira `enable`, `show`, tipo de requisito e a dependência do painel. Abrir o botão não significa que a tela funcionará offline.

## Dados persistidos e estado do aparelho

Os seeds empacotados ficam em `offline/assets/`. Na primeira inicialização, o bridge os copia para o armazenamento privado do aplicativo e passa os modelos aos managers originais. Depois disso, as cópias locais têm prioridade:

```text
<app filesDir>/offline_save/seed_player.json
<app filesDir>/offline_save/seed_shop_chests.json
<app filesDir>/offline_save/seed_regions.json
<app filesDir>/offline_save/seed_items.json
<app filesDir>/offline_save/seed_equipment.json
<app filesDir>/offline_save/seed_talents.json
<app filesDir>/offline_save/seed_heroes.json
<app filesDir>/offline_save/seed_guide.json
<app filesDir>/offline_save/last_fight.json
<app filesDir>/offline_bootstrap_status.json
```

Editar um seed em `offline/assets/` não muda o perfil já criado. Para corrigir saves existentes, implemente uma migração idempotente no bootstrap: ela deve reconhecer versão/estado antigo, preservar dados válidos e gravar apenas o reparo necessário. Evite pedir para apagar dados do usuário como primeira opção.

## Build e instalação de laboratório

Use PowerShell na raiz do repositório. Cada `BuildId` deve ser novo:

```powershell
./offline/native/build-native.ps1
./offline/build.ps1 `
  -BuildId v36-descricao-curta `
  -PackageName com.habby.kinjarun.offline.lab6 `
  -NativeLibrary ./offline/build/arm64-v8a/libkinjaoffline.so `
  -PackageApk
```

O APK é gerado em `build/kinja-run-offline-lab-vNN-descricao-curta.apk`. A build não instala automaticamente. Atualizações que devem preservar o save precisam usar o mesmo package de laboratório e o keystore de laboratório já configurado. Não distribuir a chave de assinatura nem sobrescrever o APK original. O versionamento continua incremental: escolha sempre um BuildId ainda não usado. A v35 adicionou estado local para baús; a v39 escondeu visualmente as ofertas de dinheiro real; v41/v42 substituíram as duas RPCs de HangUp; v43/v44 ocultaram o `x5` estático da fase. A v44 validou entrada na fase com Power 0, mas não mediu dedução partindo de um saldo positivo. Volta Rápida grátis alcança a resposta local, ainda vazia; recompensa calculada e fluxos que gastam gemas seguem sem implementação/teste. Consulte `FUNCTIONAL_AUDIT.md` para hash e limites da validação do dispositivo.

Na build v44, a divergência energia `∞`/`x5` foi rastreada ao nó estático `imgPower` no prefab, não a custo aplicado dinamicamente. `UIMainPanel.DoOpen` chama o original por trampoline com prólogo conferido e desativa somente esse GameObject via APIs Unity/IL2CPP originais na thread Unity. A captura confirma o rótulo removido. A instrumentação de `OnClickFight` lê Power via getter original e `ObscuredInt.GetDecrypted`; aparelho leu 0→0 ao entrar na Fase 1/10. Não edite o seed/salvamento para fabricar saldo: save atual é privado e não debuggable, portanto o custo com saldo positivo continua pendente.

Com ADB conectado e autorizado, comandos úteis:

```powershell
./tools/android/toolchain/platform-tools/adb.exe devices -l
./tools/android/toolchain/platform-tools/adb.exe install -r ./build/kinja-run-offline-lab-v32-descricao-curta.apk
./tools/android/toolchain/platform-tools/adb.exe logcat -c
./tools/android/toolchain/platform-tools/adb.exe shell am force-stop com.habby.kinjarun.offline.lab6
./tools/android/toolchain/platform-tools/adb.exe shell monkey -p com.habby.kinjarun.offline.lab6 1
./tools/android/toolchain/platform-tools/adb.exe logcat -d -s KinjaOffline:I Unity:E AndroidRuntime:E
```

Uma instalação pode falhar com `INSTALL_FAILED_USER_RESTRICTED` até “Instalar via USB” ser habilitado e confirmado no aparelho. O pacote de laboratório não é debuggable; `run-as` não consegue ler o armazenamento privado. Use logs/statuses do bridge e observação do jogo, sem presumir acesso shell ao save.

## Playbook quando o usuário reportar um erro

1. Resuma o sintoma em termos reproduzíveis: tela, ação anterior, resultado esperado e resultado visto. Use imagem, texto exato e número de build quando disponíveis.
2. Identifique se é protótipo de browser ou APK Unity. Para o APK, mapeie o texto/botão à classe/configuração em `dump.cs` e às tabelas `.ht` antes de editar.
3. Reconstrua a sequência real: caller → método/manager → callback/resposta → persistência → atualização visual. Não deduza sucesso visual a partir de log de instalação ou compilação.
4. Leia o código em torno do fluxo e os logs `KinjaOffline`; confira `offline_bootstrap_status.json` e `last_fight.json` quando forem relevantes. Distinga causas provadas de hipóteses.
5. Faça a menor mudança local que corrija a causa. Preserve as rotas originais de UI e lifecycle quando elas realizam cleanup/input/estado de jogo.
6. Compile e empacote em um `BuildId` novo quando autorizado para continuar a build. Registre hash e o que foi realmente observado no aparelho. Não diga “corrigido” se apenas compilou.
7. Atualize `MAINTENANCE_GUIDE.md` quando uma descoberta mudar o mapa técnico ou status. Se uma observação antiga ficar falsa, marque-a como histórica em vez de deixar instruções contraditórias.

Quando faltar informação, faça uma pergunta curta e específica (por exemplo, tela atual, build, texto do cadeado ou log relevante) e continue a análise que não depende da resposta. Não peça ao usuário para repetir algo já relatado neste histórico.

## Diagnóstico rápido por sintoma

| Sintoma | Primeiro ponto de investigação |
|---|---|
| APK para em “Cannot Resolve destination Host” | Confirmar que é a build offline/lab, estado `offline_bootstrap_status.json` e sucesso do hook `AccountInit`. |
| Home continua com cadeados | Verificar log `Local menu progression`, IDs enviados a `SetOpeningFunc`, `Level`/`IsPass` do save e se `UpdateFuncLock`/reabertura da tela ocorreu. |
| Botão abre painel vazio/erro | `FunctionCfg`, classe/painel configurado, dependências remotas e null references no log Unity. Não confundir botão visível com serviço implementado. |
| Moeda/recompensa não aparece na partida | `kill_enemy_hook`, valor `Gold`, callback ET, contador da UI, `end_game_hook`, inventário persistido e chave/ID do item. |
| Capítulo não avança | `fight_won`, `persist_local_clear`, `MaxRegion`, `SelectRegion`, registro da região seguinte e `refresh_local_regions`. |
| Reviver fecha painel mas deixa tela escura ou sem input | Confirmar `OnResurgenceSucceed` aplicado antes da coroutine original concluir o fechamento; seguir o estado do herói, pause/time scale, overlay e input. |
| Buff de dano não muda combate | Interceptar o caminho real de aplicação de HP; escrever stats durante bootstrap não prova que sobrevivem a recalculações. Exigir logs de dano de saída/entrada no capítulo correto. |

## O que está validado e o que não está

**Evidência recente:** a v31 foi empacotada, instalada e inicializada. O log confirmou `[101,102,103,116]`; captura posterior mostrou a home no Capítulo 3, mas `adb input tap` foi bloqueado pelo Android. Logs de uso anteriores mostram exceções de runtime em Shop/baús, HangUp, talento e hero management. A lista completa, evidência e pontos não testados estão em `FUNCTIONAL_AUDIT.md`.

**Pendente:** desbloqueio de capítulos posteriores, atualização de menu sem reiniciar, todos os prêmios/telas, comportamento de cada painel remoto e teste de gameplay completo automatizado. A abertura dos ícones na home foi observada; funcionamento dos painéis é parcial e tem falhas confirmadas.

A validação de um fluxo deve registrar o nível de evidência:

- **Análise estática:** nomes, tabelas, assinaturas e instruções vistos nos artefatos locais.
- **Bridge/log:** método resolvido/chamada aceita durante execução; não prova por si só o efeito visual ou de gameplay.
- **Tela:** UI observada no aparelho após o fluxo.
- **Gameplay/save:** ação executada e consequência conferida dentro do jogo e após reabrir, quando aplicável.

## Registro de atualização

- **v31-menu-progression:** reconstrói a lista local de funções 101/102/103/116 no login. A chamada IL2CPP foi confirmada por log no Android; a home após essa versão ainda precisa de confirmação visual.
- **Reviver local:** o caminho local aplica `GameSceneManager.OnResurgenceSucceed` antes de retornar a tarefa ao fluxo de coroutine original, para que gameplay e limpeza da UI ocorram ambos.
- **Assistência do Capítulo 2:** região local 3 recebe dano de saída mínimo 9999 e dano recebido reduzido a 10%, nos hooks de aplicação de HP. O ajuste não se aplica aos outros capítulos.

### HangUp local v51 — cálculo, concessão e estado atual

O fluxo validado em v51 substitui `ET.HangUpHelper.GetHangUpData()` e `RewardHangUp(short)` mantendo os `ETTask` originais. `get_hangup_rates` consulta `TimeGainCfg` pelo capítulo local; no Capítulo 3, o dispositivo retornou 180 ouro e 180 EXP/h. O acúmulo normal é limitado a 6 h. O hook `reward_hangup_hook` type 0 só salva `LastClaim` quando o inventário confirma a concessão. A UI só despacha `UITimeGainPanel.OnClickReceive` se `_items` tiver ao menos um item; resposta vazia torna o botão visualmente enganoso e a coroutine retorna cedo.

IDs 101 (ouro) e 104 (EXP) são campos especiais de `PlayerDataManager`, não linhas comuns de `PlayerItem`. As tentativas com `ProtocolManager.AddCoin/AddExp`, `ItemManager.AddLocalItem` e `ItemManager.OnItemUpdate` não mudaram `ItemManager.GetOwnItemNum`. O caminho que funcionou foi construir `CodeStage.AntiCheat.ObscuredTypes.ObscuredInt` via `op_Implicit(int)`, chamar os setters originais `PlayerDataManager.set_PlayerGold/set_PlayerExp` e `SyncValueToShow`, e então conferir `GetOwnItemNum(101/104)`. O valor persistente deve ser atualizado somente nesses campos de `files/offline_save/seed_player.json` (`Coin`/`Exp`), preservando o restante do JSON; nunca substituir o seed inteiro pelo asset durante a coleta. v50 confirmou só runtime e perdeu o valor no relaunch; v51 escreve os campos localmente via temp + rename. Dispositivo validou uma coleta: 1500/0 → 1507/7; depois de `am force-stop` + relaunch, a home mostrou 1507 e barra 7/2000. Não houve limpeza de dados.

A state machine original `<OnClickReceive>d__45.MoveNext` retorna cedo se `_items` estiver vazia; não culpar o hitbox antes de confirmar a lista. A Volta Rápida type 1 calcula seis horas completas da mesma taxa, com limite 3/dia em `hangup_local_state.json`. Uma build anterior chegou à callback de anúncio grátis sem abrir anúncio, mas isso precedeu a concessão persistente v51. Na tela v51 capturada, somente a opção de 50 gemas estava visível; não tocar nela. Tipos pagos (diamond/power) retornam `Error=1`, mensagem PT-BR e custos zero, mas o feedback visual desse erro ainda não foi testado. Não afirmar que a Volta Rápida está completa até validar o botão grátis ou deixar a rota explicitamente indisponível na UI.

O consumo real de energia com saldo positivo permanece pendente. O save existente estava com `Power=0`; a entrada de fase observada anteriormente foi 0→0. Não editar `Power` apenas para teste sem um método que preserve/retorne fielmente o estado do jogador. Consulte `FUNCTIONAL_AUDIT.md` para builds, capturas, hashes e classificação detalhada.

### Moedas de combate — estado v51 (histórico, superado por v52–v54)

Na região local 4 da v51, cinco mortes comuns geraram 5 ouro cada na resposta `G2C_KillEnemy` e a HUD da partida mostrou 25. Encerrar o processo antes de `EndGame` manteve o saldo da home 1507; isso só comprova recompensa transitória, não vitória. O hook atual `kill_enemy_hook` preenche `G2C_KillEnemy.Gold` e não altera `PlayerDataManager` nem `seed_player.json`. `end_game_hook` monta a resposta a partir de `seed_end_game.json` (Gold=0); `persist_local_clear` grava os itens de marco/baú em `seed_items.json`, não moedas. Investigar a rotina original de consumo da resposta por kill e o acúmulo até `G2C_EndGame` antes de implementar crédito para evitar pagamento duplicado. Classificação à época da v51: **payout na HUD confirmado; payout persistente após vitória ainda não testado**. A tentativa parcial v51 foi interrompida sem `EndGame`, sem crash e sem progresso de fase salvo; evidências em `FUNCTIONAL_AUDIT.md`.

Na tentativa seguinte, a derrota chamou `EndGame` normalmente: log `passed=0`, HUD de combate chegou a 45, tela indicou nenhum loot e a home permaneceu em 1507 ouro/7 EXP; não avançou a região. Isso é compatível com descartar ganhos da run perdida, não é falha de crédito de vitória. Naquele ciclo v51 nenhuma run chegou a `passed=1`; as vitórias e a persistência foram testadas depois, conforme a seção v52–v54 ao final deste guia. Para uma vitória, comparar `PlayerDataManager.GetOwnItemNum(101/104)` e `seed_player.json` antes/depois e relançar sem limpar dados. Evidências de v51 e resultado da derrota estão no apêndice `Continuação — run completa até derrota...` de `FUNCTIONAL_AUDIT.md`.

Reviver grátis em gameplay v51 chamou `OnResurgenceSucceed` e o callback rewarded-ad local sem SDK/rede, restaurou 90 HP e permitiu retomar após selecionar habilidade; o painel de pausa apareceu após o callback e precisou do botão Continuar. Assim, callback/HP estão validados e a retomada autônoma da UI é parcial, não marcar o recurso como totalmente aprovado.

### Moedas de combate — liquidação e ordem assíncrona (v52–v54)

O bridge atribui ouro imediato em cada resposta original `G2C_KillEnemy` (comum 5, elite 25, boss 50) e mantém um acumulador restrito à run. `EndGame` só habilita liquidação quando os contadores originais confirmam `passed=1`; derrota e saída não creditam o acumulador. Usa-se `PlayerDataManager` e `ItemManager.GetOwnItemNum(101/104)` no thread Unity; depois somente `Coin`/`Exp` do save local existente são atualizados.

A ordem dos callbacks não é garantida: no dispositivo, o callback EndGame chegou 2 ms após o payout do boss e a última resposta KillEnemy veio 5 ms depois. O acumulador/total creditado da v53 aplica apenas a diferença no callback tardio, depois da confirmação de vitória, sem creditar derrota nem duplicar moedas. A tela de resultado usa `G2C_EndGame.Gold`; a v54 mantém esse objeto por `il2cpp_gchandle_new/get_target` e usa o setter `set_Gold` original se um kill chegar depois do EndGame. O objeto é liberado no próximo EnterFight. Não escreva campos gerenciados por offset.

Em runtime, v53 confirmou delta tardio de 5 (495 no EndGame + 5 após o último kill), saldo total 2502, mas a tela ainda exibiu 495. Em v54, outra vitória do Capítulo 2 processou 500 antes do EndGame: saldo 2502→3002, resultado exibiu 500 e o valor persistiu depois de reiniciar. Esse teste valida o caminho normal; falta capturar em v54 uma execução onde o setter tardio de resultado seja de fato chamado. A fórmula de EXP de combate segue desconhecida; não conceder XP com base no ouro.

Evidência, hash, passos e limitações estão em `FUNCTIONAL_AUDIT.md`, seção “Continuação — liquidação de moedas de combate v51–v54”. Atualizações no dispositivo devem continuar usando o package de laboratório, keystore existente e `adb install -r`; nunca limpar o perfil.

> **Atualização do estado de combate:** o bloco “Moedas de combate — validar liquidação no EndGame” acima é um registro histórico da v51. As vitórias v53/v54 confirmaram o crédito e a persistência de 500 ouro por run; veja “Moedas de combate — liquidação e ordem assíncrona (v52–v54)” abaixo. O ramo tardio de atualização da tela v54 ainda precisa ser observado diretamente.


A busca por `exp_reward` nas tabelas 1.9.1 encontrou regras de expedição (`RegionAdventureCfg`) e `hour_exp` de HangUp (`TimeGainCfg`), mas nenhuma fórmula de EXP por combate. Os seeds das respostas de kill e EndGame permanecem em zero e duas vitórias v54 deixaram o XP em 7 enquanto creditaram o ouro de combate. Não inferir XP de combate nem derivá-lo do ouro sem uma regra local comprovada; isso indica apenas falta de regra offline confirmada.

### Moedas de combate e gastos na economia offline

O ouro do combate só é útil se houver um destino local. No cliente 1.9.1, a aba principal de baús não é um sink de ouro: `TreasureBoxCfg` define custo de gemas (ex.: baú 101: 60 diamantes, 0 moedas); os produtos com dinheiro real ficam escondidos e `PurchaseManager.Purchase` não deve encaminhar compras. Não apresentar essa aba como lugar para gastar ouro.

Os nós de `TalentCfg` são o destino existente das moedas (nó 1001 custa 500 do item 101; nó 1002 custa 1000). Desde a v56, `ET.TalentHelper.UpgradeTalent(int)` tem hook local com prólogo 1.9.1 conferido: consulta o custo no `TalentCfg`, confere saldo e nível salvo/vivo, debita usando os setters originais de `PlayerDataManager`, envia `G2C_TalentUpgrade` ao callback original `TalentManager.OnTalentDataResp`, e persiste `TalentLevel`. A transação usa `talent_purchase_pending.json` para recuperar uma interrupção entre as duas gravações; essa recuperação ainda não foi exercitada por falha injetada. O método original sem sessão offline lança `NullReferenceException` antes da resposta. A v56 confirmou duas compras, ATQ/HP atualizados e persistência após relançar; detalhes no audit.

O relato foi reproduzido na v54 e a compra por talento foi corrigida na v56. `TalentCostCfg.exp` foi inicialmente interpretado como requisito, mas isso não estava provado; um nó de 500 moedas foi comprado com somente 7 EXP. Não usar esse campo para barrar compras sem traçar seu uso no IL2CPP. A Loja mantém baús cotados em gemas: com 0 gemas, o botão de bronze exibiu “Gemas insuficientes”. Não há, na UI atual, troca de ouro por equipamento; isso segue como lacuna de economia offline, separada da correção de talentos.

Esse roteiro de investigação foi concluído para os baús 101/115 na v63/v66. A compra local usa `BuyTreasureChest` → resposta `G2C_BuyChestMsg` → `OnBuyTreasureChestSucceed` → `AnalysisChest`/`DoOpenChest`, e atualiza/persiste itens e equipamentos por callbacks/seeds originais, tudo na thread Unity. `OpenBoxResp` (RVA `0x26D8BF4`) não é usado porque os caches de pool são nulos no bootstrap offline. A compra foi testada para bronze e diamante com entrega observada, saldos debitados e reinícios. O guard bloqueia reentrada concorrente e v66 adiciona janela de 650 ms contra toque duplicado; journal cross-file ainda falta. Não alterar preço sem conceder o conteúdo correspondente nem escrever campos de objetos gerenciados por offset. Para preços/pools/configuração e limitações, consulte a seção “Compra local de baús (1.9.1, v63/v66)” abaixo.

Registro histórico de baseline v57/v58: no Xiaomi a Loja mostrava bronze 60 gemas e diamante 250; havia 422 gemas/1845 ouro. Toques ADB não mudaram tela/saldo e ainda não havia hook de compra instalado. Isso foi supersedido pelas compras v63 e bloqueios v65/v66 descritos acima; o saldo atual depois dos testes é 112 gemas/1845 ouro. Na v65 os cartões monetizados seguem ocultos; anúncio grátis e conversões gemas→ouro têm hooks próprios de bloqueio e não devem ser confundidos com o callback de rewarded-ad usado por outras rotas offline. Consulte `FUNCTIONAL_AUDIT.md` para evidências atuais e limitações.

## Atualização v57 — gemas por recompensa de combate (14/10/2026)

Cada `FightHelper.KillEnemy` mantém o ouro atual e soma também `floor(gold_do_evento / 2)` gemas: 5 ouro comum → 2 gemas, 25 elite → 12 e 50 boss → 25. A divisão arredonda para baixo a cada evento, sem carregar frações entre kills. O acumulador de gemas e seu total creditado são independentes do ouro (`run_diamond_payout`/`run_diamond_credited`). Derrota e saída antes de `EndGame` não liquidam moeda. Vitória liquida ouro e gemas juntas quando `fight_won` retorna `passed=1`, e callback tardio soma somente o delta ainda não creditado.

O item de gemas é 102. No APK 1.9.1, o bridge lê `ItemManager.GetOwnItemNum(102)`, usa `ObscuredInt.op_Implicit` e `PlayerDataManager.set_PlayerDiamond`, além de `set_PlayerGold`/`set_PlayerExp` e `SyncValueToShow`, sempre no contexto Unity. O readback verifica os três saldos. A persistência altera somente `Coin`, `Diamond` e `Exp` no `offline_save/seed_player.json` existente e confirma que os campos anteriores batem com os valores vivos antes de gravar. A resposta original EndGame define `Gold` e `Diamond`; resposta tardia atualiza o objeto retido via GC handle e setters IL2CPP `set_Gold`/`set_Diamond`, sem acesso direto a offsets de objeto.

No Xiaomi, duas vitórias v57 mostraram 500 ouro/211 gemas no resultado por run. O saldo foi de 2345 ouro/0 gemas para 3345/422 após duas runs, e persistiu após encerrar e reiniciar o processo. Uma run curta encerrada com `am force-stop` não gravou os 5 ouro/2 gemas temporários. APK `build/kinja-run-offline-lab-v57-combat-gems.apk`, SHA-256 `013F92391D619AD611AC6DE9F9738C75764C7B0F6B907ECAEBC823A24CDCCD59`; package e firstInstallTime preservados. Veja a seção v57 do `FUNCTIONAL_AUDIT.md` para versão pré-instalada, saldos completos, screenshots e logs. Ainda falta observar no aparelho a ordem tardia `EndGame` antes do último kill já com gemas, e conferir nesse ramo a atualização tardia da UI.
### Compra local de baús (1.9.1, v63/v66)

O bridge intercepta `ShopManager.BuyTreasureChest(int,bool,Transform)` em RVA `0x26D8AD0`, só após validar os 16 bytes do prólogo. Executa no Unity thread, consulta `TreasureBoxCfg.GetCfg`, `ItemManager.GetOwnItemNum(102)` e `cost_diamond`, recusa ID não suportado/anúncio e confere o saldo vivo com `seed_player.json`. IDs locais permitidos: bronze 101/60 gemas usando pool `TBContentCfg` 1001–1036 (`box_id=1011`) e diamante 115/250 gemas usando 1175–1240 (`box_id=1021`). Seleção usa `base_weight` e número de sorteios da configuração.

Após validar o pool e preparar a resposta `ET.G2C_BuyChestMsg`, o hook debita via setter original `PlayerDataManager` (com readback e persistência no seed de jogador), acrescenta consumíveis/equipamentos aos seeds atuais e chama `ShopManager.OnBuyTreasureChestSucceed` original. Itens são recarregados por `ItemManager.OnItemInit`; equipamentos por `EquipManager.OnEquipDataInitResp`. A UI original usa esses IDs para a animação e tela de recebimento. Não chamar `OpenBoxResp` no caminho local: suas listas/caches ficam nulas offline e causaram crash nos experimentos v58–v62.

Uma flag atômica recusa concorrência/reentrada durante a execução síncrona; v66 também ignora outro toque do mesmo baú por 650 ms. Não existe request online nem callback assíncrono externo a ser aceito depois. Falha detectada de write/callback tenta rollback de preço e seeds, porém não há journal cross-file para encerramento abrupto. O debounce foi compilado/instalado, mas não exercitado via compra dupla para não consumir mais itens; mantenha outros IDs fechados até regras/preços/pools serem verificados na mesma versão do cliente.

Na v65, `PurchaseManager.Purchase(int)` retorna sem compra real e três métodos de `UITreasureBoxPage` são interceptados com prólogos 1.9.1 conferidos: `OnClickAdCoin` (RVA `0x13E0F80`), `OnClickBuyLotCoin` (`0x13E145C`) e `OnClickBuyHugeCoin` (`0x13E1730`). Isso impede anúncios da Loja e conversões gemas→ouro que dependem do servidor. Os cartões de OURO ainda aparecem visualmente porque o caminho de hierarquia para ocultar seu grupo não foi encontrado; logs e captura de toques confirmam handlers sem callback/requisição e sem alteração de saldo. Não confundir o hook genérico de anúncio (usado por algumas rotas offline como revive) com a oferta de ouro da Loja, que tem bloqueio próprio.

O saldo insuficiente foi tentado com 112 gemas contra preço 250 e ficou intacto, mas a página de conteúdo desapareceu e não houve log de entrada em `BuyTreasureChest`; a mensagem não foi validada. A comparação de saldo existe antes do branch original de aviso, porém a aparência do alerta precisa ser confirmada por um novo teste reproduzível. O APK final v66 `build/kinja-run-offline-lab-v66-local-gem-chests-debounce.apk` tem SHA-256 `5038CB596BD3043E208F17DDB0EA7A86CF0923F988E6AC6BF98628C77E7B0902`; consulte `FUNCTIONAL_AUDIT.md` para saldos/evidências.

## Atualização v67/v68: resgate, equipamento e drops

v67 transforma `UITreasureBoxPage.OnClickAdCoin` em resgate diário local de 4000 ouro. O clique não chama o SDK de anúncios; o saldo é atualizado por `PlayerDataManager`/`ItemManager` originais e persistido no `seed_player.json` com journal e marcação de data local. `UpdateBuyCoin` sincroniza o contador da UI por trampoline após conferir o prólogo 1.9.1. O aparelho confirmou 1845→5845 ouro, 52 gemas inalteradas, indicador de 0 resgates restantes e bloqueio de repetição no mesmo dia; reinício manteve o saldo. Mudar o relógio do aparelho não foi testado.

`EquipHelper.WearEquip(EquipPart,int)` agora atualiza os flags `IsWear` no save atual, rejeita UID/parte inválidos e reenvia `ET.G2C_EquipData` por `EquipManager.OnEquipDataInitResp`. A tarefa `ETTask<bool>` concluída usa fábrica/setter original. Sem offsets gerenciados. No aparelho, Vestir persistiu parte 3/UID 2 e o bootstrap carregou novamente o seed após reinício. A confirmação visual explícita do item vestido após reinício falta. O NRE `UIHeroPage.HeroProperty -> UpdatePage` continua ocorrendo ao entrar na tela; o inventário e os popups renderizam mesmo assim.

`FightHelper.KillEnemy` consulta `Monster.GetCfg.reward_id` e `RewardManager.RandomRewardPack` usando a tabela 1.9.1; os itens são enfileirados por instância e só persistidos em `EndGame` com vitória, por APIs e seeds existentes. A tabela Chapter 3 reward 100030 contém consumíveis 20001–20006 com peso 48 por entrada (peso total 9000, cerca de 0,53% por livro); a tabela de boss 100031 usa peso 100 por entrada. A observação v67 (histórica, antes do log v68 e da proteção v69) terminou em derrota sem loot. Na v68 os campos vazios foram confirmados; a entrega positiva, inventário e persistência foram validados na vitória v69, descrita abaixo.

O fluxo de revive segue o callback local já instalado. Na run v67, duas ativações completaram `GameSceneManager.OnResurgenceSucceed` e a callback de rewarded-ad sem anúncio e sem custo de gemas. A rota da Loja para baús continua sendo a v63: IDs 101/115, custos configurados 60/250 gemas, pools ponderados e concessão pelo callback original; v68 não altera compra. O hook de compra suficiente funciona e foi demonstrado anteriormente. O aviso de saldo insuficiente ainda precisa ser confirmado via toque real chegando ao handler.



## Resultado atualizado v69: drops e persistência

A instrumentação v68 registrou `RewardManager.RandomRewardPack` retornando `ItemId=0, ItemNum=1` em inimigos comuns. A tabela 1.9.1 explica a frequência: reward 100020 tem peso 8700 sem item, 45 por livro 20001–20006 e 30 para 300101 (total 9000; 0,5% por livro). Reward 100030 usa peso 8682 sem item e 48 por livro (0,53%). Bosses 100021/100031 escolhem um dos seis livros. Portanto, o sintoma de livros ausentes era compatível com a taxa original extremamente baixa.

v69 mantém o sorteador e as recompensas configuradas; como ajuste exclusivamente offline, dez rolagens vazias de reward IDs de capítulo (100020–100990, sufixo zero) na mesma run acionam um livro 20001–20006, count 1. Livro natural reinicia a contagem. Todos os itens ficam pendentes por instância e só são persistidos se `EndGame` confirmar vitória. A derrota/saída não grava o loot. Na vitória, callbacks `ItemManager.OnItemInit` e `EquipManager.OnEquipDataInitResp` e os seeds locais originais fazem a entrega/persistência.

Teste v69 no Xiaomi: vitória Capítulo 2/Região 3, fase 10/10; resultado 500 ouro/211 gemas e itens. O log confirma persistência de livros 20001–20005 e equipamento 300101. Após reinício, saldos 6345 ouro/263 gemas/350 EXP continuaram; bolsa mostrou Livro de Forja de C x4 e o Colar de Cura Santa ainda equipado. Build/hash e capturas: seção v69 de `FUNCTIONAL_AUDIT.md`.

A página de equipamento ainda lança NRE em `UIHeroPage.HeroProperty`, apesar de mostrar o equipamento e a bolsa. Saldo insuficiente segue sem validação ADB do handler; o teste tentado em v68 não debitou gemas. A regra de livro é local e limitada a drops comuns elegíveis; não altera baús nem concede item após derrota/abandono.


## Evidência de triagem v69 — 14/10/2026

O aparelho confirmou a v69 instalada por igualdade do SHA-256 do APK extraído via ADB com o artefato v69 documentado. Estado de referência preservado: Lv.2, Capítulo 3, 1845 ouro, 13 gemas, 350 EXP, 2 cristais 40001; Anel do Oráculo Lv.1/20, Livro de Forja de A 1/1. Capturas/logs adicionais estão listados na seção “Investigação pré-correção — build v69 no Xiaomi” de `FUNCTIONAL_AUDIT.md`.

A reprodução dos dois botões de melhoria terminou em `NullReferenceException` no `MoveNext` dos helpers originais 1.9.1 (`UpgradeEquip` e `OneKeyUpgradeEquip`); nenhum item/moeda foi consumido. O botão de um clique encaminha apenas o UID escolhido pelo fluxo `EquipManager.UpgradeEquip(uid, true)`. O bridge ainda não contém substituição offline para esses helpers.

A inicialização do Correio emite POST para mail config e lista; o domínio falha com `SocketError=11001`, quatro tentativas. Não há seed/callback local de Correio no bridge. Reabrir o app sem limpar dados manteve os saldos e capítulo na UI.

Pendente nesta triagem: corrigir os helpers com transação persistente e callbacks IL2CPP originais; confirmar o algoritmo one-key em 1.9.1; avaliar esconder Correio sem afetar outros ícones; inspecionar âncora/hierarquia real do botão de capítulo antes de alterar; e concluir o inventário de entradas de menu por gating/backend. Sem BuildId novo até completar essas análises.

## Estado atual após v72 — 14/10/2026

O APK-alvo exato continua `kinja-run-1-9-1.apk` (SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`). A v72 de laboratório instalada é `build/kinja-run-offline-lab-v72-local-equipment-transaction.apk`, SHA-256 `CC722CC4D664EF93245D778960E4CF0FECA1F935A1F48B42C41FBAC114E80E42`; package `com.habby.kinjarun.offline.lab6`, keystore DEV existente. Instalação por `adb install -r` manteve `firstInstallTime` em `2026-10-13 19:43:33`. Histórico, logs/capturas, causas e classificação das validações estão em `FUNCTIONAL_AUDIT.md`, seção “Resultado v72”.

`EquipHelper.UpgradeEquip` e `OneKeyUpgradeEquip` não podem continuar pela sessão online ausente na versão 1.9.1. Os hooks só são instalados após comparação dos prólogos exatos do APK. A melhoria local prepara o custo/configuração e JSON antes do gasto, cria snapshots/marcador, atualiza saves e managers por callbacks originais, valida live/save, protege contra reentrada e recupera snapshots no próximo bootstrap se um processo morrer com a transação pendente. A melhoria unitária do UID 4 foi testada: nível1→2, ATQ15→19, um Livro A e 100 ouro consumidos; reinício confirmou persistência. Não testar repetidamente se não houver recurso natural.

`OneKeyUpgradeEquip(uid)` recebe o UID selecionado, não o inventário. O bridge planeja níveis sequenciais para esse UID até limite ou falta de recursos e recusa material misto sem gasto parcial. O caminho positivo de oneKey ainda não foi validado: o NRE pré-correção foi reproduzido sem gasto e o último Livro A foi usado na validação unitária. Não descrever o “até o limite” como confirmação observada; antes de alterar semântica, obter evidência adicional da rotina exata no APK 1.9.1 e testar só com recursos naturais.

O Correio é backend-only (sem seed/entrega local). O nó `btnMail` e notificações `mail*red/point/notice/badge` são ocultados após `UIMainPanel.DoOpen`. O SDK ainda tenta carregar configuração/lista em background, com falhas DNS documentadas. **Status visual histórico da v72:** a rota do rótulo aplicava pivot/alinhamento, mas a captura mostrava o texto alto. A v73 adicionou âncora ao retângulo do botão; sua captura confirma alinhamento vertical e horizontal. Não usar varredura recursiva ampla da UI: v70 crashou nessa etapa; a captura e backtrace foram preservados e a rota usa `Transform.Find` exato.

Recompensas de cristal azul existem em tabelas de progresso por capítulo, mas o bridge atualmente concede localmente só o marco inicial do Capítulo 1; não fabricar cristal nem liberar Atividade ID104 (nível8/Capítulo7) ou ID125 (nível5). HangUp ID116 requer Capítulo2. Entradas que dependem de servidor/anúncio/pagamento devem seguir bloqueadas/ocultas.

Próxima prioridade registrada na v72 (histórico): centralizar o capítulo e validar OneKey. O alinhamento visual foi concluído na v73; agora confirmar o algoritmo OneKey exato no 1.9.1 antes de qualquer toque, e esperar recursos naturais suficientes. Em seguida avaliar resgate de recompensa azul por progressão local sem repetição. Consulte `FUNCTIONAL_AUDIT.md` para os detalhes e limitações antes de mexer nesses sistemas.

**Retificação de saldo — ação UI acidental já persistida:** depois do teste do Anel e da captura que mostrava 2465 ouro, uma navegação automatizada acionou melhoria unitária do UID2 (log `build/kinja-v72-final-action-logcat.txt`): gastou 100 ouro e um Livro C. Saldo final observado, após reinício: 2365 ouro/13 gemas/1070 EXP (`build/kinja-v72-final-after-extra-transaction-restart.png`). Não editar manualmente o save para reverter. UID2 não foi inspecionado visualmente após isso.

## Continuação após v73 — 14/10/2026

A v73 `v73-chapter-label-anchor` está instalada no Xiaomi pelo mesmo package/certificado; APK SHA-256 `7F36012506293B13C964EC9474FC001CE6AFF5FD5D302DC5963F9C1AD54127E5`. A atualização preservou `firstInstallTime=2026-10-13 19:43:33`. A captura `build/kinja-v73-home.png` confirma “Capítulo 3” centralizado vertical e horizontalmente; após force-stop/relaunch, `build/kinja-v73-after-restart.png` confirma o mesmo. O log `build/kinja-v73-after-restart-logcat.txt` confirma sucesso das chamadas `UIRect.SetAnchor`, `UIWidget.set_pivot` e `UILabel.set_alignment` (`anchor=1 pivot=1 alignment=1`) na thread Unity. Antes da atualização `build/kinja-current-before-v73.png` registrava 2365 ouro, 13 gemas e 1070/2000 EXP. Após reiniciar, esses saldos permaneceram; `build/kinja-equip-detail-before-v73.png` mostra o Anel do Oráculo Lv.2 e Livro A `0/1`. Nenhum toque de melhoria foi feito nesta passagem.

No OneKey, a versão 1.9.1 confirma passagem do UID selecionado até o helper assíncrono `ET.EquipHelper.OneKeyUpgradeEquip(int)`, não melhoria de todo o inventário. Não está confirmado quantos níveis aplica nem a regra de parada: `MoveNext` RVA `0xF3934C` não pôde ser descompilado via REA porque o provedor Ghidra não está configurado neste host. O bridge local ainda calcula várias melhorias sequenciais até limite/insuficiência; trate esse detalhe como hipótese não validada. Não testar OneKey até confirmar a semântica nativa e haver livros/ouro obtidos naturalmente suficientes. Para o Anel, há saldo para ouro, mas o requisito visível é Livro A `0/1`.

Próximo passo: confirmar algoritmo OneKey no binário exato 1.9.1, depois validar com recurso natural sem toque se insuficiente; adicionalmente, coletar no log do aparelho retornos explícitos das chamadas de âncora e confirmar a posição após reinício.

## Atualização v74/v75 — perfil QA e talentos azuis

O usuário designou o save atual como perfil de QA e autorizou prepará-lo com recursos abundantes; o perfil jogável futuro deverá ser novo/limpo. A opção `offline/native/build-native.ps1 -QaMaxProfile` compila uma preparação de laboratório que eleva moedas 101/102, EXP, nível de conta e os 56 IDs `ItemCfg.bag_item=1` para pelo menos 1.000.000 (nível 100). Usa as seeds já existentes e deixa o marker `offline_save/qa_max_profile_applied.json`. A preparação ocorre uma única vez; depois o marker evita repor compras em reinícios. Builds sem essa opção não alteram os saldos dessa forma. Não usar o perfil QA como progressão normal.

O handler original de talentos agora separa `TalentCfg.type=0` (`item=101`, `TalentLevel`/`CurUpgradeNum`) de `type=1` (`item=40001`, `ExtraTalentLevel`/`CurExtraUpgradeNum`). Para os nós azuis, valida o custo/tipo/level_limit do cfg, saldo live/save e sequência `2001 + ExtraTalentLevel`; persiste o débito em `seed_items.json`, atualiza ItemManager pelo callback original e entrega ambos os contadores ao `TalentManager.OnTalentDataResp`. Journal transacional cobre moeda 101 ou 40001 e o contador correspondente; antiga estrutura GoldBefore/GoldAfter continua lida para saves pendentes legados. Todas as invocações e callbacks permanecem na thread Unity e usam APIs IL2CPP originais.

No alvo 1.9.1, ID2001/Escolhido Divino custa 1 cristal e ID2002/Mão Divina custa 3; IDs 2001–2026 formam cadeia parent consecutiva. O defeito foi reproduzido na v73: hook recusou `currency=40001` antes de gastar. Na v74, gameplay registrou 2001 (saldo 1.000.000→999.999, extra 0→1) e 2002 (999.999→999.996, extra 1→2), com callback de inventário/talento bem-sucedido. A v74 reiniciava o piso QA e repunha recursos; a v75 corrigiu o preparador para uso único. No segundo reinício v75, o log `QA max profile already prepared; preserving subsequent gameplay spends` confirma o marcador ativo. Capturas e limites em `FUNCTIONAL_AUDIT.md`, seção v74/v75.

Build instalada v75: `build/kinja-run-offline-lab-v75-qa-profile-once-blue-talent-persist.apk`, SHA-256 `2171B6026F31DD6E391ABD9031B6E0376D57D2BD00A2B24BFE64B3CF9DD5802D`, package `com.habby.kinjarun.offline.lab6`, update por `adb install -r`, firstInstallTime preservado. APK original 1.9.1 continua SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. O efeito dos talentos em combate, saldo insuficiente, double-tap e recovery por falha injetada ainda não foram observados.

### Retificação QA v75 — ações acidentais e saldo inconclusivo

Uma sequência de toques ADB por coordenadas atingiu outros controles. `build/kinja-v75-unexpected-home-logcat.txt` registra talentos comuns 1006–1012 (31.500 ouro), cinco baús locais 115 (250 gemas cada; conteúdos 1175, 1200, 1191, 1205, 1178) e OneKey no UID3 (nível 2→10, 2.500 ouro e 17 livros 20002). Foram ações involuntárias; não usar isso para afirmar paridade do algoritmo OneKey com 1.9.1. Nada foi revertido ou editado manualmente.

Às 17:30:27, o bridge leu e persistiu 966.000 ouro, 998.750 gemas e 1.000.000 EXP. Depois de force-stop/relaunch sem entrada, `build/kinja-v75-unexpected-after-restart.png` mostrou aproximadamente 235 mil ouro e 998 mil gemas. `build/kinja-v75-unexpected-after-restart-logcat.txt` não contém gasto/reposição e confirma marker QA ativo. `run-as` não permitiu leitura direta do save privado; portanto saldo de ouro após restart permanece inconclusivo. Não continuar testes de gasto até diagnosticar a diferença. O aparelho ficou na home sem novos toques.

## Atualização v77 — conversão local de gemas em ouro

Defeito: os hooks 1.9.1 `UITreasureBoxPage.OnClickBuyLotCoin` (`0x13E145C`) e `OnClickBuyHugeCoin` (`0x13E1730`) retornavam sem ação. A tabela alvo exata (`951442175585326260_VariableCfgData.json`) dá lot `[20000,100]` e huge `[118888,500]` (ouro, gemas). `UpdateBuyDiamond` não deve ocultar o grupo OURO: os cards permanecem visíveis e os dois handlers têm agora transações locais.

`perform_shop_coin_exchange` consulta `Variable.GetCfg`, lê `int_array_value` com `System.Array.GetValue(Int32)`, valida contra os valores esperados do APK 1.9.1, compara moedas live/save, confere saldo/overflow, chama setters originais de PlayerDataManager e `SyncValueToShow`, confere readback e grava `Coin`+`Diamond` em uma única substituição atômica de `seed_player.json`. Um atomic guard previne reentrada entre handlers. O clique e callbacks rodam na Unity thread; sem offsets gerenciados. Pagamentos reais continuam ocultos e a opção de resgate diário continua local.

A tentativa v76 apontou que os símbolos C de arrays não estavam disponíveis no runtime e não gastou moeda; `build/kinja-v76-shop-lot-preconfirm-logcat.txt`. v77 usa API gerenciada. Um clique no pacote barato comprovou 198.200→218.200 ouro e 996.940→996.840 gemas; 1.000.000 EXP inalterado. O jogo mostrou 218K/996K após o clique e após reiniciar. Evidências: `build/kinja-v77-shop-before-purchase.png`, `build/kinja-v77-shop-lot-purchased.png`, `build/kinja-v77-shop-lot-purchase-logcat.txt`, `build/kinja-v77-after-restart-shop.png`, `build/kinja-v77-after-restart-logcat.txt`.

BuildId `v77-local-gem-gold-exchange-array-api`; APK `build/kinja-run-offline-lab-v77-local-gem-gold-exchange-array-api.apk`, SHA-256 `C8008999C37F6539716FC501BB6301F526203746A6F754378A47E0453B9B8614`; .so SHA-256 `7DD4A152CB58A7BF8F5FF2633434703CAD93F00E5718391F7A56E4D60DD7EB01`. Instalada com `adb install -r` no mesmo package/certificado; firstInstallTime preservado. Os dois pacotes foram testados e persistiram após reinício (detalhes na retificação ao final). Saldo insuficiente e double tap/reentrada não foram testados. Static high-level disassembly via REA indisponível neste Windows (Ghidra não configurado); valores, IDs e RVAs foram conferidos nos recursos/tabelas e dump 1.9.1.

Retificação: em v77 o pacote grande também foi testado. Uma compra 500 gemas debitou 996.840→996.340 e creditou 218.200→337.088 ouro; após reiniciar, a Loja mostrou 337K/996K. Evidências: `build/kinja-v77-shop-huge-purchase-logcat.txt`, `build/kinja-v77-shop-huge-purchased.png`, `build/kinja-v77-after-huge-restart-logcat.txt`, `build/kinja-v77-after-huge-restart-shop.png`. Assim, ambos os pacotes estão validados visualmente e após relançamento. Só insuficiência e concorrência/double-tap faltam.



## Fusão individual de equipamento — estado v79 (14/10/2026)

No alvo 1.9.1, `UICompoundEquipTips.OnClickCompound` chama `EquipManager.CompoundEquip` e o helper ET online; sem `Game.Scene`, o helper lança NRE antes da sessão. O bridge v79 intercepta `ET.EquipHelper.CompoundEquip(int,int,int)` com VA/prólogo exatos validados contra o APK original. Valida os três UIDs contra o save e `EquipManager`, exige mesmo item/qualidade e peças não equipadas, confere a qualidade seguinte com `EquipQualityCfg` e prepara uma fusão sem cobrança de moeda. Invoca `EquipManager.OnGetCompoundEquip` e `OnEquipDataInitResp` originais na Unity thread. O journal conserva snapshot e permite rollback/reconciliação; chamadas repetidas com materiais já removidos falham sem consumo adicional. A rota coletiva não foi alterada.

**Evidência disponível:** BuildId `v79-local-equipment-fusion-rollback-refresh`, APK SHA-256 `5AD160E8F0590B7E5E9766B1BCAB4D3D7E213AE26114B74E27261974F9668E02`, biblioteca SHA-256 `6B48154C57CB0E865039322CBCAB4D3D7E213AE26114B74E27261974F9668E02`. Instalada com `adb install -r` no package `com.habby.kinjarun.offline.lab6`, preservando `firstInstallTime=2026-10-13 19:43:33`. O log `build/kinja-v79-startup-filtered-logcat.txt` comprova instalação do hook após relançar. Antes/depois, a UI mostrou cerca de 992K gemas, 925K ouro e 1.000.000 EXP (`build/kinja-v78-before-install.png`, `build/kinja-v79-after-restart.png`). A fusão positiva não foi feita porque o seletor informou que não havia outro item compatível (`build/kinja-v78-root-select6.png`). Por isso, callback visual de resultado, alteração do inventário e persistência de uma fusão real ainda não estão validados. Ver a separação detalhada de evidências e a causa original em `FUNCTIONAL_AUDIT.md`.

Para reproduzir build e instalação, use o original com hash guardado pelo script e o keystore de laboratório:

```powershell
.\offline\build.ps1 -BuildId v79-local-equipment-fusion-rollback-refresh -PackageName com.habby.kinjarun.offline.lab6 -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v79-local-equipment-fusion-rollback-refresh.apk
```

Próximo passo de gameplay: esperar ou obter por jogo normal duas peças iguais e compatíveis, confirmar que a UI as habilita, então validar uma fusão e conferir qualidade, duas remoções, moedas inalteradas e persistência após reinício. Não selecionar itens com cadeado nem alterar o save para criar materiais.

### v81–v84 — investigação e pool de duplicatas QA de fusão

O jogador autorizou gerar itens repetidos neste save de teste. O seeding QA está isolado em `KINJA_OFFLINE_QA_FUSION_REPEATS` (`offline/native/build-native.ps1 -QaFusionRepeats`), roda no bootstrap da Unity thread depois dos callbacks normais, clona registros existentes com UIDs novos, define `IsWear=false`, grava atomicamente e chama `EquipManager.OnEquipDataInitResp`. Marcadores `qa_fusion_repeats.json` e `_v2.json` tornam cada lote one-shot. Não altera moeda/livros e não repõe cópias gastas em reinícios. A lógica de fusão normal mantém `EquipCanCompound` e `EquipRelateCompound` como requisitos.

v83 deixou sete cópias totais do `EquipId=6001`, qualidade 1; v84 deixou sete totais do `EquipId=3002`, qualidade 2. A UI mostra ambos os grupos na bolsa depois de reiniciar. As chamadas originais ainda retornam false para raiz/material, e o seletor os mostra bloqueados; nenhuma fusão foi confirmada com esses itens. Não mude o retorno das APIs, o lock visual, ou os dados para forçar elegibilidade. Próximo passo técnico é examinar os corpos 1.9.1 `EquipCanCompound`/`EquipRelateCompound` e explicar por que os grupos repetidos não ficam elegíveis antes de alterar outra regra.

A fusão já observada no aparelho aumentou qualidade 2→3 mantendo o nível atual em 20 (UID13) e em 1 (UID15). Isso é esperado: fusão aumenta a qualidade e o limite de nível; `EquipLevel` muda pela ação separada Melhorar.

Build v84 SHA-256 APK `C585F0AC89884A3ABBF06FD788EF630CFF3392DFF26174D3C2D6A9BB18181B70`, ARM64 `.so` `C9D7C7440B54C29E887093D59143E42205A6C3F39CD4537050FD8CF5B7B87C58`; package lab6 e assinatura de laboratório. Atualizada via `adb install -r`, `firstInstallTime` preservado. Capturas/logs e distinção entre análise estática, bridge, visual e gameplay estão na seção v81–v84 do `FUNCTIONAL_AUDIT.md`.

### Revisão v80 — elegibilidade e estado mais recente

v80 acrescenta validação pelos métodos 1.9.1 `EquipManager.EquipCanCompound(rootUid)` e `EquipRelateCompound(materialUid)` nos dois materiais, então o bridge recusa qualquer conjunto que o manager original não marque elegível. BuildId `v80-local-equipment-fusion-original-eligibility`, APK SHA-256 `82B74E8048B53B3D959D264EFE2D33F7B2075FF37C9D2C867C2714BFD641810B`, lib SHA-256 `FF5130D24B3B812C059686F8353379161CEC95B4214EE330A6D76F4F677A011F`. A instalação por `adb install -r` preservou `firstInstallTime=2026-10-13 19:43:33`; `lastUpdateTime=2026-10-14 18:15:57`. O hook carregou após reinício (`build/kinja-v80-startup-filtered-logcat.txt`). A UI mostrou ~632K ouro, ~992K gemas e 1.000.000 EXP antes e depois de atualizar (`build/kinja-v80-before-install.png`, `build/kinja-v80-after-restart.png`).

Entre as capturas v79 e v80, `build/kinja-v80-runtime-logcat.txt` registrou sete transações OneKey no jogo em uso paralelo: cinco equipamentos Nv.1→20 (57.600 ouro/85 livros cada) e dois Nv.1→10 (2.600 ouro/18 livros cada). O log registra readback persistido até 632.228 ouro, 992.190 gemas, 1.000.000 EXP. Isso explica a diferença de ouro; não é fusão nem efeito da instalação. Nenhuma mutação de fusão aconteceu. O seletor v78 informou que não havia peças compatíveis (`build/kinja-v78-root-select6.png`); a validação de uma fusão bem-sucedida ainda depende de duas cópias habilitadas pela UI. Consulte `FUNCTIONAL_AUDIT.md` para o histórico separado.

Reprodução de laboratório v80 (se repetir neste checkout, escolha outro BuildId ainda não usado, pois a pasta v80 já existe):

```powershell
.\offline\build.ps1 -BuildId v80-local-equipment-fusion-original-eligibility -PackageName com.habby.kinjarun.offline.lab6 -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v80-local-equipment-fusion-original-eligibility.apk
```

### Loja: baú de diamante 10x (14/10/2026)

A configuração original 1.9.1 contém `TreasureBoxCfg` ID108 (2200 gemas, 10 sorteios) e ID116 (mesma quantidade/preço com outro `open_item_id`); a tela comum chama `UpdateTreasurePage`, que passa IDs 101, 115 e 108 a `TreasureBoxItem.UpdateItem`. O bridge aceita agora 101/108/115 e lê custo, contagem e pool pelas APIs originais. ID108 usa o pool `TBContentCfg` 1175–1240 (`box_id=1021`), dez sorteios, débito/persistência e callbacks comuns. Isso prova o caminho de serviço local, não que o cartão esteja visível.

O cartão 115 continuou sendo exibido sozinho na Loja v87. A tentativa v86 de substituir sua configuração no hook `TreasureBoxItem.UpdateItem` causou SIGBUS/PC `0xa`; o hook foi retirado e não deve ser reintroduzido. O layout ainda precisa ser investigado no prefab/hierarquia original para expor corretamente o ID108; evitar sobrescrever o cartão unitário com outro ID sem atualizar junto nome, preço, quantidade e callbacks. Estado e passos de ADB, evidências e hashes estão na seção “Loja — investigação do baú de diamante 10x” em `FUNCTIONAL_AUDIT.md`.

### Loja: cartão original de baú de diamante x10 — v94

O prefab 1.9.1 contém `TenDiamondBox` inativo dentro de `UITreasureBoxPage/obj/ContentPanel/Top/ChestBox`; o ID108 da configuração é o pacote de dez sorteios (2200 gemas). O bridge ativa esse objeto original após `UpdateTreasurePage` e, após `UITreasureBoxPage.UpdatePage`, reposiciona `BuyCoin` e `imgDownFill` através de APIs gerenciadas de `Transform.localPosition`, atualizando a rolagem NGUI. Não editar offsets gerenciados nem substituir `UpdateItem` do cartão unitário (a tentativa antiga v86 causou SIGBUS). A v94 usa prólogo ARM64 little-endian exato para o hook `UpdatePage`; v93 foi recusada pela checagem por ordem de bytes incorreta.

A captura v94 `build/kinja-v94-shop.png` confirma visualmente o cartão x10 sem sobreposição e seção de Ouro abaixo dele. BuildId `v94-local-ten-draw-after-page-update`, APK SHA-256 `6E5A93CAF051AFF33F468D39D2F8BBDD9FDD8820B0C7344AF2B0ADEA7936F3C1`, `.so` `00677F02CECB81CC46FE7EA9654B0B50E9735D5EFE4E6ED4E496688661BB79FC`; log de runtime `build/kinja-v94-shop.log`. Instalação ADB `-r` preservou `firstInstallTime=2026-10-13 19:43:33`. A compra x10 não foi efetuada; recompensas e persistência após compra permanecem pendentes. Consulte a seção v88–v94 no `FUNCTIONAL_AUDIT.md` para separação de evidências e saldo registrado.

### Auditoria adicional de UX da Loja — v94

Capturas da reprodução: `build/kinja-shop-ux-current.png` e `build/kinja-shop-ux-scroll-bottom.png`; log: `build/kinja-shop-ux-current.log`. Ao rolar para baixo, o `ContentPanel` passa por trás da barra fixa de saldos e o topo (título/ícone) do cartão x10 fica coberto. Os itens de Ouro ainda ficam alcançáveis; o defeito está no viewport/clip, não na distância da seção Ouro. Investigar `UIPanel`/`UIScrollView` do prefab e ajustar a região rolável por APIs NGUI gerenciadas, conferindo prólogo/versão e captura em aparelho.

A etiqueta atual do botão mostra item `4001` (Soul Stone) e `1000000/10`; a tabela ID108 também declara 2200 gemas e os campos `coin_id=4001`, `coin_num=10`, `cost_item=[10]`. O handler local registrado no evento debita gemas. Antes de alterar interface ou handler, confirmar no código 1.9.1 `TreasureBoxItem.SetButtonInfo`, `OnOpenByItem` e `OnClickBuy` qual meio de pagamento o botão representa. Não fazer uma alteração cosmética que deixe botão e débito divergentes.

A auditoria capturou uma compra ID108 no log (990440→988240 gemas, dez `ContentIds`) durante a janela de navegação. Não foi enviado toque de compra; a origem ficou indeterminada. Não há recibo visual nem validação após reinício. Sem teste de compra adicional até corrigir a ambiguidade. Detalhes e logs completos estão no complemento da auditoria em `FUNCTIONAL_AUDIT.md`.

### Retificação e continuação — Loja v97 (14/10/2026)

A etiqueta do x10 é o item `open_item_id=1002` (chave) e requisito `cost_item=[10]`; `coin_id=4001` é outro campo da tabela, não o ícone do botão. O trace do 1.9.1 diferencia `TreasureBoxItem.OnOpenByItem` estados Item=3/Currency=4, ambos chegando a `ShopManager.BuyTreasureChest`. O bridge v97 leva essa escolha até a transação local, com débito de chave no estado Item e gemas no Currency, conferência live/save e journal de recuperação. O caminho por chave não foi acionado; sem validação de compra/gameplay.

A v97 configura e relê `UIPanel.baseClipRegion` pelo runtime e chama `InvalidateClipping`/atualiza scrollbars, mas a captura rolada ainda mostra o cabeçalho x10 parcialmente sob o HUD (`build/kinja-v97-now.png`). Clipping visual continua falhando; investigar o painel/âncoras e `finalClipRegion` real do prefab antes de outro ajuste. Logs `build/kinja-v97-now-logcat.txt`.

APK v97 SHA-256 `1EFF10D7099EC975DD3EA6C2520C0FACE393EB7F7DEA463B0F9B341FED811358`; `.so` SHA-256 `CD5DFCEA8377643DB96E042012069BFCA95D0525D75E5012D77901209B7A2AE9`. Instalado via `adb install -r` no package lab6, firstInstallTime preservado em 13/10/2026 19:43:33. Antes da atualização: 632228 ouro, 986040 gemas e 1000000 EXP, registrados em `build/kinja-v95-preinstall-device-logcat.txt`. O readback inclui duas compras ID108 não planejadas já relatadas acima; nenhuma nova transação foi registrada durante a navegação v95–v97. Captura arredondada atual: 632K/986K. Sem teste de persistência de compra porque não houve compra controlada.
