# Fragmentos de herói — fonte offline (código pronto, sem build)

## Problema

O save do usuário (26/11/2026) não tem nenhum fragmento de herói (701001, 704002, 704003). Gastar fragmentos já funciona offline: obter 4002/4003 (80) e subir estrela (`HeroStarCfg.item_cost`, ex.: 120, depois 160), validado em QA na v124–v126. Mas nenhuma rota offline **gerava** fragmentos:

- os pools de combate (`RewardCfg` 1000x0) entregam livros, manuais e 300101;
- o HangUp entrega só ouro/EXP;
- os baús da Loja entregam equipamento.

No 1.9.1 os fragmentos vinham de ofertas pagas e eventos online, que não existem offline. Sem fonte, os heróis 4002/4003 não são liberados e as estrelas não sobem.

## Regra local aprovada pelo usuário

- Fonte: drop na vitória. Derrota ou abandono não dão nada.
- Quantidade: **5 fragmentos por vitória** (`kinja::kVictoryHeroFragments`).
- Destino (`choose_fragment_hero`, em `offline/native/hero_fragments.h`):
  1. Com o menu de heróis aberto (função 107, depois do Capítulo 5), vão para o primeiro herói não obtido com `can_receive=1`, na ordem 4002 e depois 4003.
  2. Senão, vão para o herói selecionado, se ele ainda tiver uma próxima estrela.
  3. Senão, vão para o primeiro outro herói obtido que ainda tenha uma próxima estrela.
  4. Se nenhum herói puder usar fragmentos, nada é concedido.
- O ID do fragmento vem de `HeroManager.GetHeroCfg(id).fragment_id` (tabela original). O limite de estrela vem de `GetHeroStarCfgOne(id, estrela+1)`. Um ID fora do catálogo do backup é recusado.
- Entrega: os fragmentos entram na lista de itens da vitória. Por isso, são salvos em `seed_items.json`, aparecem na tela de resultado original e são reconciliados por `ItemManager.OnItemInit`, como os outros drops. Qualquer falha de leitura só gera log e não bloqueia a vitória.
- Não há concessão retroativa por vitórias passadas.

## Validação alcançada

- `tests/hero_fragments_test.cpp` passou (save do usuário, ordem de liberação, limite de estrela, herói não recebível, ID inválido).
- `clang++ -fsyntax-only` do bridge não trouxe erros novos.

**Não validado:** NDK/APK/instalação, leitura real de `HeroCfg` na vitória, tela de resultado, inventário após reinício, liberação do 4002 com fragmentos ganhos. Logs esperados: `Victory hero fragments hero=… item=… count=5`, ou `Victory hero fragments skipped: …`.

## Teste sugerido (QA)

Restaurar o save do usuário num package QA. Vencer uma fase e conferir `701001 +5` no resultado e no herói 1001 (receita de estrela). Reiniciar e conferir o saldo. Num perfil com o Capítulo 5 vencido, conferir que os fragmentos vão para o 4002 até 80, e que o botão Obter libera o herói.
