/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "csengine/sector.h"
#include "csengine/meshobj.h"
#include "igraph3d.h"

IMPLEMENT_CSOBJTYPE (csMeshObject, csSprite)

csMeshObject::csMeshObject (csObject* theParent, iMeshObject* mesh)
	: csSprite (theParent), bbox (NULL)
{
  bbox.SetOwner (this);
  ptree_obj = &bbox;
  csMeshObject::mesh = mesh;
  mesh->IncRef ();
}

csMeshObject::~csMeshObject ()
{
  mesh->DecRef ();
}

void csMeshObject::ScaleBy (float factor)
{
  csMatrix3 trans = movable.GetTransform ().GetT2O ();
  trans.m11 *= factor;
  trans.m22 *= factor;
  trans.m33 *= factor;
  movable.SetTransform (trans);
  UpdateMove ();
}


void csMeshObject::Rotate (float angle)
{
  //csZRotMatrix3 rotz (angle);
  //movable.Transform (rotz);
  //csXRotMatrix3 rotx (angle);
  //movable.Transform (rotx);
  //UpdateMove ();
}


void csMeshObject::SetColor (const csColor& col)
{
  //for (int i=0; i<tpl->GetNumTexels (); i++)
    //SetVertexColor (i, col);
}


void csMeshObject::AddColor (const csColor& col)
{
  //for (int i=0; i<tpl->GetNumTexels (); i++)
    //AddVertexColor (i, col);
}


void csMeshObject::UpdateInPolygonTrees ()
{
  bbox.RemoveFromTree ();
}


void csMeshObject::Draw (csRenderView& rview)
{
  mesh->Draw (QUERY_INTERFACE ((&rview), iRenderView), QUERY_INTERFACE ((&movable), iMovable));
}


void csMeshObject::UpdateLighting (csLight** lights, int num_lights)
{
  defered_num_lights = 0;
}


bool csMeshObject::HitBeamObject (const csVector3& start, const csVector3& end,
	csVector3& isect, float* pr)
{
  return false;
}

