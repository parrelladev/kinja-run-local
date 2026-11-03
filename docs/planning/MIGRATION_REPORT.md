# Resultado da migração seletiva

Migração de trabalho autorizada. A pasta de origem foi preservada; nenhum APK, build histórico, toolchain, chave/senha, dump de metadata, bundle ou save privado foi copiado. `.git` e `.gitattributes` do destino foram preservados. Nenhum commit/push, instalação ou execução do jogo foi realizado por esta migração.

## Cópia e integridade

Foram selecionados **72 arquivos existentes**, total **1288070 bytes** antes dos ajustes. Cada cópia inicial foi comparada por SHA256. [copied-files.json](copied-files.json) registra hash de origem, hash final e quais arquivos receberam ajustes de portabilidade/documentação. Fontes C++/Java, planejador, seeds e guards do jogo devem permanecer idênticos.

Os arquivos adicionais são ignore, configuração de exemplo, helper de caminhos, README, setup, plano de testes, decisão e este registro. `config/local.json` é pessoal e ignorado; apenas referencia o acervo externo. O inventário completo pré-migração permanece local e ignorado; o painel do plano usa uma tabela compacta dos candidatos selecionados.

## Ajustes de organização

- Resolução explícita dos caminhos de APK, NDK, toolchain e assinatura nos scripts. Não há fallback automático para a pasta antiga sem configuração.
- Atlas funciona em checkout leve, identifica referências externas/ausentes e usa índice recuperado externo opcional. Dados gerados continuam ignorados.
- README conecta setup, conhecimento, painel, testes e decisões. Caminhos críticos das fontes e relatórios foram mantidos.
- BlueStacks é o alvo planejado de testes, ainda sem validação de instância/ABI/ADB/APK ou gameplay.

## Verificação

Resultados efetivos são registrados em `migration-validation.json` após execução das verificações. Não interpretar compilação/contrato do wrapper como instalação ou gameplay. A fonte atual contém o refresh v111; não chamar uma recompilação dela de reprodução bit a bit da v110 histórica.

## Verificação realizada

- 72 arquivos copiados conferidos por SHA256 contra a origem e contra a versão final; fonte do bridge/planejador, Java e seeds sem alterações.
- Biblioteca ARM64 compilada no checkout novo usando o NDK externo: `125B4D555D6FF04E5934CE0A59505CBCA2B117CBD72C482D00D4C7D06DF62A9F`. Um warning preexistente de parâmetro não usado; não foi alterada lógica para silenciá-lo.
- Wrapper Java/D8 compilado; contrato JNI, uma classe e stub fora do DEX aprovados. DEX mantém SHA256 histórico `cbb4731d4a5f995205144e42bf287a8432d2c999ff862ac52b93bdd909224e7c`.
- Sintaxe Python/PowerShell/JavaScript e seeds verificadas; Atlas gerado com índice externo e testado em estágio sem configuração externa. Navegador confirmou visão geral, caminho externo do APK e seleção no grafo, sem erros de console.
- Git ignora configuração pessoal, APKs, chave/senha, índice gerado e saídas; candidatos não contêm binários proibidos nem padrões de alta confiança de chave privada/token GitHub/AWS. Isso não substitui revisão de conteúdo antes de publicação.
- APK original externo confere com SHA256 esperado. Não foi empacotado/instalado APK nem iniciado o jogo; BlueStacks continua não testado.

Detalhes legíveis por máquina: [migration-validation.json](migration-validation.json). As compilações de verificação geraram somente saídas pequenas e ignoradas; não fazem parte da migração dos builds históricos.

O empacotamento também recusa BuildIds já usados no acervo configurado: v110 foi recusada sem gerar saídas. A identidade do histórico não reinicia na pasta nova.
