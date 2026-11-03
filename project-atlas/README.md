# Kinja Run · Project Atlas

Abra `index.html` nesta pasta por duplo clique. Funciona por `file://`, sem servidor, instalação, CDN ou internet. O arquivo `inventory.js` acompanha o HTML e fornece os dados locais. Não abra apenas o HTML isolado dos demais arquivos.

## Navegação

- **Visão geral:** sistemas, estado documentado e perfis lab6/v107, entrega v110 e audit1/v111 separados.
- **Sistema:** responsabilidade, classes, métodos, arquivos centrais, relações, validação e riscos.
- **Explorador:** árvore com expansão sob demanda, filtros de sistema/extensão/personalização, detalhes e cópia do caminho absoluto.
- **Dependências:** níveis de sistemas, arquivos centrais e classes/hooks; isolamento por sistema, confiança, zoom e arraste; seleção mostra relações de entrada/saída e alcance transitivo potencial.
- **Offline:** fluxos locais de inicialização, combate, fusão e compra; limitações ao lado de cada fluxo.
- **Modificações:** marcos por BuildId e acesso aos registros integrais. Não atribui datas ou commits inexistentes.
- **Documentação:** fontes Markdown completas incorporadas, sumário por linha e referências cruzadas a arquivos existentes.
- **Busca global:** nomes/caminhos, classes/métodos recuperados e conteúdo dos documentos. Atalho `/`.

## Atualização

Na raiz do projeto:

```powershell
python project-atlas/build_inventory.py
```

Python padrão, sem instalar pacotes. Recarregue o HTML após gerar. O script não executa engenharia reversa, ADB, builds ou jogo. Reaproveita o índice existente `analysis/native/game-method-map.json`, lê documentos, percorre arquivos e extrai chamadas `patch_entry`/símbolos do bridge. `inventory.json` é intercambiável; `inventory.js` permite abrir sem `fetch` e sem servidor.

Edite `catalog.json` para atualizar sistemas, padrões de associação, relações, perfis, fluxos e marcos. Cada conclusão curada aponta para um documento local. Atualize primeiro a evidência técnica original; depois revise a síntese no catálogo e regenere. A presença do relatório mais recente prevalece sobre resumos históricos: `OFFLINE_READINESS_AUDIT.md` contém o checkpoint v110/v111, posterior aos resumos v109 do guia/auditoria.

## Escopo e confiança

O inventário cobre arquivos físicos fora das exclusões registradas no painel. Dependências de terceiros, toolchains, caches, metadados internos e chaves de assinatura são excluídos. Os arquivos do próprio painel ficam fora do inventário do jogo para evitar autorreferência.

Associação de arquivo/método a sistema por nome é **inferida**. Caminhos centrais e relações do catálogo são curados a partir da documentação indicada. Referências entre documentos e arquivos são apenas citações, não chamadas runtime. Métodos em metadata não são código-fonte e não comprovam disponibilidade offline. Hooks na fonte não comprovam que foram instalados/exercitados em determinada build.

“Personalizado” identifica o código/seeds do adaptador. Não é detecção de diferenças contra o APK nem contra Git: esta pasta não tem repositório Git acessível. Não se atribui modificação a binários originais ou assets sem evidência. Os marcos são uma síntese; as fontes integrais preservam o histórico detalhado, inclusive hipóteses refutadas e builds intermediárias.

Não há cobertura semântica integral de todos os tipos/assets do APK, nem grafo completo de chamadas IL2CPP. O painel explicita desconhecidos e permite pesquisar todo o índice já recuperado. Não foi repetida engenharia reversa para preencher lacunas. O protótipo de navegador citado em documentação histórica não está presente como `index.html` na raiz atual e não foi fabricado.

## Organização mantida

| Área | Responsabilidade |
|---|---|
| `offline/` | Implementação, wrapper, seeds e scripts originais de build |
| `analysis/` | Conhecimento extraído e relatórios técnicos |
| `build/` | Artefatos/evidências por BuildId; não editar outputs |
| `tools/` | Ferramentas existentes de inspeção e preparação |
| `project-atlas/` | Catálogo curado, gerador, índice e apresentação visual |

Não é necessária reorganização física do Android. Um mesmo arquivo pode pertencer a vários sistemas. Para futuras mudanças, siga Sistema → arquivo/método → evidência → riscos, reproduza o fluxo e preserve os guards de hash/prólogo, APIs originais e thread Unity. Seeds em assets não substituem saves privados já existentes.

## Verificação e limites

O gerador valida caminhos centrais e IDs dos fluxos antes de produzir o índice. A interface usa somente recursos locais e APIs nativas do navegador. Não concede recompensas nem altera saves. O índice representa um snapshot: alterações posteriores em arquivos/status exigem regeneração e revisão do catálogo.

O painel é um artefato de documentação. Nenhum resultado de teste do painel deve ser interpretado como nova validação de instalação, tela ou gameplay do Android.

Validação desta entrega: gerador e sintaxe JavaScript aprovados; interface exercitada no navegador por servidor restrito a `127.0.0.1`, incluindo árvore/filtros/busca, arquivo/conteúdo, cópia, evidência, método, grafo por classe e fluxos. A automação do navegador bloqueia protocolo `file://`, portanto o duplo clique é suportado pela estrutura sem fetch, mas não foi validado por essa automação. Se preferir HTTP, execute `python -m http.server 8766 --bind 127.0.0.1 --directory project-atlas` na raiz e abra `http://127.0.0.1:8766/`; encerre com Ctrl+C.

## Checkout leve migrado

Consulte `../docs/development/SETUP.md`. Índices gerados são locais ignorados. Referências de APK/dump/índice podem existir apenas no acervo externo configurado; cada uma mostra sua disponibilidade. Sem índice de métodos externo, o painel mantém sistemas, documentos e hooks de fonte, sem inventar métodos. Use `open.ps1` para gerar antes de abrir.
