# Nível da conta — v112, 03/11/2026

**Atualização posterior:** os testes com saves artificiais autorizados validaram migração, crédito com passagem de nível e reinício, mas encontraram uma leitura incorreta de ObscuredInt na v112. Corrigida na **v116 normal**, que substitui a v112. [Resultados e artefatos QA](ACCOUNT_LEVEL_QA_V116.md). As pendências abaixo descrevem o estado anterior a esses testes.

## Evidência e causa

Imagem fornecida pelo usuário da v111 no celular: Capítulo 6, nível 2 e barra **5838/2000 EXP**. O anexo é uma captura, não um arquivo de save acessível ao agente. Nenhum dado privado do celular foi lido, copiado ou modificado.

`grant_local_currency_reward` aplicava `PlayerExp` e persistia `Exp`, sem calcular a passagem de nível nem atualizar `PlayerLevel`/`Level`. Isso permite acumular EXP acima do limite da barra. A reprodução isolada usa os números observados na imagem e a tabela já recuperada do APK 1.9.1, sem repetir engenharia reversa. `AccountLevelCfg` tem 200 linhas: nível 1 custa 1250, nível 2 custa 2000, nível 3 custa 3000. A EXP é tratada como saldo restante no nível salvo, conforme o custo por nível exibido na UI; não é convertida em EXP vitalícia.

## Alteração

O planejador `offline/native/account_progression.h` desconta cada custo alcançado e conserva o excedente. Exemplo: nível 2/5838 → nível 4/838. No último nível configurado, conserva o saldo, sem ultrapassar o limite. Dados inválidos são recusados. Níveis legados já concedidos não são rebaixados.

O bridge consulta a tabela original por `AccountLevelCfg.GetAllCfg/GetCfg`, validando continuidade, chaves e custos com APIs IL2CPP. Antes de desserializar o jogador no login, normaliza somente `Level` e `Exp` no save existente por substituição atômica; o processo é idempotente e não concede EXP adicional.

Novos créditos positivos de EXP calculam nível e saldo antes de mutar os managers. Comparam moeda/EXP/nível live e salvos antes da alteração; usam `ObscuredInt.op_Implicit`, setters originais de `PlayerDataManager`, `SyncValueToShow` e readback, na thread Unity. Persistem `Coin`, `Diamond`, `Exp` e `Level` no mesmo JSON, preservando os outros campos. Falha de setter/readback/gravação tenta restaurar os valores vivos; falha de rollback é registrada. Após uma subida de nível, atualizam a lista local de menus. Falha nesse refresh não transforma uma recompensa já gravada em resgate repetível.

Esta correção não cria fórmula de EXP por combate, não acrescenta recompensas de level-up, não altera preços/itens, nem trata os bugs de reciclagem, heróis ou tutorial. O mínimo histórico de nível 2 da reparação do Capítulo 2 permanece como compatibilidade anterior; não é a nova regra de EXP. Falhas preexistentes do ledger HangUp após conceder moeda não foram alteradas aqui.

## Validação

- `tests/run-account-progression.ps1 -Serial 127.0.0.1:6610`: compila o planejador real como executável Android x86_64 e roda no BlueStacks, com custos lidos da tabela externa existente. Passaram o caso da imagem, idempotência, os 199 limites, crédito novo atravessando limite, nível legado, teto, negativos/overflow e dados inválidos. Executável isolado em `/data/local/tmp`, sem acesso aos saves.
- Bridge ARM64 compilado com NDK existente, sem flags QA. Warning preexistente de `equipment_rewards` sem uso. Contrato wrapper/JNI/D8, hash do APK original e assinatura aprovados.
- v112 instalada por `adb -s 127.0.0.1:6610 install -r` somente no `com.habby.kinjarun.offline.audit1`, com o mesmo certificado. `firstInstallTime=2026-11-03 13:45:26` preservado; `lastUpdateTime=2026-11-03 14:07:25` conforme relógio Android.
- PID5824: 200 consultas de configuração aceitas, bootstrap `initialized`, home nível 1/1500 ouro/0 gemas/0 EXP e inventário vazio preservados. Não houve seeding, partida, coleta ou compra para fabricar uma passagem de nível.
- Reinício PID5824 → PID6528: initialized, mesmos saldos, nível 1 e menus bloqueados; home novamente capturada. Isso verifica recarga da baseline, não persistência de um nível elevado.
- **Limite:** caso avançado validado no planejador, não no save privado do celular. Migração de save com excesso, setters de nível numa concessão positiva, mudança visual/desbloqueio e persistência de nível elevado ainda precisam de execução real. Não declarar gameplay completo aprovado.

Evidências locais ignoradas pelo Git: `build/tests/account-progression/` e `build/tests/v112-account-progression/`. Fonte e testes ficam versionados; acervo externo preservado. Rede do emulador não foi alterada.

## Artefato

BuildId `v112-account-exp-level-progression`, package `com.habby.kinjarun.offline.audit1`.

- APK: `build/kinja-run-offline-lab-v112-account-exp-level-progression.apk`, SHA256 `A2978E847596B31A7691D5E954D49D328A3A7B81383F814FAF051FB6752AC2B1`.
- Biblioteca ARM64: SHA256 `A1F278823CADD42DD86C84D10FAC9A471A05EA7341181DDAF8ACC8E0B422754D`.

Antes de qualquer atualização no celular, conferir package/certificado do perfil correspondente. Não desinstalar nem limpar dados. Aguardar avaliação do usuário sobre este bug antes de passar ao próximo.
