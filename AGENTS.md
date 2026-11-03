# Instruções para manutenção do Kinja Run offline

Antes de investigar ou alterar este repositório, leia [`MAINTENANCE_GUIDE.md`](MAINTENANCE_GUIDE.md). Ele descreve a arquitetura, os pontos de entrada do APK original, o bridge offline, os saves, a progressão, os hooks e as limitações já observadas. Leia [`FUNCTIONAL_AUDIT.md`](FUNCTIONAL_AUDIT.md) para os defeitos confirmados na build de laboratório atual e os testes que ainda faltam.

Regras de trabalho:

- Faça um commit local após cada mudança concluída, com mensagem que descreva a alteração e validação pertinente. Não deixe alterações próprias pendentes ao encerrar a tarefa. O usuário fará o push; não execute push automaticamente.
- Trate `kinja-run-1-9-1.apk` como artefato-alvo da versão 1.9.1; não presuma que nomes de métodos ou endereços servem para outra versão.
- Reproduza e rastreie o fluxo relatado antes de alterar lógica. Separe evidência observada de hipótese e evite mudanças amplas para sintomas ainda não explicados.
- Preserve as verificações de hash/prologue e as chamadas às APIs IL2CPP originais. Não escreva offsets de objetos gerenciados diretamente.
- O jogo original roda em Unity/IL2CPP ARM64. Chamadas a singletons, callbacks e `runtime_invoke` devem ocorrer na thread Unity, conforme o fluxo do bridge.
- O APK de laboratório é separado do original. Para preservar dados locais ao atualizar o aparelho, mantenha o package de laboratório e assine com o keystore de desenvolvimento existente.
- Não trate funções online, anúncios ou compras como disponíveis offline sem implementar e documentar uma substituição local coerente.
- Depois de mudar uma área, atualize o guia com o comportamento e o nível de validação alcançado. Não descreva instalação, tela ou gameplay como validados se só houve compilação.
