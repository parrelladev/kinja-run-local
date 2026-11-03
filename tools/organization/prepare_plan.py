"""Read-only migration inventory; writes planning artifacts, never copies project inputs."""
from pathlib import Path
import os, json, hashlib, html

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/organization-plan'
OUT.mkdir(parents=True, exist_ok=True)
rows=[]
for folder, dirs, names in os.walk(ROOT):
    dirs[:] = [d for d in dirs if d not in {'.git','__pycache__'} and not (Path(folder)/d).is_symlink()]
    for name in names:
        p=Path(folder)/name
        if OUT==p.parent or OUT in p.parents or p.is_symlink(): continue
        rel=p.relative_to(ROOT).as_posix()
        bucket='local'; reason='Material de análise/artefato a preservar fora do Git.'
        if rel.startswith(('node_modules/','.codex/','.agents/')):
            bucket='environment';reason='Dependência instalada ou configuração da máquina; não copiar automaticamente.'
        elif any(x in rel for x in ['/toolchain/','/native-toolchain/','/python-libs/','/bundle-cache/','/downloads/','/Il2CppDumper/']):
            bucket='environment';reason='Toolchain/terceiro instalado; manter local e documentar provisionamento.'
        elif any(x in name.lower() for x in ['password','keystore','credential','token']) or p.suffix.lower() in {'.jks','.p12','.key','.pem'}:
            bucket='secret';reason='Nome sugere material sensível; excluir do Git e preservar privadamente sem ler conteúdo.'
        elif rel.startswith('build/') or rel.startswith('offline/build/'):
            bucket='archive';reason='Saída de build ou evidência histórica; manter caminho e BuildId na origem.'
        elif rel in {'AGENTS.md','MAINTENANCE_GUIDE.md','FUNCTIONAL_AUDIT.md','REVERSE_ENGINEERING.md','package.json','package-lock.json'}:
            bucket='git';reason='Entrada documental/dependências do projeto; preservar caminho.'
        elif rel.startswith('offline/'):
            bucket='git';reason='Implementação própria, scripts e seeds iniciais; preservar caminho.'
        elif rel.startswith('project-atlas/') and name not in {'inventory.json','inventory.js'}:
            bucket='git';reason='Interface/gerador/catálogo; índices gerados permanecem locais.'
        elif rel.startswith('tools/') and p.suffix.lower() in {'.py','.ps1'}:
            bucket='git';reason='Script próprio fora das pastas de terceiros; revisar dependências antes da cópia.'
        elif rel.startswith('analysis/') and p.suffix.lower()=='.md':
            bucket='git-review';reason='Relatório técnico candidato ao Git após revisar dados locais; manter caminho.'
        rows.append(dict(path=rel,bytes=p.stat().st_size,bucket=bucket,reason=reason))

groups={}
for row in rows:
    g=groups.setdefault(row['bucket'],dict(files=0,bytes=0));g['files']+=1;g['bytes']+=row['bytes']
manifest=dict(source=str(ROOT),destination='D:\\Programacao\\kinja-run-local',mode='PLANO — nenhuma migração executada',groups=groups,files=rows,
              warnings=['Classificação automática por caminho/extensão, não auditoria de conteúdo.','Configuração/compatibilidade BlueStacks ainda não verificadas.','Não há plano de exclusão de dados na origem.'])
(OUT/'migration-manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,separators=(',',':')),encoding='utf-8')
plan='''# Plano de organização — Kinja Run local

Estado: proposta para revisão. Nenhum material do jogo foi movido, copiado ou excluído. A pasta nova já possui `.git` e `.gitattributes`; preservá-los. Este pacote acrescenta somente documentação do planejamento.

## Decisão central

Criar um checkout leve em `D:\\Programacao\\kinja-run-local`, preservando os caminhos funcionais de `offline/`, `tools/`, `analysis/` e `project-atlas/`. A pasta `D:\\Programacao\\kinja-run` continua como acervo local intacto. Primeira migração por cópia seletiva com hashes, não por movimentação destrutiva. Não copiar os ~90 GB de `build/` para o Git.

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

1. `offline/native/build-native.ps1` contém NDK absoluto em `D:\\Programacao\\kinja-run`. Tornar configurável e registrar resolução efetiva; não alegar portabilidade enquanto isso persistir.
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
'''
(OUT/'ORGANIZATION_PLAN.md').write_text(plan,encoding='utf-8')
ignore='''# Proposta; aplicar como .gitignore somente na implantação do plano.
node_modules/
__pycache__/
*.pyc
.codex/
.agents/
/config/local.json
/build/
/offline/build/
*.apk
*.jks
*.keystore
*.p12
*.pem
*.key
*password*
*.so
*.dll
*.exe
*.zip
/tools/**/toolchain/
/tools/**/native-toolchain/
/tools/**/python-libs/
/tools/**/bundle-cache/
/tools/**/downloads/
/tools/**/Il2CppDumper/
/analysis/**
!/analysis/**/
!/analysis/**/*.md
/project-atlas/inventory.js
/project-atlas/inventory.json
'''
(OUT/'gitignore.proposed').write_text(ignore,encoding='utf-8')
page='''<!doctype html><html lang="pt-BR"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Plano de organização · Kinja Run</title><style>
:root{color-scheme:dark}*{box-sizing:border-box}body{margin:0;background:#0d1420;color:#dce7f6;font:14px/1.6 system-ui}main{max-width:1250px;margin:auto;padding:40px 24px}h1{font-size:36px;letter-spacing:-1px}h2{font-size:21px}.muted,p{color:#9daec4}a{color:#68ddc4}.badge{display:inline-block;background:#34301f;color:#e8ca7d;border:1px solid #66542b;border-radius:20px;padding:5px 14px;font-size:11px;letter-spacing:1px}.cards,.stages{display:grid;grid-template-columns:repeat(3,1fr);gap:15px;margin:25px 0}.card,.panel{background:#152132;border:1px solid #2c4058;border-radius:10px;padding:20px}.card strong{display:block;color:#6fe0c5;font-size:25px}.card b{display:block;margin-top:10px}.tree{white-space:pre-wrap;font:13px/1.8 monospace;color:#99dacc}.arrow{font-size:24px;color:#6fe0c5}input,select,button{background:#18283b;color:#dce7f6;padding:10px;border:1px solid #3d526c;border-radius:6px}input{width:min(100%,480px)}table{width:100%;border-collapse:collapse;font-size:12px}td,th{text-align:left;border-bottom:1px solid #2a3c52;padding:10px;word-break:break-word}.scroll{overflow:auto}details{margin:20px 0}summary{cursor:pointer;color:#6fe0c5}.warn{border-left:3px solid #e4b65e;padding:12px 20px;background:#29251b}.label{font-size:11px;letter-spacing:2px;color:#6fe0c5}@media(max-width:700px){.cards,.stages{grid-template-columns:1fr}h1{font-size:28px}}
</style><main><span class="badge">PROPOSTA · NENHUM ARQUIVO DO JOGO MIGRADO</span><p class="label">KINJA RUN / ORGANIZAÇÃO DO WORKSPACE</p><h1>Fontes leves. Acervo preservado.<br>Um caminho claro para trabalhar.</h1><p>Origem: D:\\Programacao\\kinja-run<br>Destino: D:\\Programacao\\kinja-run-local</p><p><a href="ORGANIZATION_PLAN.md">Plano técnico completo</a> · <a href="migration-manifest.json">Manifesto por arquivo</a> · <a href="gitignore.proposed">Gitignore proposto</a></p><div class="cards" id="stats"></div><div class="stages"><article class="panel"><span class="label">01 / REPOSITÓRIO</span><h2>Fonte + conhecimento</h2><p>Bridge, wrapper, seeds, scripts próprios, relatórios revisados e interface do Atlas. Caminhos críticos preservados.</p></article><article class="panel"><span class="label">02 / ACERVO LOCAL</span><h2>Preservar sem duplicar</h2><p>APKs, builds, dumps, assets, ferramentas instaladas e evidências pesadas continuam locais. Chaves permanecem privadas.</p></article><article class="panel"><span class="label">03 / TESTES</span><h2>BlueStacks isolado</h2><p>Registrar instância, Android, ABI e serial ADB antes de instalar. Não foi testado nesta etapa; saves do celular não são importados.</p></article></div><div class="warn">Antes da cópia: ajustar o NDK absoluto, os caminhos de ferramentas/assinatura e o Atlas para índices locais ausentes. Não basta trocar o nome da pasta para obter uma build portátil.</div><div class="panel" style="margin:25px 0"><h2>Estrutura proposta</h2><div class="tree">kinja-run-local/
├─ README.md · AGENTS.md · guias existentes
├─ offline/        fontes e seeds (mesmos caminhos)
├─ tools/          scripts próprios (sem toolchains no Git)
├─ analysis/       relatórios + extrações locais ignoradas
├─ project-atlas/  exploração visual do projeto
├─ docs/           planning · development · testing · decisions
├─ config/         exemplo versionado; configuração local ignorada
├─ tests/          fixtures sintéticas revisadas
└─ build/          saídas locais ignoradas</div></div><h2>Destino de cada arquivo</h2><p>Classificação automática por caminho/extensão. Relatórios exigem revisão; o conteúdo não foi auditado para publicação. Exibimos até 100 linhas por vez.</p><input id="query" aria-label="Pesquisar arquivo" placeholder="Pesquisar nome ou caminho…"><select id="bucket" aria-label="Categoria"><option value="">Todas as categorias</option><option value="git">Candidato ao Git</option><option value="git-review">Relatório para revisão</option><option value="archive">Acervo de builds</option><option value="local">Análise/artefato local</option><option value="environment">Ambiente/terceiros</option><option value="secret">Sensível pelo nome</option></select><p id="count"></p><div class="scroll"><table><thead><tr><th>Arquivo na origem</th><th>Categoria</th><th>Tamanho</th><th>Decisão proposta</th></tr></thead><tbody id="rows"></tbody></table></div><p><button id="prev">← Anterior</button> <button id="next">Próximos 100 →</button></p><div class="panel"><h2>Execução em etapas verificáveis</h2><p>Revisar plano → aplicar ignore → copiar candidatos e comparar hashes → configurar ambiente → corrigir portabilidade → conferir painel e Git → preparar commit revisável.</p><p>Sem exclusões, push, instalação de APK ou alteração de gameplay nesta proposta.</p></div></main><script src="planning-data.js"></script><script>
const e=v=>String(v).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));const titles={'git':'Fonte / candidato ao Git','git-review':'Relatório / revisar','archive':'Acervo de builds','local':'Material local','environment':'Ambiente / terceiros','secret':'Sensível pelo nome'};const size=n=>n>1e9?(n/1e9).toFixed(2)+' GB':n>1e6?(n/1e6).toFixed(2)+' MB':(n/1e3).toFixed(1)+' KB';let page=0;document.querySelector('#stats').innerHTML=Object.entries(PLAN.groups).map(([k,g])=>'<article class="card"><strong>'+size(g.bytes)+'</strong><b>'+titles[k]+'</b><span>'+g.files.toLocaleString('pt-BR')+' arquivos</span></article>').join('');function render(){let q=document.querySelector('#query').value.toLowerCase(),b=document.querySelector('#bucket').value;const list=PLAN.files.filter(f=>(!q||f.path.toLowerCase().includes(q))&&(!b||f.bucket===b));page=Math.min(page,Math.max(0,Math.ceil(list.length/100)-1));document.querySelector('#count').textContent=list.length.toLocaleString('pt-BR')+' resultados · página '+(page+1);document.querySelector('#rows').innerHTML=list.slice(page*100,page*100+100).map(f=>'<tr><td>'+e(f.path)+'</td><td>'+titles[f.bucket]+'</td><td>'+size(f.bytes)+'</td><td>'+e(f.reason)+'</td></tr>').join('');document.querySelector('#prev').disabled=!page;document.querySelector('#next').disabled=(page+1)*100>=list.length}document.querySelector('#query').oninput=()=>{page=0;render()};document.querySelector('#bucket').onchange=()=>{page=0;render()};document.querySelector('#prev').onclick=()=>{page--;render()};document.querySelector('#next').onclick=()=>{page++;render()};render();
</script></html>'''
(OUT/'organization.html').write_text(page,encoding='utf-8')
(OUT/'planning-data.js').write_text('const PLAN='+json.dumps(manifest,ensure_ascii=False,separators=(',',':'))+';',encoding='utf-8')
print(json.dumps(groups,ensure_ascii=False))
print('Planning files: '+str(OUT))
