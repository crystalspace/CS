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

#define IMPLEMENT_ARRAY_INTERFACE_NONUM(clname,type,sing_name,mult_name) \
  type clname::Get##sing_name (int n) const				\
  { return mult_name[n]; }						\
  void clname::Set##sing_name (int n, type val)				\
  { mult_name[n] = val; }

#define IMPLEMENT_ARRAY_INTERFACE(clname,type,sing_name,mult_name)	\
  IMPLEMENT_ARRAY_INTERFACE_NONUM (clname, type, sing_name, mult_name)	\
  int clname::GetNum##mult_name () const				\
  { return mult_name.Length (); }					\
  void clname::Add##sing_name (type v)					\
  { mult_name.Push (v); }

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

#define IMPLEMENT_OBJECT_INTERFACE(clname)				\
  IMPLEMENT_EMBEDDED_OBJECT (clname::Embedded_csObject);		\
  iObject* clname::QueryObject ()					\
  { return &scfiObject; }

/*** csModelDataPolygon ***/

IMPLEMENT_IBASE (csModelDataPolygon)
  IMPLEMENTS_INTERFACE (iModelDataPolygon)
  IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
IMPLEMENT_IBASE_END

IMPLEMENT_OBJECT_INTERFACE (csModelDataPolygon);

csModelDataPolygon::csModelDataPolygon ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiObject);
  Material = NULL;
}

csModelDataPolygon::~csModelDataPolygon ()
{
  if (Material) Material->DecRef ();
}

int csModelDataPolygon::GetVertexCount () const
{
  return Vertices.Length ();
}

void csModelDataPolygon::AddVertex (int PositionIndex,
  const csVector3 &Normal, const csColor &Color, const csVector2 &TexCoords)
{
  Vertices.Push (PositionIndex);
  Normals.Push (Normal);
  Colors.Push (Color);
  TextureCoords.Push (TexCoords);
}

void csModelDataPolygon::DeleteVertex (int n)
{
  Vertices.Delete (n);
  Normals.Delete (n);
  Colors.Delete (n);
  TextureCoords.Delete (n);
}

IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon,
	int, Vertex, Vertices);
IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon,
	const csVector3 &, Normal, Normals);
IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon,
	const csVector2 &, TextureCoords, TextureCoords);
IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon,
	const csColor &, Color, Colors);
IMPLEMENT_ACCESSOR_METHOD (csModelDataPolygon, iModelDataMaterial *, Material);

/*** csModelDataObject ***/

IMPLEMENT_IBASE (csModelDataObject)
  IMPLEMENTS_INTERFACE (iModelDataObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
IMPLEMENT_IBASE_END

IMPLEMENT_OBJECT_INTERFACE (csModelDataObject);

IMPLEMENT_ARRAY_INTERFACE (csModelDataObject,
	const csVector3 &, Vertex, Vertices);

csModelDataObject::csModelDataObject ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}

/*** csModelDataCamera ***/

IMPLEMENT_IBASE (csModelDataCamera)
  IMPLEMENTS_INTERFACE (iModelDataCamera)
  IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
IMPLEMENT_IBASE_END

IMPLEMENT_OBJECT_INTERFACE (csModelDataCamera);

IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, Position);
IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, UpVector);
IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, FrontVector);
IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, RightVector);

csModelDataCamera::csModelDataCamera ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}

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
  IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
IMPLEMENT_IBASE_END

IMPLEMENT_OBJECT_INTERFACE (csModelDataLight);

IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, float, Radius);
IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, const csColor &, Color);

csModelDataLight::csModelDataLight ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}

/*** csModelDataMaterial ***/

IMPLEMENT_IBASE (csModelDataMaterial)
  IMPLEMENTS_INTERFACE (iModelDataMaterial)
  IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
IMPLEMENT_IBASE_END

IMPLEMENT_OBJECT_INTERFACE (csModelDataMaterial);

csModelDataMaterial::csModelDataMaterial ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}

/*** csModelData ***/

IMPLEMENT_IBASE (csModelData)
  IMPLEMENTS_INTERFACE (iModelData)
  IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
IMPLEMENT_IBASE_END

IMPLEMENT_OBJECT_INTERFACE (csModelData);

csModelData::csModelData ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}
