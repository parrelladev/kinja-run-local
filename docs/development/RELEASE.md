# Build normal: passo a passo

A **build normal** é a que o usuário instala no próprio celular. Ela usa o package do perfil dele (hoje `com.habby.kinjarun.offline.audit1`), seeds normais, nenhuma opção QA e o keystore de laboratório, para atualizar sem perder o save. Builds QA usam packages `*.qa.*` e não seguem este roteiro.

## 1. Antes

```powershell
git checkout main
git pull
```

Confira no `git log` que os commits esperados estão na main. O script não faz pull.

## 2. Conferir sem compilar

```powershell
./tools/workspace/new-normal-build.ps1 -BuildId v152-descricao-curta -DryRun
```

Confere: main limpa, `config/local.json` (APK original com hash 1.9.1, NDK, toolchain, keystore e senha), package da última build normal no histórico, BuildId novo (histórico, `build/` e acervo) e testes de host.

- BuildId: `vNNN-descricao` em minúsculas, com o próximo número livre em `docs/development/build-history.json`.
- Trocar de package perde o save do usuário. O script recusa, a menos que se passe `-AllowPackageChange` com autorização explícita.
- Testes C++ aparecem como **COMPILADO** quando não há compilador de host. Isso não é falha, mas também não é aprovação. Para executá-los sem instalar nada, abra o BlueStacks e passe `-TestSerial <dispositivo adb>`: o binário de teste vai para `/data/local/tmp`, sem instalar APK nem tocar em saves.

## 3. Gerar

```powershell
./tools/workspace/new-normal-build.ps1 -BuildId v152-descricao-curta
```

Depois das conferências, o script compila (`build-native.ps1`, sem `-Qa*`) e empacota (`offline/build.ps1 -PackageApk`). Em seguida confere três coisas: o certificado (SHA-256 `7508ca8d…a75b3f`), a `.so` dentro do APK igual à compilada e os seeds do APK iguais a `offline/assets`. Por fim, acrescenta a entrada em `build-history.json` e grava `build/kinja-run-offline-lab-<BuildId>.apk.release-summary.json`.

Avisos esperados:

- `warning: unused parameter 'equipment_rewards'` do compilador (pendência de limpeza conhecida).
- 69 linhas `WARNING: META-INF/... not protected by signature` do apksigner. Vêm das bibliotecas do APK original.

## 4. Registrar

1. Atualizar no [guia](../../MAINTENANCE_GUIDE.md) a seção "Build normal" e a linha de cada sistema alterado: o que entrou, os hashes, os testes e o nível de evidência. "Compilada e assinada" não é "testada".
2. Commit local com a validação na mensagem. O usuário faz o push.

## 5. Depois dos testes do usuário

Somente quando o usuário disser que terminou de testar:

```powershell
./tools/workspace/clean-builds.ps1 -KeepBuildId v152-descricao-curta -WhatIf
./tools/workspace/clean-builds.ps1 -KeepBuildId v152-descricao-curta
```

Mantém em `build/` só o APK e o relatório de assinatura da build indicada. O script exige o nome original `kinja-run-offline-lab-<BuildId>.apk`: se o arquivo tiver sido renomeado, volte o nome antes.

## Sem o script

Os mesmos passos, à mão:

```powershell
./tests/run-host-tests.ps1
./offline/native/build-native.ps1
./offline/build.ps1 -BuildId <novo> -PackageName com.habby.kinjarun.offline.audit1 -NativeLibrary ./offline/build/arm64-v8a/libkinjaoffline.so -PackageApk
```

Depois, conferir o certificado no `.signature-verification.txt`, calcular SHA-256 do APK e da `.so` e acrescentar a entrada no histórico com `Write-KinjaBuildHistory` (`tools/workspace/build-history.ps1`).
