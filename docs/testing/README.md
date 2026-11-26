# Relatórios de sistemas e builds

Um relatório por sistema ou correção. O estado resumido de cada sistema fica no [guia](../../MAINTENANCE_GUIDE.md#estado-por-sistema); aqui está o detalhe. Relatórios citam arquivos em `build/` que podem já ter sido apagados pelas limpezas autorizadas.

## Vigentes

| Relatório | Sistema | Evidência mais alta |
|---|---|---|
| [CHAPTER_REWARDS_V151](CHAPTER_REWARDS_V151.md) | Recompensas de capítulo (cristais 40001) | teste de host; build v151 sem QA |
| [GROUND_COINS_V151](GROUND_COINS_V151.md) | Moedas coletadas do chão | build v151 sem QA |
| [HERO_FRAGMENTS_V151](HERO_FRAGMENTS_V151.md) | Fragmentos de herói por vitória | teste de host; build v151 sem QA |
| [COMBAT_EXP_V150](COMBAT_EXP_V150.md) | EXP de combate e nível da conta | gameplay em QA |
| [CHESTS_V148](CHESTS_V148.md) | Baús da Loja: preços, Divino 2% | gameplay em QA (saldo exato) |
| [MANUAL_BACKUP_V141](MANUAL_BACKUP_V141.md) | Backup manual nativo | gameplay em QA |
| [COMBAT_INVENTORY_V129](COMBAT_INVENTORY_V129.md) | Crédito imediato de materiais | gameplay; aprovado pelo usuário |
| [HERO_QA_COMBAT_V127](HERO_QA_COMBAT_V127.md) | Build QA de heróis fortes | gameplay em QA |
| [HEROES_V126](HEROES_V126.md) | Heróis: seleção, nível, estrela, obter | gameplay em QA |
| [EQUIPMENT_RECYCLE_V120](EQUIPMENT_RECYCLE_V120.md) | Reciclagem individual | gameplay em QA |
| [ACCOUNT_LEVEL_QA_V116](ACCOUNT_LEVEL_QA_V116.md) | Nível da conta (migração e crédito) | gameplay em QA |
| [BLUESTACKS](BLUESTACKS.md) | Uso do emulador para testes | log e tela |

## Superados (registro histórico)

| Relatório | Substituído por |
|---|---|
| [ACCOUNT_LEVEL_V112](ACCOUNT_LEVEL_V112.md) | ACCOUNT_LEVEL_QA_V116 |
| [HERO_MATERIAL_V126](HERO_MATERIAL_V126.md) | HERO_QA_COMBAT_V127 e COMBAT_INVENTORY_V129 |
| [MANUAL_BACKUP_V131](MANUAL_BACKUP_V131.md) | MANUAL_BACKUP_V141 |

Os arquivos `.json` com o mesmo nome guardam as sínteses de evidência e hashes. `images/` tem as capturas citadas pelos relatórios de backup.

## Ao criar um relatório

Nome `<SISTEMA>_V<NNN>.md`. Incluir: relato ou problema, causa provada vs hipótese, regra implementada, testes executados e nível de evidência, o que não foi validado e como testar. Acrescentar a linha aqui e atualizar a linha do sistema no guia.
