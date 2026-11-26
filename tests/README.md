# Testes

Todos de uma vez:

```powershell
./tests/run-host-tests.ps1                 # Java executa; C++ só compila se não houver g++/clang++ no PATH
./tests/run-host-tests.ps1 -Serial <adb>   # executa também os C++ no BlueStacks, sem instalar APK
```

A saída vai para `build/tests/host-<data>/`, que `clean-builds.ps1` remove depois. O resultado de cada teste é **PASSOU**, **FALHOU**, **COMPILADO** (compilou e não foi executado) ou **IGNORADO** (falta dependência do acervo).

| Teste | Cobre | Observação |
|---|---|---|
| `backup/BackupStoreTest.java` | Codec, validação, transação e recuperação do backup manual | Usa o JDK do toolchain. `backup/run.ps1` roda sozinho; `backup/run-android.ps1` roda no ART do emulador. |
| `chapter_rewards_test.cpp` | `offline/native/chapter_rewards.h` | |
| `hero_fragments_test.cpp` | `offline/native/hero_fragments.h` | |
| `combat_exp_test.cpp` | `offline/native/combat_exp.h` | |
| `account_progression_test.cpp` | `offline/native/account_progression.h` | Gera a fixture a partir da tabela `AccountLevelCfg` do acervo. `run-account-progression.ps1 -Serial` é o script antigo, só deste teste. |

## O que não é coberto

O `offline_bridge.cpp` em si (hooks, chamadas IL2CPP e edição dos JSON dos saves) não tem teste de host. Ele só é validado no emulador ou no aparelho. Por isso, lógica nova que puder ser separada do runtime vai num header puro em `offline/native/` com um `tests/<nome>_test.cpp`, que o runner descobre sozinho pelo sufixo `_test.cpp`.

## Escrevendo um teste C++

- Só biblioteca padrão; `assert` para as verificações (o runner não define `NDEBUG`).
- Retornar 0 em sucesso. Imprimir uma linha de resumo ajuda no log.
- Incluir o header com caminho relativo: `#include "../offline/native/<nome>.h"`.
