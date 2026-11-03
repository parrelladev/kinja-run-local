"""Extract response layouts from recovered Il2CppDumper declarations; no target execution."""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
text = (ROOT / 'analysis/il2cpp/dump.cs').read_text(encoding='utf-8-sig')
names = [
    'C2G_EnterGame', 'G2C_EnterGame', 'RegionMonsterInfo',
    'C2G_EndGame', 'G2C_EndGame', 'G2C_EndGlobefish', 'G2C_EndWorldCup',
    'G2C_EndActivity', 'G2C_EndSecret', 'G2C_EndBigSecret',
    'G2C_EndDifficultGame', 'G2C_EndBountyOrder', 'G2C_KillEnemy',
    'C2G_KillEnemy', 'ItemInfo',
]
out = {}
for name in names:
    start = re.search(r'^public class ' + re.escape(name) + r'\b.*$', text, re.M)
    if not start:
        raise RuntimeError(name)
    end = text.index('\n}', start.start()) + 2
    block = text[start.start():end]
    fields = []
    for access, typ, field, offset in re.findall(
        r'^\s+(public|private|protected|internal) (\S+(?:<[^\n]+?>)?) ([^\n;]+); // (0x[0-9A-F]+)', block, re.M
    ):
        pretty = re.sub(r'^<(.+)>k__BackingField$', r'\1', field)
        fields.append({'name': pretty, 'type': typ, 'offset': offset.lower()})
    ctor = re.search(r'// RVA: (0x[0-9A-F]+)[^\n]*\n\s+public void \.ctor\(\)', block)
    out[name] = {
        'typeDefIndex': int(re.search(r'TypeDefIndex: (\d+)', block).group(1)),
        'constructorVA': ctor.group(1).lower() if ctor else None,
        'fields': fields,
    }
dest = ROOT / 'analysis/metadata/fight-response-models.json'
dest.write_text(json.dumps(out, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
print(dest)
for name, data in out.items():
    print(name, 'ctor=' + str(data['constructorVA']), '; '.join(f"{f['offset']} {f['type']} {f['name']}" for f in data['fields']))
