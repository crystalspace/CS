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

#ifndef __MOUSE_H__
#define __MOUSE_H__

#pragma pack(1)

struct mouse_queue_element
{
  int x, y;
  char button;
  bool down;
};

/// Mouse events ring buffer
extern volatile mouse_queue_element mouse_queue [64];
/// Mouse events buffer top and bottom pointer
extern volatile int mouse_queue_head, mouse_queue_tail;

#pragma pack()

extern void InstallMouseHandler ();
extern void DeinstallMouseHandler ();
extern bool MousePresent ();

const MOUSE_QUEUE_MASK = sizeof (mouse_queue) - 1;

#endif // __MOUSE_H__
