# Guia de manutenção do Kinja Run offline

Este guia descreve o **estado atual**: arquitetura, regras, hooks, saves, sistemas, build e testes. Ele substitui o antigo diário de builds. Quando uma informação mudar, **edite a seção correspondente**; não acrescente uma nova seção datada no topo. O detalhe de cada mudança vai para um relatório em `docs/testing/` e para a mensagem de commit.

Histórico, consultado só quando necessário:

- [docs/history/MAINTENANCE_LOG.md](docs/history/MAINTENANCE_LOG.md): o guia anterior, um diário de v31 a v151 com causas, RVAs, retificações e evidências por build.
- [docs/history/FUNCTIONAL_AUDIT.md](docs/history/FUNCTIONAL_AUDIT.md): auditoria funcional no aparelho, por build, com logs, capturas e saldos.
- [docs/history/NATIVE_BRIDGE_LOG.md](docs/history/NATIVE_BRIDGE_LOG.md): o antigo README do bridge, em inglês, até v109.
- [docs/history/REVERSE_ENGINEERING.md](docs/history/REVERSE_ENGINEERING.md) e [docs/history/migration/](docs/history/migration/): a investigação estática inicial e a migração para este checkout.
- `docs/testing/*.md`: um relatório por sistema/build, com contrato, testes e limites. É a fonte primária de cada linha da tabela de sistemas abaixo.

Muitos relatórios antigos citam arquivos em `build/` (logs, capturas) que já foram apagados pelas limpezas autorizadas. As sínteses continuam válidas; os arquivos brutos não existem mais.

## Objetivo e arquitetura

- `offline/` empacota uma Activity JNI e uma biblioteca nativa sobre o APK Unity original. Combate, telas e assets são os do jogo; o bridge substitui as respostas que vinham do servidor por respostas e saves locais.
- APK-alvo: `kinja-run-1-9-1.apk`, SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`, cliente IL2CPP ARM64. RVAs, prólogos e conclusões valem **só para essa versão**.
- O código IL2CPP não é reescrito em disco. `offline_bridge.cpp` redireciona entradas de métodos em memória, depois de conferir os 16 bytes do prólogo. `offline/build.ps1` troca manifest/`level0` e adiciona DEX, biblioteca e seeds.

## Regras invariáveis

- Preservar a conferência de hash do APK e de prólogo (`patch_entry`). Nunca atualizar um RVA por tentativa: usar `analysis/il2cpp/dump.cs` e o disassembly do binário correto (`libil2cpp.stripes.so` com XOR **e** permutação, não a antiga `code-analysis.so`).
- Toda chamada a singleton, `runtime_invoke`, callback de manager ou mudança de estado IL2CPP roda na thread Unity. O worker só espera o runtime e instala hooks.
- Não escrever offsets de objetos gerenciados. Usar setters, factories e callbacks originais (por exemplo `ObscuredInt.op_Implicit` + `PlayerDataManager.set_*` + `SyncValueToShow`).
- Transações que gastam ou concedem recursos: validar live × save antes, preparar modelos/JSON antes do gasto, journal + fsync + temp/rename, readback e rollback.
- Saves existentes nunca são substituídos pelos seeds. Reparos são migrações idempotentes no bootstrap. Não pedir para limpar dados.
- Não tratar online, anúncios, compras, correio ou guilda como disponíveis sem uma substituição local documentada.
- Trampolines copiam 16 bytes e retomam em `entry + 16`. Antes de reutilizar, confirmar que as instruções copiadas não usam endereçamento relativo ao PC.

## Armadilhas conhecidas

Erros que já custaram crash ou conclusão errada. Detalhes em [MAINTENANCE_LOG.md](docs/history/MAINTENANCE_LOG.md), na build indicada.

- Não chamar `OpenBoxResp` na compra local de baú: os caches de pool são nulos offline (crashes v58–v62).
- Não fazer hook em `TreasureBoxItem.UpdateItem` para trocar a configuração do cartão (SIGBUS na v86).
- Ouro 101, gemas 102 e EXP 104 são campos de `PlayerDataManager`, não `PlayerItem`. `AddCoin`/`AddLocalItem` não mudam `GetOwnItemNum`; usar os setters e `SyncValueToShow` (v50/v51).
- `KillEnemy` pode chegar depois de `EndGame`. Creditar só a diferença e manter o `G2C_EndGame` por GC handle para atualizar o resultado (v53/v54).
- O seletor de fusão não desempata por UID: nunca identificar peça pela posição na tela. O `ProtocolManager.HandleEquipOneKeyCompound` legado não é o contrato ET (v102).
- `TalentCostCfg.exp` não é requisito comprovado; não usá-lo para barrar compras (v56).
- HangUp: `OnClickReceive` retorna cedo com `_items` vazia. Não culpar o toque (v41/v42).
- Não varrer a UI recursivamente: crash na v70. Usar caminho exato com `Transform.Find`.
- Os hooks `UISetting.DoOpen`/`DoClose` causaram crash UnityMain/Houdini e foram removidos na v141. Não reintroduzir.
- Revive: não fechar só a UI com um `CloseTween` isolado; o herói pode ficar morto ou sem input.
- Prólogos são comparados como bytes little-endian: a v93 foi recusada por ordem de bytes errada.
- Toques ADB por coordenada já acionaram compras e melhorias sem querer (v72, v75). Conferir a tela antes de tocar.
- Scripts rodam no **Windows PowerShell 5.1** (não há PowerShell 7 aqui). Nele, `Get-Content … | ConvertFrom-Json | Sort-Object` passa o array inteiro como um objeto só: envolver em parênteses (`(… | ConvertFrom-Json) | …`). `-Encoding utf8NoBOM` não existe. `Tee-Object -FilePath` grava UTF-16. `.ps1` com acentos precisa de UTF-8 com BOM. `2>&1` em programa nativo transforma avisos em erro.

## Mapa do repositório

| Caminho | Conteúdo |
|---|---|
| `AGENTS.md` | Regras de trabalho para agentes. |
| `MAINTENANCE_GUIDE.md` | Este guia: estado atual. |
| `offline/native/offline_bridge.cpp` | Bridge principal (~6.200 linhas): API IL2CPP, JSON dos saves, todos os sistemas e `install_hook`. |
| `offline/native/*.h` | Lógica separada. Puros e testáveis no host: `chapter_rewards.h`, `hero_fragments.h`, `combat_exp.h`, `account_progression.h`, `fusion_batch_plan.h`. Ligados ao runtime: `hero_offline.h` (heróis), `backup_ui.h`, `backup_items.h`, `backup_regions.h`, `backup_preferences_v2.h`. |
| `offline/java/local/kinja/` | `OfflineActivity` (configura o bridge antes da Unity e cuida do backup/SAF), `BackupStore` e `BackupCatalog`. |
| `offline/assets/seed_*.json` | Estado inicial de um perfil novo e modelos de resposta ET. |
| `offline/build.ps1`, `offline/native/build-native.ps1` | Empacotamento/assinatura e compilação ARM64. Ver [offline/BUILDING.md](offline/BUILDING.md) para o contrato do wrapper. |
| `tests/` | Testes de host e o runner único `run-host-tests.ps1` ([tests/README.md](tests/README.md)). |
| `tools/workspace/` | `new-normal-build.ps1` (build normal com conferências), `clean-builds.ps1`, `build-history.ps1`, `paths.ps1` e `check-doc-links.py`. Índice de todas as ferramentas: [tools/README.md](tools/README.md). |
| `docs/` | Índice em [docs/README.md](docs/README.md): `development/` (setup, [RELEASE.md](docs/development/RELEASE.md), `build-history.json`), `testing/` (relatórios, [índice](docs/testing/README.md)), `decisions/`, `history/`. |
| `analysis/` | Relatórios de análise estática ([índice](analysis/README.md)). As extrações grandes (dump, tabelas, `.so` de análise) ficam no acervo. |
| `project-atlas/` | Painel local de exploração (somente leitura). Regenerar com `python project-atlas/build_inventory.py`. |
| `build/` | Saídas geradas. Não editar; usar sempre um BuildId novo. |

## Fluxo de inicialização

1. `local.kinja.OfflineActivity` verifica uma importação de backup pendente, depois chama `configure(filesDir, AssetManager)` e só então a Activity Unity original.
2. O worker espera `libil2cpp.so` e o domínio, resolve a base ASLR por `il2cpp_init` e roda `install_hook`.
3. `account_init_hook`, na thread Unity, lê ou cria `files/offline_save/seed_*.json`, roda migrações e reparos e desserializa as respostas ET pelos tipos originais.
4. Os callbacks originais carregam itens, equipamento, talentos, heróis, regiões, tutorial, jogador e baús. Depois vêm UID/PID locais, relógio e status offline.
5. `FuncManager.SetOpeningFunc` recebe a lista de menus local. O callback de jogador conclui o login original.

## Hooks instalados (39)

Fonte: `install_hook()` em `offline_bridge.cpp`. Todos conferem o prólogo de 16 bytes. **Falha:** "aborta" significa que o bridge para (login offline não acontece); "segue" significa só `LOGE` e o jogo continua sem aquela função local. **Original** indica que o hook chama o método original por trampoline.

| Método original | RVA | Handler | Falha | Original | Comportamento offline | Fonte |
|---|---:|---|---|---|---|---|
| `AccountInit` | `0x27972e0` | `account_init_hook` | aborta | não | Login local, bootstrap dos managers, migrações. | este guia |
| `UIMainPanel.OnClickStorageBg` | `0xfca444` | `backup_ui::click` | aborta | não | Menu Backup acima de Configurações. | [MANUAL_BACKUP_V141](docs/testing/MANUAL_BACKUP_V141.md) |
| `UIMainPanel.DoOpen` | `0xfd1bb8` | `main_panel_do_open_hook` | segue | sim | Oculta `imgPower`, Correio e centraliza rótulo do capítulo. | log v44/v72/v73 |
| `UIMainPanel.OnClickFight` | `0xfda1fc` | `main_panel_on_click_fight_hook` | segue | sim | Só instrumentação: loga energia antes/depois. | log v44 |
| `UIUnionPanel.DontHaveUnionPartAwake` | `0x186c74c` | `skip_union_online_panel_awake` | segue | não | Ignora warmup de guilda online. | log |
| `UIUnionPanel.InitDontHaveUnionPartInfo` | `0x1870870` | `skip_union_online_panel_refresh` | segue | não | Ignora carga de guilda online. | log |
| `UIUnionPanel.UpdateDontHaveUnionPage` | `0x186e9b8` | `skip_union_online_panel_refresh` | segue | não | Ignora página de guilda online. | log |
| `AdManager.ShowRewardAdWithCallback` | `0xF08120` | `complete_offline_ad_hook` | segue | não | Completa a callback de anúncio sem abrir anúncio (revive etc.). | log v51/v67 |
| `UITreasureBoxPage.UpdateTreasurePage` | `0x13df2cc` | `show_local_ten_draw_chest` | segue | sim | Ativa o cartão original x10 (`TenDiamondBox`). | log v94 |
| `UITreasureBoxPage.UpdatePage` | `0x13deca0` | `update_page_local_ten_draw_chest` | segue | sim | Reposiciona `BuyCoin`/`imgDownFill` abaixo do x10. | log v94/v97 |
| `UITreasureBoxPage.DoRefreshPage` | `0x13e0ed0` | `refresh_local_ten_draw_chest` | segue | sim | Reaplica o layout do x10 no refresh. | log v97 |
| `UITreasureBoxPage.UpdateBuyDiamond` | `0x13dfb00` | `hide_offline_paid_shop_section` | segue | sim | Oculta ofertas pagas. | log v39 |
| `UITreasureBoxPage.UpdateBuyCoin` | `0x13dfdf0` | `update_shop_daily_claim_ui` | segue | sim | Recupera journal e sincroniza contador do resgate diário. | log v67 |
| `PurchaseManager.Purchase(int)` | `0x2aa0120` | `block_offline_purchase` | segue | não | Recusa compra com dinheiro real. | log |
| `TreasureBoxItem.OnOpenByItem` | `0x1341a9c` | `treasure_box_on_open_by_item_hook` | segue | sim | Guarda se o pagamento é Item (chave) ou Currency. | log v97 |
| `ShopManager.BuyTreasureChest` | `0x26d8ad0` | `local_buy_treasure_chest_hook` | segue | sim | Compra local de baús 101/115/108. | [CHESTS_V148](docs/testing/CHESTS_V148.md) |
| `UITreasureBoxPage.OnClickAdCoin` | `0x13e0f80` | `block_shop_ad_coin` | segue | não | Resgate diário local de ouro (o nome do handler é histórico). | log v67 |
| `UITreasureBoxPage.OnClickBuyLotCoin` | `0x13e145c` | `block_shop_coin_exchange_lot` | segue | não | Troca local 100 gemas → 20.000 ouro. | log v77 |
| `UITreasureBoxPage.OnClickBuyHugeCoin` | `0x13e1730` | `block_shop_coin_exchange_huge` | segue | não | Troca local 500 gemas → 118.888 ouro. | log v77 |
| `ET.TalentHelper.UpgradeTalent` | `0x28fd814` | `local_talent_upgrade_hook` | segue | não | Talentos comuns (101) e azuis (40001). | log v56/v75 |
| `ET.HeroHelper.EquipHero` | `0x296b5b8` | `hero_equip_hook` | segue | não | Seleção de herói. | [HEROES_V126](docs/testing/HEROES_V126.md) |
| `ET.HeroHelper.ExchangeHero` | `0x296b6ac` | `hero_acquire_hook` | segue | não | Obter herói por fragmentos. | [HEROES_V126](docs/testing/HEROES_V126.md) |
| `ET.HeroHelper.HeroLevelUp` | `0x296b7a0` | `hero_level_hook` | segue | não | Nível de herói (ouro + manual 6001). | [HEROES_V126](docs/testing/HEROES_V126.md) |
| `ET.HeroHelper.HeroStarUp` | `0x296b894` | `hero_star_hook` | segue | não | Estrela de herói (fragmentos). | [HEROES_V126](docs/testing/HEROES_V126.md) |
| `ET.EquipHelper.WearEquip` | `0x2c2d014` | `local_wear_equip_hook` | segue | não | Vestir equipamento. | log v67/v68 |
| `ET.EquipHelper.BreakEquip` | `0x2c2d118` | `local_equip_break_hook` | segue | não | Reciclagem individual. | [EQUIPMENT_RECYCLE_V120](docs/testing/EQUIPMENT_RECYCLE_V120.md) |
| `ET.EquipHelper.UpgradeEquip` | `0x2c2d20c` | `local_equip_upgrade_hook` | segue | não | Melhoria unitária. | log v72 |
| `ET.EquipHelper.OneKeyUpgradeEquip` | `0x2c2d300` | `local_equip_onekey_upgrade_hook` | segue | não | Melhoria sequencial do UID escolhido (regra de parada local, não confirmada no original). | log v72/v73 |
| `ET.EquipHelper.CompoundEquip` | `0x2c2d3f4` | `local_equip_compound_hook` | segue | não | Fusão individual. | [FUSION_1_9_1_V102](analysis/native/FUSION_1_9_1_V102.md) |
| `ET.EquipHelper.OneKeyCompoundEquip` | `0x2c2d50c` | `local_equip_onekey_compound_hook` | segue | não | "Unir tudo" com política local autorizada. | [ONEKEY_OFFLINE_V107](analysis/native/ONEKEY_OFFLINE_V107.md) |
| `UICompoundEquipTips.UpdateCompoundList` | `0x12f8530` | `log_fusion_selection_update` | segue | sim | Só observa: loga os UIDs selecionados. | [FUSION_1_9_1_V102](analysis/native/FUSION_1_9_1_V102.md) |
| `ET.HangUpHelper.GetHangUpData` | `0x2964eb0` | `hangup_data_hook` | segue | não | Calcula a recompensa de inatividade local. | log v51 |
| `ET.HangUpHelper.RewardHangUp` | `0x2964f98` | `reward_hangup_hook` | segue | não | Coleta (tipo 0) e Volta Rápida grátis (tipo 1); tipos pagos recusados. | log v51 |
| `ET.FightHelper.EnterFight` | `0x29c3728` | `enter_fight_hook` | segue | não | Entra na fase localmente e guarda o RegionId. | log |
| `ET.FightHelper.KillEnemy` | `0x29c4abc` | `kill_enemy_hook` | segue | não | Ouro/gemas/EXP por kill e drops pendentes. | [COMBAT_EXP_V150](docs/testing/COMBAT_EXP_V150.md) |
| `ET.FightHelper.EndGame` | `0x29c3f44` | `end_game_hook` | segue | não | Vitória/derrota, liquidação, progresso e recompensas. | [CHAPTER_REWARDS_V151](docs/testing/CHAPTER_REWARDS_V151.md) |
| `ET.FightHelper.Resurgence` | `0x29c54c0` | `resurgence_hook` | segue | não | Revive local via `GameSceneManager.OnResurgenceSucceed`. | log v51 |
| `FightDataManager.DeductEnemyHp` | `0x27C9D4C` | `deduct_enemy_hp_hook` | segue | sim | Região 3: dano mínimo 9999. Build QA heróis: mínimo 1.000.000. | código |
| `FightDataManager.DeductPlayerHp` | `0x27E027C` | `deduct_player_hp_hook` | segue | sim | Região 3: dano recebido a 10%. Build QA heróis: 0. Kills forçados preservados. | código |

"log vNN" aponta para a seção da build correspondente em [MAINTENANCE_LOG.md](docs/history/MAINTENANCE_LOG.md) ou [FUNCTIONAL_AUDIT.md](docs/history/FUNCTIONAL_AUDIT.md). Ao adicionar ou remover um hook, atualizar esta tabela no mesmo commit.

## Saves e arquivos privados

Tudo fica no `filesDir` do package, que não é debuggable: `run-as` não lê. Diagnosticar por logs `KinjaOffline`.

| Arquivo (`offline_save/`) | Conteúdo | No Backup |
|---|---|---|
| `seed_player.json` | Nível, EXP, ouro (`Coin`), gemas (`Diamond`), energia | sim |
| `seed_items.json`, `seed_equipment.json` | Inventário e equipamentos | sim |
| `seed_talents.json`, `seed_heroes.json`, `seed_guide.json` | Talentos, heróis, tutorial | sim |
| `seed_regions.json` | Capítulos/regiões vencidos, `MaxRegion` | sim |
| `seed_shop_chests.json` | Estado dos baús da Loja | sim |
| `chapter_rewards.json` | Ledger das recompensas de capítulo já pagas | sim, se existir |
| `hangup_local_state.json` | Último resgate de inatividade e Volta Rápida | sim, se existir |
| `shop_daily_claim.json` | Data do último resgate diário | sim, se existir |
| `local_preferences.json` | Som, música, sensibilidade, idioma | sim |
| `*_pending*` | Journals de transação (talento, baú, melhoria, heróis, resgate diário), recuperados no boot | não: com um journal pendente, o backup é recusado |
| `last_fight.json` | Diagnóstico da última partida | não |
| `qa_*.json` | Marcadores de preparação QA de uso único | não |

Fora de `offline_save/`: `offline_bootstrap_status.json` (status do bootstrap) e os diagnósticos `kinja_onekey_*.json`, que **nunca** são usados como entrada de inventário. A lista exata do backup está em `BackupStore.java`.

## Estado por sistema

Níveis de evidência: **estática** (tabelas/código), **log** (bridge chamado), **tela** (UI observada), **gameplay** (ação e consequência conferidas, inclusive após reiniciar). "Sem jogo" significa que só houve teste de host e/ou compilação.

| Sistema | Regra atual | Maior evidência | Pendente | Relatório |
|---|---|---|---|---|
| Login e menus | Login local; menus 101 (Capítulo 1), 102/103 (nível 2), 116 (Capítulo 2); a lista é montada no login | gameplay | Refresh de menu sem reiniciar; capítulos posteriores | [OFFLINE_READINESS_AUDIT](analysis/native/OFFLINE_READINESS_AUDIT.md) |
| Ouro/gemas de combate | Kill 5/25/50 ouro e `floor(ouro/2)` gemas; só vitória liquida; callback tardio credita só a diferença | gameplay | Atualização tardia da tela de resultado | log v52–v57 |
| Moedas do chão | Diferença de `FightDataManager.CoinCount` entra no ouro da vitória | sem jogo | Confirmar se `CoinCount` exclui o ouro de kills | [GROUND_COINS_V151](docs/testing/GROUND_COINS_V151.md) |
| EXP de combate e nível | 5/25/50 EXP por kill, só em vitória; `AccountLevelCfg` converte em níveis | gameplay (QA) | Derrota e callback tardio | [COMBAT_EXP_V150](docs/testing/COMBAT_EXP_V150.md), [ACCOUNT_LEVEL_QA_V116](docs/testing/ACCOUNT_LEVEL_QA_V116.md) |
| Recompensas de capítulo | `GameRegion.first_win_reward`/`progress_reward` pagos uma vez via ledger; migra saves antigos | sem jogo | Leitura real das tabelas, valores pagos, reinício | [CHAPTER_REWARDS_V151](docs/testing/CHAPTER_REWARDS_V151.md) |
| Drops | `RandomRewardPack` original; 10 rolagens vazias garantem um livro 20001–20006; só vitória persiste | gameplay | — | log v68/v69 |
| Fragmentos de herói | 5 por vitória; após Capítulo 5 vão para o próximo herói a liberar | sem jogo | Tudo em jogo | [HERO_FRAGMENTS_V151](docs/testing/HERO_FRAGMENTS_V151.md) |
| Heróis | Seleção, obter, nível, estrela com tabelas originais e journal | gameplay (QA) | Máximos, talentos de herói, falha injetada | [HEROES_V126](docs/testing/HEROES_V126.md), [COMBAT_INVENTORY_V129](docs/testing/COMBAT_INVENTORY_V129.md) |
| Talentos | Comuns gastam 101; azuis gastam 40001 | gameplay | Efeito em combate, saldo insuficiente | log v56/v74/v75 |
| Equipamento | Vestir, melhoria unitária, OneKey, reciclagem individual | gameplay (exceto OneKey) | Semântica exata do OneKey original | [EQUIPMENT_RECYCLE_V120](docs/testing/EQUIPMENT_RECYCLE_V120.md), log v72 |
| Fusão | Individual com receitas `EquipQualityCfg.cost`; "Unir tudo" com política local autorizada | gameplay (individual); log (coletiva) | Coletiva consumindo itens; materiais equipados | [FUSION_1_9_1_V102](analysis/native/FUSION_1_9_1_V102.md), [ONEKEY_OFFLINE_V107](analysis/native/ONEKEY_OFFLINE_V107.md) |
| Loja — baús | 101 bronze: 10.000 ouro; 115 diamante e 108 (x10): gemas; Divino 2%; rota por chave (Item) implementada |  gameplay (QA, saldo exato) | Saldo abaixo/acima, x10, rota por chave, recorte visual do x10 | [CHESTS_V148](docs/testing/CHESTS_V148.md) |
| Loja — resgate diário | **30.000 ouro** por dia (`kShopDailyGold`; era 4.000 até v150) | gameplay com 4.000; **sem jogo com 30.000** | Conferir 30.000 no aparelho; mudança de relógio | log v67 |
| Loja — troca de gemas | 100 → 20.000 e 500 → 118.888 ouro | gameplay | Saldo insuficiente, toque duplo | log v77 |
| Inatividade (HangUp) | `TimeGainCfg` por capítulo, até 6 h; Volta Rápida grátis 3/dia | gameplay (coleta) | Feedback dos tipos pagos | log v51 |
| Revive | Callback local de anúncio + `OnResurgenceSucceed` | gameplay | Painel de pausa reaparece após o revive | log v51 |
| Backup manual | Formato 2, exportação/importação SAF, recuperação transacional | gameplay (QA) | Dois aparelhos físicos, run completa após restore | [MANUAL_BACKUP_V141](docs/testing/MANUAL_BACKUP_V141.md) |
| Serviços online | Compras bloqueadas; Correio oculto; guilda ignorada; SDKs ainda tentam rede | tela/log | Sem substituto local de correio/guilda | log v69–v73 |
| Sem rota offline | `DifficultPass`, contagem de derrotas, dias de login, tutorial (`GuideIds`) | estática | Decisão do usuário | [CHAPTER_REWARDS_V151](docs/testing/CHAPTER_REWARDS_V151.md) |

## Packages e perfis

| Package | Uso |
|---|---|
| `com.habby.kinjarun.offline.audit1` | **Perfil normal do usuário.** Builds normais desde v112; seeds normais, sem defines QA. |
| `com.habby.kinjarun.offline.lab6` | Perfil antigo de laboratório (até v107), designado como perfil QA com recursos abundantes. Não representa progressão natural. |
| `com.habby.kinjarun.offline.qa.*` | Perfis isolados de teste com fixtures: `expmigration`, `expreward`, `expcombat`, `equipbreak`, `heroes`, `chests.{below,exact,above}`, `backup.{source,destination}`. `build.ps1` recusa fixtures fora do package exato. |

Todos usam o keystore de laboratório (certificado SHA-256 `7508ca8d…a75b3f`). Atualizar com o mesmo package e certificado preserva o save.

## Build

Configuração da máquina: [docs/development/SETUP.md](docs/development/SETUP.md). Passo a passo da build normal: [docs/development/RELEASE.md](docs/development/RELEASE.md). Resumo:

```powershell
./tools/workspace/new-normal-build.ps1 -BuildId <vNNN-descricao> -DryRun   # só conferências e testes
./tools/workspace/new-normal-build.ps1 -BuildId <vNNN-descricao>           # gera, confere e registra
```

- O script exige main limpa, usa o package da última build normal do histórico, recusa BuildId repetido, roda os testes, compila sem QA e empacota. Depois confere certificado, `.so` e seeds dentro do APK e acrescenta a entrada em `build-history.json`. Não instala, não commita e não faz push.
- Opções QA de compilação (`-QaMaxProfile`, `-QaFusionRepeats`, `-QaHeroCombat`) e fixtures de `build.ps1` são só para packages QA, chamando `build-native.ps1` e `build.ps1` diretamente.
- Avisos esperados: `unused parameter 'equipment_rewards'` e 69 linhas `META-INF … not protected by signature`.
- Instalação: manual pelo usuário, ou `adb install -r` no mesmo package quando autorizado.
- Depois que o usuário terminar os testes: `./tools/workspace/clean-builds.ps1 -KeepBuildId <build-normal-atual>` (`-WhatIf` antes). O APK precisa estar com o nome original.

## Testes de host

`./tests/run-host-tests.ps1` roda tudo e mostra PASSOU, FALHOU, COMPILADO ou IGNORADO por teste. Detalhes e como escrever um teste: [tests/README.md](tests/README.md).

| Teste | Cobre |
|---|---|
| `tests/backup` (`BackupStoreTest`) | Codec, validação e transação do backup |
| `tests/chapter_rewards_test.cpp` | Planejador de recompensas de capítulo |
| `tests/hero_fragments_test.cpp` | Destino dos fragmentos |
| `tests/combat_exp_test.cpp` | EXP de combate |
| `tests/account_progression_test.cpp` | Conversão EXP → nível (fixture gerada da tabela do acervo) |

**Limite conhecido:** esta máquina não tem compilador C++ de host e o usuário preferiu não instalar. Sem `-Serial`, os testes C++ ficam COMPILADOS (clang do NDK), não executados. Com `-Serial <BlueStacks>`, rodam no emulador sem instalar APK. Registrar o resultado real ao validar uma build. O bridge em si (hooks, JSON dos saves) não tem teste de host.

## Quando o usuário relatar um erro

1. Resumir o sintoma de forma reproduzível: tela, ação, esperado, visto, build.
2. Separar protótipo de navegador e APK. No APK, mapear texto/botão à classe em `dump.cs` e às tabelas antes de editar.
3. Reconstruir a sequência: caller → método/manager → callback → persistência → atualização visual.
4. Ler o código do fluxo e os logs `KinjaOffline`. Separar causa provada de hipótese.
5. Fazer a menor mudança que corrija a causa, preservando rotas originais de UI e lifecycle.
6. Compilar e empacotar com BuildId novo quando autorizado. Não dizer "corrigido" se só compilou.
7. Atualizar a linha do sistema e a tabela de hooks neste guia, e criar ou atualizar o relatório em `docs/testing/`.

| Sintoma | Primeiro ponto |
|---|---|
| Para em "Cannot Resolve destination Host" | É a build offline? `offline_bootstrap_status.json`, sucesso do `AccountInit`. |
| Home com cadeados | Log `Local menu progression`, IDs de `SetOpeningFunc`, `Level`/`IsPass` do save. |
| Botão abre painel vazio ou com erro | `FunctionCfg`, dependência remota, NRE no log Unity. Botão visível não é serviço implementado. |
| Moeda ou recompensa não aparece | `kill_enemy_hook`, `end_game_hook`, callback ET, inventário persistido, ID do item. |
| Capítulo não avança | `fight_won`, `persist_local_clear`, `MaxRegion`, `refresh_local_regions`. |
| Revive deixa tela escura ou sem input | Ordem de `OnResurgenceSucceed` e da coroutine original; não fechar só a UI. |
| Item ganho só aparece depois de comprar na Loja | Reconciliação por `OnItemInit` (resolvida na v129; ver [COMBAT_INVENTORY_V129](docs/testing/COMBAT_INVENTORY_V129.md)). |

Quando faltar informação, fazer uma pergunta curta e específica e continuar o que não depende dela.

## Build normal v151 — compilada e assinada; instalada pelo usuário (relato), sem QA por sistema

`v151-chapter-rewards-fragments-coins` reúne recompensas de capítulo, moedas do chão, fragmentos de herói e resgate diário de 30.000 ouro. Package `audit1`, sem defines QA, seeds idênticos a `offline/assets`, sem fixtures; certificado igual ao da v150.

- APK `build/kinja-run-offline-lab-v151-chapter-rewards-fragments-coins.apk`, SHA-256 `944BB5644FEE1D797E8327E60802214C6F1DD7668E694DF9DE13408B022F7958`.
- `libkinjaoffline.so` SHA-256 `815D2268E9BD85F9029870BC4CCA4BB5CF763E5FE04BC29D4844F0190C89461C`, igual ao que está no APK.
- Avisos: parâmetro não usado `equipment_rewards` (`offline_bridge.cpp:1360`) e os 69 avisos `META-INF` habituais do apksigner.
- Testes: `BackupStoreTest` passou (2136 verificações, 82 cenários, 41 pontos). `chapter_rewards_test` e `hero_fragments_test` só compilaram com o NDK.
- O agente não instalou a build e não houve QA. O usuário a instalou depois e relatou que funcionou, sem detalhes por sistema; os itens "sem jogo" da tabela acima continuam pendentes até confirmação específica. `clean-builds` ainda não rodou.
