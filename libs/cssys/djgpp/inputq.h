/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __INPUTQ_H__
#define __INPUTQ_H__

#pragma pack(1)

#define EVENT_TYPE_KEYBOARD	1
#define EVENT_TYPE_MOUSE	2

#define EVENT_QUEUE_MASK	0x3f

/// Events ring buffer structure
struct RawEventQueue
{
  unsigned short Type;
  union
  {
    struct
    {
      char Button;
      bool Down;
      int x, y;
    } Mouse;
    struct
    {
      unsigned char ScanCode;
    } Keyboard;
  };
};

/// Mouse and keyboard event queue
extern volatile RawEventQueue event_queue[];

/// Event buffer head and tail pointer
extern volatile int event_queue_head, event_queue_tail;

#pragma pack()

#endif // __INPUTQ_H__

