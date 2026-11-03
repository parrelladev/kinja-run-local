"""Rebuild the local atlas from existing evidence. Standard library only; no APK work."""
from pathlib import Path
import os, re, json, datetime, hashlib

ROOT = Path(__file__).resolve().parents[1]
OUT = Path(__file__).resolve().parent
CAT = json.loads((OUT / 'catalog.json').read_text(encoding='utf-8'))
ROLES = {
    'offline/native/offline_bridge.cpp':'Implementação central JNI/IL2CPP: bootstrap, hooks, respostas ET, economia, journals e persistência local. Compartilhada por vários sistemas.',
    'offline/native/fusion_batch_plan.h':'Planejador puro da política coletiva offline autorizada: grupos q1–3, exclusão de equipados, raiz por nível/UID e cascata até roxo.',
    'offline/java/local/kinja/OfflineActivity.java':'Activity wrapper que configura filesDir e AssetManager no bridge antes de iniciar Unity.',
    'offline/build.ps1':'Empacotamento Android: wrapper, manifest/level0, DEX, biblioteca nativa, seeds e assinatura de laboratório.',
    'offline/native/build-native.ps1':'Compilação ARM64 via NDK; opções QA são separadas da progressão normal.',
    'offline/prepare_wrapper.ps1':'Prepara e valida o wrapper Java/JNI.',
    'analysis/il2cpp/dump.cs':'Tipos e assinaturas recuperados da metadata; não contém corpos do código-fonte original.',
    'analysis/native/game-method-map.json':'Índice recuperado de classes/métodos, imagens e endereços usado pelo painel sem nova engenharia reversa.',
    'kinja-run-1-9-1.apk':'Artefato-alvo original 1.9.1; preservar. Hash esperado no guia; o gerador não recalcula o APK nem modifica seu conteúdo.'
}
SKIP = {'.git', '.codex', '.agents', 'node_modules', '__pycache__', 'python-libs', 'toolchain', 'native-toolchain', 'bundle-cache', 'downloads', 'Il2CppDumper'}
files, excluded = [], []
config_path = ROOT/'config/local.json'
config = json.loads(config_path.read_text(encoding='utf-8-sig')) if config_path.exists() else {}
archive = Path(config['archiveRoot']) if config.get('archiveRoot') else None
def external_path(rel):
    return archive/rel if archive else None

def systems(path):
    result = [s['id'] for s in CAT['systems'] if path in s['files'] or any(re.search(p, path, re.I) for p in s['patterns'])]
    return result or ['unmapped']

for directory, dirs, names in os.walk(ROOT):
    for name in list(dirs):
        if name in SKIP:
            excluded.append((Path(directory) / name).relative_to(ROOT).as_posix())
            dirs.remove(name)
    for name in names:
        p = Path(directory) / name
        rel = p.relative_to(ROOT).as_posix()
        if rel.startswith(('project-atlas/', 'build/', '.local/')) or rel in {'config/local.json','docs/planning/migration-manifest.json','docs/planning/planning-data.js'}:
            continue
        if p.suffix.lower() in {'.keystore', '.jks', '.p12', '.pem', '.key'} or any(word in p.name.lower() for word in ('password','credential','token')):
            excluded.append(rel + ' [assinatura: omitida]')
            continue
        if p.is_symlink():
            excluded.append(rel + ' [link: omitido]')
            continue
        ids = systems(rel)
        role = 'Papel semântico não catalogado; associação por caminho.'
        if rel.startswith('build/'): role = 'Artefato/evidência gerada; não editar como fonte.'
        elif rel.startswith('offline/assets/'): role = 'Seed inicial; não sobrescreve perfil já criado no aparelho.'
        elif rel.startswith('analysis/bundles/tables/'): role = 'Configuração extraída do cliente 1.9.1; não é código-fonte.'
        elif rel.startswith('analysis/'): role = 'Análise/evidência existente; verificar versão e contexto.'
        elif rel.startswith('tools/'): role = 'Ferramenta de inspeção ou preparação existente.'
        role = ROLES.get(rel,role)
        custom = rel.startswith(('offline/native/', 'offline/java/', 'offline/assets/')) or rel in {'offline/build.ps1','offline/prepare_wrapper.ps1'}
        files.append(dict(path=rel, size=p.stat().st_size, ext=p.suffix.lower() or '(sem extensão)', systems=ids, role=role, custom=custom))

files.sort(key=lambda f:f['path'])
known = {f['path'] for f in files}
for system in CAT['systems']:
    for rel in system['files']:
        if rel in known: continue
        p = external_path(rel)
        present = p is not None and p.is_file()
        files.append(dict(path=rel,size=p.stat().st_size if present else 0,ext=Path(rel).suffix.lower(),systems=systems(rel),role=ROLES.get(rel,'Referência documental externa ao checkout.'),custom=False,availability='acervo externo' if present else 'ausente neste checkout',realPath=str(p) if present else '',sourceRoot=str(archive) if present else ''))
        known.add(rel)
files.sort(key=lambda f:f['path'])
documents=[]
for f in files:
    if f['ext']=='.md' and not f.get('availability'):
        body=(ROOT / f['path']).read_text(encoding='utf-8', errors='replace')
        sections=[]
        for match in re.finditer(r'^#{1,4}\s+(.+)$', body, re.M):
            sections.append(dict(title=match[1], line=body[:match.start()].count('\n')+1))
        documents.append(dict(path=f['path'], body=body, sections=sections))

# Cross references are textual evidence, never promoted to runtime dependencies.
references=[]
for d in documents:
    paths=set(re.findall(r'`([^`\n]+)`',d['body']))
    for path in paths:
        normalized=path.replace('\\','/').removeprefix('./')
        if normalized in known:
            references.append(dict(source=d['path'], target=normalized, kind='referência documental', confidence='confirmado'))

bridge='offline/native/offline_bridge.cpp'
source=(ROOT/bridge).read_text(encoding='utf-8')
symbols=[]
for m in re.finditer(r'^([\w:<>,*&\s]+?)\b([A-Za-z_]\w*)\([^;{}]*\)\s*\{', source, re.M):
    name=m[2]
    if name in {'if','while','for','switch','catch'}: continue
    start=source.rfind('\n',0,m.start(2))+1
    symbols.append(dict(name=name,file=bridge,line=source[:start].count('\n')+1,
                        excerpt='\n'.join(source[start:].splitlines()[:18])))
hooks=[]
for m in re.finditer(r'patch_entry\(base,\s*(0x[\da-fA-F]+|kAccountInitVa),\s*\w+,\s*"([^"]+)"',source):
    label=m[2]
    hooks.append(dict(name=label, rva=m[1], file=bridge, line=source[:m.start()].count('\n')+1,
                      systems=systems(label), confidence='confirmado', evidence='Chamada patch_entry na fonte; não prova instalação no aparelho.'))

# Reuse the recovered method index; no decoding or reverse engineering is performed.
methods=[]
method_map=Path(config['methodMap']) if config.get('methodMap') else ROOT/'analysis/native/game-method-map.json'
if not method_map.is_absolute(): method_map=ROOT/method_map
if method_map.exists():
    for m in json.loads(method_map.read_text(encoding='utf-8')):
        ids=systems(m.get('class_name',''))
        methods.append(dict(name=m.get('name'), cls=m.get('class_name'), rva=m.get('native_address'), image=m.get('image'), systems=ids))

for s in CAT['systems']:
    s['count']=sum(s['id'] in f['systems'] and f.get('availability')!='ausente neste checkout' for f in files)
    for path in s['files']:
        if path not in known: raise ValueError('Referência ausente: '+path)
for flow in CAT['flows']:
    for step in flow['steps']:
        if step['system'] not in {s['id'] for s in CAT['systems']}: raise ValueError(step)

previews={}
for s in CAT['systems']:
    for path in s['files']:
        p=ROOT/path
        if p.is_file() and p.suffix.lower() in {'.java','.h','.ps1','.json'} and p.stat().st_size<100000:
            previews[path]=p.read_text(encoding='utf-8',errors='replace')
data=dict(**CAT, files=files, documents=documents, references=references, hooks=hooks, methods=methods, symbols=symbols,previews=previews,
          root=str(ROOT), generated=datetime.datetime.now(datetime.timezone.utc).isoformat(), excluded=excluded,
          evidenceHashes={d['path']:hashlib.sha256((ROOT/d['path']).read_bytes()).hexdigest() for d in documents},
          methodMapSource=str(method_map) if method_map.exists() else 'não disponível', checkoutFiles=sum(not f.get('availability') for f in files), externalFiles=sum(f.get('availability')=='acervo externo' for f in files), missingFiles=sum(f.get('availability')=='ausente neste checkout' for f in files), coverage='Checkout leve: fontes e documentação locais, referências centrais externas identificadas por disponibilidade. Arquivo ausente não é funcionalidade validada. Índice de métodos externo opcional; acervo pesado não migrado. Associações por nome são inferidas e dependências não exaustivas.')
(OUT/'inventory.json').write_text(json.dumps(data,ensure_ascii=False,separators=(',',':')),encoding='utf-8')
(OUT/'inventory.js').write_text('window.ATLAS='+json.dumps(data,ensure_ascii=False,separators=(',',':'))+';\n',encoding='utf-8')
print(json.dumps(dict(files=len(files),documents=len(documents),methods=len(methods),hooks=len(hooks),references=len(references),excluded=len(excluded)),ensure_ascii=False))
