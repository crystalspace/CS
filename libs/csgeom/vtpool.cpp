/*
  Copyright (C) 2001 by Jorrit Tyberghein

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

#include "cssysdef.h"
#include "csgeom/vtpool.h"

csDefaultVertexArrayPool csDefaultVertexArrayPool::default_pool;
csPooledVertexArrayPool csPooledVertexArrayPool::default_pool;

csDefaultVertexArrayPool::csDefaultVertexArrayPool ()
{
}

csPooledVertexArrayPool::csPooledVertexArrayPool ()
{
  miscpool = NULL;
  int i;
  for (i = 0 ; i < 6 ; i++)
    pool[i] = NULL;
}

csPooledVertexArrayPool::~csPooledVertexArrayPool ()
{
  int i;
  for (i = 0 ; i < 6 ; i++)
    while (pool[i])
    {
      PoolEl* pel = pool[i]->next;
      free (pool[i]);
      pool[i] = pel;
    }
  while (miscpool)
  {
    PoolEl* pel = miscpool->next;
    free (miscpool);
    miscpool = pel;
  }
}

csVector3* csPooledVertexArrayPool::GetVertexArray (int n)
{
  if (n >= 3 && n <= 8)
  {
    if (!pool[n-3])
    {
      PoolEl* pel = (PoolEl*)malloc (sizeof (PoolEl)+(n-1)*sizeof (csVector3));
      pel->n = n;
      return &pel->first_vertex;
    }
    else
    {
      PoolEl* pel = pool[n-3];
      pool[n-3] = pool[n-3]->next;
      return &pel->first_vertex;
    }
  }
  else
  {
    if (!miscpool)
    {
      PoolEl* pel = (PoolEl*)malloc (sizeof (PoolEl)+(n-1)*sizeof (csVector3));
      pel->n = n;
      return &pel->first_vertex;
    }
    else
    {
      PoolEl* pel = miscpool;
      miscpool = miscpool->next;
      if (n > pel->n)
      {
        // First reallocate.
        pel = (PoolEl*)realloc (pel, sizeof (PoolEl)+(n-1)*sizeof (csVector3));
	pel->n = n;
      }
      return &pel->first_vertex;
    }
  }
}

void csPooledVertexArrayPool::FreeVertexArray (csVector3* ar, int n)
{
  if (!ar) return;
  PoolEl p;
  PoolEl* pel = (PoolEl*)(((char*)ar)-(((long)&p.first_vertex) - (long)&p.next));
  if (n >= 3 && n <= 8)
  {
    pel->next = pool[n-3];
    pool[n-3] = pel;
  }
  else
  {
    pel->next = miscpool;
    miscpool = pel;
  }
}

