# Documentação

| Pasta | O que tem | Quando ler |
|---|---|---|
| [`../MAINTENANCE_GUIDE.md`](../MAINTENANCE_GUIDE.md) | Estado atual do projeto inteiro | Sempre, antes de qualquer mudança |
| [`development/`](development/) | [SETUP.md](development/SETUP.md) (dependências locais e `config/local.json`), [RELEASE.md](development/RELEASE.md) (passo a passo de uma build normal) e `build-history.json` (todo BuildId já usado, com package e hashes) | Ao configurar a máquina ou gerar uma build |
| [`testing/`](testing/) | Um relatório por sistema ou build: contrato, testes, evidência e limites. Índice em [testing/README.md](testing/README.md) | Ao mexer num sistema |
| [`decisions/`](decisions/) | Decisões de arquitetura do workspace (ADR) | Ao questionar a organização |
| [`history/`](history/) | Registros que não descrevem mais o estado atual: diário de builds, auditoria funcional, README antigo do bridge, investigação inicial e a migração para este checkout | Só para a causa, a evidência ou o RVA de uma mudança antiga |

Relatórios de análise estática do APK (metadata, bundles, código nativo) ficam em [`../analysis/`](../analysis/README.md).

## Convenções

- **Estado atual** vai no guia, na linha do sistema. Não criar seção datada nem "Atualização vNN" no guia.
- **Detalhe de uma mudança** vai num relatório `docs/testing/<SISTEMA>_V<NNN>.md` e na mensagem de commit. Quando um relatório for superado, marcar no topo "Registro histórico, substituído por …" e atualizar o índice de `testing/`.
- **Nível de evidência** sempre explícito: estática, log, tela ou gameplay. Compilar não valida instalação nem jogo.
- Português do Brasil em textos novos. Arquivos antigos em inglês ficam como estão.
- Depois de mover ou renomear documentos: `python tools/workspace/check-doc-links.py`.
