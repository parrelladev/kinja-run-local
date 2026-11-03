"""Non-instantiating NRBF data parser for locally packaged game tables.

Subset of MS-NRBF record grammar; unknown records fail with exact position.
Reference: https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-nrbf/954a0657-b901-4813-9398-4ec732fe8b32
"""
import struct, math, json, pathlib, collections

class Ref:
    def __init__(self, oid): self.oid=oid
class Nulls:
    def __init__(self,count): self.count=count
class Reader:
    def __init__(self, data):
        self.data=data; self.pos=0; self.objects={}; self.schemas={}; self.libraries={}; self.root=None; self.types=collections.Counter()
    def unpack(self,f):
        n=struct.calcsize('<'+f); r=struct.unpack_from('<'+f,self.data,self.pos); self.pos+=n; return r[0] if len(r)==1 else r
    def byte(self): return self.unpack('B')
    def integer(self): return self.unpack('i')
    def string(self):
        size=0; shift=0
        while True:
            b=self.byte(); size|=(b&127)<<shift
            if not b&128: break
            shift+=7
            if shift>28: raise ValueError('Bad length at '+str(self.pos))
        if size<0 or self.pos+size>len(self.data): raise ValueError('Out of bounds string')
        result=self.data[self.pos:self.pos+size].decode('utf-8'); self.pos+=size; return result
    def primitive(self,t):
        fmts={1:'?',2:'B',6:'d',7:'h',8:'i',9:'q',10:'b',11:'f',12:'q',13:'q',14:'H',15:'I',16:'Q'}
        if t in fmts: return self.unpack(fmts[t])
        if t in (5,18): return self.string()
        if t==17:return None
        raise ValueError('Unsupported primitive '+str(t)+' at '+str(self.pos))
    def extra(self,t):
        if t in (0,7): return self.byte()
        if t==3: return self.string()
        if t==4: return (self.string(),self.integer())
        if t in (1,2,5,6): return None
        raise ValueError('Unsupported binary type '+str(t))
    def value(self,t,extra): return self.primitive(extra) if t==0 else self.next_value()
    def next_value(self):
        while True:
            value=self.record()
            if value is not ...: return value
    def class_members(self,oid,schema,offset):
        name,names,types,extras=schema
        obj={'$class':name,'$id':oid,'$offset':offset,'$member_offsets':{}}
        self.objects[oid]=obj
        for n,t,e in zip(names,types,extras):
            obj['$member_offsets'][n]=self.pos
            obj[n]=self.value(t,e)
        return obj
    def values(self,count,t=2,extra=None):
        values=[]
        if count<0 or count>10000000: raise ValueError('Bad array count '+str(count))
        while len(values)<count:
            value=self.value(t,extra)
            if isinstance(value,Nulls): values.extend([None]*value.count)
            else: values.append(value)
        if len(values)!=count: raise ValueError('Null array overflow')
        return values
    def record(self):
        start=self.pos; tag=self.byte(); self.types[tag]+=1
        if tag==0:
            self.root,self.header_id,self.major,self.minor=self.unpack('iiii'); return ...
        if tag==12:
            lid=self.integer(); self.libraries[lid]=self.string(); return ...
        if tag in (4,5):
            oid=self.integer(); name=self.string(); count=self.integer(); names=[self.string() for _ in range(count)]; types=[self.byte() for _ in range(count)]; extras=[self.extra(t) for t in types]
            if tag==5: library=self.integer()
            schema=(name,names,types,extras); self.schemas[oid]=schema
            return self.class_members(oid,schema,start)
        if tag==1:
            oid=self.integer(); mid=self.integer(); return self.class_members(oid,self.schemas[mid],start)
        if tag==6:
            oid=self.integer(); s=self.string(); self.objects[oid]=s; return s
        if tag==7:
            oid=self.integer(); kind=self.byte(); rank=self.integer(); lengths=[self.integer() for _ in range(rank)]
            if kind in (3,4,5): lower=[self.integer() for _ in range(rank)]
            t=self.byte(); extra=self.extra(t); values=[]; self.objects[oid]=values
            values.extend(self.values(math.prod(lengths),t,extra)); return values
        if tag in (15,16,17):
            oid=self.integer(); count=self.integer(); t=0 if tag==15 else (2 if tag==16 else 1); extra=self.byte() if tag==15 else None
            values=[]; self.objects[oid]=values; values.extend(self.values(count,t,extra)); return values
        if tag==8:return self.primitive(self.byte())
        if tag==9:return Ref(self.integer())
        if tag==10:return None
        if tag==11:return ...
        if tag==13:return Nulls(self.byte())
        if tag==14:return Nulls(self.integer())
        raise ValueError('Unsupported tag '+str(tag)+' at '+str(start))
    def resolve(self,value,trail=None):
        trail=set() if trail is None else trail
        if isinstance(value,Ref):
            if value.oid in trail: return {'$ref':value.oid}
            return self.resolve(self.objects[value.oid],trail|{value.oid})
        if isinstance(value,list): return [self.resolve(v,trail) for v in value]
        if isinstance(value,dict): return {k:self.resolve(v,trail) for k,v in value.items()}
        return value
    def parse(self):
        while self.pos<len(self.data): self.record()
        return self.resolve(self.objects[self.root],{self.root})

if __name__=='__main__':
    out=pathlib.Path(__file__).parents[2]/'analysis'/'bundles'
    report=json.loads((out/'table_index.json').read_text()) if (out/'table_index.json').exists() else []
    complete={i['payload'] for i in report if 'json' in i and i.get('consumed')==i.get('payload_size')}
    for p in sorted((out/'textassets').rglob('*.bytes')):
        label=str(p.relative_to(out)).replace('\\','/')
        if label in complete:continue
        if not p.read_bytes().startswith(b'\x00\x01\x00\x00\x00\xff\xff\xff\xff'):continue
        r=Reader(p.read_bytes()); item={'payload':label}
        try:
            data=r.parse(); target=out/'tables'/p.parent.name/(p.stem+'.json'); target.parent.mkdir(parents=True,exist_ok=True)
            target.write_text(json.dumps(data,ensure_ascii=True,indent=2),encoding='utf-8')
            item.update({'json':str(target.relative_to(out)).replace('\\','/'),'rows':len(data) if isinstance(data,list) else None,'classes':sorted({v['$class'] for v in r.objects.values() if isinstance(v,dict) and '$class' in v}),'libraries':r.libraries,'records':dict(r.types),'consumed':r.pos,'payload_size':len(r.data)})
        except Exception as e:item.update({'error':str(e),'consumed':r.pos,'payload_size':len(r.data)})
        report=[i for i in report if i['payload']!=label]
        report.append(item)
        print(p.stem,item.get('rows',item.get('error')),flush=True)
    (out/'table_index.json').write_text(json.dumps(report,ensure_ascii=True,indent=2),encoding='utf-8')
