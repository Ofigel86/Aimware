#!/usr/bin/env python3
"""
Aimware 2016 Dump Analyzer - Full Reverse Tool
Analyzes b34E10000.h, b43AF0000.h, b76ED0000.h, b7C4A0000.h
"""

import re
import struct
import collections
import math
import sys
import os

def load_dump(path):
    with open(path,'r') as f:
        data=f.read()
    m=re.findall(r'0x([0-9A-Fa-f]{1,2})', data)
    return bytes(int(x,16) for x in m)

def entropy(data):
    if not data: return 0
    freq=collections.Counter(data)
    ent=0
    for count in freq.values():
        p=count/len(data)
        ent-=p*math.log2(p)
    return ent

def extract_strings(data, min_len=4):
    result=[]
    cur=b''
    cur_off=0
    for i,b in enumerate(data):
        if 32 <= b <= 126:
            if not cur:
                cur_off=i
            cur+=bytes([b])
        else:
            if len(cur)>=min_len:
                result.append((cur_off, cur))
            cur=b''
    if len(cur)>=min_len:
        result.append((cur_off, cur))
    return result

def find_pointers(data, regions):
    ptrs=[]
    for i in range(0, len(data)-4):
        val=struct.unpack('<I', data[i:i+4])[0]
        for base, end, name in regions:
            if base <= val < end:
                ptrs.append((i, val, name))
                break
    return ptrs

def find_x86_prologues(data):
    prologues={
        b'\x55\x8B\xEC': 'push ebp; mov ebp, esp',
        b'\x53\x8B\xDC': 'push ebx; mov ebp, esp',
        b'\x56\x8B\xF1': 'push esi; mov esi, ecx',
        b'\x8B\xFF': 'mov edi, edi (hotpatch)',
    }
    found=[]
    for i in range(len(data)-3):
        for pat, desc in prologues.items():
            if data[i:i+len(pat)] == pat:
                found.append((i, desc))
                break
    return found

def analyze_dump(path, base, regions):
    print(f"\n{'='*60}")
    print(f"Analyzing {path}")
    print(f"Base: 0x{base:08X}")
    data=load_dump(path)
    print(f"Size: 0x{len(data):X} ({len(data)} bytes)")
    print(f"Entropy: {entropy(data):.3f} {'(encrypted)' if entropy(data)>7.5 else '(data)' if entropy(data)<6.5 else '(code/mix)'}")
    
    ptrs=find_pointers(data, regions)
    print(f"Pointers to known regions: {len(ptrs)}")
    # Group by target
    by_target=collections.Counter([name for _,_,name in ptrs])
    for name,count in by_target.most_common():
        print(f"  -> {name}: {count}")
    
    if len(ptrs)>0:
        print(f"  Sample pointers:")
        for off,val,name in ptrs[:10]:
            print(f"    0x{off:05X} -> 0x{val:08X} ({name})")
    
    prologues=find_x86_prologues(data)
    print(f"X86 function prologues: {len(prologues)}")
    if prologues:
        for off,desc in prologues[:10]:
            print(f"  0x{off:05X}: {desc} at VA 0x{base+off:08X}")
    
    strs=extract_strings(data, 5)
    print(f"Printable strings (len>=5): {len(strs)}")
    for off,s in strs[:20]:
        try:
            print(f"  0x{off:05X} (VA 0x{base+off:08X}): {s.decode()}")
        except:
            pass
    
    # Try to detect PE
    if data[:2]==b'MZ':
        print("  Has MZ header - valid PE!")
        if len(data)>0x3C:
            lfanew=struct.unpack('<I', data[0x3C:0x40])[0]
            if lfanew < len(data)-2 and data[lfanew:lfanew+2]==b'PE':
                print(f"  Has PE header at 0x{lfanew:X}")
    else:
        # Search for MZ inside
        for i in range(len(data)-2):
            if data[i:i+2]==b'MZ':
                print(f"  Found embedded MZ at offset 0x{i:X} VA 0x{base+i:08X}")
                break

    return data

def main():
    base_dir=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    dumps=[
        (os.path.join(base_dir, "aimware2016/b34E10000.h"), 0x34E10000, "CODE"),
        (os.path.join(base_dir, "aimware2016/b43AF0000.h"), 0x43AF0000, "DATA"),
        (os.path.join(base_dir, "aimware2016/b76ED0000.h"), 0x76ED0000, "STRING"),
        (os.path.join(base_dir, "aimware2016/b7C4A0000.h"), 0x7C4A0000, "CRT"),
    ]
    
    regions=[
        (0x34E10000, 0x34E10000+192512, "CODE"),
        (0x43AF0000, 0x43AF0000+90112, "DATA"),
        (0x76ED0000, 0x76ED0000+110592, "STRING"),
        (0x7C4A0000, 0x7C4A0000+122880, "CRT"),
    ]
    
    print("Aimware 2016 Dump Analyzer")
    print("Analyzing 4 memory dumps...")
    
    all_data={}
    for path, base, name in dumps:
        if not os.path.exists(path):
            print(f"File not found: {path}")
            continue
        data=analyze_dump(path, base, regions)
        all_data[name]=data
    
    # Cross-reference
    print(f"\n{'='*60}")
    print("Cross-reference analysis:")
    if "DATA" in all_data:
        data=all_data["DATA"]
        # Look for interface strings
        # Interfaces are stored as pointers to strings? Check
        # Actually interfaces table at 0x43AFF000 contains pointers to interfaces
        # Let's dump around 0x43AFF000
        offset=0x43AFF000-0x43AF0000
        if offset+0x200 < len(data):
            print(f"\nInterface table at 0x43AFF000 (offset 0x{offset:X} in DATA):")
            for i in range(0, 0x120, 4):
                val=struct.unpack('<I', data[offset+i:offset+i+4])[0]
                print(f"  0x{0x43AFF000+i:08X}: 0x{val:08X}")

if __name__=="__main__":
    main()
