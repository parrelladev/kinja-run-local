# Instruções para manutenção do Kinja Run offline

Antes de investigar ou alterar este repositório, leia [`MAINTENANCE_GUIDE.md`](MAINTENANCE_GUIDE.md) inteiro. Ele descreve o estado atual: arquitetura, regras, os hooks instalados, os saves, o estado e a validação de cada sistema, packages, build e testes. Para trabalhar num sistema, leia também o relatório em `docs/testing/` ou `analysis/native/` indicado na linha dele.

O histórico por build fica em [`docs/history/`](docs/history/): o antigo guia (`MAINTENANCE_LOG.md`), a auditoria funcional (`FUNCTIONAL_AUDIT.md`) e o antigo README do bridge. Consulte-o só quando precisar da causa, da evidência ou do RVA de uma mudança antiga. Não acrescente seções datadas ao guia: edite a linha ou seção do sistema afetado e coloque o detalhe no relatório e no commit.

Regras de trabalho:

- Faça um commit local após cada mudança concluída, com mensagem que descreva a alteração e validação pertinente. Não deixe alterações próprias pendentes ao encerrar a tarefa. O usuário fará o push; não execute push automaticamente.
- Após concluir os testes e documentar uma nova build normal, limpe `build` com `tools/workspace/clean-builds.ps1 -KeepBuildId <build-normal-atual>`, mantendo somente o APK assinado atual e seu relatório de assinatura. O usuário autorizou excluir builds antigas, intermediários e evidências brutas locais; preserve as sínteses versionadas e o histórico pequeno de IDs/hashes. Não limpe o acervo externo, dependências, chave/senha ou saves do Android. Durante uma investigação, mantenha temporariamente apenas os artefatos necessários aos testes em curso.
- Trate `kinja-run-1-9-1.apk` como artefato-alvo da versão 1.9.1; não presuma que nomes de métodos ou endereços servem para outra versão.
- Reproduza e rastreie o fluxo relatado antes de alterar lógica. Separe evidência observada de hipótese e evite mudanças amplas para sintomas ainda não explicados.
- Preserve as verificações de hash/prologue e as chamadas às APIs IL2CPP originais. Não escreva offsets de objetos gerenciados diretamente.
- O jogo original roda em Unity/IL2CPP ARM64. Chamadas a singletons, callbacks e `runtime_invoke` devem ocorrer na thread Unity, conforme o fluxo do bridge.
- O APK de laboratório é separado do original. Para preservar dados locais ao atualizar o aparelho, mantenha o package de laboratório e assine com o keystore de desenvolvimento existente.
- Não trate funções online, anúncios ou compras como disponíveis offline sem implementar e documentar uma substituição local coerente.
- Depois de mudar uma área, atualize o guia com o comportamento e o nível de validação alcançado. Não descreva instalação, tela ou gameplay como validados se só houve compilação.
