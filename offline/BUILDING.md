# Build do adaptador sobre o APK original

> This file documents the original wrapper/build setup. Current maintenance status and later runtime changes are in [`../MAINTENANCE_GUIDE.md`](../MAINTENANCE_GUIDE.md). Historical statements below about validation apply only to that setup step, not to later APK builds.

Este diretório acrescenta uma Activity e uma biblioteca JNI ao cliente original. A Activity real do Unity continua em `classes3.dex`, e `libil2cpp.so`, `libunity.so` e os bundles originais permanecem intactos. O package da instalação de laboratório continua `com.habby.kinjarun.offline.lab`, assinado com o certificado DEV já usado em v1/v2.

## Contrato Java/JNI

`java/local/kinja/OfflineActivity.java` estende `com.unity3d.player.UnityPlayerActivity`. Seu inicializador estático carrega `libkinjaoffline.so`. Em `onCreate`, ela retém `AssetManager`, chama `configure(filesDir, AssetManager)` e então chama `super.onCreate`. A configuração ocorre antes de Unity começar sua inicialização.

O símbolo JNI correspondente é `Java_local_kinja_OfflineActivity_configure`, implementado em `native/offline_bridge.cpp`. A biblioteca também exporta `JNI_OnLoad`.

Desde v131, antes de configure/Unity.onCreate, o wrapper verifica uma eventual importação manual e sua recuperação, uma vez por processo. Falha incerta encerra antes de iniciar Unity. BackupStore contém codec/validação/transação e BackupCatalog as tabelas de compatibilidade geradas. A interface e o SAF ficam em OfflineActivity; preferências lógicas são lidas/aplicadas pelas APIs IL2CPP originais na thread Unity. Contrato e limites de validação em [MANUAL_BACKUP_V131.md](../docs/testing/MANUAL_BACKUP_V131.md).

A assinatura de `UnityPlayerActivity` em `compile-only/` serve apenas para o compilador Java. Ela não entra no DEX nem substitui a classe do jogo. O compilador usa o `android.jar` oficial API33. D8 recebe as classes reais do wrapper/backup e seus auxiliares; o stub fica apenas no classpath. `validate_wrapper.py` restringe as classes a essas famílias, verifica o superclass original, o loadLibrary, a referência estática ao AssetManager e a ordem configure→Unity.onCreate. A validação histórica de uma única classe abaixo precede o backup manual.

O DEX preparado/validado está em `build/offline-wrapper-setup/dex/classes.dex`, SHA-256 `cbb4731d4a5f995205144e42bf287a8432d2c999ff862ac52b93bdd909224e7c`. A futura injeção o nomeia `classes6.dex`; os cinco DEX originais são preservados.

## Compilar e empacotar

Na raiz do projeto, com o bridge já compilado pelo agente responsável:

```powershell
./offline/build.ps1 -BuildId v3 -NativeLibrary offline/build/arm64-v8a/libkinjaoffline.so -PackageApk
```

O comando gera `build/kinja-run-offline-lab-v3.unsigned.apk` e `build/kinja-run-offline-lab-v3.apk`. Para uma nova execução, selecione um `BuildId` novo; os arquivos existentes não são sobrescritos. Sem `-NativeLibrary`, o script compila `native/offline_bridge.cpp` com Clang NDKr27d para ARM64/API23 e libc++ estática.

Sem `-PackageApk`, o script prepara a biblioteca, o DEX, as substituições de manifest/resources/level0 e o plano em `build/offline-bridge-<BuildId>/build-plan.json`, sem gerar APK. Uma preparação também ocupa seu BuildId; escolha um ID novo na execução posterior que empacotar.

O script verifica os exports JNI e a arquitetura da biblioteca, valida os nove seeds obrigatórios e todos os demais `seed_*.json` disponíveis, compila/valida o wrapper, aplica o manifest MAIN `local.kinja.OfflineActivity`, conserva o namespace de recursos corrigido em v2 e conserva o byte de update check desabilitado em level0. Depois acrescenta:

- `classes6.dex`.
- `lib/arm64-v8a/libkinjaoffline.so`.
- Todos os `offline/assets/seed_*.json`, incluindo `seed_items.json`, `seed_equipment.json`, `seed_talents.json`, `seed_heroes.json`, `seed_regions.json`, `seed_guide.json`, `seed_player.json`, `seed_kill_enemy.json` e `seed_end_game.json`, como `assets/offline/...`.

O repacker exige o hash do APK original, aceita `--replace` para membros existentes e `--add` para membros novos, recusa colisões, preserva payloads originais e verifica hashes/CRC do resultado. Os membros novos usam timestamp do manifest original e ordem determinística. O alinhamento é aplicado por padding ZIP antes da assinatura e conferido pelo zipalign oficial; o modo de escrita nativo do zipalign apresentou access violation neste host.

A assinatura usa `build/offline-lab.jks` e o arquivo de senha local já existente, sem exibir a senha. V1/v2/v3 e alinhamento são verificados após a assinatura. O script não instala nem executa o APK. A instalação posterior pelo coordenador pode usar `adb install -r` para conservar os dados do mesmo package/certificado de laboratório.

## Validações já realizadas nesta etapa

- Compilação Java/D8 do wrapper com sucesso; uma classe, nenhum stub empacotado.
- Assinatura interna/checksum DEX e ordem dos métodos JNI/lifecycle conferidas.
- Sintaxe de `build.ps1` e `prepare_wrapper.ps1` conferida.
- Injector testado em ZIP sintético com DEX/biblioteca: conteúdo preservado, assinatura antiga removida, CRC e zipalign oficial conferidos. Relatório: `build/injector-fixture/validation-report.json`.
- Nenhum APK foi escrito ou instalado pelo agente que preparou este setup.

Ferramentas: JDK17 e D8/Build Tools37 em `tools/android/toolchain`; NDKr27d/Clang18.0.4 em `tools/android/native-toolchain`. Documentação primária D8: https://developer.android.com/tools/d8.
