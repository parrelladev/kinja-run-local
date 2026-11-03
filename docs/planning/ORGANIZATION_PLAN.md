# Plano de organização — Kinja Run local

Registro histórico da proposta aprovada; execução em [MIGRATION_REPORT.md](MIGRATION_REPORT.md).

Estado antes da aprovação: proposta para revisão. Nenhum material do jogo foi movido, copiado ou excluído. A pasta nova já possui `.git` e `.gitattributes`; preservá-los. Este pacote acrescenta somente documentação do planejamento.

## Decisão central

Criar um checkout leve em `D:\Programacao\kinja-run-local`, preservando os caminhos funcionais de `offline/`, `tools/`, `analysis/` e `project-atlas/`. A pasta `D:\Programacao\kinja-run` continua como acervo local intacto. Primeira migração por cópia seletiva com hashes, não por movimentação destrutiva. Não copiar os ~90 GB de `build/` para o Git.

## Estrutura proposta

```text
kinja-run-local/
  .git/                       existente; preservar
  .gitattributes              existente; preservar
  .gitignore                  criar antes de copiar material
  README.md                   entrada do projeto e links
  AGENTS.md                   regras de manutenção na raiz
  MAINTENANCE_GUIDE.md         preservar nome/caminho
  FUNCTIONAL_AUDIT.md          preservar nome/caminho
  REVERSE_ENGINEERING.md      histórico existente, sem recomeçar análise
  offline/                    fontes JNI/C++, Java, seeds e build atuais
  tools/                      apenas scripts próprios; subpastas preservadas
  analysis/                   relatórios versionados + índices/extrações locais ignorados
  project-atlas/              dashboard, catálogo e gerador
  docs/
    planning/                 este plano, manifest e painel visual
    development/              setup e referência de ferramentas locais
    testing/                  matriz de teste, BlueStacks e limites por ambiente
    decisions/                decisões arquiteturais novas, sem reescrever histórico
  config/
    local.example.json        esquema de caminhos sem segredos
    local.json                configuração pessoal ignorada
  tests/
    fixtures/                 casos sintéticos seguros que já existirem e forem revisados
  build/                      gerados locais; ignorados
```

`docs/development`, `docs/testing`, `docs/decisions`, `config` e `tests` são destinos propostos, ainda sem implantação. Não criar arquivos de teste fictícios para preencher pastas.

## Destino de cada categoria

| Categoria | GitHub | Operação proposta |
|---|---|---|
| Fonte do bridge, wrapper Java, seeds iniciais e scripts próprios | Sim | Copiar seletivamente, conservar caminhos e comparar SHA256 |
| Guia, auditoria, regras e relatórios técnicos | Sim, após revisão | Preservar referências; documentos novos ligados pelo README e Atlas |
| APK original e APKs de laboratório | Não no Git normal | Preservar acervo local e hashes; obtenção/entrega separada da árvore de fontes |
| Bibliotecas extraídas, bundles, metadata/dumps e índices recuperados | Local por padrão | Manter sem reexecutar engenharia reversa; cadastrar caminho/proveniência |
| Logs, screenshots e planos históricos de build | Local por padrão | Conservar BuildIds; publicar somente recortes revisados quando necessários |
| NDK/JDK/SDK, ferramentas binárias e bibliotecas Python instaladas | Não | Provisionamento documentado; caminhos externos configuráveis |
| Keystore e arquivo de senha | Nunca | Permanecer privados; mesma chave necessária para atualizar instalações existentes |
| Saves privados e diagnósticos de inventário do aparelho | Não | Não importar no Git; diferenciar dos seeds iniciais do código |
| node_modules, caches e configurações pessoais de agente | Não | Reinstalação/configuração local; package-lock preservado |
| inventory.js / inventory.json do Atlas | Gerados locais por padrão | Regenerar no novo checkout com escopo de publicação explícito |

## Ajustes necessários antes de compilar na pasta nova

1. `offline/native/build-native.ps1` contém NDK absoluto em `D:\Programacao\kinja-run`. Tornar configurável e registrar resolução efetiva; não alegar portabilidade enquanto isso persistir.
2. `offline/build.ps1` espera APK original na raiz, ferramentas em `tools/android/...` e assinatura em `build/offline-lab.jks` mais arquivo de senha. `prepare_wrapper.ps1` também fixa subcaminhos de JDK, android.jar e D8. Criar configuração local/overrides mantendo os defaults compatíveis, sem alterar runtime do jogo.
3. Não criar junctions/symlinks para esconder dependências. Registrar explicitamente o uso do acervo antigo e verificar hash do APK1.9.1 antes de qualquer build.
4. Atlas hoje valida existência de caminhos centrais, incluindo dumps não versionados. Adaptar para arquivos locais ausentes com status explícito e links portáveis. Não gerar um painel vazio ao clonar nem transformar ausência em sucesso.
5. Revisar documentos/índices gerados antes de versionar: podem incorporar caminhos absolutos e estado de perfis. O manifesto classifica nomes, não garante ausência de dados sensíveis no conteúdo.

## BlueStacks como ambiente de testes

Intenção confirmada pelo usuário: testar no BlueStacks em vez de depender do Android físico. A instalação foi informada pelo usuário; instância, caminhos e compatibilidade ainda não foram verificados nesta etapa. Nenhum emulador foi iniciado nem APK instalado.

Antes do primeiro teste, registrar versão/instância, Android/API, ABIs expostas e eventual tradução ARM, resolução e endpoint ADB. O cliente e o bridge são ARM64: não presumir compatibilidade apenas porque o emulador abre. Se houver tradução de instruções, observar prólogos e endereços antes de concluir que hooks são equivalentes. Preservar sempre a validação de hash/prologue e chamadas IL2CPP na thread Unity.

Usar `adb -s <serial-confirmado>` em todas as ações, impedindo instalação acidental em outro alvo. Detectar o endpoint da instância em vez de assumir porta padrão. Preparar fluxo de logs/capturas por execução; registrar APK SHA256, biblioteca, package, perfil, ABI e ambiente junto ao resultado.

Priorizar conta natural separada `com.habby.kinjarun.offline.audit1`, sem QA e sem importar save do celular. Não atualizar ou limpar `lab6` como parte da organização. Começar por login/home, restart e menus, depois combate/transações com recursos naturais e baseline. Testes no BlueStacks terão coluna própria e não substituem evidência do Android físico.

Checkpoint existente: lab6/v107; v110 principal assinada não instalada; audit1/v111 com refresh pós-vitória ainda sem vitória validada. A fonte atual difere da biblioteca v109 usada em v110: não chamar recompilação atual de reprodução idêntica da v110.

## Sequência de migração revisável

1. Revisar este plano e as categorias do manifesto/painel. Nenhuma publicação ou push nesta fase.
2. Criar `.gitignore` antes da cópia e conferir `git check-ignore` para APK, chave/senha, extrações e caches.
3. Copiar somente candidatos aprovados para a pasta nova; comparar hash de origem/destino e gravar relatório. Não sobrescrever arquivos existentes divergentes nem tocar em `.git`.
4. Configurar ferramentas/APK/assinatura locais; documentar setup BlueStacks sem instalar jogo implicitamente.
5. Ajustar portabilidade dos scripts e Atlas; atualizar links, guia e índice. Separar mudanças de organização de mudanças no jogo.
6. Verificar sintaxe, resolução de caminhos, links, geração/abertura do painel e seleção de arquivos para Git. Compilar apenas quando o setup estiver explícito; compilação não equivale a instalação/gameplay.
7. Revisar `git diff`, arquivos ignorados, dados sensíveis e arquivos grandes. Preparar commit local revisável; push/publicação requer solicitação própria.

## Critérios de conclusão

- README conduz ao painel, setup e testes; relatórios mantêm proveniência e links válidos.
- Fonte copiada confere por hash; arquivo divergente não é sobrescrito silenciosamente.
- Nenhuma chave/senha, APK, toolchain ou save privado entre os arquivos para Git.
- Dependências locais ausentes são reportadas claramente; checkout não depende silenciosamente do caminho antigo.
- BlueStacks aparece como ambiente não validado até haver evidência real de execução.
- Origem e builds históricas preservadas; nenhuma alteração da lógica offline misturada à migração.

## Inventário desta proposta

Os totais exatos por categoria e o destino de cada arquivo estão em `migration-manifest.json` e em `organization.html`. Caminhos são relativos à origem; não são links de migração executada. Secret significa suspeita por nome, sem leitura de conteúdo. `git-review` exige revisão antes de inclusão. Este plano não adota Git LFS automaticamente nem publica artefatos.
