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
#include "isys/vfs.h"

#define CS_IMPLEMENT_ARRAY_INTERFACE_NONUM(clname,type,sing_name,mult_name) \
  type clname::Get##sing_name (int n) const				\
  { return mult_name[n]; }						\
  void clname::Set##sing_name (int n, type val)				\
  { mult_name[n] = val; }

#define CS_IMPLEMENT_ARRAY_INTERFACE(clname,type,sing_name,mult_name)	\
  CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (clname, type, sing_name, mult_name)	\
  int clname::Get##sing_name##Count () const				\
  { return mult_name.Length (); }					\
  int clname::Add##sing_name (type v)					\
  { mult_name.Push (v); return mult_name.Length () - 1; }		\
  void clname::Delete##sing_name (int n)				\
  { mult_name.Delete (n); }

#define CS_IMPLEMENT_ACCESSOR_METHOD(clname,type,name)			\
  type clname::Get##name () const					\
  { return name; }							\
  void clname::Set##name (type val)					\
  { name = val; }

#define CS_IMPLEMENT_ACCESSOR_METHOD_REF(clname,type,name)		\
  type clname::Get##name () const					\
  { return name; }							\
  void clname::Set##name (type val)					\
  { if (name) name->DecRef (); name = val; if (name) name->IncRef (); }

#define CS_IMPLEMENT_OBJECT_INTERFACE(clname)				\
  CS_IMPLEMENT_EMBEDDED_OBJECT (clname::Embedded_csObject);		\
  iObject* clname::QueryObject ()					\
  { return &scfiObject; }

SCF_DECLARE_FAST_INTERFACE (iModelDataTexture);
SCF_DECLARE_FAST_INTERFACE (iModelDataMaterial);
SCF_DECLARE_FAST_INTERFACE (iModelDataObject);
SCF_DECLARE_FAST_INTERFACE (iModelDataPolygon);
SCF_DECLARE_FAST_INTERFACE (iModelDataVertices);
SCF_DECLARE_FAST_INTERFACE (iModelDataAction);

CS_DECLARE_OBJECT_ITERATOR (csModelDataObjectIterator, iModelDataObject);
CS_DECLARE_OBJECT_ITERATOR (csModelDataPolygonIterator, iModelDataPolygon);
CS_DECLARE_OBJECT_ITERATOR (csModelDataActionIterator, iModelDataAction);
CS_DECLARE_OBJECT_ITERATOR (csModelDataTextureIterator, iModelDataTexture);
CS_DECLARE_OBJECT_ITERATOR (csModelDataMaterialIterator, iModelDataMaterial);

CS_TYPEDEF_GROWING_ARRAY (csIntArray, int);

//----------------------------------------------------------------------------

/*** csModelDataTexture ***/

SCF_IMPLEMENT_IBASE (csModelDataTexture)
  SCF_IMPLEMENTS_INTERFACE (iModelDataTexture)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelDataTexture);

csModelDataTexture::csModelDataTexture ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
  FileName = NULL;
  Image = NULL;
  TextureWrapper = NULL;
}

csModelDataTexture::~csModelDataTexture ()
{
  delete[] FileName;
  SCF_DEC_REF (Image);
  SCF_DEC_REF (TextureWrapper);
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

CS_IMPLEMENT_ACCESSOR_METHOD_REF (csModelDataTexture, iImage*, Image);
CS_IMPLEMENT_ACCESSOR_METHOD_REF (csModelDataTexture, iTextureWrapper*, TextureWrapper);

void csModelDataTexture::LoadImage (iVFS *vfs, iImageIO *io, int Format)
{
  if (!FileName) return;
  SCF_DEC_REF (Image);
  Image = NULL;

  iDataBuffer *dbuf = vfs->ReadFile (FileName);
  if (!dbuf) return;

  Image = io->Load (dbuf->GetUint8 (), dbuf->GetSize (), Format);
  dbuf->DecRef ();
}

void csModelDataTexture::Register (iTextureList *tl)
{
  if (!Image) return;
  SetTextureWrapper (tl->NewTexture (Image));
}

/*** csModelDataMaterial ***/

SCF_IMPLEMENT_IBASE (csModelDataMaterial)
  SCF_IMPLEMENTS_INTERFACE (iModelDataMaterial)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelDataMaterial);

csModelDataMaterial::csModelDataMaterial ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
  BaseMaterial = NULL;
  MaterialWrapper = NULL;
}

csModelDataMaterial::~csModelDataMaterial ()
{
  SCF_DEC_REF (BaseMaterial);
  SCF_DEC_REF (MaterialWrapper);
}

CS_IMPLEMENT_ACCESSOR_METHOD_REF (csModelDataMaterial, iMaterial*, BaseMaterial);
CS_IMPLEMENT_ACCESSOR_METHOD_REF (csModelDataMaterial, iMaterialWrapper*, MaterialWrapper);

void csModelDataMaterial::Register (iMaterialList *ml)
{
  if (!BaseMaterial) return;
  SetMaterialWrapper (ml->NewMaterial (BaseMaterial));
}

/*** csModelDataVertices ***/

SCF_IMPLEMENT_IBASE (csModelDataVertices)
  SCF_IMPLEMENTS_INTERFACE (iModelDataVertices)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelDataVertices);

CS_IMPLEMENT_ARRAY_INTERFACE (csModelDataVertices,
	const csVector3 &, Vertex, Vertices);
CS_IMPLEMENT_ARRAY_INTERFACE (csModelDataVertices,
	const csVector3 &, Normal, Normals);
CS_IMPLEMENT_ARRAY_INTERFACE (csModelDataVertices,
	const csColor &, Color, Colors);
CS_IMPLEMENT_ARRAY_INTERFACE (csModelDataVertices,
	const csVector2 &, Texel, Texels);

csModelDataVertices::csModelDataVertices ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}

/*** csModelDataAction ***/

SCF_IMPLEMENT_IBASE (csModelDataAction)
  SCF_IMPLEMENTS_INTERFACE (iModelDataAction)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelDataAction);

csModelDataAction::csModelDataAction ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}
  
int csModelDataAction::GetFrameCount () const
{
  return Times.Length ();
}

float csModelDataAction::GetTime (int Frame) const
{
  return Times[Frame];
}

iObject *csModelDataAction::GetState (int Frame) const
{
  return States.Get (Frame);
}

void csModelDataAction::SetTime (int Frame, float NewTime)
{
  // save the object
  iObject *obj = States.Get (Frame);
  obj->IncRef ();

  // remove it from the vectors
  Times.Delete (Frame);
  States.Delete (Frame);

  // add it again with the new time value
  AddFrame (NewTime, obj);

  // release it
  obj->DecRef ();
}

void csModelDataAction::SetState (int Frame, iObject *State)
{
  States.Replace (Frame, State);
}

void csModelDataAction::AddFrame (float Time, iObject *State)
{
  int i;
  for (i=0; i<Times.Length (); i++)
    if (Times.Get (i) > Time) break;
  Times.Insert (i, Time);
  States.Insert (i, State);
}

void csModelDataAction::DeleteFrame (int n)
{
  Times.Delete (n);
  States.Delete (n);
}

/*** csModelDataPolygon ***/

SCF_IMPLEMENT_IBASE (csModelDataPolygon)
  SCF_IMPLEMENTS_INTERFACE (iModelDataPolygon)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelDataPolygon);

csModelDataPolygon::csModelDataPolygon ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
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

int csModelDataPolygon::AddVertex (int ver, int nrm, int col, int tex)
{
  Vertices.Push (ver);
  Normals.Push (nrm);
  Colors.Push (col);
  Texels.Push (tex);
  return Vertices.Length () - 1;
}

void csModelDataPolygon::DeleteVertex (int n)
{
  Vertices.Delete (n);
  Normals.Delete (n);
  Colors.Delete (n);
  Texels.Delete (n);
}

CS_IMPLEMENT_ACCESSOR_METHOD_REF (csModelDataPolygon, iModelDataMaterial*, Material);
CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, int, Vertex, Vertices);
CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, int, Normal, Normals);
CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, int, Color, Colors);
CS_IMPLEMENT_ARRAY_INTERFACE_NONUM (csModelDataPolygon, int, Texel, Texels);

/*** csModelDataObject ***/

#define CS_MERGE_VERTICES_HELPER(vnum,obj)	\
  for (i=0; i<vnum->Get##obj##Count (); i++)	\
    ver->Add##obj (vnum->Get##obj (i));

static iModelDataVertices *MergeVertices (const iModelDataVertices *v1,
	const iModelDataVertices *v2)
{
  int i;
  iModelDataVertices *ver = new csModelDataVertices ();

  if (v1)
  {
    CS_MERGE_VERTICES_HELPER (v1, Vertex)
    CS_MERGE_VERTICES_HELPER (v1, Normal)
    CS_MERGE_VERTICES_HELPER (v1, Texel)
    CS_MERGE_VERTICES_HELPER (v1, Color)
  }
  if (v2)
  {
    CS_MERGE_VERTICES_HELPER (v2, Vertex)
    CS_MERGE_VERTICES_HELPER (v2, Normal)
    CS_MERGE_VERTICES_HELPER (v2, Texel)
    CS_MERGE_VERTICES_HELPER (v2, Color)
  }

  return ver;
}

SCF_IMPLEMENT_IBASE (csModelDataObject)
  SCF_IMPLEMENTS_INTERFACE (iModelDataObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelDataObject);

CS_IMPLEMENT_ACCESSOR_METHOD_REF (csModelDataObject,
	iModelDataVertices *, DefaultVertices);

csModelDataObject::csModelDataObject ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
  DefaultVertices = NULL;
}

csModelDataObject::~csModelDataObject ()
{
  SCF_DEC_REF (DefaultVertices);
}

void MergeAction (iModelDataAction *Out, iModelDataAction *In1,
  iModelDataVertices *In2, bool Swap)
{
  for (int i=0; i<In1->GetFrameCount (); i++)
  {
    iModelDataVertices *ver = SCF_QUERY_INTERFACE_FAST (In1->GetState (i),
      iModelDataVertices);
    if (ver)
    {
      iModelDataVertices *NewVertices = Swap ?
        MergeVertices (In2, ver) : MergeVertices (ver, In2);
      Out->AddFrame (In1->GetTime (i), NewVertices->QueryObject ());
      NewVertices->DecRef ();
      ver->DecRef ();
    }
  }
}

CS_DECLARE_OBJECT_VECTOR (csActionVector, iModelDataAction);

void csModelDataObject::MergeCopyObject (iModelDataObject *obj)
{

  // store vertex, normal, texel and color offset
  int VertexOffset = DefaultVertices ? DefaultVertices->GetVertexCount () : 0;
  int NormalOffset = DefaultVertices ? DefaultVertices->GetNormalCount () : 0;
  int TexelOffset = DefaultVertices ? DefaultVertices->GetTexelCount () : 0;
  int ColorOffset = DefaultVertices ? DefaultVertices->GetColorCount () : 0;

  // copy the default vertices
  iModelDataVertices *OrigDefaultVertices = GetDefaultVertices ();
  SCF_INC_REF (OrigDefaultVertices);

  iModelDataVertices *ver = MergeVertices (DefaultVertices, obj->GetDefaultVertices ());
  SetDefaultVertices (ver);
  ver->DecRef ();

  // copy all polygons
  csModelDataPolygonIterator *polyit = new csModelDataPolygonIterator (obj->QueryObject ());
  while (!polyit->IsFinished ())
  {
    iModelDataPolygon *poly = polyit->Get ();
    iModelDataPolygon *NewPoly = new csModelDataPolygon ();
    scfiObject.ObjAdd (NewPoly->QueryObject ());

    int i;
    for (i=0; i<poly->GetVertexCount (); i++)
    {
      NewPoly->AddVertex (
      poly->GetVertex (i) + VertexOffset,
      poly->GetNormal (i) + NormalOffset,
      poly->GetColor (i) + ColorOffset,
      poly->GetTexel (i) + TexelOffset);
    }
    NewPoly->SetMaterial (poly->GetMaterial ());
    NewPoly->DecRef ();

    polyit->Next ();
  }
  delete polyit;

  // build the action mapping
  csActionVector ActionMap1, ActionMap2;

  csModelDataActionIterator *actit = new csModelDataActionIterator (&scfiObject);
  while (!actit->IsFinished ())
  {
    iModelDataAction *Action = actit->Get ();

    ActionMap1.Push (Action);
    ActionMap2.Push (NULL);
    scfiObject.ObjRemove (Action->QueryObject ());

    actit->Next ();
  }
  delete actit;

  actit = new csModelDataActionIterator (obj->QueryObject ());
  while (!actit->IsFinished ())
  {
    iModelDataAction *Action = actit->Get ();

    int n = ActionMap1.GetIndexByName (Action->QueryObject ()->GetName ());
    if (n == -1) {
      ActionMap1.Push (NULL);
      ActionMap2.Push (Action);
    } else {
      ActionMap2.Replace (n, Action);
    }

    actit->Next ();
  }
  delete actit;

  // merge the actions
  for (int i=0; i<ActionMap1.Length (); i++)
  {
    iModelDataAction *Action1 = ActionMap1.Get (i),
                     *Action2 = ActionMap2.Get (i),
		     *NewAction = new csModelDataAction ();
    NewAction->QueryObject ()->SetName (Action1 ?
      Action1->QueryObject ()->GetName () :
      Action2->QueryObject ()->GetName ());
    scfiObject.ObjAdd (NewAction->QueryObject ());
    NewAction->DecRef ();

    if (Action1) {
      if (Action2) {
        // merge two actions
	CS_ASSERT (("Action conflict detection missed a conflict!!!", false));
      } else {
        // merge action 1 and the default frame of object 2
	MergeAction (NewAction, Action1, obj->GetDefaultVertices (), false);
      }
    } else {
      // merge action 2 and the default frame of object 1
      MergeAction (NewAction, Action2, OrigDefaultVertices, true);
    }
  }
  SCF_DEC_REF (OrigDefaultVertices);
}

/*** csModelDataCamera ***/

SCF_IMPLEMENT_IBASE (csModelDataCamera)
  SCF_IMPLEMENTS_INTERFACE (iModelDataCamera)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelDataCamera);

CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, Position);
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, UpVector);
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, FrontVector);
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataCamera, const csVector3 &, RightVector);

csModelDataCamera::csModelDataCamera ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
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

SCF_IMPLEMENT_IBASE (csModelDataLight)
  SCF_IMPLEMENTS_INTERFACE (iModelDataLight)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelDataLight);

CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, float, Radius);
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, const csColor &, Color);
CS_IMPLEMENT_ACCESSOR_METHOD (csModelDataLight, const csVector3 &, Position);

csModelDataLight::csModelDataLight ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}

/*** csModelData ***/

SCF_IMPLEMENT_IBASE (csModelData)
  SCF_IMPLEMENTS_INTERFACE (iModelData)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_OBJECT_INTERFACE (csModelData);

csModelData::csModelData ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
}

void csModelData::LoadImages (iVFS *vfs, iImageIO *io, int Format)
{
  csModelDataTextureIterator it (&scfiObject);
  while (!it.IsFinished ())
  {
    it.Get ()->LoadImage (vfs, io, Format);
    it.Next ();
  }
}

void csModelData::RegisterTextures (iTextureList *tm)
{
  csModelDataTextureIterator it (&scfiObject);
  while (!it.IsFinished ())
  {
    it.Get ()->Register (tm);
    it.Next ();
  }
}

void csModelData::RegisterMaterials (iMaterialList *ml)
{
  csModelDataMaterialIterator it (&scfiObject);
  while (!it.IsFinished ())
  {
    it.Get ()->Register (ml);
    it.Next ();
  }
}

CS_DECLARE_TYPED_VECTOR_NODELETE (csModelDataMaterialVector, iModelDataMaterial);

static bool CheckActionConflict (iModelDataObject *obj1, iModelDataObject *obj2)
{
/*
  @@@ untested, thus disabled.

  csModelDataActionIterator it (obj1->QueryObject ());
  while (!it.IsFinished ())
  {
    iModelDataAction *Action = it.Get ();

    iModelDataAction *Action2 = CS_GET_NAMED_CHILD_OBJECT_FAST (obj2->QueryObject (),
      iModelDataAction, Action->QueryObject ()->GetName ());

    if (Action2)
    {
      Action2->DecRef ();
      return true;
    }
    it.Next ();
  }
*/

  return false;
}

static bool CheckMaterialConflict (iModelDataObject *obj1, iModelDataObject *obj2)
{
/*
  @@@ untested, thus disabled.

  csModelDataMaterialVector mat1, mat2;

  {
    csModelDataPolygonIterator it (obj1->QueryObject ());
    while (!it.IsFinished ())
    {
      iModelDataPolygon *poly = it.Get ();
      if (poly->GetMaterial ()) mat1.Push (poly->GetMaterial ());
      it.Next ();
    }
  }

  {
    csModelDataPolygonIterator it (obj2->QueryObject ());
    while (!it.IsFinished ())
    {
      iModelDataPolygon *poly = it.Get ();
      if (poly->GetMaterial ()) mat2.Push (poly->GetMaterial ());
      it.Next ();
    }
  }

  for (int i=0; i<mat1.Length (); i++)
  {
    iModelDataMaterial *m1 = mat1.Get (i);
    for (int j=0; j<mat2.Length (); j++)
    {
      if (mat2.Get (j) != m1)
        return true;
    }
  }
*/
  return false;
}

static bool CheckMaterialConflict (iModelDataObject *obj1)
{
  iModelDataMaterial *mat = NULL;

  csModelDataPolygonIterator it = (obj1->QueryObject ());
  while (!it.IsFinished ())
  {
    iModelDataMaterial *mat2 = it.Get ()->GetMaterial ();

    if (mat2)
      if (!mat) mat = mat2;
    else if (mat != mat2)
      return true;

    it.Next ();
  }

  return false;
}

CS_DECLARE_TYPED_VECTOR_NODELETE (csModelDataObjectVector, iModelDataObject);

void csModelData::MergeObjects (bool MultiTexture)
{
  csModelDataObjectVector OldObjects, NewObjects;

  while (1)
  {
    iModelDataObject *obj = CS_GET_CHILD_OBJECT_FAST ((&scfiObject), iModelDataObject);
    if (!obj) break;
    OldObjects.Push (obj);
    scfiObject.ObjRemove (obj->QueryObject ());
  }

  while (OldObjects.Length () > 0)
  {
    iModelDataObject *obj = OldObjects.Pop ();

    // look if we can merge this object with an existing one
    for (int i=0; i<NewObjects.Length (); i++)
    {
      iModelDataObject *obj2 = NewObjects.Get (i);
      if ((MultiTexture || !CheckMaterialConflict (obj, obj2)) &&
        !CheckActionConflict (obj, obj2))
      {
        obj2->MergeCopyObject (obj);
	break;
      }
    }
    if (i == NewObjects.Length ())
    {
      iModelDataObject *NewObject = new csModelDataObject ();
      scfiObject.ObjAdd (NewObject->QueryObject ());
      NewObject->MergeCopyObject (obj);
      NewObject->DecRef ();
    }
    obj->DecRef ();
  }
}

void csModelData::SplitObjectsByMaterial ()
{
/*
  csModelDataObjectVector OldObjects, NewObjects;

  while (1)
  {
    iModelDataObject *obj = CS_GET_CHILD_OBJECT_FAST ((&scfiObject), iModelDataObject);
    if (!obj) break;
    OldObjects.Push (obj);
    scfiObject.ObjRemove (obj->QueryObject ());
  }

  while (OldObjects.Length () > 0)
  {
    iModelDataObject *obj = OldObjects.Pop ();

    if (!CheckMaterialConflict (obj)) {
      scfiObject.ObjAdd (obj->QueryObject ());
    } else {
      csModelDataMaterialVector NewMaterials;
      csModelDataObjectVector NewObjects;
      csIntArray PolygonToNewObject;

      csModelDataPolygonIterator it (obj->QueryObject ());
      while (!it.IsFinished ())
      {
        iModelDataPolygon *poly = it.Get ();

        int n = NewMaterials.Find (poly->GetMaterial ());
        if (n == -1) {
          PolygonToNewObject.Push (NewObjects.Length ());
          iModelDataObject *newobj = new csModelDataObject ();
          scfiObject.ObjAdd (newobj->QueryObject ());
          NewMaterials.Push (poly->GetMaterial ());
          NewObjects.Push (newobj);
        } else {
          PolygonToNewObject.Push (n);
        }

        it.Next ();
      }
    }
    obj->DecRef ();
  }
*/
}
