/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csutil/typedvec.h"
#include "csengine/rdrprior.h"
#include "iengine/mesh.h"

csRenderQueueSet::csRenderQueueSet ()
{
}

csRenderQueueSet::~csRenderQueueSet ()
{
}

void csRenderQueueSet::Add (iMeshWrapper *mesh)
{
  long pri = mesh->GetRenderPriority ();

  // look if the desired priority queue exists, and possibly
  // extend the list of queues
  for (int i = Queues.Length () ; i <= pri ; i++)
    Queues[i] = NULL;

  // look if the desired queue exists, and create it if not
  if (!Queues [pri])
    Queues [pri] = new csMeshVectorNodelete ();

  // add the mesh wrapper
  Queues [pri]->Push (mesh);
}

void csRenderQueueSet::Remove (iMeshWrapper *mesh)
{
  long pri = mesh->GetRenderPriority ();

  // look if the queue of the mesh exists
  if (pri < Queues.Length () && Queues [pri] != NULL)
  {
    // delete the object from the queue
    Queues [pri]->Delete (mesh);
  }
}

void csRenderQueueSet::RemoveUnknownPriority (iMeshWrapper *mesh)
{
  for (int i = 0 ; i < Queues.Length () ; i++)
  {
    if (Queues [i])
    {
      int n = Queues [i]->Find (mesh);
      if (n != -1)
      {
        Queues [i]->Delete (n);
	return;
      }
    }
  }
}
