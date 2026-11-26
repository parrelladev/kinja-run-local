# Kinja Run offline

Adaptador que faz o cliente original **Kinja Run Android 1.9.1** (Unity IL2CPP ARM64) funcionar sem servidor. O APK original não é reescrito: uma Activity JNI e uma biblioteca nativa são acrescentadas a ele e, em tempo de execução, redirecionam as chamadas ao backend para respostas e saves locais. Combate, telas e assets continuam sendo os do jogo.

Este repositório guarda fonte, scripts e conhecimento técnico. APK original, toolchains, keystore e builds antigas ficam num acervo local fora do Git, apontado por `config/local.json`.

## Começar

1. Leia o [guia de manutenção](MAINTENANCE_GUIDE.md): estado atual, regras, hooks, saves e sistemas.
2. Configure a máquina: [docs/development/SETUP.md](docs/development/SETUP.md).
3. Para gerar uma build para o celular: [docs/development/RELEASE.md](docs/development/RELEASE.md).
4. Para rodar os testes: [tests/README.md](tests/README.md).

Agentes de IA seguem também o [AGENTS.md](AGENTS.md).

## Estrutura

| Pasta | Conteúdo |
|---|---|
| `offline/native/` | Bridge JNI/IL2CPP (`offline_bridge.cpp`) e lógica separada em headers |
| `offline/java/` | Activity wrapper e backup manual (Java) |
| `offline/assets/` | Seeds de um perfil novo; não são o save do jogador |
| `offline/build.ps1` | Empacotamento e assinatura ([contrato do wrapper](offline/BUILDING.md)) |
| `tests/` | Testes de host e runner único |
| `tools/` | Scripts de build, workspace e análise ([índice](tools/README.md)) |
| `docs/` | Setup, build, relatórios por sistema, decisões e histórico ([índice](docs/README.md)) |
| `analysis/` | Relatórios de análise estática do APK ([índice](analysis/README.md)) |
| `project-atlas/` | Painel visual de exploração (`./project-atlas/open.ps1`) |
| `config/` | `local.example.json`; o `local.json` pessoal não é versionado |
| `build/`, `offline/build/` | Saídas geradas, ignoradas pelo Git |

## Limites

Funções que dependem de servidor (correio, guilda, compras reais, anúncios reais) não existem offline; algumas foram substituídas por regras locais documentadas no guia. Compilar não valida instalação nem jogo: cada sistema tem o nível de evidência registrado no guia. Não há licenciamento definido para materiais cuja titularidade não foi estabelecida.
