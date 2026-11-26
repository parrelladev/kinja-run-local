# Ferramentas

Scripts próprios do projeto. Binários de terceiros (JDK, build-tools, NDK, ADB, UnityPy, Il2CppDumper) não são versionados: ficam no acervo externo, resolvidos por `config/local.json`.

## Uso diário

| Script | Para quê |
|---|---|
| `workspace/new-normal-build.ps1` | Gera uma build normal completa com conferências: main, config, package, testes, compilação, empacotamento, certificado, hashes e histórico. Ver [RELEASE.md](../docs/development/RELEASE.md). |
| `workspace/clean-builds.ps1` | Depois que o usuário terminar os testes: mantém em `build/` só o APK normal atual e o relatório de assinatura. `-WhatIf` mostra sem apagar. |
| `workspace/paths.ps1` | Resolve os caminhos de `config/local.json`. Não lê chave nem senha. |
| `workspace/check-doc-links.py` | Confere os links relativos de todos os `.md` versionados. |

## Usados pelo empacotamento (`offline/build.ps1`)

| Script | Para quê |
|---|---|
| `android/prepare_offline_lab.py` | Prepara manifest e `level0` do wrapper sobre o APK original. |
| `android/patch_resource_package.py` | Troca o nome do package em `resources.arsc`, preservando IDs. |
| `android/repack_apk.py` | Reempacota o APK original com substituições e adições explícitas, conferindo hash e CRC. |
| `android/sign_apk.ps1` | Alinha, assina com o keystore de laboratório e grava o relatório de verificação. |

## Análise estática (histórico, uso pontual)

Ferramentas que produziram os relatórios de [`analysis/`](../analysis/README.md). Muitas exigem extrações locais do acervo e pacotes Python específicos. **Não rodar só para "preencher" pastas vazias** nem refazer engenharia reversa já registrada.

| Pasta | Scripts | Uso |
|---|---|---|
| `android/` | `inspect_android_static.py` | Inspeção de AXML/DEX do APK original. |
| `bundles/` | `extract_bundles.py`, `find_bootstrap_assets.py`, `summarize_content.py`, `trace_bootstrap.py`, `nrbf_reader.py`, `inspect_backup_ui.py` | Bundles Unity, tabelas `.ht` e prefabs de UI. |
| `metadata/` | `recover_metadata.py`, `recover_native_stripes.py`, `index_game_types.py`, `extract_fight_models.py`, `map_native_patch.py`, `native_xrefs.py`, `trace_offline_player.py` | Recuperação da metadata IL2CPP e do código protegido; mapeamento de métodos. |
| `metadata/` | `build_backup_catalog.py` | **Ainda em uso:** regenera `BackupCatalog.java`/`backup_regions.h` a partir das tabelas 1.9.1 (`python tools/metadata/build_backup_catalog.py <tabelas>`). |
| `native/` | `decode_code_stripes.py`, `recover_rodata.py`, `elf_probe.py`, `map_game_methods.py`, `trace_game_native.py` | Imagens de análise e disassembly do `libil2cpp.so`. |
| `organization/` | `prepare_plan.py` | Inventário da migração para este checkout (já executada; ver `docs/history/migration/`). |

`package.json` declara `rea-agents` só para o tooling REA histórico. `npm ci` não é necessário para compilar, testar nem abrir o Atlas.
