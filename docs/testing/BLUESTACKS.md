# Testes no BlueStacks

BlueStacks é o ambiente pretendido pelo usuário para evitar conectar o celular. **Atualização por relato do usuário em 03/11/2026:** APK v111 executou normalmente no BlueStacks. Não houve observação instrumental por este agente; versão/instância, Android/API, ABI, endpoint ADB, modo offline, persistência e fluxos específicos de gameplay continuam sem registro de teste. A migração não iniciou nem configurou o emulador.

## Antes de instalar

### Retomada em 03/11/2026

Leitura das instruções e do checkpoint de prontidão retomada no checkout leve. O ADB configurado em `config/local.json` iniciou com sucesso; `adb devices -l` retornou uma lista vazia. Não foi identificado serial/endpoint do BlueStacks nesta coleta. Nenhum APK foi instalado, nenhum app foi iniciado e nenhum save ou arquivo do acervo externo foi alterado. A execução da v111 continua confirmada pelo relato do usuário, sem nova validação instrumental.

Para continuar, abrir a instância usada e obter o endpoint exibido na configuração de depuração Android/ADB. Após conectar ao endpoint confirmado, coletar ambiente e identidade do pacote instalado antes de qualquer atualização. A baseline do telefone não é a baseline do BlueStacks: observar primeiro os saldos/progresso locais. Prioridade: abertura sem rede, reinício com preservação e, depois, refresh de menus após vitória na mesma sessão. Reutilizar os relatórios e artefatos existentes, sem repetir engenharia reversa.

Inspeção posterior da interface: BlueStacks Desktop **16.0.0.34, Android 16 (Alpha)**; instalação `C:\Program Files\BlueStacks_polar`, dados `D:\Softwares\BlueStacks_polar`. Configurações → Avançado anuncia ABI x86 32-bit/x86 64-bit/ARM 64-bit e endpoint **127.0.0.1:6610**, com ADB desligado. Visualização selecionada: retrato 900×1600, 240 DPI. Launcher contém “Kinja Run Offline Lab”; identidade/hash do APK ainda não conferidos. Tela deixada em Avançado para o usuário ativar o ADB; nenhuma configuração foi alterada pelo agente. ABI anunciada não valida hooks ARM64 em execução.

1. Identificar a instância correta no BlueStacks, versão, Android/API, resolução e configuração de depuração Android.
2. Descobrir o endpoint ADB real da instância; não assumir porta padrão. O caminho do ADB pode vir de `toolchainDirectory/paths.json`.
3. Após conexão autorizada ao endpoint confirmado, registrar `adb devices -l`. Usar sempre `adb -s <serial-confirmado>` nas ações seguintes.
4. Observar `shell getprop ro.product.cpu.abilist` e `shell getprop ro.build.version.sdk`. O APK e bridge são ARM64; ABI anunciada não comprova compatibilidade dos hooks em tradução de instruções.
5. Registrar SHA256 do APK, package e perfil antes da instalação. Não copiar save do telefone nem usar `pm clear`, desinstalação ou reset como setup.

## Matriz inicial

### Coleta instrumental após habilitação manual do ADB — 03/11/2026

Conexão `127.0.0.1:6610` aprovada pelo dispositivo. Android 16/API36, ABI `x86_64,x86,arm64-v8a`, resolução física 900×1600. Único package Kinja encontrado: `com.habby.kinjarun.offline.audit1`, primaryCpuAbi arm64-v8a, versão1.9.1/code51, firstInstallTime/lastUpdateTime `2026-11-03 13:45:26` conforme relógio do Android. SHA256 do APK instalado `7456C7566259F69D5F9808B89055B639F25E385067F5A051B4A55EDFA80EA56D`: corresponde à v111 histórica. Nenhuma instalação nesta coleta.

Abertura PID3724: hooks instalados, callbacks na thread Unity, `stage=initialized` e home capturada. Baseline própria do emulador: nível1, ouro1500, gemas0, EXP0, inventário0, capítulo1 e `functionIds=[]`. Reinício somente do processo, PID4487: bootstrap initialized, mesmos valores e inventário0 nos logs. Não transportar os saldos/progresso do telefone para esta conta. Sem partida, compra, coleta ou fusão nesta rodada; dry-run coletivo com zero passos, sem consumo. Evidências locais em `build/tests/bluestacks-v111-20261103-baseline/` (package, logs, capturas).

Rede original observada: avião0/Wi-Fi0/dados1; rede padrão100 validada por Ethernet virtual `eth0`. Portanto, Wi-Fi desligado não comprova isolamento no BlueStacks. Nenhuma configuração de rede foi alterada; cold start sem rede continua pendente. Timer de energia excessivo aparece na home. Logs anteriores ao PID3724 contêm gameplay/exceções de outra sessão: não atribuir essas ações a esta coleta. Refresh pós-vitória, recompensas e persistência de uma transação continuam sem validação no emulador.

| Verificação | BlueStacks | Evidência necessária |
|---|---|---|
| Instância / API / ABI / serial | Observados por UI e ADB | Coleta instrumental acima |
| Instalação/abertura da v111 | APK instalado identificado; abertura observada | Hash v111, initialized e captura da home; instalação feita pelo usuário |
| Cold start sem rede e home | Não testado | Logs de hooks, prólogos conferidos e captura |
| Restart e preservação | Baseline preservada nos logs | PID3724→4487,1500/0/0 e inventário0; transações ainda pendentes |
| Menus após vitória v111 | Não testado | Vitória real e abertura na mesma sessão |
| Combate e persistência | Não testado | Recompensas + restart, recursos naturais |
| Transações / fusão coletiva | Não testado | Plano específico sem consumir itens reais implicitamente |

Os resultados do telefone continuam em `FUNCTIONAL_AUDIT.md` e nos relatórios existentes; não transferir status aprovado para o emulador. Validar prólogos/hash e manter chamadas IL2CPP na thread Unity. Se houver tradução ARM, confirmar o caminho executado em vez de supor equivalência por versão Android.

Preferir perfil natural isolado `com.habby.kinjarun.offline.audit1`, sem flags QA, após confirmar ausência ou estado prévio desse package na instância. Preservar `lab6` e o mesmo keystore em atualizações. Cada execução futura deve registrar ambiente, APK/biblioteca, serial, baseline, ações, evidências e nível de validação. Logs/capturas novos vão a `build/tests/<id-unico>/`, ignorado pelo Git; sínteses revisadas vão a `docs/testing/`.
