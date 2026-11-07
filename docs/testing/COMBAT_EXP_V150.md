# EXP de combate offline — v150, 06/11/2026

## Decisão e implementação

O cliente original 1.9.1 recebe `G2C_KillEnemy.Exp` do servidor e soma esse valor ao acumulador da partida. `G2C_EndGame.Exp` é outro campo recebido; a fórmula do produtor remoto e sua relação com a EXP por inimigo não foram recuperadas. A tabela `RoundExpCfg` tem valores 200/500/1500, mas não há evidência de quando são usados. **A regra abaixo é uma escolha offline aprovada pelo usuário, não uma reconstrução comprovada da fórmula original.**

Cada resposta local `KillEnemy` informa 5 EXP para inimigo comum, 25 para elite ou 50 para boss, usando o setter IL2CPP original `set_Exp`. O boss prevalece se ambos os flags vierem marcados. `EndGame.Exp` mostra o total de EXP da mesma run vencedora; esse campo não é somado novamente ao saldo. O bridge mantém totais e valores já creditados separados para EXP, ouro e gemas. Somente após `fight_won` confirmar vitória, `grant_local_currency_reward` aplica a diferença pendente de EXP junto com as outras moedas, pelas APIs originais de `PlayerDataManager`, normaliza `Level/Exp` com `AccountLevelCfg`, confere os getters e salva os campos no seed existente. Derrota e saída sem `EndGame` não liquidam EXP. Um `KillEnemy` tardio após a vitória atualiza a resposta `EndGame` retida e liquida apenas a diferença não creditada. Prólogos/hash e thread Unity permanecem como antes; nenhum offset gerenciado é escrito.

## Validação

- **Teste automatizado isolado no BlueStacks:** `tests/combat_exp_test.cpp` passou para 5/25/50, prioridade de boss, crédito inicial, repetição com delta zero, callback tardio de 5, totais inválidos e nível 2 com 1990+80 EXP → nível 3 com 70. `tests/run-account-progression.ps1` passou com a tabela original de 200 níveis, os 199 limites e entradas inválidas.
- **Compilação:** ARM64 `libkinjaoffline.so` SHA-256 `710B3244D7410F8457C5DA8472BB41F1E53A6B0608795936FFC45960557D2FFE`. QA v149 e normal v150 usam essa biblioteca idêntica. O warning anterior de `equipment_rewards` não usado permanece.
- **Jogo no BlueStacks, somente `com.habby.kinjarun.offline.qa.expcombat`:** fixture novo começou em nível 2, 1990/2000 EXP, 1500 moedas e 0 gemas. Antes da fase houve coleta diária de 4000 moedas na Loja do QA, confirmada por log; EXP permaneceu 1990. O usuário jogou o Capítulo 1 e venceu. Logs registraram 5 EXP por comum, 25 por elite e 50 por boss. `EndGame` confirmou `passed=1`, 620 ouro, 257 gemas, 620 EXP, e `Local account level advanced level=2->3 exp=1990+620->610`. Readback do manager e gravação: 6120 moedas, 257 gemas, nível 3, 610 EXP. A home mostrou nível 3 e 610/3000 antes e depois de `am force-stop` + relançamento desse package QA. Nenhuma coleta HangUp ocorreu nessa medição.
- **APK normal:** `build/kinja-run-offline-lab-v150-combat-exp-account-level.apk`, package `com.habby.kinjarun.offline.audit1`, seeds normais, SHA-256 `6A0F32AA39A660BD51261BDDF894D34B8775AC290FC7D7A9E961BCF38BCFCD58`, assinatura de desenvolvimento verificada. **Gerado, não instalado.** O package principal e seu save não foram atualizados.

## Limites

Não houve partida perdida controlada, callback `KillEnemy` após `EndGame`, teste de modo avião nesta rodada, nem partida com a build normal. Nesses casos, a evidência é o fluxo do código e o teste isolado da diferença; não classificá-los como gameplay validado. A EXP por inimigo é aproximação offline aprovada, não valor comprovado do servidor original. Capturas e logs brutos em `build/tmp` são temporários e removidos pela limpeza final; esta síntese permanece versionada.
