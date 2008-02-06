/*
    Copyright (C) 2007 by Jorrit Tyberghein

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
#include "cstool/objmodel.h"

class csTMIterator : public scfImplementation1<csTMIterator,
  iTriangleMeshIterator>
{
private:
  // Keep a ref to the object model to guarantee it stays alive
  // during iteration.
  csRef<csObjectModel> model;
  csHash<csRef<iTriangleMesh>,csStringID>::GlobalIterator it;

public:
  csTMIterator (csObjectModel* model) : scfImplementationType (this),
    model (model), it (model->trimesh.GetIterator ())
  {
  }
  virtual ~csTMIterator ()
  {
  }
  virtual bool HasNext ()
  {
    return it.HasNext ();
  }
  virtual iTriangleMesh* Next (csStringID& id)
  {
    return it.Next (id);
  }
};

iTriangleMesh* csObjectModel::GetTriangleData (csStringID id)
{
  return trimesh.Get (id, 0);
}

csPtr<iTriangleMeshIterator> csObjectModel::GetTriangleDataIterator ()
{
  csTMIterator* it = new csTMIterator (this);
  return it;
}

void csObjectModel::SetTriangleData (csStringID id, iTriangleMesh* tridata)
{
  trimesh.PutUnique (id, tridata);
}

bool csObjectModel::IsTriangleDataSet (csStringID id)
{
  return trimesh.Contains (id);
}

void csObjectModel::ResetTriangleData (csStringID id)
{
  trimesh.DeleteAll (id);
}

