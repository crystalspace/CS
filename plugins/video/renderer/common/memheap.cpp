/*
    Copyright (C) 1998 by Olivier Langlois <olanglois@sympatico.ca>

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

#include "sysdef.h"
#include "cs3d/common/memheap.h"

#if defined( OS_MACOS )
#  include <string.h>
#else
#  include <memory.h>
#endif // OS_MACOS

/* Definitions & Macros */

#define MINSIZE     (sizeof (bufhd)+56)

#define HEAP_START      ((heap *)memory)->start
#define HEAP_FIRST_FREE ((heap *)memory)->first_free
#define HEAP_END        ((heap *)memory)->end

/* a ==> Offset of the buffer header */
#define HEAPBUF_SIZE(a) ((bufhd *)(memory + (ULong)(a)))->size
#define HEAPBUF_FULL(a) ((bufhd *)(memory + (ULong)(a)))->full
#define NEXT(a)         (ULong)(a + HEAPBUF_SIZE(a))
#define OFFTOADDR(a)    (char *)(memory + (ULong)(a) + sizeof(bufhd))

/* a ==> Address of the buffer header */
#define ADDRTO_OFF(a)   (char *)(a) - memory - sizeof(bufhd)

/*---------------------------------------------------------------------------*/

MemoryHeap::MemoryHeap( size_t s )
{
  cache_size = (s+7)&~7;    // Round to higher multiple of 8
  CHK (memory = new char[cache_size]);

  HEAP_START      = sizeof(heap);
  HEAP_FIRST_FREE = HEAP_START;
  HEAP_END        = HEAP_START;

  /* Initialize first buffer */
  HEAPBUF_SIZE(HEAP_START) = (ULong)(cache_size - sizeof(heap));
  HEAPBUF_FULL(HEAP_START) = 0;
}

MemoryHeap::~MemoryHeap()
{
	if (memory) CHKB (delete [] memory);
}

char *MemoryHeap::alloc( size_t s )
{
  ULong cur;
  ULong newBuf;

  if ( !s ) return NULL; // Silly !!

  s  = (s+7)&~7;
  s += sizeof(bufhd); /* sizeof(bufhd) == 8 */

  for( cur = HEAP_FIRST_FREE; cur <= HEAP_END; )
  {
    if ( !HEAPBUF_SIZE(cur) )/* Garbled buffer header !! */
    {
      return NULL;
      }
    /*
     * If the current buffer is empty and the next buffer isn't the last one
     * and it is also empty then
     */
      else if ( !HEAPBUF_FULL(cur) && NEXT(cur) <= HEAP_END && !HEAPBUF_FULL(NEXT(cur)) )
    {
          if ( NEXT(cur) == HEAP_END )
      {
        /* Change the heap's end pointer from next buffer to current buffer */
        HEAP_END = cur;
      }

      /* Add next buffer size to current buffer size */
      HEAPBUF_SIZE(cur) += HEAPBUF_SIZE(NEXT(cur));
    }
      else if ( !HEAPBUF_FULL(cur) &&
            HEAPBUF_SIZE(cur) >= s )
    {
      /* Found space for buffer */
      HEAPBUF_FULL(cur) = 1;

      /* Room for next ? */
      if ( HEAPBUF_SIZE(cur) - s >= MINSIZE )
      {
        newBuf = cur + s; /* buffer */

        /* Fill up the new empty buffer */
        HEAPBUF_SIZE(newBuf) = HEAPBUF_SIZE(cur) - s;
        HEAPBUF_FULL(newBuf) = 0;
        HEAPBUF_SIZE(cur)    = s;

        if ( cur == HEAP_END )
        {
          /* Current is the last buffer ? */
          HEAP_END = newBuf;
        }
        if ( cur == HEAP_FIRST_FREE )
        {
          /*
           * It is not sure that the next buffer is free but
           * I don't want to waste too much time to find the next
           * free buffer since it can change with a free() or a realloc()
           * call anyway !!!
           */
          HEAP_FIRST_FREE = newBuf;
        }
      }

      return (OFFTOADDR(cur));
    }
    else
      cur = NEXT(cur);
  }
  return NULL; /* No room left ... */
}

char *MemoryHeap::realloc( void *buf, size_t ns )
{
  ULong  cur;
  ULong  newBuf;
  ULong  next;
  char  *allocatedBuf;

  cur = ADDRTO_OFF(buf);

  /* Valid buffer ? */
  if ( cur < HEAP_START || cur > HEAP_END )
    return NULL;

  if ( !ns ) // Silly
  {
    MemoryHeap::free(buf);
    return NULL;
  }

  ns  = (ns+7)&~7;
  ns += sizeof(bufhd); /* sizeof(bufhd) == 8 */

  /* Shrink current buffer room for next buffer */
  if ( HEAPBUF_SIZE(cur) >= ns )
  {
    if ( HEAPBUF_SIZE(cur) - ns >= MINSIZE )
    {
      newBuf = cur + ns;

      /* Fill up the new empty buffer */
      HEAPBUF_SIZE(newBuf) = HEAPBUF_SIZE(cur) - ns;
      HEAPBUF_SIZE(newBuf) = 0;
      HEAPBUF_SIZE(cur)    = ns;

      /* Current is the last buffer ? */
      if ( cur == HEAP_END )
        HEAP_END = newBuf;

      /* new buffer is the first free buffer ? */
      if ( newBuf < HEAP_FIRST_FREE )
        HEAP_FIRST_FREE = newBuf;
    }
    return (char *)buf;
  }

  /* Can we extend the current buffer ? */

  next = NEXT(cur);

  if ( !HEAPBUF_FULL(next) )
  {
    while( (next <= HEAP_END) && (!HEAPBUF_FULL(next)) )
    {
      if ( !HEAPBUF_SIZE(next) ) /* Garbled buffer header ! */
        return NULL;

      /* Merge adjacent empty buffer */
            HEAPBUF_SIZE(cur) += HEAPBUF_SIZE(next);

      /* next is the last buffer ? */
      if ( next == HEAP_END )
      {
        HEAP_END = cur;
      }
      else
      {
        next = NEXT(next);
      }
    }
  }

  /* Found space for buffer !! */
  if ( HEAPBUF_SIZE(cur) >= ns )
  {
    /* room for a new buffer? */
    if ( HEAPBUF_SIZE(cur) - ns >= MINSIZE )
    {
      newBuf = cur + ns;

      /* Fill up the new empty buffer */

      HEAPBUF_SIZE(newBuf) = HEAPBUF_SIZE(cur) - ns;
      HEAPBUF_FULL(newBuf) = 0;
      HEAPBUF_SIZE(cur)    = ns;

      /* Current buffer is the last buffer ? */
      if ( cur == HEAP_END )
      {
        HEAP_END = newBuf;
      }
      if ( newBuf < HEAP_FIRST_FREE )
        HEAP_FIRST_FREE = newBuf;
    }
    else
    {
      /* First free buffer has been merged ? */
      if ( HEAP_FIRST_FREE > cur && HEAP_FIRST_FREE < NEXT(cur) )
      {
               if ( cur == HEAP_END ) /* No more free buffer !!! */
           HEAP_FIRST_FREE = cur;
         else
           HEAP_FIRST_FREE = NEXT(cur);
      }
    }
    return (char *)buf;
  }

  /* Must allocate new buffer */

  /* First free buffer has been merged ? */
  if ( HEAP_FIRST_FREE > cur && HEAP_FIRST_FREE < NEXT(cur) )
  {
       if ( cur == HEAP_END ) /* No more free buffer !!! */
     {
       HEAP_FIRST_FREE = cur;
     return NULL;
     }
     else
     HEAP_FIRST_FREE = NEXT(cur);
  }

  ns -= sizeof(bufhd);

  if ( (allocatedBuf = alloc(ns)) != NULL )
  {
    memcpy( allocatedBuf, buf, HEAPBUF_SIZE(cur)-sizeof(bufhd) );
    MemoryHeap::free(buf); /* free old buffer */
  }

  return allocatedBuf;
}

int MemoryHeap::free( void *buf )
{
  ULong cur;

  cur = ADDRTO_OFF(buf);

  /* Valid buffer ? */
  if ( cur < HEAP_START || cur > HEAP_END )
    return -1;

  HEAPBUF_FULL(cur) = 0;

  if ( cur < HEAP_FIRST_FREE )
    HEAP_FIRST_FREE = cur;

  return 0;
}
