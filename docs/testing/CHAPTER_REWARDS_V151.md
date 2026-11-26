# Recompensas de capítulo e cristais azuis — correção de código (pré-build v151)

## Relato e evidência

Relato do usuário (26/11/2026): chegou ao Capítulo 11 e só recebeu os primeiros cristais azuis; depois disso, nada. Save enviado: `Kinja-Run-20261124-232851.kinjabak`, formato 2, decodificado e validado pelo `BackupStore` atual sem erro.

Conteúdo relevante do save (somente leitura, nada foi alterado):

| Campo | Valor | Leitura |
|---|---|---|
| `seed_player` | Nível 10, 1949 EXP, 1019 ouro, 36 gemas | Conta ativa. |
| `seed_regions` | Regiões 2–12 `IsPass=true`; 13 em andamento (`MaxRegion=13`) | Capítulos 1–11 vencidos. |
| `seed_items` | 40001 ×1, 100001 ×1, 100002 ×1 | Cristal azul e chaves bronze/prata. |
| `seed_talents` | `ExtraTalentLevel=1` | 1 cristal gasto (nó 2001 custa 1). Total recebido: 2. |

Os 2 cristais e as duas chaves são exatamente as três recompensas fixas do bridge antigo.

## Causa confirmada no código

`persist_local_clear` concedia recompensas de capítulo por valores fixos:

- região 2 (Capítulo 1): primeira vitória → 100001; marco 100% → 40001 ×2;
- região 3 (Capítulo 2): primeira vitória → 100002;
- demais regiões: nada.

A tabela original 1.9.1 `GameRegionCfgData` define, por região, `first_win_reward_type`/`first_win_reward` e listas `progress`/`progress_reward`. Cada `progress_reward` aponta para `RewardCfgData.item_ids` (ex.: 10013 = `"40001_2"`). Auditorias anteriores (v69/v70) já registravam que “capítulos têm recompensas configuradas desse cristal (incluindo 1 no marco do Capítulo 3)” e que só o Capítulo 1 estava ligado. Antes, o servidor pagava essas recompensas; offline, ninguém pagava. Isso explica o sintoma.

As gemas por inimigo (metade do ouro por abate, só em vitória) continuam como antes. O saldo de 36 gemas não prova falha nessa rota: baús e câmbio da Loja gastam gemas, e o save não guarda histórico de gastos.

## Correção

Arquivos: `offline/native/chapter_rewards.h` (planejador puro), `offline/native/offline_bridge.cpp` (leitura das tabelas e integração), `offline/native/backup_items.h` (espelho de `BackupCatalog.ITEMS`), `offline/java/local/kinja/BackupStore.java` (backup do ledger).

1. **Leitura da configuração original.** Na thread Unity, `GameRegion.GetCfg(região)` e `Reward.GetCfg(id)` pelas APIs IL2CPP (`runtime_invoke`, `class_get_field_from_name`, `System.Array`/`List`). Os campos são verificados por nome e tipo (`System.Int32`, `System.Int32[]`, `System.String[]` ou `List<>`). Sem RVA, hook ou escrita em offsets novos. Os nomes de classe aceitos são `GameRegion`/`GameRegionCfg` e `Reward`/`RewardCfg`, seguindo a convenção `MonsterCfgData → Monster`. **Essa convenção ainda não foi confirmada no aparelho para essas duas tabelas.**
2. **Slots e ledger.** Slot 0 = primeira vitória; slot n = `progress_reward[n-1]`. Os resgates ficam em `offline_save/chapter_rewards.json`, no formato `{"Version":1,"Claims":[RegiãoId*100+slot,...]}`. Cada slot é pago uma vez.
3. **Política offline.** Todos os marcos configurados (≤100%) são pagos na primeira vitória completa da região. O jogo não persiste o maior percentual de uma derrota; por isso, marcos abaixo de 100% não são pagos em derrota.
4. **Itens aceitos.** Somente tokens fixos `ItemId_Count`. Pools ponderados (`Id_Peso_Qtd`), tipos de primeira vitória desconhecidos, campos de equipamento/herói não vazios e IDs fora do catálogo do backup ficam **devidos**, sem concessão. Eles são tentados de novo no próximo boot e registrados em log.
5. **Destino.** 101/102/104 (ouro/gemas/EXP) entram no acerto da vitória (`settle_completed_run_currency`) e aparecem no resultado. Os demais itens entram na lista `Items` do `G2C_EndGame`, em `seed_items.json` e na reconciliação `OnItemInit` já existente.
6. **Migração de saves antigos (seu caso).** No boot, antes de desserializar os seeds, `settle_saved_chapter_rewards` cria o ledger se ele não existir. Ela marca como já pagos os slots que o código antigo entregou: (2,0), (2,1) e (3,0). Depois, paga os slots devidos de todas as regiões vencidas. A ordem é ledger primeiro e depois itens/jogador: uma interrupção pode perder uma recompensa, mas nunca a duplica. Falhas não bloqueiam o login.
7. **Fallback.** Se a tabela não puder ser lida na vitória das regiões 2/3, o comportamento antigo é mantido (chaves e 40001 ×2) e registrado no ledger. Nas outras regiões, os slots ficam devidos.
8. **Diagnóstico.** No primeiro uso, o log `Config schema chapter region/reward` lista campos e tipos das duas classes (exports opcionais `il2cpp_class_get_fields`/`il2cpp_field_get_name`). Se a convenção de nomes estiver errada, o log mostra isso sem afetar o jogo.
9. **Backup.** `chapter_rewards.json` é um arquivo opcional (índice 11). Ele é validado: versão 1, claims únicos, região do catálogo, slot 0–98, sem chaves extras. Backups antigos sem o arquivo continuam válidos. Ao restaurá-los, o ledger é recriado com a suposição legada, o que está correto. **Backups novos que contêm o ledger são recusados por builds anteriores.**

## Validação alcançada

- **Host, planejador puro:** `tests/chapter_rewards_test.cpp` passou (tokens, ledger estrito, migração legada do formato do save do usuário, idempotência, slots não resolvidos, overflow).
  `g++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined tests/chapter_rewards_test.cpp && ./a.out`
- **Host, backup:** `BackupStoreTest` passou com 2133 verificações, 82 cenários e 41 pontos (era 1943 antes do novo arquivo). Inclui ledger malformado recusado, ledger opcional e round trip.
- **Host, sintaxe do bridge:** `clang++ -fsyntax-only` com stubs Android não trouxe erros novos. O único erro é o já existente `AttachCurrentThread(JNIEnv**)` do `jni.h` do JDK, que não ocorre com o NDK.
- **Save do usuário:** decodificado e validado pelo `BackupStore` novo.

**Não validado:** compilação ARM64 com NDK, empacotamento/assinatura, instalação, boot, leitura real das tabelas `GameRegion`/`Reward`, valores reais pagos por capítulo, tela de resultado e persistência após reinício. Este ambiente não tem o APK, o NDK, a keystore nem o acervo de tabelas. Não promover a corrigido em jogo antes desses passos.

## Próximos passos (Windows/BlueStacks)

1. `./offline/native/build-native.ps1`, depois `./offline/build.ps1 -BuildId v151-chapter-rewards-config -PackageName <package QA> -PackageApk`.
2. Em package QA, restaurar uma cópia do save do usuário pelo Backup e abrir o jogo. Conferir `Config schema chapter region/reward`, `Chapter reward ledger created from legacy save`, `Chapter rewards owed region=…` e `Chapter rewards settled at bootstrap`.
3. Comparar 40001/chaves/gemas na home e em Talentos com os valores do log. Reiniciar e conferir que nada é pago duas vezes.
4. Vencer a região 13 e conferir o resultado (`Chapter clear rewards region=13`), o inventário e o reinício.
5. Somente depois disso, gerar a build normal e atualizar este documento com hashes e resultado.
