# APK original: manifest e bootstrap Android

Análise estática do arquivo `D:\Programacao\kinja-run\kinja-run-1-9-1.apk`, SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. Nenhum APK foi executado ou alterado nesta investigação.

## Identidade confirmada pelo manifest binário

O parser portátil em `tools/android/inspect_android_static.py` leu o AXML diretamente do APK, sem JADX ou Apktool. O XML reconstruído está em `AndroidManifest.decoded.xml`; os offsets originais, tipos de cada valor e linhas AXML estão em `manifest-events.json`.

- Package: `com.habby.kinjarun`.
- Versão: `1.9.1`, versionCode `51`.
- minSdkVersion `23`, targetSdkVersion e compileSdkVersion `33`.
- Application: `com.ano.gshell.AnoApplication`.
- Activity principal: `com.unity3d.player.UnityPlayerActivity`, exported `true`, launchMode `2` (singleTask), orientação `1` (portrait).
- MAIN/LAUNCHER e LEANBACK_LAUNCHER apontam diretamente à Activity Unity; há também app links para `applink.kinjarun.com`.
- `extractNativeLibs=true`: bibliotecas nativas são extraídas na instalação.

As permissões e os providers completos estão em `manifest-summary.json`. Há INTERNET, ACCESS_NETWORK_STATE, ACCESS_WIFI_STATE, BILLING, AD_ID e SDKs de anúncios/analytics (Facebook, AppLovin, Google Ads, Firebase). A presença de permissões/providers não estabelece uma barreira obrigatória de login.

## DEX e fluxo observado

Há **cinco** arquivos DEX, todos versão 035, com 30.241 definições de classe somadas. O quinto contém classes da biblioteca Bolts. Isso corrige a contagem anterior de quatro DEX. Os SHA-256 por DEX estão em `identity.json`; o catálogo de classes está em `classes.json`.

`bootstrap.disassembly.txt` e `selected-classes.json` guardam instruções com offset de arquivo original, PC, palavras DEX e referências de método/campo/string recuperadas. Esta ferramenta resolve partes úteis do bytecode, mas não produz Java decompilado completo, nem reconstrói todo o tratamento de exceções ou reflexão.

| Evidência | Observação |
| --- | --- |
| `classes3.dex`, `UnityPlayerActivity.onCreate(Bundle)`, code item `0x4964b0`, instruções a partir de `0x4964c0` | Chama `Activity.onCreate`, lê argumento Intent `unity`, passa por `updateUnityCommandLineArguments`, instancia `UnityPlayer`, chama `setContentView` e `requestFocus`. Não há chamada a login, API HTTP ou disponibilidade de rede nessa implementação da Activity. |
| `classes3.dex`, `UnityPlayerActivity.updateUnityCommandLineArguments(String)`, code item `0x496460` | Retorna o próprio argumento. |
| `classes3.dex`, `UnityPlayer(Context,IUnityPlayerLifecycleEvents)`, code item `0x497360`, PC `0xb1` / offset `0x4974d2` | Chama `UnityPlayer.loadNative(String)`. |
| Mesmo construtor, PC `0x101` / offset `0x497572` | Chama `UnityPlayer.initJni(Context)`, método nativo sem corpo DEX. |
| `classes3.dex`, `UnityPlayer.loadNative(String)`, code item `0x497234` | Carrega `libmain.so`/`main` e chama o método nativo `NativeLoader.load`. |
| `classes.dex`, `AnoApplication.<clinit>()`, code item `0x567af0`, offset `0x567b04` | `System.loadLibrary("anort")`, correspondente à biblioteca `lib/arm64-v8a/libanort.so` de 1.625.944 bytes. |
| `classes.dex`, `AnoApplication.attachBaseContext(Context)`, code item `0x567b34`, PC `0x3f` / offset `0x567bc2` | Chama o método nativo `initialize(packageName, filesDir, nativeLibraryDir, sourceDir, splitSourceDirs, versionName)`. A biblioteca recebe o APK e o nome de pacote antes do bootstrap Unity. |
| `classes.dex`, `AnoApplication.onCreate()`, code item `0x567c0c` | Apenas chama `Application.onCreate`. |

**Inferência delimitada:** a barreira de autenticação do jogo não aparece na Activity Java de entrada. O lugar mais provável para a lógica de conta e início de partida é o gameplay Unity/IL2CPP. Os métodos nativos de AnoApplication e os providers de SDK ainda podem afetar o bootstrap; sua execução/validação não foi estabelecida.

`AnoApplication` declara `initialize`, `handleLoad`, `handleLoadV22` e `ioctl` nativos. Não foi determinado se `libanort.so` impõe integridade, ofuscação, certificados, licença ou alguma condição de rede. Retornar sem inicializá-la ou trocar a Application no manifest antes dessa investigação seria uma alteração sem evidência suficiente.

## Reempacotar preservando o jogo original

Um patch de `libil2cpp.so` ou dos bundles pode manter todos os outros DEX, recursos e bibliotecas originais; isso **não exige recompilar o jogo inteiro em Unity**. Os trechos que substituem estado/respostas de servidor ainda precisam ser recuperados antes de gerar uma edição realmente offline.

O APK original contém assinatura JAR v1 em `META-INF/BNDLTOOL.SF`, `META-INF/BNDLTOOL.RSA` e `META-INF/MANIFEST.MF`. Seu APK Signing Block começa em offset `0x1625a000`, ocupa 12.288 bytes e contém os IDs:

- `0x7109871a`: APK Signature Scheme v2, payload de 2.623 bytes.
- `0xf05368c0`: APK Signature Scheme v3, payload de 2.623 bytes.
- `0x6dff800d`: SourceStamp, payload de 3.593 bytes; identificado e verificado posteriormente com apksigner.
- `0x42726577`: padding, payload de 3.369 bytes.

Após preparar a toolchain portátil oficial, `apksigner verify --verbose --print-certs` confirmou v1/v2/v3 e SourceStamp no APK original (registro em `original-signature-verification.txt`). `zipalign -c -P 16 4` também concluiu com exit code 0. Alterar conteúdo invalida as assinaturas originais. Para produzir um APK instalável modificado será necessário:

1. Gerar um APK novo a partir dos membros originais com substituições explícitas e remover a assinatura v1 antiga; não reutilizar os blocos v2/v3 antigos.
2. Manter o manifest/resources intactos quando o patch só precisar mudar IL2CPP. Caso seja necessário editar o manifest, usar um editor AXML ou Apktool/aapt2; este parser é somente leitura.
3. Alinhar o ZIP com `zipalign`, assinar com uma chave local usando `apksigner` e validar a assinatura final. Android 6/minSdk23 precisa da assinatura v1 se esse Android continuar suportado; v2/v3 cobrem versões posteriores.
4. Instalar por `adb` em dispositivo/emulador ARM64 e observar o bootstrap com logcat. A nova chave não atualiza uma instalação existente assinada pelo produtor. Alterar o nome de pacote exige investigar também authorities, recursos e a shell nativa.
5. Validar que o jogo entra na fase, termina a fase e conserva progresso sem conexão. Remover INTERNET sozinho apenas bloqueia rede; não fornece o perfil/configuração que o cliente espera.

## Ferramentas do host

Python disponível: `C:\Python314\python.exe` (executa o parser; exibe uma mensagem de localização mas conclui). Nenhuma biblioteca de terceiros é necessária para os arquivos gerados.

Na busca inicial não foram encontrados `java`, `keytool`, `jarsigner`, `adb`, `zipalign` ou `apksigner` no PATH ou em localizações comuns. A etapa seguinte preparou Temurin JDK17.0.20.1+1 e Android Build Tools37.0.0/Platform Tools37.0.1 oficialmente em `tools/android/toolchain`; hashes, fontes e comandos estão no README desse diretório. Java, apksigner e zipalign funcionam, sem configuração global. O adb Windows tenta criar o diretório de usuário `.android` mesmo na consulta de versão e foi bloqueado pelo sandbox; não houve instalação/execução do APK nem criação de chave. JADX continua útil para análise de Java, mas não é um pré-requisito para o patch nativo nem para este inventário.

## Reprodução

```powershell
python tools/android/inspect_android_static.py kinja-run-1-9-1.apk --output analysis/android --class-pattern com/ano/gshell --class-pattern UnityPlayerActivity --class-pattern HabbyBilling --class-pattern 'Lcom/unity3d/player/UnityPlayer;' --class-pattern com/unity3d/player/NativeLoader --class-pattern com/unity3d/player/NetworkConnectivity
```

O comando só lê o APK original e escreve os relatórios no diretório indicado.
