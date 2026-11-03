# Testes no BlueStacks

BlueStacks é o ambiente pretendido pelo usuário para evitar conectar o celular. **Atualização por relato do usuário em 03/11/2026:** APK v111 executou normalmente no BlueStacks. Não houve observação instrumental por este agente; versão/instância, Android/API, ABI, endpoint ADB, modo offline, persistência e fluxos específicos de gameplay continuam sem registro de teste. A migração não iniciou nem configurou o emulador.

## Antes de instalar

1. Identificar a instância correta no BlueStacks, versão, Android/API, resolução e configuração de depuração Android.
2. Descobrir o endpoint ADB real da instância; não assumir porta padrão. O caminho do ADB pode vir de `toolchainDirectory/paths.json`.
3. Após conexão autorizada ao endpoint confirmado, registrar `adb devices -l`. Usar sempre `adb -s <serial-confirmado>` nas ações seguintes.
4. Observar `shell getprop ro.product.cpu.abilist` e `shell getprop ro.build.version.sdk`. O APK e bridge são ARM64; ABI anunciada não comprova compatibilidade dos hooks em tradução de instruções.
5. Registrar SHA256 do APK, package e perfil antes da instalação. Não copiar save do telefone nem usar `pm clear`, desinstalação ou reset como setup.

## Matriz inicial

| Verificação | BlueStacks | Evidência necessária |
|---|---|---|
| Instância / API / ABI / serial | Não testado | Registro do ambiente |
| Instalação/abertura da v111 | Funcionamento relatado pelo usuário | Hash/package/instância não reconferidos por ADB |
| Cold start sem rede e home | Não testado | Logs de hooks, prólogos conferidos e captura |
| Restart e preservação | Não testado | Saldos/inventário antes e depois |
| Menus após vitória v111 | Não testado | Vitória real e abertura na mesma sessão |
| Combate e persistência | Não testado | Recompensas + restart, recursos naturais |
| Transações / fusão coletiva | Não testado | Plano específico sem consumir itens reais implicitamente |

Os resultados do telefone continuam em `FUNCTIONAL_AUDIT.md` e nos relatórios existentes; não transferir status aprovado para o emulador. Validar prólogos/hash e manter chamadas IL2CPP na thread Unity. Se houver tradução ARM, confirmar o caminho executado em vez de supor equivalência por versão Android.

Preferir perfil natural isolado `com.habby.kinjarun.offline.audit1`, sem flags QA, após confirmar ausência ou estado prévio desse package na instância. Preservar `lab6` e o mesmo keystore em atualizações. Cada execução futura deve registrar ambiente, APK/biblioteca, serial, baseline, ações, evidências e nível de validação. Logs/capturas novos vão a `build/tests/<id-unico>/`, ignorado pelo Git; sínteses revisadas vão a `docs/testing/`.
