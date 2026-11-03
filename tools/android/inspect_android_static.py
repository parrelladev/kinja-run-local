"""Dependency-free AXML and focused DEX inspection of an original APK.

Only reads the APK. Results are static evidence, never execution evidence.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path
import xml.etree.ElementTree as ET


def u16(b, o): return struct.unpack_from('<H', b, o)[0]
def u32(b, o): return struct.unpack_from('<I', b, o)[0]


def leb(b, o):
    v = shift = 0
    while True:
        n = b[o]
        o += 1
        v |= (n & 127) << shift
        if not n & 128:
            return v, o
        shift += 7
        if shift > 35:
            raise ValueError('Malformed ULEB128')


def axml(b):
    strings, root, stack, events = [], None, [], []
    pos = u16(b, 2)
    if u16(b, 0) != 3:
        raise ValueError('Expected binary XML')
    while pos < len(b):
        kind, header, size = struct.unpack_from('<HHI', b, pos)
        if size < header or size < 8 or pos + size > len(b):
            raise ValueError(f'Bad AXML chunk at {pos:#x}')
        if kind == 1:
            count, flags, start = u32(b, pos + 8), u32(b, pos + 16), u32(b, pos + 20)
            for i in range(count):
                o = pos + start + u32(b, pos + header + i * 4)
                if flags & 0x100:
                    def length8(p):
                        n = b[p]
                        return (((n & 127) << 8) | b[p + 1], p + 2) if n & 128 else (n, p + 1)
                    _, o = length8(o)
                    n, o = length8(o)
                    strings.append(b[o:o + n].decode('utf8', 'replace'))
                else:
                    n = u16(b, o); o += 2
                    if n & 0x8000:
                        n = ((n & 0x7fff) << 16) | u16(b, o); o += 2
                    strings.append(b[o:o + n * 2].decode('utf-16-le', 'replace'))
        elif kind == 0x102:
            name = strings[u32(b, pos + 20)]
            node = ET.Element(name)
            attrs = []
            attr_start, attr_size, count = struct.unpack_from('<HHH', b, pos + 24)
            for i in range(count):
                o = pos + 16 + attr_start + i * attr_size
                ns, idx, raw, _, _, typ, data = struct.unpack_from('<IIIHBBI', b, o)
                aname = strings[idx]
                namespace = '' if ns == 0xffffffff else strings[ns]
                if raw != 0xffffffff: value = strings[raw]
                elif typ == 3: value = strings[data]
                elif typ == 0x12: value = 'true' if data else 'false'
                elif typ == 0x10: value = str(data if data < 0x80000000 else data - 0x100000000)
                elif typ == 0x11: value = f'0x{data:x}'
                elif typ == 1: value = f'@0x{data:08x}'
                elif typ == 2: value = f'?0x{data:08x}'
                else: value = f'type:{typ:#x}:data:{data:#x}'
                node.set(f'{{{namespace}}}{aname}' if namespace else aname, value)
                attrs.append(dict(name=aname, namespace=namespace, value=value, type=typ, data=data, file_offset=f'0x{o:x}'))
            if stack: stack[-1].append(node)
            else: root = node
            stack.append(node)
            events.append(dict(kind='start', name=name, line=u32(b, pos + 8), file_offset=f'0x{pos:x}', attributes=attrs))
        elif kind == 0x103:
            stack.pop()
        pos += size
    ET.register_namespace('android', 'http://schemas.android.com/apk/res/android')
    ET.indent(root)
    return root, events


class Dex:
    def __init__(self, data, name):
        self.b, self.name = data, name
        if not data.startswith(b'dex\n'): raise ValueError('Not a DEX')
        self.strings = []
        for i in range(u32(data, 56)):
            o = u32(data, u32(data, 60) + i * 4)
            _, o = leb(data, o)
            end = data.index(0, o)
            self.strings.append(data[o:end].replace(b'\xc0\x80', b'\0').decode('utf8', 'replace'))
        self.types = [self.strings[u32(data, u32(data, 68) + i * 4)] for i in range(u32(data, 64))]
        self.protos = []
        for i in range(u32(data, 72)):
            _, ret, params = struct.unpack_from('<III', data, u32(data, 76) + i * 12)
            pt = [] if not params else [self.types[u16(data, params + 4 + j * 2)] for j in range(u32(data, params))]
            self.protos.append('(' + ''.join(pt) + ')' + self.types[ret])
        self.fields = []
        for i in range(u32(data, 80)):
            cls, typ, nameidx = struct.unpack_from('<HHI', data, u32(data, 84) + i * 8)
            self.fields.append(self.types[cls] + '->' + self.strings[nameidx] + ':' + self.types[typ])
        self.methods = []
        for i in range(u32(data, 88)):
            cls, proto, nameidx = struct.unpack_from('<HHI', data, u32(data, 92) + i * 8)
            self.methods.append(self.types[cls] + '->' + self.strings[nameidx] + self.protos[proto])
        self.classes = []
        for i in range(u32(data, 96)):
            o = u32(data, 100) + i * 32
            cls, flags, sup, interfaces, source, annotations, classdata, staticvalues = struct.unpack_from('<8I', data, o)
            c = dict(name=self.types[cls], super=None if sup == 0xffffffff else self.types[sup], access_flags=flags,
                     dex=name, file_offset=f'0x{o:x}', methods=[], source=None if source == 0xffffffff else self.strings[source])
            if classdata:
                counts=[]
                for _ in range(4): n, classdata=leb(data,classdata); counts.append(n)
                for count in counts[:2]:
                    idx=0
                    for _ in range(count):
                        delta,classdata=leb(data,classdata); idx+=delta
                        _,classdata=leb(data,classdata)
                for count in counts[2:]:
                    idx=0
                    for _ in range(count):
                        delta,classdata=leb(data,classdata); idx+=delta
                        flags,classdata=leb(data,classdata)
                        code,classdata=leb(data,classdata)
                        c['methods'].append(dict(name=self.methods[idx], access_flags=flags, code_offset=code))
            self.classes.append(c)

    def code(self, off):
        if not off: return []
        count=u32(self.b,off+12)
        words=struct.unpack_from('<'+'H'*count,self.b,off+16)
        result=[]; pc=0
        lengths={0:1,1:1,2:2,3:3,4:1,5:2,6:3,7:1,8:2,9:3,10:1,11:1,12:1,13:1,14:1,15:1,16:1,17:1,
                 18:1,19:2,20:3,21:2,22:2,23:3,24:5,25:2,26:2,27:3,28:2,29:1,30:1,31:2,32:2,33:1,34:2,35:2,
                 36:3,37:3,38:3,39:1,40:1,41:2,42:3,43:3,44:3}
        for start,end,n in [(0x2d,0x3d,2),(0x44,0x6d,2),(0x6e,0x72,3),(0x74,0x78,3),(0x7b,0x8f,1),
                            (0x90,0xaf,2),(0xb0,0xcf,1),(0xd0,0xe2,2),(0xfa,0xfb,4),(0xfc,0xfd,3),(0xfe,0xff,2)]:
            lengths.update({i:n for i in range(start,end+1)})
        names={0:'nop',1:'move',2:'move/from16',3:'move/16',7:'move-object',8:'move-object/from16',9:'move-object/16',
               10:'move-result',11:'move-result-wide',12:'move-result-object',14:'return-void',15:'return',16:'return-wide',17:'return-object',
               18:'const/4',19:'const/16',20:'const',21:'const/high16',26:'const-string',27:'const-string/jumbo',28:'const-class',
               29:'monitor-enter',30:'monitor-exit',31:'check-cast',32:'instance-of',34:'new-instance',35:'new-array',36:'filled-new-array',
               38:'fill-array-data',39:'throw',40:'goto',41:'goto/16',42:'goto/32',43:'packed-switch',44:'sparse-switch'}
        names.update(dict(zip(range(0x32,0x3e),['if-eq','if-ne','if-lt','if-ge','if-gt','if-le','if-eqz','if-nez','if-ltz','if-gez','if-gtz','if-lez'])))
        names.update(dict(zip(range(0x6e,0x73),['invoke-virtual','invoke-super','invoke-direct','invoke-static','invoke-interface'])))
        names.update(dict(zip(range(0x74,0x79),['invoke-virtual/range','invoke-super/range','invoke-direct/range','invoke-static/range','invoke-interface/range'])))
        while pc<count:
            w=words[pc]; op=w&255
            if op==0 and w>>8:
                typ=w>>8
                if typ==1: size=4+words[pc+1]*2
                elif typ==2: size=2+words[pc+1]*4
                elif typ==3: size=4+(words[pc+1]*(words[pc+2]|words[pc+3]<<16)+1)//2
                else: raise ValueError(f'Unknown payload {typ}')
            else: size=lengths.get(op,1)
            if pc+size>count: raise ValueError(f'Truncated opcode at {pc:#x}')
            x=dict(pc=f'0x{pc:x}', file_offset=f'0x{off+16+pc*2:x}', opcode=f'0x{op:02x}', mnemonic=names.get(op,f'op_{op:02x}'),
                   raw=' '.join(f'{n:04x}' for n in words[pc:pc+size]))
            if op==26: x['string']=self.strings[words[pc+1]]
            elif op==27: x['string']=self.strings[words[pc+1]|words[pc+2]<<16]
            elif 0x6e<=op<=0x72 or 0x74<=op<=0x78: x['method']=self.methods[words[pc+1]]
            elif op in [28,31,32,34,35,36,37]: x['type']=self.types[words[pc+1]]
            elif 0x52<=op<=0x6d: x['field']=self.fields[words[pc+1]]
            elif 0x32<=op<=0x3d or op==41:
                delta=words[pc+1]; delta=delta if delta<0x8000 else delta-0x10000
                x['branch_target']=f'0x{pc+delta:x}'
            elif op==40:
                delta=w>>8; delta=delta if delta<128 else delta-256
                x['branch_target']=f'0x{pc+delta:x}'
            result.append(x); pc+=size
        return result


def main():
    p=argparse.ArgumentParser()
    p.add_argument('apk'); p.add_argument('--output',required=True)
    p.add_argument('--class-pattern',action='append',default=[])
    args=p.parse_args(); out=Path(args.output); out.mkdir(parents=True,exist_ok=True)
    apk=Path(args.apk)
    digest=hashlib.file_digest(apk.open('rb'),'sha256').hexdigest()
    allclasses=[]; selected=[]; dexinfo=[]
    with zipfile.ZipFile(apk) as z:
        manifest=z.read('AndroidManifest.xml')
        root,events=axml(manifest)
        (out/'AndroidManifest.decoded.xml').write_text(ET.tostring(root,encoding='unicode'),encoding='utf8')
        (out/'manifest-events.json').write_text(json.dumps(events,indent=2,ensure_ascii=False),encoding='utf8')
        ns='{http://schemas.android.com/apk/res/android}'
        app=root.find('application')
        launchers=[]
        for activity in app.findall('activity'):
            for filt in activity.findall('intent-filter'):
                actions=[x.get(ns+'name') for x in filt.findall('action')]
                cats=[x.get(ns+'name') for x in filt.findall('category')]
                if 'android.intent.action.MAIN' in actions and 'android.intent.category.LAUNCHER' in cats:
                    launchers.append(dict(activity=activity.get(ns+'name'),attributes=activity.attrib))
        summary=dict(apk=str(apk),sha256=digest,manifest_sha256=hashlib.sha256(manifest).hexdigest(),
                     manifest=root.attrib,sdk=root.find('uses-sdk').attrib,application=app.attrib,
                     permissions=[x.get(ns+'name') for x in root.findall('uses-permission')],launchers=launchers,
                     providers=[x.attrib for x in app.findall('provider')])
        (out/'manifest-summary.json').write_text(json.dumps(summary,indent=2,ensure_ascii=False),encoding='utf8')
        for name in z.namelist():
            if name.startswith('classes') and name.endswith('.dex'):
                data=z.read(name); d=Dex(data,name)
                dexinfo.append(dict(name=name,sha256=hashlib.sha256(data).hexdigest(),class_count=len(d.classes),method_ids=len(d.methods),version=data[:8].decode('ascii','replace')))
                for c in d.classes:
                    allclasses.append({k:v for k,v in c.items() if k!='methods'} | {'method_count':len(c['methods'])})
                    if any(s.lower() in c['name'].lower() for s in args.class_pattern):
                        for m in c['methods']: m['instructions']=d.code(m['code_offset'])
                        selected.append(c)
    for name,value in [('classes.json',allclasses),('selected-classes.json',selected),('identity.json',dict(apk=str(apk),sha256=digest,dex=dexinfo))]:
        (out/name).write_text(json.dumps(value,indent=2,ensure_ascii=False),encoding='utf8')
    text=['# Original APK bytecode, static inspection only', '# Opcode coverage resolves calls, strings, types, fields and conditional branches; other opcodes remain raw.']
    for c in selected:
        text.append('\n.class '+c['name']+'\n.dex '+c['dex']+'\n.super '+str(c['super']))
        for m in c['methods']:
            text.append('\n.method '+m['name']+' access_flags='+hex(m['access_flags'])+' code_offset='+hex(m['code_offset']))
            for ins in m['instructions']:
                annotation=' '.join(str(ins[k]) for k in ['method','string','type','field','branch_target'] if k in ins)
                text.append('  '+ins['pc']+' '+ins['file_offset']+' '+ins['mnemonic']+' ['+ins['raw']+'] '+annotation)
            text.append('.end method')
    (out/'bootstrap.disassembly.txt').write_text('\n'.join(text),encoding='utf8')
    print(json.dumps(dict(sha256=digest,dex=dexinfo,selected_class_names=[c['name'] for c in selected]),indent=2))


if __name__=='__main__': main()
