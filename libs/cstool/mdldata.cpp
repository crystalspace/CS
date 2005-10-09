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
#include "cstool/mdldata.h"
#include "csutil/nobjvec.h"
#include "csutil/objiter.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "iengine/texture.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"

#define CS_IMPLEMENT_ARRAY_INTERFACE_NONUM(clname,type,sing_name,mult_name) \
  type clname::Get##sing_name (size_t n) const				\
  { return mult_name[n]; }						\
  void clname::Set##sing_name (size_t n, type val)			\
  { mult_name[n] = val; }

#define CS_IMPLEMENT_ARRAY_INTERFACE(clname,type,sing_name,mult_name)	\
  CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (clname, type, sing_name, mult_name)	\
  size_t clname::Get##sing_name##Count () const				\
  { return mult_name.Length (); }					\
  size_t clname::Add##sing_name (type v)				\
  { mult_name.Push (v); return mult_name.Length () - 1; }		\
  void clname::Delete##sing_name (size_t n)				\
  { mult_name.DeleteIndex (n); }

#define CS_IMPLEMENT_ACCESSOR_METHOD(clname,type,name)			\
  type clname::Get##name () const					\
  { return name; }							\
  void clname::Set##name (type val)					\
  { name = val; }

#define CS_IMPLEMENT_OBJECT_INTERFACE(clname)				\
  CS_IMPLEMENT_EMBEDDED_OBJECT (clname::Embedded_csObject)		\
  iObject* clname::QueryObject ()					\
  { return &scfiObject; }

//----------------------------------------------------------------------------

/*** csModelDataTexture ***/


csModelDataTexture::csModelDataTexture ()
  : scfImplementationType (this)
{
  FileName = 0;
}

csModelDataTexture::~csModelDataTexture ()
{
  delete[] FileName;
}

void csModelDataTexture::SetFileName (const char *fn)
{
  delete[] FileName;
  FileName = csStrNew (fn);
}

const char *csModelDataTexture::GetFileName () const
{
  return FileName;
}

CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataTexture, iImage*, Image)
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataTexture, iTextureWrapper*, TextureWrapper)

void csModelDataTexture::LoadImage (iVFS *vfs, iImageIO *io, int Format)
{
  if (!FileName) return;
  Image = 0;

  csRef<iDataBuffer> dbuf (vfs->ReadFile (FileName, false));
  if (!dbuf) return;

  Image = io->Load (dbuf, Format);
}

void csModelDataTexture::Register (iTextureList *tl)
{
  if (!Image) return;
  SetTextureWrapper (tl->NewTexture (Image));
}

iModelDataTexture *csModelDataTexture::Clone () const
{
  csModelDataTexture *t = new csModelDataTexture ();
  t->SetFileName (FileName);
  t->SetImage (Image);
  t->SetTextureWrapper (TextureWrapper);
  return t;
}

/*** csModelDataMaterial ***/

csModelDataMaterial::csModelDataMaterial ()
  : scfImplementationType (this)
{
}

csModelDataMaterial::~csModelDataMaterial ()
{
}

CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataMaterial, iMaterial*, BaseMaterial)
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataMaterial, iMaterialWrapper*, MaterialWrapper)

void csModelDataMaterial::Register (iMaterialList *ml)
{
  if (!BaseMaterial) return;
  SetMaterialWrapper (ml->NewMaterial (BaseMaterial, 0));
}

iModelDataMaterial *csModelDataMaterial::Clone () const
{
  csModelDataMaterial *m = new csModelDataMaterial ();
  m->SetBaseMaterial (BaseMaterial);
  m->SetMaterialWrapper (MaterialWrapper);
  return m;
}

/*** csModelDataVertices ***/


CS_IMPLEMENT_ARRAY_INTERFACE (csModelDataVertices,
	const csVector3 &, Vertex, Vertices)
CS_IMPLEMENT_ARRAY_INTERFACE (csModelDataVertices,
	const csVector3 &, Normal, Normals)
CS_IMPLEMENT_ARRAY_INTERFACE (csModelDataVertices,
	const csColor &, Color, Colors)
CS_IMPLEMENT_ARRAY_INTERFACE (csModelDataVertices,
	const csVector2 &, Texel, Texels)

csModelDataVertices::csModelDataVertices ()
  : scfImplementationType (this)
{
}

csModelDataVertices::csModelDataVertices (const iModelDataVertices *orig,
  const iModelDataVertices *orig2)
  : scfImplementationType (this)
{
  CopyFrom (orig);
  CopyFrom (orig2);
}

csModelDataVertices::~csModelDataVertices()
{
}

void csModelDataVertices::CopyFrom (const iModelDataVertices *v)
{
  if (!v) return;
  size_t i;
  for (i=0; i<v->GetVertexCount (); i++)
    AddVertex (v->GetVertex (i));
  for (i=0; i<v->GetNormalCount (); i++)
    AddNormal (v->GetNormal (i));
  for (i=0; i<v->GetColorCount (); i++)
    AddColor (v->GetColor (i));
  for (i=0; i<v->GetTexelCount (); i++)
    AddTexel (v->GetTexel (i));
}

iModelDataVertices *csModelDataVertices::Clone () const
{
  csModelDataVertices *v = new csModelDataVertices ();
  size_t i;

  for (i=0; i<Vertices.Length (); i++)
    v->AddVertex (Vertices [i]);
  for (i=0; i<Normals.Length (); i++)
    v->AddNormal (Normals [i]);
  for (i=0; i<Colors.Length (); i++)
    v->AddColor (Colors [i]);
  for (i=0; i<Texels.Length (); i++)
    v->AddTexel (Texels [i]);
  return v;
}

size_t csModelDataVertices::FindVertex (const csVector3 &v) const
{
  size_t i;
  for (i=0; i<Vertices.Length (); i++)
    if (Vertices [i] - v < EPSILON)
      return i;
  return (size_t)-1;
}

size_t csModelDataVertices::FindNormal (const csVector3 &v) const
{
  size_t i;
  for (i=0; i<Normals.Length (); i++)
    if (Normals [i] - v < EPSILON)
      return i;
  return (size_t)-1;
}

size_t csModelDataVertices::FindColor (const csColor &v) const
{
  size_t i;
  for (i=0; i<Colors.Length (); i++)
    if ((Colors[i].red - v.red < EPSILON) &&
        (Colors[i].green - v.green < EPSILON) &&
        (Colors[i].blue - v.blue < EPSILON))
      return i;
  return (size_t)-1;
}

size_t csModelDataVertices::FindTexel (const csVector2 &v) const
{
  size_t i;
  for (i=0; i<Texels.Length (); i++)
    if (Texels [i] - v < EPSILON)
      return i;
  return (size_t)-1;
}

/*** csModelDataAction ***/

csModelDataAction::csModelDataAction ()
  : scfImplementationType (this)
{
}

csModelDataAction::~csModelDataAction()
{
}

size_t csModelDataAction::GetFrameCount () const
{
  return Times.Length ();
}

float csModelDataAction::GetTime (size_t Frame) const
{
  return Times[Frame];
}

iObject *csModelDataAction::GetState (size_t Frame) const
{
  return States.Get (Frame);
}

void csModelDataAction::SetTime (size_t Frame, float NewTime)
{
  // save the object
  iObject *obj = States.Get (Frame);
  obj->IncRef ();

  // remove it from the vectors
  Times.DeleteIndex (Frame);
  States.DeleteIndex (Frame);

  // add it again with the new time value
  AddFrame (NewTime, obj);

  // release it
  obj->DecRef ();
}

void csModelDataAction::SetState (size_t Frame, iObject *State)
{
  States[Frame] = State;
}

void csModelDataAction::AddFrame (float Time, iObject *State)
{
  size_t i;
  for (i=0; i<Times.Length (); i++)
    if (Times.Get (i) > Time) break;
  Times.Insert (i, Time);
  States.Insert (i, State);
}

void csModelDataAction::DeleteFrame (size_t n)
{
  Times.DeleteIndex (n);
  States.DeleteIndex (n);
}

float csModelDataAction::GetTotalTime () const
{
  return (Times.Length () > 0) ? (Times [Times.Length () - 1]) : 0;
}

/*** csModelDataPolygon ***/


csModelDataPolygon::csModelDataPolygon ()
  : scfImplementationType (this)
{
  Material = 0;
}

csModelDataPolygon::~csModelDataPolygon ()
{
  if (Material)
    Material->DecRef ();
}

size_t csModelDataPolygon::GetVertexCount () const
{
  return Vertices.Length ();
}

size_t csModelDataPolygon::AddVertex (int ver, int nrm, int col, int tex)
{
  Vertices.Push (ver);
  Normals.Push (nrm);
  Colors.Push (col);
  Texels.Push (tex);
  return Vertices.Length () - 1;
}

void csModelDataPolygon::DeleteVertex (size_t n)
{
  Vertices.DeleteIndex (n);
  Normals.DeleteIndex (n);
  Colors.DeleteIndex (n);
  Texels.DeleteIndex (n);
}

CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataPolygon, iModelDataMaterial*, Material)
CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, int, Vertex, Vertices)
CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, int, Normal, Normals)
CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, int, Color, Colors)
CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, int, Texel, Texels)

iModelDataPolygon *csModelDataPolygon::Clone () const
{
  csModelDataPolygon *p = new csModelDataPolygon ();
  size_t i;
  for (i=0; i<Vertices.Length (); i++)
    p->AddVertex (Vertices[i], Normals[i], Colors[i], Texels[i]);
  p->SetMaterial (Material);
  return p;
}

/*** csModelDataObject ***/

CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataObject,
	iModelDataVertices *, DefaultVertices)

csModelDataObject::csModelDataObject ()
  : scfImplementationType (this)
{
}

csModelDataObject::~csModelDataObject ()
{
}

/*** csModelDataCamera ***/

CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, Position)
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, UpVector)
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, FrontVector)
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, RightVector)

csModelDataCamera::csModelDataCamera ()
  : scfImplementationType (this)
{
}

csModelDataCamera::~csModelDataCamera()
{
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

iModelDataCamera *csModelDataCamera::Clone () const
{
  csModelDataCamera *c = new csModelDataCamera ();
  c->SetPosition (Position);
  c->SetUpVector (UpVector);
  c->SetFrontVector (FrontVector);
  c->SetRightVector (RightVector);
  return c;
}

/*** csModelDataLight ***/


CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, float, Radius)
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, const csColor &, Color)
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, const csVector3 &, Position)

csModelDataLight::csModelDataLight ()
  : scfImplementationType (this)
{
}

csModelDataLight::~csModelDataLight()
{
}

iModelDataLight *csModelDataLight::Clone () const
{
  csModelDataLight *l = new csModelDataLight ();
  l->SetPosition (Position);
  l->SetColor (Color);
  l->SetRadius (Radius);
  return l;
}

/*** csModelData ***/

csModelData::csModelData ()
  : scfImplementationType (this)
{
}

csModelData::~csModelData()
{
}

void csModelData::LoadImages (iVFS *vfs, iImageIO *io, int Format)
{
  csTypedObjectIterator<iModelDataTexture> it (this);
  while (!it.HasNext ())
  {
    it.Next ()->LoadImage (vfs, io, Format);
  }
}

void csModelData::RegisterTextures (iTextureList *tm)
{
  csTypedObjectIterator<iModelDataTexture> it (this);
  while (it.HasNext ())
  {
    it.Next ()->Register (tm);
  }
}

void csModelData::RegisterMaterials (iMaterialList *ml)
{
  csTypedObjectIterator<iModelDataMaterial> it (this);
  while (!it.HasNext ())
  {
    it.Next ()->Register (ml);
  }
}
