/*
  DOS support for Crystal Space input library
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Slavik Levtchenko <Smirnov@bbs.math.spbu.ru>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __WATDPMI_H__
#define __WATDPMI_H__

struct __dpmi_regs
{
  unsigned long edi,esi,ebp,res,ebx,edx,ecx,eax;
  unsigned short flags, es, ds, fs, gs, ip, cs, sp, ss;
};

void DebugClick ();
#pragma aux DebugClick =                \
               "push   eax"             \
               "push   ecx"             \
               "mov    al,0xb6"         \
               "out    0x43, al"        \
               "mov    al,8"            \
               "out    0x42,al"         \
               "out    0x42,al"         \
               "in     al,0x61"         \
               "or     al,3"            \
               "out    0x61,al"         \
               "mov    ecx,0x1000000"   \
"loop_here:     dec    ecx"             \
               "jne    loop_here"       \
               "and    al,0xfc"         \
               "out    0x61,al"         \
               "pop    ecx"             \
               "pop    eax";

void LockMemory (void *Addr, int Size);
#pragma aux LockMemory parm [eax] [ecx] = \
                "push   eax"              \
                "push   ecx"              \
                "mov    ax,0x0006"        \
                "mov    bx,ds"            \
                "int    0x31"             \
                "shl    ecx,16"           \
                "mov    cx,dx"            \
                "pop    eax"              \
                "add    ecx,eax"          \
                "shld   ebx,ecx,16"       \
                "pop    edi"              \
                "shld   esi,edi,16"       \
                "mov    ax,0x0600"        \
                "int    0x31"             \
                modify [eax ebx ecx esi edi];

void UnlockMemory (void *Addr, int Size);
#pragma aux UnlockMemory parm [eax] [ecx] = \
                "push   eax"              \
                "push   ecx"              \
                "mov    ax,0x0006"        \
                "mov    bx,ds"            \
                "int    0x31"             \
                "shl    ecx,16"           \
                "mov    cx,dx"            \
                "pop    eax"              \
                "add    ecx,eax"          \
                "shld   ebx,ecx,16"       \
                "pop    edi"              \
                "shld   esi,edi,16"       \
                "mov    ax,0x0601"        \
                "int    0x31"             \
                modify [eax ebx ecx esi edi];

unsigned char GetIRQ (char IRQ);
#pragma aux GetIRQ parm [eax] =           \
                "push   eax"              \
                "mov    ax, 0x0400"       \
                "int    0x31"             \
                "pop    eax"              \
                "add    al,dh"            \
                value [al] modify [eax ebx edx ecx esi edi];

void GetVect (unsigned char Int, void (__interrupt __far **IntVec) ());
#pragma aux GetVect parm [bl] [edi] =     \
                "mov    ax,0x0204"        \
                "int    0x31"             \
                "mov    [edi], edx"       \
                "mov    [edi+4], cx"      \
                modify [eax ecx edx];

void SetVect (unsigned char Int, void (__interrupt __far *IntVec) ());
#pragma aux SetVect parm [bl] [edx] =     \
                "mov    cx, cs"           \
                "mov    ax,0x0205"        \
                "int    0x31"             \
                modify [eax ecx];

void AllocateCallback (__dpmi_regs *regs, void (__interrupt __far *Callback) (),
  void **DosAddress);
#pragma aux AllocateCallback parm [edi] [esi] [ebx] = \
                "mov    ax, 0x303"                    \
                "int    0x31"                         \
                "mov    [ebx],dx"                     \
                "mov    [ebx+2],cx"                   \
                modify [eax];

void DosInt (int Interrupt, __dpmi_regs *regs);
#pragma aux DosInt parm [ebx] [edi] =           \
                "mov     word ptr [edi+0x2e],0" \
                "mov     word ptr [edi+0x30],0" \
                "mov     word ptr [edi+0x20],0" \
                "mov     eax, 0x0300"           \
                "xor     ecx, ecx"              \
                "int     0x31"                  \
                modify [eax ecx];

extern int mTaskSystem, mTaskVersion;
const mtNothing = 0;
const mtWindows = 1;
const mtOS2 = 2;

void DetectMultiTask ();
#pragma aux DetectMultiTask =                   \
               "mov     ax,0x1600"              \
               "int     0x2F"                   \
               "mov     bx,0x0200"              \
               "cmp     al,1"                   \
               "je      setWin"                 \
               "cmp     al,0x0FF"               \
               "je      setWin"                 \
               "mov     ax,0x160A"              \
               "xor     bx,bx"                  \
               "int     0x2F"                   \
               "test    bx,bx"                  \
               "jz      noWindows"              \
"setWin:        mov     mTaskSystem,1"          \
               "movzx   ebx,bx"                 \
               "mov     mTaskVersion,ebx"       \
               "jmp     locEx"                  \
"noWindows:     clc"                            \
               "mov     ax,0x3306"              \
               "int     0x21"                   \
               "jc      DRDOS"                  \
"noDRDOS:       cmp     al,0x0FF"               \
               "je      locEx"                  \
               "cmp     bh,100"                 \
               "jae     locEx"                  \
               "cmp     bl,10"                  \
               "jb      checkHLT"               \
               "mov     al,bl"                  \
               "mov     ah,0"                   \
               "mov     bl,10"                  \
               "div     bl"                     \
               "mov     ah,bh"                  \
               "xchg    al,ah"                  \
"setOS2:        mov     mTaskSystem,2"          \
               "movzx   eax,ax"                 \
               "mov     mTaskVersion,eax"       \
               "jmp     locEx"                  \
"checkHLT:      xor     dx,dx"                  \
               "mov     ax,1"                   \
               "hlt"                            \
               "db      0x35,0xCA,0xEB,1,0xC0"  \
               "cmp     ax,1"                   \
               "ja      locEx"                  \
               "mov     ax,0x0201"              \
               "jmp     setOS2"                 \
"DRDOS:         cmp     ax,1"                   \
               "jne     noDRDOS"                \
"locEx:" modify [eax ebx ecx edx];

void GiveUpTimeSlice ();
#pragma aux GiveUpTimeSlice =                   \
               "mov     al,byte ptr mTaskSystem"\
               "cmp     al,byte mtOS2"          \
               "jne     noOS2"                  \
               "xor     dx,dx"                  \
               "mov     ax,1"                   \
               "hlt"                            \
               "db      0x35,0xCA"              \
               "clc"                            \
               "jmp     locEx"                  \
"noOS2:         cmp     al,byte mtWindows"      \
               "jne     locEx"                  \
               "mov     ax,0x1680"              \
               "int     0x2F"                   \
               "clc"                            \
               "ret"                            \
"locEx:" modify [eax ecx edx];

#endif // __WATDPMI_H__
