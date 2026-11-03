"""Verify a prepared DEX contains only the launcher and its agreed JNI calls."""
from __future__ import annotations
import argparse
import hashlib
import json
import struct
import sys
import zlib
from pathlib import Path

PROJECT=Path(__file__).absolute().parent.parent
sys.path.insert(0,str(PROJECT/'tools'/'android'))
from inspect_android_static import Dex


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--dex',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    args=p.parse_args(); data=args.dex.read_bytes()
    if hashlib.sha1(data[32:]).digest()!=data[12:32]: raise ValueError('DEX SHA-1 signature mismatch')
    if zlib.adler32(data[12:]) & 0xffffffff != struct.unpack_from('<I',data,8)[0]: raise ValueError('DEX checksum mismatch')
    dex=Dex(data,str(args.dex))
    if len(dex.classes)!=1 or dex.classes[0]['name']!='Llocal/kinja/OfflineActivity;':
        raise ValueError('Only local.kinja.OfflineActivity may be included; compile-only stubs must be excluded')
    cls=dex.classes[0]
    if cls['super']!='Lcom/unity3d/player/UnityPlayerActivity;': raise ValueError('Wrong launcher superclass')
    methods={m['name'].split('->',1)[1]:m for m in cls['methods']}
    config='configure(Ljava/lang/String;Landroid/content/res/AssetManager;)V'
    if config not in methods or not methods[config]['access_flags'] & 0x100 or not methods[config]['access_flags'] & 8:
        raise ValueError('JNI configure must be static native(String,AssetManager)')
    init=dex.code(methods['<clinit>()V']['code_offset'])
    if not any(x.get('string')=='kinjaoffline' for x in init) or not any(x.get('method')=='Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V' for x in init):
        raise ValueError('Launcher must load libkinjaoffline.so')
    create=dex.code(methods['onCreate(Landroid/os/Bundle;)V']['code_offset'])
    calls=[x['method'] for x in create if 'method' in x]
    configured='Llocal/kinja/OfflineActivity;->'+config
    inherited='Lcom/unity3d/player/UnityPlayerActivity;->onCreate(Landroid/os/Bundle;)V'
    if configured not in calls or inherited not in calls or calls.index(configured)>=calls.index(inherited):
        raise ValueError('JNI configure must run before inherited Unity onCreate')
    retained=any(x.get('field')=='Llocal/kinja/OfflineActivity;->assetManager:Landroid/content/res/AssetManager;' for x in create)
    if not retained: raise ValueError('Launcher does not retain its AssetManager')
    report=dict(dex=str(args.dex.absolute()),sha256=hashlib.sha256(data).hexdigest(),class_count=1,
                class_name=cls['name'],superclass=cls['super'],jni_export='Java_local_kinja_OfflineActivity_configure',
                native_library='libkinjaoffline.so',configure_before_unity_onCreate=True,asset_manager_retained=True,
                compile_only_stubs_excluded=True,onCreate_calls=calls)
    args.report.parent.mkdir(parents=True,exist_ok=True)
    args.report.write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps(report,indent=2))


if __name__=='__main__': main()
