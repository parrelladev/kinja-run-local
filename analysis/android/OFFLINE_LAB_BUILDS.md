# APKs de diagnóstico do Kinja Run original

O APK de origem permanece com SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. Os builds preservam os cinco DEX, as 23 bibliotecas nativas originais e os bundles do jogo. As cópias de bibliotecas descriptografadas usadas na análise não foram incorporadas aos APKs.

## Variante v1

Artefato: `build/kinja-run-offline-lab.apk`, SHA-256 `3b482061863adc911e55db249ff729f8eadcd79e15e2f32a76b1f9f5ff65151c`.

Mudanças de payload: `AndroidManifest.xml` e `assets/bin/Data/level0`. O manifest usa package `com.habby.kinjarun.offline.lab` e label `Kinja Run Offline Lab`; as nove authorities foram renomeadas, incluindo a authority Facebook por app-ID, evitando colisões com a instalação original. Custom permission e host `cct` foram ajustados. Strings foram anexadas ao pool AXML mantendo os índices anteriores e resource map. Em `level0`, a única mudança é byte `4980`, `IsCheckUpdate` de `1` para `0`, com tamanho de 5.036 bytes preservado.

Assinaturas v1/v2/v3 e alinhamento foram verificados. A instalação e captura de runtime foram feitas pelo agente principal. O primeiro build falhou no bootstrap Java porque o nome do package em `resources.arsc` continuava sendo `com.habby.kinjarun`.

## Causa observada no aparelho

`analysis/runtime/lab-crash.log`, linhas 93 e 108–112, registra `Resources$NotFoundException: String resource ID #0x0` em `UnityPlayer.GetGlViewContentDescription`. O bytecode original em `analysis/android/bootstrap.disassembly.txt`, linhas 495–508 (`classes3.dex`, code item `0x496fe8`), mostra exatamente esta sequência:

1. `Context.getPackageName()`.
2. `Resources.getIdentifier("game_view_content_description", "string", packageName)`.
3. `Resources.getString(id)`.

O manifest v1 retorna o package novo; o resource table ainda declarava o antigo. Portanto, o lookup por nome retornou zero, utilizado pela chamada seguinte a `getString`. A exceção e o caminho de bytecode corroboram a causa da falha.

O mesmo log registra carregamento bem-sucedido de `libanort.so` na linha 27 e `IL2CPP: JNI_OnLoad` na linha 73, antes da exceção Java. A nova assinatura/pacote permitiram chegar a esse ponto do bootstrap. Ainda não foi estabelecida a ausência de verificações nativas posteriores.

## Variante v2

Artefato: `build/kinja-run-offline-lab-v2.apk`, SHA-256 `e94d96b64bc41030afdfbc02f0dbf2eb844db2ddf77c879f13fa51fd11d9c945`, tamanho 375.380.894 bytes.

A única alteração adicional de payload versus v1 está em `resources.arsc`: nome fixo de 128 unidades UTF-16 do `RES_TABLE_PACKAGE`, a partir de offset `0x64184`. O nome foi alterado para `com.habby.kinjarun.offline.lab`; 12 bytes anteriormente zero passam a conter o sufixo. Package ID `0x7f`, tamanho de 1.014.736 bytes, todos os IDs, offsets, chunks, strings e valores de recursos permanecem preservados.

O parser formato-aware está em `tools/android/patch_resource_package.py`; diff e hashes em `build/offline-lab-patch/resources.arsc.patch.json`. AAPT oficial aceitou o resource table e confirmou o recurso `com.habby.kinjarun.offline.lab:string/game_view_content_description` com o ID original `0x7f1200c1`. Dump completo: `build/kinja-run-offline-lab-v2.resources.txt`.

O build usa o mesmo certificado DEV de v1, SHA-256 `7508ca8dd3ad4914f42b7d6dbd537c564a78925fb8f41b76a905324220a75b3f`, permitindo atualizar a instalação de laboratório preservando seus dados. O keystore e sua senha ficam somente nos arquivos locais de build; o valor da senha não foi exibido.

Verificações concluídas: assinaturas v1/v2/v3, `zipalign -c -P 16 4`, AAPT resource dump e comparação dos payloads original/v1/v2. Dos membros originais de payload, 3.587 estão byte-identical; somente manifest, level0 e nome de package do resource table diferem. Relatório completo: `build/offline-lab-v2-build-report.json`.

Este agente não instalou v2. A eliminação da exceção no aparelho deve ser confirmada pelo agente principal na instalação/execução seguinte. Desabilitar o update check ainda não estabelece funcionamento offline de login, perfil ou início da partida.

## Comandos utilizados

```powershell
python tools/android/patch_resource_package.py --input kinja-run-1-9-1.apk --output build/offline-lab-patch/resources.arsc

python tools/android/repack_apk.py --input kinja-run-1-9-1.apk --output build/kinja-run-offline-lab-v2.unsigned.apk --align 4 --replace 'AndroidManifest.xml=build/offline-lab-patch/AndroidManifest.xml' --replace 'assets/bin/Data/level0=build/offline-lab-patch/level0' --replace 'resources.arsc=build/offline-lab-patch/resources.arsc'

./tools/android/sign_apk.ps1 -InputApk build/kinja-run-offline-lab-v2.unsigned.apk -OutputApk build/kinja-run-offline-lab-v2.apk -Keystore build/offline-lab.jks -Alias offline -StorePasswordFile build/offline-lab.keystore-password.txt
```

Os scripts exigem saídas novas; não sobrescrevem o APK original ou as versões anteriores.
