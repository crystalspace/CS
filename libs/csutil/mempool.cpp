/*
    Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>

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

#include "csutil/mempool.h"
#include <string.h>

void* csMemoryPool::Alloc(size_t n)
{
  CS_ASSERT(n > 0);
  uint8* p;
  if (n > granularity) // Too big for any block, allocate a big one specially.
  {
    p = new uint8[n];
    blocks.Insert(0,p); // Don't disturb `remaining' which refers to end block.
  }
  else
  {
    if (n > remaining) // Last block has insufficient free, so allocate new.
    {
      blocks.Push(new uint8[granularity]);
      remaining = granularity;
    }
    CS_ASSERT(blocks.Length() > 0);
    p = blocks[blocks.Length() - 1] + granularity - remaining;
    remaining -= n;
  }
  return p;
}

void csMemoryPool::Empty()
{
  blocks.Empty();
  remaining = 0;
}

void const* csMemoryPool::Store(void const* m, size_t n)
{
  void* p = Alloc(n);
  CS_ASSERT(p != 0);
  memcpy(p, m, n);
  return p;
}

char const* csMemoryPool::Store(char const* s)
{
  return (char const*)(s != 0 ? Store(s, strlen(s) + 1) : Store("",1));
}
