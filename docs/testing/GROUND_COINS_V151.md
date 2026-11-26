# Moedas coletadas do chão — crédito offline (código pronto, sem build)

## Relato

O usuário pega moedas no chão durante a fase, mas elas não entram no saldo de moedas.

## Causa encontrada no código

O bridge credita só o ouro de inimigos derrotados (`KillEnemy`: 5/25/50, acumulado em `run_gold_payout`). Ele é liquidado só em vitória, e `EndGame.Gold` mostra esse mesmo total. Nada lia as moedas coletadas na cena. O cliente 1.9.1 tem o contador próprio `FightDataManager.CoinCount`, separado de `FightRecordManager.GainGold`, que acumula as respostas `KillEnemy`. O pedido original `C2G_EndGame` envia só `Score/Stage/BudCount/PerfectSkillCount/SkillCount`. O servidor devolvia `Gold`, e a fórmula dele não foi recuperada. Offline, ninguém convertia a coleta em saldo.

## Correção

- `read_fight_coin_count` lê `FightDataManager.CoinCount` por nome e tipo via IL2CPP: `Int32`, `Int64`, `Single`, `Double` ou `ObscuredInt` (este decifrado por `GetDecrypted` original). Sem offsets e sem hook novo.
- `EnterFight` guarda o valor inicial. Na vitória, o crédito é `fim - início`. Se o contador foi zerado depois do `EnterFight`, o crédito é o `fim`.
- O valor entra em `run_gold_payout`. Assim aparece no `Gold` da tela de resultado e é creditado uma vez pelo acerto da run (`settle_completed_run_currency`), com o mesmo save e readback do ouro de combate.
- A regra é a mesma do ouro de combate: **só em vitória**. Derrota ou abandono não creditam.
- Um retry de `EndGame` não soma duas vezes. Valores negativos ou acima de 1.000.000 não são creditados.
- Log de conferência: `Ground coins counter start=… end=… credited=… killGold=… buds=…`.

## Hipótese a confirmar em QA

Supõe-se que `CoinCount` conta só as moedas do chão. Se ele também incluir o ouro dos inimigos, o crédito duplicaria esse ouro. Teste: numa fase, compare `credited` com o contador de moedas do HUD e com `killGold`. Se `credited` ≈ HUD − `killGold`, a hipótese está certa. Se `credited` ≈ HUD, é preciso subtrair `killGold`. O campo `buds` (`BudCount`, enviado pelo cliente original) aparece no mesmo log para comparação.

## Validação

`clang++ -fsyntax-only` do bridge não trouxe erros novos. **Não validado:** NDK/APK, tipo real de `CoinCount`, valores no HUD/resultado, saldo após reinício.

## Build normal v151

Incluído em `v151-chapter-rewards-fragments-coins` (audit1, APK SHA-256 `944BB564…2F7958`). Compilado com NDK e assinado com o certificado de laboratório; **não instalado nem testado no jogo**. Ver `MAINTENANCE_GUIDE.md`, seção "Build normal v151". Os itens "Não validado" acima continuam pendentes, exceto compilação ARM64 e empacotamento/assinatura.
