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

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

extern unsigned char keyboard_queue [64];
extern int keyboard_queue_head, keyboard_queue_tail;

extern void InstallKeyboardHandler ();
extern void DeinstallKeyboardHandler ();

#define KEYBOARD_QUEUE_MASK (sizeof (keyboard_queue) - 1)

#endif // __KEYBOARD_H__
