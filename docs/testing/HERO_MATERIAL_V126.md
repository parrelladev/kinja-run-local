# Manual do Herói — auditoria da obtenção na v126

Em03/11/2026, o usuário relatou teste positivo do jogo com novos heróis. Evidência por relato; não confundir com uma bateria instrumental de todos os heróis. Solicitou validar a obtenção do recurso de evolução antes de seguir ao tutorial.

## Resultado

O material é **Manual do Herói**, ItemId6001, ícone `hero_exp`, tradução original `goods_6001`; descrição `goods_desc_6001`: usado para aumentar nível do herói. Nível consome ouro101+manual6001. Estrelas consomem fragmentos específicos (701001/704002/704003), não manuais. Primeira melhoria custa170ouro+50manuais, custo já consumido/readback/restart validado com fixture na rodada v126. Isso não comprova obtenção natural.

Reutilizadas tabelas recuperadas do acervo externo, somente leitura. Nenhuma nova engenharia reversa/extracão ou mudança de economia.

## Fontes conferidas

- `HeroExpCfg`:99 linhas de custo, níveis1–4 custam170ouro/50manuais; depois os custos aumentam. A UI busca o custo do nível atual.
- `ItemCfg` id6001 e `LanguageCfg` confirmam identidade e nome. Não confundir com EquipId6001 (Calças de Couro), que usa o mesmo número em outro domínio.
- `RewardCfg`: pool comum do capítulo1 (`100010`) não contém itens; capítulo5 (`100050`) não inclui6001. Pool do capítulo6 (`100060`) inclui `6001_5000_6`; capítulo13 (`100130`) também. Tripla de configuração, sem promover peso5000 a uma probabilidade percentual verificada. `MonsterCfg` tem60 registros que apontam ao pool100060; exemplo monstro10601. Não é drop garantido de todo inimigo.
- `collect_local_combat_drop` chama Monster.GetCfg/RewardManager.RandomRewardPack originais. Item positivo, incluindo6001, entra em pending_run_items. `end_game_hook` usa a fila somente em vitória, `persist_local_clear` grava seed_items e `apply_local_reward_items` chama AddLocalItem original. Derrota/abandono não concedem a fila.
- Ouro: política offline de mortes5normal/25elite/50boss; crédito local da partida exige vitória. Espera/HangUp fornece ouro e EXP de conta por TimeGainCfg, limitado a6horas; já houve validação de coleta persistente em QA v116. **HangUp offline atual não fornece manuais**: hangup_items_json só emite101/104, sem executar pools de itens. Não confundir EXP104 com Manual6001.
- Tabelas também contêm referências a6001 em recompensas/pacotes online. Não considerar compras/eventos como fontes offline disponíveis. Baús de equipamentos podem mencionar EquipId6001: isso não demonstra concessão de Manual6001.

## Evidência do emulador

BlueStacks127.0.0.1:6610, QA `com.habby.kinjarun.offline.qa.heroes`. Logs existentes do teste do usuário, PID6896: EnterFight region2/Chapter1 às18:49:24; vitória EndGame stage11 às18:54:01, passed1,620ouro/257gemas, itemStacks0. Os drops observados usaram100010/100011 com item0. Resultado consistente com capítulo1 sem manuais; não comprova recebimento de6001. Atividade anterior do jogador alterou saldos QA; não atribuir esse histórico às ações de auditoria atuais.

Tentativa instrumental desta auditoria: abriu QA existente sem update/wipe, navegou Chapter13→6 e iniciou region7 às18:58:46, PID9495, enemyCount94. O herói QA sem equipamento não concluiu: tela Renascimento, sem compra de revive; EndGame stage1 às18:59:54, passed0/gold0/diamond0/itemStacks0. Retorno à home Chapter6 com1950ouro/257gemas, sem crédito de manuais. Não houve drop6001 observado. Perfil normal e telefone não alterados. Tentativa não prova defeito do drop nem sucesso da concessão.

**Status:** identidade/custos e rota de obtenção no código/configuração confirmados; consumo já validado artificialmente. A bateria “drop natural6001 → vitória → aumento de inventário → reinício → usar manual ganho” permanece pendente. Não anunciar obtenção natural como validada ou inventar fonte/recompensa para encerrar o teste. Combate com novos heróis aceito pelo relato do usuário; obtenção do material ainda exige teste positivo em capítulo6 ou posterior com perfil capaz de vencer. Tutorial não iniciado.

Sem alteração de código ou nova build. V126 normal permanece atual; somente síntese/hashes dos arquivos brutos locais preservados após limpeza autorizada.
