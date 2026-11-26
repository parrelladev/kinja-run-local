# Bridge nativo offline

`offline_bridge.cpp` é o adaptador JNI ARM64 que redireciona métodos do cliente Unity 1.9.1 original em memória, depois de conferir o prólogo de cada um. Os headers `*.h` contêm a lógica separada; os puros têm testes em `tests/`.

- Estado atual, tabela dos hooks, saves, sistemas, build e testes: [`../../MAINTENANCE_GUIDE.md`](../../MAINTENANCE_GUIDE.md).
- Compilação: `./offline/native/build-native.ps1`, na raiz do projeto. As opções `-Qa*` são só para packages QA.
- O conteúdo anterior deste README (notas de v31 a v109, em inglês) está em [`../../docs/history/NATIVE_BRIDGE_LOG.md`](../../docs/history/NATIVE_BRIDGE_LOG.md).
