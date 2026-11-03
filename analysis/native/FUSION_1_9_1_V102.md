# Fusão 1.9.1 — investigação e correção v102 (14/10/2026)

Alvo: `kinja-run-1-9-1.apk`, SHA-256 `7953EB369BA9B8C814C4736DAC4A7E3AA116A5583837F6F12D795EA70C20595E`. Biblioteca extraída do APK: `18FA330ED22BB35DB0BB9966A5ED8086EC8FC7125BD865AC73E92921FC4639D7`. Bundle `assets/AssetBundles/Android/clientdata.ht`: `900AE16D54FEEB11A40CE9AC427E1520F88FCBA69FD68E7CAC8CCE540C16887F`, reconferido diretamente no APK, igual ao inventário da extração.

## Análise estática do artefato exato

`tools/metadata/recover_native_stripes.py` aplicado à biblioteca original reproduziu SHA-256 `A2A4510A6650A6BFC9C23949E466392A098F9A1C992E21194B531C1E45BABDBE`, igual a `analysis/metadata/libil2cpp.stripes.so`. Essa cópia desfaz XOR66 **e permutação** de stripes conforme o loader original; não é instalada no jogo. As janelas AES iniciais continuam opacas e ficam fora dos métodos estudados. A antiga `analysis/native/libil2cpp.code-analysis.so` desfaz somente XOR: pode conter outro trecho no RVA permutado. Não usar suas referências a callers sem reconciliar com a reconstrução completa. Nenhum APK/libil2cpp original foi modificado.

O dump, script de metadados e mapa de métodos foram reconciliados com essa biblioteca. Disassemblies anotados novos ficam em `analysis/bundles/EquipManager.*.annotated.txt` e `UICompoundEquipTips.*.annotated.txt`.

| Método original | RVA | Papel comprovado |
|---|---|---|
| `UICompoundEquipTips.OnClickCompound` | `0x12F9AF4` | inicia a seleção individual; state machine em `0x1016D08` passa raiz e até dois materiais; segundo UID é zero quando há só um material |
| `EquipManager.CompoundEquip(int,int,int,bool)` | `0x2C3971C` | chama helper ET individual |
| `ET.EquipHelper.CompoundEquip(int,int,int)` | `0x2C2D3F4` | hook individual; prólogo `FF8301D1 F65703A9 F44F04A9 FD7B05A9` mantido |
| `EquipManager.CheckCompound` | `0x2C36858` | verifica existência da próxima qualidade, **depois lê cost da qualidade atual**; pesquisa candidatos por ID ou parte |
| `EquipManager.UpdateEquipCompoundRedPoint` | `0x2C347E0` | sai se `FuncManager.FuncIsOpen(102)` é false; quando aberta, limpa/reconstrói os dois caches com `CheckCompound` |
| `EquipCanCompound` / `EquipRelateCompound` | `0x2C36A04` / `0x2C36AA4` | consultam caches da raiz/material; não contêm toda a receita |
| `EquipManager.OnEquipDataInitResp` / `DoUpdateEquipData` | `0x2C35544` / `0x2C3564C` | atualizam inventário original; DoUpdate chama cálculo dos caches |
| `EquipManager.OnGetCompoundEquip` | `0x2C33C0C` | evento e apresentação de sucesso por `UICompoundEquipResult`; recebe `ET.G2C_CompoundEquip` |
| `UICompoundEquipTips.UpdateCompoundList` | `0x12F8530` | observação dos UIDs selecionados; trampoline chama original e só lê `_selected` pelas APIs IL2CPP/List |
| `UICompoundEquipTips.OnClickOneKey` / `OnClickOneKeyDetail` | `0x12F689C` / `0x12F6B08` | primeiro chama fusão coletiva; segundo abre `UIOneKeyDetailPanel` |
| `ET.EquipHelper.OneKeyCompoundEquip` | `0x2C2D50C` | rota ET coletiva separada, sem hook local |

Prólogo adicional do observador: `FF4307D1 EAAB00FD E923166D FC6F17A9`, validado na reconstrução do APK e novamente em memória antes da instalação do hook. Não há escrita por offsets de objetos gerenciados.

## Receita e resultado originais

Não há `EquipCompoundCfg` por esse nome no inventário de tabelas consultado. A regra **está explicitamente configurada** em `EquipQualityCfg.cost`, interpretada pelo código original: `[tipo, qualidadeDoMaterial, quantidade]`. Tipo 1 exige mesmo EquipId, tipo 2 exige mesma parte. A raiz permanece e os materiais são sacrificados; `CheckCompound` soma uma unidade à contagem exigida quando a raiz está na mesma qualidade dos materiais, porque a pesquisa inclui a própria raiz. O seletor compara os mesmos critérios. Não inventar receita a partir de uma ausência de tabela separada.

| Qualidade atual da raiz | Materiais adicionais | Resultado |
|---|---|---|
| 1 / cinza | 2 do mesmo ID, qualidade 1 | qualidade 2 / verde |
| 2 / verde | 2 do mesmo ID, qualidade 2 | qualidade 3 / azul |
| 3 / azul | 2 do mesmo ID, qualidade 3 | qualidade 4 / roxo |
| 4 / roxo | 1 da mesma parte, qualidade 4 | qualidade 5 / roxo +1 |
| 5 / roxo +1 | 1 da mesma parte, qualidade 4 | qualidade 6 / roxo +2 |
| 6 / roxo +2 | 1 do mesmo ID, qualidade 4 | qualidade 7 / dourado |
| 7, 8, 9 / dourado até +2 | 1 da mesma parte, qualidade 7 | qualidade atual +1 |
| 10 / dourado +3 | 1 do mesmo ID, qualidade 7 | qualidade 11 / vermelho |
| 11, 12, 13, 14 / vermelho até +3 | 1 do mesmo ID, qualidade 11 | qualidade atual +1 |
| 15 / vermelho +4 | próxima configuração ausente | não elegível |

Os sufixos ficam representados por `Quality` 5/6, 8/9/10 e 12/13/14/15; `quality_add` contém o sufixo e `EquipCfg.show_quality` agrupa a cor. Limites de nível: 10,20,30,40,50,60,70,80,90,100,140,180,220,260,300. O APK tem vermelho +4, divergindo da referência do jogador que terminava em +3. Só existe uma qualidade azul (3). `e_quality_3` usa Excellent/Excelente; textos de baús usam Rare com cor azul. Não há configuração de um rank azul separado chamado Rare.

A tabela do jogador para dourado descreve o custo **acumulado** de levar um roxo básico a dourado: duas peças roxas da mesma parte nos passos +1/+2, e uma cópia roxa de mesmo ID na passagem a dourado. Não é uma transação com quatro UIDs entregue a `CompoundEquip`.

EquipId, UID, parte e nível corrente são conservados; Quality aumenta em um. `EquipCfg` muda atributos conforme ID/nível/qualidade e `EquipQualityCfg` muda teto. Para 3002 Nv.5, q2 ATK=15+4×4=31; q3 ATK=23+5×4=43. Buff q3=52, q4=2000102, q7=53, q11=2000103; `GetWearEquipBuffs` (`0x2C36B44`) conserva os buffs das qualidades inferiores ao montar os efeitos do equipamento vestido. `exp`, `extra_coin` e `extra_item` da qualidade são zero nestes registros; não há cobrança de moeda nesta fusão. Efeito de restauração de HP em combate não foi exercitado.

## Causas e alterações no bridge

1. **Qualidade descartada na preparação do save:** `json_set_int(updated_root,"Quality",next)` era seguido por outro `find_equipment_record(...,updated_root,&start,&end)`, que sobrescrevia a string alterada com o registro antigo. Os materiais eram removidos, mas a raiz persistia sem subir de qualidade. Agora posição e registro são obtidos juntos, alterados e substituídos uma única vez. O teste ARM64 `build/kinja-fusion-json-regression.cpp`, extraído dos helpers/trechos reais antes/depois, reproduziu o defeito antigo e passou com a correção. Usa fixtures em memória em um executável isolado; não acessa o save nem cria equipamentos no jogo. Resultado: `build/kinja-v99-json-regression.txt`.
2. **Caches vazios no bootstrap:** inventário e seed QA eram carregados antes de `SetOpeningFunc`. `UpdateEquipCompoundRedPoint` saía por função 102 fechada. Agora chama a API original novamente depois da resposta de abertura de funções e no preflight da fusão. Não modifica retornos dos validadores nem as receitas. Logs v99/v102 demonstram os mesmos UIDs passando de can=0/relate=0 para 1/1 sem alteração de registros.
3. **Receita fixa incorreta nas faixas superiores:** bridge exigia sempre três peças de mesmo ID/qualidade e UID2 positivo. Agora lê cost da configuração viva pela API original `System.Array.GetValue(int)`; valida quantidade 1/2, qualidade configurada, ID/parte, existência da próxima qualidade, wear e concordância save/live. Aceita UID2=0 para receitas de um material. Raiz equipada pode permanecer equipada; materiais equipados são recusados por segurança (essa condição ainda não foi comparada em gameplay com o servidor original).
4. **Ordem da resposta:** prepara/deserializa `G2C_CompoundEquip` e `G2C_EquipData` antes de consumir; salva snapshot/journal, grava resultado, atualiza inventário original e confere raiz/materials/save/moedas, depois chama `OnGetCompoundEquip`. O callback visual passa a ler o resultado já aplicado. Falha retorna false e restaura save/managers pelos callbacks originais. Cleanup de commit com erro também provoca rollback. Há guard contra reentrada e operações de upgrade/baú durante a fusão. UIDs consumidos são rejeitados na repetição; journal existente conserva recuperação por rollback no bootstrap.

`CostEquips` contém os modelos ET originais com EquipId/EquipLevel/EquipQuality; não inventar UID em campos inexistentes. Todas as invocações/observações ocorrem na thread Unity, disparadas pelo bootstrap ou pela UI.

## Evidência de execução, visual e gameplay/save

Build final instalada: **v102-fusion-atomic-recipes-selection-evidence**. APK SHA-256 `D4B367A3E54B7A30093F6237AA54ECBD36B3A61DBCDEE25DB121498FEB87E5FC`; `.so` `A7DED0DD84EC19309A7E58D3BD8E2537D0655F24E1FC56C0CDDC6492608DAD7D`. Fonte `offline_bridge.cpp` SHA-256 `49FE354EC8F31023E5A2D8F2B056E1922C4E92FF20AEEFF8C9CBFD71936BFB96`. Certificado de laboratório SHA-256 `7508CA8DD3AD4914F42B7D6DBD537C564A78925FB8F41B76A905324220A75B3F`. Mesmo package `com.habby.kinjarun.offline.lab6`, versionName 1.9.1/code51, `firstInstallTime=2026-10-13 19:43:33`; `lastUpdateTime=2026-10-14 20:21:21`. Instalação por `adb install -r`; nenhum wipe/desinstalação. Builds anteriores, incluindo v57, preservadas. Nenhum define QA usado nestas builds; seeds e markers existentes preservados.

Pré-instalação v97: `build/kinja-v99-preinstall.png`, `kinja-v99-preinstall-logcat.txt`. v98 ficou como diretório de compilação parcial porque clang não executou no sandbox; v99/v100/v101/v102 foram assinadas com BuildIds inéditos; só v99 e v102 foram instaladas. Logs de compilação finais: `build/kinja-v102-build-log.txt`.

**Execução do bridge:** `build/kinja-v102-startup-logcat.txt` confirma os hooks e cache recalculado. UIDs28–32: EquipId3002, qualidade2, Nv.5, parte3, IsWear=false, concordância save/live e can/relate=1. Esses UIDs vêm das cinco cópias QA adicionadas na v84 (log histórico com cloneUid28/copiesAdded5); nenhuma cópia nova foi adicionada. A origem foi revalidada antes do teste. Logs de seleção confirmam `[28]`, `[28,29]`, `[28,29,30]`; não confiar na posição visual, pois o comparador não desempata pelo UID.

**Ação controlada:** `build/kinja-v102-fusion-test-plan.json`, `kinja-v102-pre-fusion-logcat.txt`, `kinja-v102-pre-fusion.png` registram UIDs/estado/saldos antes da confirmação. Um único toque ADB `(640,2640)` em Criar às 20:23:48, depois de confirmar os três UIDs QA. Raiz28 q2→q3, somente29/30 consumidos, EquipId3002/parte3/Nv.5/IsWear=false conservados. `kinja-v102-fusion-once-logcat.txt` e `kinja-v102-fusion-result-logcat.txt` confirmam recipe=1,2,2, Array.GetValue, callbacks originais, readbacks de qualidade e ausência dos materiais no save e no manager. O guard não foi testado por double tap.

**Resultado visual:** `kinja-v102-fusion-result.png` mostra Colar de Cura Santa, Excelente/azul, teto20→30, ATK31→43, novo atributo de restauração de HP. `kinja-v102-bag-after-fusion.png` registra bolsa atualizada. Não foi uma simples troca do preview.

**Após reinício:** force-stop/start do package, processo22143. `kinja-v102-after-restart-logcat.txt` lê UID28 com Quality3 e dados vivos iguais ao save; UIDs31/32 permanecem q2/Nv.5. A bolsa `kinja-v102-after-restart-bag.png` contém a nova peça azul Nv.5 e três colares verdes Nv.5, antes eram seis verdes; os dois materiais e o estado antigo da raiz não reapareceram. `kinja-v102-after-restart-selection-logcat.txt` reconfirma que a peça azul selecionada é UID28. Ouro persistido7.490.720, gemas938.307, EXP1.000.640 antes/depois; a UI mostra valores arredondados. Nenhuma compra/upgrade/outra fusão atribuída ao teste. JSON de observação: `kinja-v99-fusion-baseline.json` e `kinja-v102-after-restart-inventory.json` têm cobertura parcial (122/139 UIDs capturados), devido à perda de linhas no buffer/diagnóstico; não são cópias integrais nem hashes do save privado. O campo de log `count=14457` é tamanho em bytes da seed, não quantidade de peças.

## Reconciliação histórica e limites

Conferência final: `build/kinja-v102-installed-sha256.txt` contém o SHA-256 calculado por ADB no `base.apk` instalado, igual a `D4B367A3...`. `kinja-v102-after-restart-root-ready.png` mostra a raiz azul Nv.5 com ATK atual43/teto atual30 (preview da próxima etapa43→56/30→40); o log de seleção confirma UID28. Nenhuma segunda fusão foi confirmada. `kinja-v102-fusion-validation.json` registra o único commit da sessão PID20096 e o processo de recarga. O buffer foi ampliado a8MiB para preservar mais linhas, sem limpar os registros anteriores. Ainda assim os snapshots JSON de logs têm cobertura parcial; a ausência pós-reinício dos materiais é corroborada pela bolsa (seis verdes Nv.5 antes, três depois mais a raiz azul), pela recarga do resultado e pelos readbacks completos do commit individual. Não tratar esses arquivos como um backup integral do save.

- Falha v77: `kinja-merge-after-once-logcat-v77.txt`, NRE em helper com Game.Scene nula, antes do envio/callback/consumo.
- Recusa QA v83/v84: false/cadeados explicados pela ordem de abertura de funções/cache; não demonstravam uma receita defeituosa.
- As mensagens antigas `Local equipment fusion committed` não verificavam a qualidade gravada. Root13 aparece em `kinja-v80-runtime-logcat.txt` às18:14:20, antes da instalação v80; root15 está em `kinja-v81-before-install-logcat.txt` às18:17:57. A atribuição histórica de ambas ao arquivo v80 era imprecisa. Os sinais de callback/commit antigos não provam aumento persistente, diante do defeito de string confirmado. Não repor materiais nem editar o save para reparar retrospectivamente operações sem snapshot completo.
- A fusão individual q2→q3 foi validada no aparelho, visualmente e após reinício. Receitas de um material, transições roxas/douradas/vermelhas, raiz equipada, material equipado, erros de callback, interrupção no meio do journal e toques concorrentes têm análise/guard, mas **não** validação de gameplay por este teste. Não consumir peças reais para ampliar cobertura.
- `OnClickOneKey` procura candidatos nas qualidades3,2,1 e chama `OneKeyCompound`/helper ET. `OnClickOneKeyDetail` é outra chamada e só abre detalhes. O texto `c_onekey_compound_limit_hint` fala em Epic e inferiores, enquanto o laço UI examina1–3: não tomar o texto como prova de inclusão da qualidade4. `G2C_OneKeyCompoundEquip` tem EquipUids e CostEquips; a escolha de raízes, ordem e grupos efetivamente processados pelo servidor ET ainda não está confirmada. O handler legado `ProtocolManager.HandleEquipOneKeyCompound` não é o pedido ET da UI. **Fusão coletiva não foi corrigida nem acionada**. Próximo trabalho recomendado: fechar esse contrato e a seleção de raízes antes de implementar a transação coletiva.
- NRE periódico em `ET.RecoverPowerHelper.RequestNextRecoverPowerTime`/UIResourceBar continua fora deste escopo. Visual da loja não foi retrabalhado.

## Reprodução

Escolher BuildId novo (os diretórios/APKs existentes são referência):

```powershell
.\offline\build.ps1 -BuildId NOVO-ID -PackageName com.habby.kinjarun.offline.lab6 -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-NOVO-ID.apk
.\tools\android\toolchain\platform-tools\adb.exe logcat -d -v threadtime -s KinjaOffline Unity AndroidRuntime
```

Antes de confirmar qualquer fusão: capturar inventário/saldos, ler `Fusion UI selected`, conferir todos os registros e origem. Coordenadas acima só descrevem o teste em1280×2772; não são automação genérica nem autorização para consumir outro conjunto. Não executar novamente28/29/30:29/30 já foram consumidos.
