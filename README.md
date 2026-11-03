# Kinja Run local

Workspace leve do adaptador offline para o cliente **Kinja Run Android 1.9.1 / Unity IL2CPP ARM64**. Fonte, scripts e conhecimento técnico ficam aqui; APKs, toolchains, chaves e builds históricos ficam no acervo local, fora do Git.

## Começar

1. Leia [MAINTENANCE_GUIDE.md](MAINTENANCE_GUIDE.md) e [FUNCTIONAL_AUDIT.md](FUNCTIONAL_AUDIT.md).
2. Siga o [setup local](docs/development/SETUP.md). Nesta máquina, `config/local.json` já aponta explicitamente para o acervo existente; ele não é versionado.
3. Execute `python project-atlas/build_inventory.py` e abra [project-atlas/index.html](project-atlas/index.html) no navegador, ou execute `./project-atlas/open.ps1`.
4. Consulte a [preparação de testes no BlueStacks](docs/testing/BLUESTACKS.md) antes de instalar qualquer APK.

## Onde trabalhar

| Área | Conteúdo |
|---|---|
| `offline/native/` | Bridge JNI/IL2CPP e planejador de fusão |
| `offline/java/` | Activity wrapper |
| `offline/assets/` | Seeds iniciais; não são o save privado do jogador |
| `tools/` | Scripts próprios de preparação e análise; binários/terceiros não incluídos |
| `analysis/` | Relatórios preservados; extrações e índices grandes são dependências locais opcionais |
| `project-atlas/` | Painel visual, catálogo e gerador |
| `docs/` | Setup, testes, decisões e rastreabilidade da migração |
| `config/` | Exemplo versionado; configuração pessoal ignorada |
| `build/`, `offline/build/` | Novas saídas geradas localmente e ignoradas |

## Estado e rastreabilidade

- [Resultado da migração](docs/planning/MIGRATION_REPORT.md) e [decisão de organização](docs/decisions/0001-light-workspace.md).
- [Plano visual](docs/planning/organization.html): resumo do acervo e arquivos selecionados, sem duplicar os 90 GB de builds.
- [Checkpoint mais recente](analysis/native/OFFLINE_READINESS_AUDIT.md): lab6/v107, v110 assinada não instalada e audit1/v111. Preservar os limites de validação registrados.

Migração não altera gameplay, prólogos, hooks ou saves. Compilação/validação de wrapper não comprovam instalação ou gameplay no BlueStacks. Não há APK original, keystore, senha, toolchain ou save privado neste repositório. Não foi definido licenciamento novo para materiais cuja titularidade não foi estabelecida.
