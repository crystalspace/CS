/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "litmeshsel.h"

bool litMeshSelectAnd::SelectMesh (iMeshWrapper* mesh)
{
  size_t i;
  for (i = 0 ; i < a.Length () ; i++)
  {
    bool rc = a[i]->SelectMesh (mesh);
    if (!rc) return false;
  }
  return true;
}

bool litMeshSelectOr::SelectMesh (iMeshWrapper* mesh)
{
  size_t i;
  for (i = 0 ; i < a.Length () ; i++)
  {
    bool rc = a[i]->SelectMesh (mesh);
    if (rc) return true;
  }
  return false;
}

bool litMeshSelectByType::SelectMesh (iMeshWrapper* mesh)
{
  iMeshObjectFactory* factory = mesh->GetMeshObject ()->GetFactory ();
  iMeshObjectType* otype = factory->GetMeshObjectType ();
  csRef<iFactory> ifact = SCF_QUERY_INTERFACE (otype, iFactory);
  if (!ifact) return false;
  printf ("%s\n", ifact->QueryDescription ()); fflush (stdout);
  // @@@ Do strcmp.
  return false;
}


