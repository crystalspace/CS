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

#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include "watdpmi.h"
#include "keyboard.h"

unsigned char keyboard_queue [64];
int keyboard_queue_head, keyboard_queue_tail;
static void (__interrupt __far *kbOldInterrupt) () = NULL;

void __interrupt __far kbInterrupt ()
{
  unsigned char c = inp(0x60);
  if (c > 0xe1)
  {
    kbOldInterrupt ();
    return;
  }

  keyboard_queue [keyboard_queue_head] = c;
  keyboard_queue_head = (keyboard_queue_head + 1) & KEYBOARD_QUEUE_MASK;

  unsigned char status = inp(0x61);
  outp(0x61, status|0x80);
  outp(0x61, status);
  outp(0x20, 0x20);
}

void InstallKeyboardHandler ()
{
  atexit (&DeinstallKeyboardHandler);
  LockMemory (&keyboard_queue, 0x1000);
  LockMemory ((void *)&kbInterrupt, 0x1000);

  keyboard_queue_head = keyboard_queue_tail = 0;

  unsigned char irq1 = GetIRQ (1);
  GetVect (irq1, &kbOldInterrupt);
  SetVect (irq1, &kbInterrupt);
}

void DeinstallKeyboardHandler ()
{
  if (kbOldInterrupt)
  {
    unsigned char irq1 = GetIRQ (1);
    SetVect (irq1, *kbOldInterrupt);
    kbOldInterrupt = NULL;

    UnlockMemory ((void *)&kbInterrupt, 0x1000);
    UnlockMemory (&keyboard_queue, 0x1000);
  }
}

#if 0

#include <stdio.h>

int main ()
{
  InstallKeyboardHandler();

  for (;;)
  {
    while (keyboard_queue_head != keyboard_queue_tail)
    {
       unsigned int c = keyboard_queue [keyboard_queue_tail];
       keyboard_queue_tail = (keyboard_queue_tail + 1) & KEYBOARD_QUEUE_MASK;
       if (c == 0x81)
         goto done;
    }
  }

done:
  DeinstallKeyboardHandler ();
  return 0;
}

#endif
