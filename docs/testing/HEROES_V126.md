# Heróis offline — v126 (03/11/2026)

Escopo solicitado: selecionar herói, subir nível e aumentar estrelas. Nível de conta e reciclagem individual foram aprovados pelo usuário como resolvidos. Tutorial permanece fora desta rodada. [Síntese instrumental e hashes](HEROES_V126.json).

## Causas e contrato

Reutilizados os índices, tabelas e binário recuperado do acervo externo, sem nova extração ou alteração do acervo. Alvo 1.9.1 original SHA256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`.

1. O seed antigo tinha Star0. `UIHeroPage.HeroProperty` procura a configuração dessa estrela, que não existe; a NRE deixava placeholders e interrompia o painel. A estrela mínima original é1. Migração conservadora altera somente0→1 quando a configuração original existe e comporta o nível salvo; não cobra fragmentos, muda nível ou concede recursos. Seed normal novo já nasce com1.
2. A função107 nunca era enviada pelo bridge. `FunctionCfg.GetCfg(107)` exige unlock_type1, unlock_lv7. O bridge confere MaxRegion>=7 e região6 concluída, correspondente ao texto original “Termine o capítulo5”. Conta inicial permanece bloqueada. Não abre os outros menus.
3. `ET.HeroHelper.EquipHero/HeroLevelUp/HeroStarUp` falharam em runtime na v123 QA por ausência de sessão online. `ExchangeHero` usa o mesmo contrato remoto na análise existente; não houve reprodução separada da falha desse helper antes do hook.

Os quatro helpers devolvem `ETTask<bool>`; a atualização é uma mensagem independente com **lista completa**, `G2C_PlayerHeroDataUpdate`. O bridge chama o handler original `G2C_PlayerHeroDataUpdateHandler.Run(null,message)`, que atualiza `HeroManager`, atributos, caches/redpoints e evento de UI. Instância criada por `Activator.CreateInstance(Type)`. Nenhum offset gerenciado escrito. Execução na thread Unity; prólogos e hash originais preservados.

| Operação | Configuração/custo |
|---|---|
| Equipar | Herói já adquirido, sem custo; conserva todos os registros/progressos |
| Obter | HeroCfg.can_receive1, fragment_id/fragment_num; novo herói nível1/estrela1; sem oferta paga simulada |
| Nível | HeroExpCfg do nível atual: coin_cost/item_id/item_cost; limite lv_limit da estrela atual |
| Estrela | HeroStarCfg atual.item_cost e HeroCfg.fragment_id; exige configuração da próxima estrela; não inventa requisito de nível20 |

Hooks com prólogo `ff4301d1f44f03a9fd7b04a9fd030191`: EquipHero0x296B5B8, ExchangeHero0x296B6AC, HeroLevelUp0x296B7A0, HeroStarUp0x296B894. Todas as chamadas de configuração/callback continuam pelas APIs IL2CPP originais.

## Persistência

`hero_offline.h` valida identidades únicas, posse, seleção, níveis/estrelas salvos versus getters originais, contagem completa de AllHeroInfo, recursos vivos/salvos e configuração antes de consumir. Conserva JSON de cada registro, inclusive HeroTalent. Modelos de ida/volta e moeda obscured preparados antes da alteração. Snapshots dos três arquivos, marker `hero_pending`, gravação atômica e fsync precedem callbacks/readbacks. Só retorna sucesso depois de conferir manager, moeda, materiais e bytes salvos. Rollback restaura os três arquivos, HeroDataInit, cache original de combate, itens e moeda. Marker retido em restauração incompleta; bootstrap recupera os snapshots antes da migração/desserialização. Recuperação sob interrupção e rollback de falhas **não foram exercitados em runtime**.

Fixture opt-in `-HeroFixture` restrita a `com.habby.kinjarun.offline.qa.heroes`; recusa outros packages/fixtures combinados. Seeds artificiais só na primeira criação do perfil: conta20/5000ouro/Chapter13, heróis1001+4002, materiais6001×50,701001×120,704003×80. Não repõe recursos em updates/restarts. Nenhum save do celular importado/modificado.

## Testes no BlueStacks

ADB127.0.0.1:6610, Android16/API36. v121 reproduziu Star0/placeholders; v122 migrou1 e exibiu nome/Lv1/20; v123 abriu chooser e reproduziu NRE dos três helpers acima. v124 validou com APIs/callbacks/readbacks e telas originais:

- Equipar4002: marca de selecionado mudou; ouro5000 inalterado.
- Herói1001 nível1→2: ouro5000→4830; item6001 50→0; painel Lv2/20, ATQ25/HP135.
- Herói1001 estrela1→2: fragmentos701001 120→0; nível2 mantido, teto20→30; tela “Mais Estrelas!” e próxima receita0/160.
- Obter4003: fragmentos704003 80→0, nível1/estrela1; modelo colorido e botão Selecionar substituem Obter. Nova seleção4003 observada na home após update/restart.
- Tentativas pelos botões sem materiais não evoluíram nem cobraram novamente (guard da UI; não é teste direto da recusa do hook).
- v125 usa a biblioteca final, acrescenta conferência de contagem total e restauração explícita do cache de combate no rollback. Update/restart carregou4830ouro, três heróis,1001 Lv2/30,4003 adquirido/selecionado, sem reposição. Selecionar1001 passou com a nova conferência de três registros; readback/save/custo zero confirmados.

Não foram testados combate com outro herói, nível/estrela máximos, talentos, compras, falhas/interrupção injetadas ou save real avançado do telefone. Seleção, primeira subida de nível/estrela, aquisição por fragmentos e persistência foram validadas nos menus QA; isso não representa bateria completa de combate/progressão.

## Build normal

`v126-local-heroes`, package `com.habby.kinjarun.offline.audit1`, mesmo certificado de desenvolvimento existente. APK SHA256 `D3C84387656D17BF3CB5FE2E8E6DFCB98F67D04419B7B264474CB775DD02900B`; biblioteca SHA256 `B351E39C02FF370A1DD5AF326E846FB0499281DB5E93EFDF59DACE5ABA2362AA`, idêntica à QA final. ZIP conferido: HeroFixturefalse, seeds normais Nv1/1500ouro e apenas1001 nível1/estrela1; sem recursos/heróis QA.

`adb install -r` bem-sucedido; firstInstallTime preservado `2026-11-03 14:32:47`, lastUpdateTime `18:44:16`. PID7738 instalou os quatro hooks, migrou o1001 existente0→1 e completou initialized. Home normal preservada:1500ouro/0gemas/0EXP/Nv1/Chapter1, menus ainda bloqueados. Não executadas transações consumidoras no perfil normal.

Compilação ARM64 e empacotamento/assinatura concluídos; warning preexistente de parâmetro equipment_rewards não usado. `git diff --check` passou. As tentativas iniciais de abertura de algumas atualizações não deixaram processo ativo; nova abertura carregou normalmente. Sem evidência suficiente para atribuir a origem; não declarar teste de estabilidade prolongada. Acervo, chave/senha e saves dos outros packages preservados.

Após a síntese, limpeza autorizada mantém apenas APK v126 e relatório de assinatura em build; IDs/hashes ficam no histórico versionado. Capturas/logs brutos citados na síntese são históricos e foram removidos pela limpeza. Correção entregue para avaliação do usuário; aguardar aprovação antes de tutorial.

Limpeza concluída:4947564667→375970459bytes,4571594208bytes liberados; somente APK assinado v126 e relatório de assinatura, hash reconferido.
