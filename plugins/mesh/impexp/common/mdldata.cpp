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
#include "mdldata.h"

#define IMPLEMENT_VECTOR_INTERFACE(clname,type,name)			\
  int clname::GetNum##name##s () const					\
  { return name##s.Length (); }						\
  i##type* clname::Get##name (int n)					\
  { return name##s.Get (n); }						\
  i##type* clname::Create##name ()					\
  { cs##type *obj = new cs##type(); name##s.Push (obj);			\
    obj->DecRef (); return obj; }					\
  void clname::Delete##name (int n)					\
  { name##s.Delete (n); }

#define IMPLEMENT_ARRAY_INTERFACE_NONUM(clname,type,name)		\
  type clname::Get##name (int n) const					\
  { return name##List[n]; }						\
  void clname::Set##name (int n, type val)				\
  { name##List[n] = val; }

#define IMPLEMENT_ARRAY_INTERFACE(clname,type,name)			\
  IMPLEMENT_ARRAY_INTERFACE_NONUM (clname, type, name)			\
  int clname::GetNum##name##s () const					\
  { return name##List.Length (); }

#define IMPLEMENT_ACCESSOR_METHOD(clname,type,name)			\
  type clname::Get##name () const					\
  { return name; }							\
  void clname::Set##name (type val)					\
  { name = val; }

#define IMPLEMENT_ACCESSOR_METHOD_REF(clname,type,name)			\
  type clname::Get##name () const					\
  { return name; }							\
  void clname::Set##name (type val)					\
  { if (name) name->DecRef (); name = val; if (name) name->IncRef (); }

/*** csModelDataPolygon ***/

IMPLEMENT_IBASE (csModelDataPolygon)
  IMPLEMENTS_INTERFACE (iModelDataPolygon)
IMPLEMENT_IBASE_END

csModelDataPolygon::csModelDataPolygon ()
{
  CONSTRUCT_IBASE (NULL);
  Material = NULL;
}

csModelDataPolygon::~csModelDataPolygon ()
{
  if (Material) Material->DecRef ();
}

int csModelDataPolygon::GetNumVertices () const
{
  return VertexList.Length ();
}

void csModelDataPolygon::AddVertex (const csVector3 &Position,
  const csVector3 &Normal, const csColor &Color, const csVector2 &TexCoords)
{
  VertexList.Push (Position);
  NormalList.Push (Normal);
  ColorList.Push (Color);
  TextureCoordsList.Push (TexCoords);
}

void csModelDataPolygon::DeleteVertex (int n)
{
  VertexList.Delete (n);
  NormalList.Delete (n);
  ColorList.Delete (n);
  TextureCoordsList.Delete (n);
}

IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, const csVector3 &, Vertex);
IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, const csVector3 &, Normal);
IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, const csVector2 &, TextureCoords);
IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, const csColor &, Color);
IMPLEMENT_ACCESSOR_METHOD (csModelDataPolygon, iModelDataMaterial *, Material);

/*** csModelDataObject ***/

IMPLEMENT_IBASE (csModelDataObject)
  IMPLEMENTS_INTERFACE (iModelDataObject)
IMPLEMENT_IBASE_END

IMPLEMENT_VECTOR_INTERFACE (csModelDataObject, ModelDataPolygon, Polygon);

/*** csModelDataCamera ***/

IMPLEMENT_IBASE (csModelDataCamera)
  IMPLEMENTS_INTERFACE (iModelDataCamera)
IMPLEMENT_IBASE_END

IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, Position);
IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, UpVector);
IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, FrontVector);
IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, RightVector);

void csModelDataCamera::ComputeUpVector ()
{
  UpVector = FrontVector % RightVector;
}

void csModelDataCamera::ComputeFrontVector ()
{
  FrontVector = RightVector % UpVector;
}

void csModelDataCamera::ComputeRightVector ()
{
  RightVector = UpVector % FrontVector;
}

void csModelDataCamera::Normalize ()
{
  UpVector.Normalize ();
  FrontVector.Normalize ();
  RightVector.Normalize ();
}

bool csModelDataCamera::CheckOrthogonality () const
{
  float x = UpVector * FrontVector;
  float y = RightVector * FrontVector;
  float z = UpVector * RightVector;
  return (ABS(x) < SMALL_EPSILON) && (ABS(y) < SMALL_EPSILON) &&
    (ABS(z) < SMALL_EPSILON);
}

/*** csModelDataLight ***/

IMPLEMENT_IBASE (csModelDataLight)
  IMPLEMENTS_INTERFACE (iModelDataLight)
IMPLEMENT_IBASE_END

IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, float, Radius);
IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, const csColor &, Color);

/*** csModelDataMaterial ***/

IMPLEMENT_IBASE (csModelDataMaterial)
  IMPLEMENTS_INTERFACE (iModelDataMaterial)
IMPLEMENT_IBASE_END

/*** csModelData ***/

IMPLEMENT_IBASE (csModelData)
  IMPLEMENTS_INTERFACE (iModelData)
IMPLEMENT_IBASE_END

IMPLEMENT_VECTOR_INTERFACE (csModelData, ModelDataObject, Object);
IMPLEMENT_VECTOR_INTERFACE (csModelData, ModelDataCamera, Camera);
IMPLEMENT_VECTOR_INTERFACE (csModelData, ModelDataLight, Light);
