/*
  DOS support for Crystal Space input library
  Copyright (C) 1998 by Jorrit Tyberghein

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

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

#include "watdpmi.h"
#include "mouse.h"

// Mouse events ring buffer
volatile mouse_queue_element mouse_queue [64];
// Mouse events buffer top and bottom pointer
volatile int mouse_queue_head, mouse_queue_tail;
// DOS registers when mouse callback is called
static __dpmi_regs ms_regs;
// DOS address of mouse callback
static void *ms_callback = 0;

extern "C" void cdecl _GETDS ();

void aux_msInterrupt ();
#pragma aux aux_msInterrupt =                           \
               "push    eax"                            \
               "push    ebx"                            \
               "push    ecx"                            \
               "push    ds"                             \
               "call    _GETDS"                         \
               "mov     eax, [esi]"                     \
               "mov     es:[edi+0x2a], eax"             \
               "add     word ptr es:[edi+0x2e], 4"      \
                                                        \
               "mov     ebx, mouse_queue_head"          \
                                                        \
               "test    es:[edi+0x1c], 1"               \
               "jz      no_ms_move"                     \
               "lea     ecx, [ebx+ebx*4]"               \
               "shl     ecx, 1"                         \
               "mov     word ptr mouse_queue [ecx+8], 0"\
               "movzx   eax, word ptr es:[edi+0x18]"    \
               "mov     dword ptr mouse_queue [ecx+0], eax"\
               "movzx   eax, word ptr es:[edi+0x14]"    \
               "mov     dword ptr mouse_queue [ecx+4], eax"\
               "inc     ebx"                            \
               "and     bl, byte MOUSE_QUEUE_MASK"      \
                                                        \
"no_ms_move:    mov     al, es:[edi+0x1c]"              \
               "test    al, 0x06"                       \
               "jz      no_lmb_changed"                 \
               "test    al, 0x02"                       \
               "setnz   ah"                             \
               "mov     al, 1"                          \
               "jmp     button_down"                    \
                                                        \
"no_lmb_changed:test    al, 0x18"                       \
               "jz      no_rmb_changed"                 \
               "test    al, 0x08"                       \
               "setnz   ah"                             \
               "mov     al, 2"                          \
               "jmp     button_down"                    \
                                                        \
"no_rmb_changed:test    al, 0x60"                       \
               "jz      no_mmb_changed"                 \
               "test    al, 0x20"                       \
               "setnz   ah"                             \
               "mov     al, 3"                          \
                                                        \
"button_down:   lea     ecx, [ebx+ebx*4]"               \
               "shl     ecx, 1"                         \
               "mov     word ptr mouse_queue [ecx+8], ax"\
               "movzx   eax, word ptr es:[edi+0x18]"    \
               "mov     dword ptr mouse_queue [ecx+0], eax"\
               "movzx   eax, word ptr es:[edi+0x14]"    \
               "mov     dword ptr mouse_queue [ecx+4], eax"\
               "inc     ebx"                            \
               "and     bl, byte MOUSE_QUEUE_MASK"      \
                                                        \
"no_mmb_changed:mov     mouse_queue_head, ebx"          \
               "pop     ds"                             \
               "pop     ecx"                            \
               "pop     ebx"                            \
               "pop     eax"                            \
               "iretd";

#pragma off (check_stack);

void msInterrupt ()
{
  DebugClick ();
  aux_msInterrupt ();
}

#pragma on (check_stack);

void InstallMouseHandler ()
{
  if (sizeof (mouse_queue_element) != 10)
  {
    printf ("Your compiler does not support packed structures!\n");
    exit (-1);
  }
return;
  atexit (&DeinstallMouseHandler);
  LockMemory (&mouse_queue, 0x1000);
  LockMemory ((void *)&msInterrupt, 0x1000);

  mouse_queue_head = mouse_queue_tail = 0;

  AllocateCallback (&ms_regs, (void __interrupt far (*) ())&msInterrupt,
    &ms_callback);

  __dpmi_regs regs;
  regs.eax = 0x000c;
  regs.ecx = 0x007f;
  regs.edx = ((int)ms_callback) & 0xffff;
  regs.es = ((int)ms_callback) >> 16;
  DosInt (0x33, &regs);
}

void DeinstallMouseHandler ()
{
  if (!ms_callback)
    return;

  __dpmi_regs regs;
  regs.eax = 0x000c;
  regs.ecx = 0x0000;
  regs.edx = 0;
  regs.es = 0;
  DosInt (0x33, &regs);

//DeallocateCallback (ms_callback);
  ms_callback = NULL;

  UnlockMemory ((void *)&msInterrupt, 0x1000);
  UnlockMemory (&mouse_queue, 0x1000);
}

bool MousePresent ()
{
  union REGS inregs, outregs;
  inregs.x.eax=0x00;
  int386 (0x33, &inregs, &outregs);
  return (outregs.x.eax != 0);
}

#if 0

int main ()
{
  InstallMouseHandler ();
  for (;;)
  {
    while (mouse_queue_head != mouse_queue_tail)
    {
      printf ("%d[%d]: %d,%d\n",
        mouse_queue [mouse_queue_tail].button,
        mouse_queue [mouse_queue_tail].down,
        mouse_queue [mouse_queue_tail].x,
        mouse_queue [mouse_queue_tail].y);
      mouse_queue_tail = (mouse_queue_tail + 1) & MOUSE_QUEUE_MASK;
      if (mouse_queue [mouse_queue_tail].button == 2)
        goto done;
    }
  }

done:
  DeinstallMouseHandler ();
  return 0;
}

#endif
