# Análise estática do APK 1.9.1

Relatórios produzidos a partir do APK original (`kinja-run-1-9-1.apk`, SHA-256 `7953eb36…c20595e`). Só os `.md` são versionados. As extrações grandes que eles citam (`dump.cs`, `libil2cpp.stripes.so`, tabelas `.ht`, `game-method-map.json`) ficam no acervo externo configurado em `config/local.json` (`archiveRoot`).

Os relatórios registram o que foi entendido na época. Para o estado atual do bridge, use o [guia](../MAINTENANCE_GUIDE.md).

| Pasta | Relatório | Conteúdo |
|---|---|---|
| `android/` | [ANDROID_FINDINGS](android/ANDROID_FINDINGS.md) | Manifest e bootstrap Android do APK original |
| `android/` | [OFFLINE_LAB_BUILDS](android/OFFLINE_LAB_BUILDS.md) | Primeiros APKs de diagnóstico (histórico, anterior ao bridge) |
| `bundles/` | [FINDINGS](bundles/FINDINGS.md) | Conteúdo Unity e evidência de bootstrap |
| `bundles/` | [offline-bootstrap](bundles/offline-bootstrap.md) | Protocolo local e bootstrap offline do cliente |
| `metadata/` | [RECOVERY](metadata/RECOVERY.md) | Como a metadata IL2CPP e o código nativo protegido foram recuperados (XOR e permutação). **Leia antes de usar endereços.** |
| `metadata/` | [OFFLINE_PLAYER_BOOTSTRAP](metadata/OFFLINE_PLAYER_BOOTSTRAP.md) | Bootstrap do jogador e pontos de adaptação |
| `metadata/` | [OFFLINE_FIGHT_GATES](metadata/OFFLINE_FIGHT_GATES.md) | Portões de entrada e fim de combate |
| `metadata/` | [startup-types](metadata/startup-types.md) | Índice de tipos de inicialização e serviços (nomes, sem endereços) |
| `native/` | [FUSION_1_9_1_V102](native/FUSION_1_9_1_V102.md) | Fusão individual: contrato 1.9.1 e correção |
| `native/` | [ONEKEY_1_9_1_INVESTIGATION](native/ONEKEY_1_9_1_INVESTIGATION.md) | "Unir tudo": contrato original (antes da política local) |
| `native/` | [ONEKEY_OFFLINE_V107](native/ONEKEY_OFFLINE_V107.md) | "Unir tudo": política local autorizada |
| `native/` | [OFFLINE_READINESS_AUDIT](native/OFFLINE_READINESS_AUDIT.md) | Auditoria de conta nova no audit1 (checkpoint v110/v111) |

Ferramentas que geraram essas análises: [`../tools/README.md`](../tools/README.md).
