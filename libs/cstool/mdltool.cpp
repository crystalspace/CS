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
#include "csutil/scf.h"
#include "csutil/nobjvec.h"
#include "csutil/objiter.h"
#include "csutil/garray.h"
#include "csutil/intarray.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "cstool/mdldata.h"
#include "cstool/mdltool.h"

// ---------------------------------------------------------------------------
// csSingleIndexVertexSet
// ---------------------------------------------------------------------------

csSingleIndexVertexSet::csSingleIndexVertexSet (bool v, bool n, bool c, bool t)
{
  Delete = true;
  Count = 0;
  if (v) Vertices = new csIntArray ();
  else Vertices = NULL;
  if (n) Normals = new csIntArray ();
  else Normals = 0;
  if (c) Colors = new csIntArray ();
  else Colors = 0;
  if (t) Texels = new csIntArray ();
  else Texels = 0;
}

csSingleIndexVertexSet::csSingleIndexVertexSet (csIntArray *v, csIntArray *n,
  csIntArray *c, csIntArray *t, bool del)
{
  Delete = del;
  Count = 0;
  if (v) Count = v->Length ();
  if (n) Count = n->Length ();
  if (c) Count = c->Length ();
  if (t) Count = t->Length ();

  Vertices = v;
  Normals = n;
  Colors = c;
  Texels = t;
}

csSingleIndexVertexSet::~csSingleIndexVertexSet ()
{
  if (Delete) {
    delete Vertices;
    delete Normals;
    delete Colors;
    delete Texels;
  }
}

int csSingleIndexVertexSet::Add (int Vertex, int Normal, int Color, int Texel)
{
  int i;

  for (i=0; i<Count; i++)
  {
    if (((!Vertices) || (Vertex == Vertices->Get (i))) &&
        ((!Normals) || (Normal == Normals->Get (i))) &&
        ((!Colors) || (Color == Colors->Get (i))) &&
        ((!Texels) || (Texel == Texels->Get (i))))
      return i;
  }
  if (Vertices) Vertices->Push (Vertex);
  if (Normals) Normals->Push (Normal);
  if (Colors) Colors->Push (Color);
  if (Texels) Texels->Push (Texel);
  Count++;
  return Count - 1;
}

void csSingleIndexVertexSet::Add (int Count, int *ver, int *nrm, int *col, int *tex)
{
  int i;

  for (i=0; i<Count; i++)
  {
    Add (ver ? ver[i] : -1, nrm ? nrm[i] : -1,
	 col ? col[i] : -1, tex ? tex[i] : -1);
  }
}

int csSingleIndexVertexSet::GetVertexCount () const
{
  return Count;
}

int csSingleIndexVertexSet::GetVertex (int n) const
{
  return Vertices->Get (n);
}

int csSingleIndexVertexSet::GetNormal (int n) const
{
  return Normals->Get (n);
}

int csSingleIndexVertexSet::GetColor (int n) const
{
  return Colors->Get (n);
}

int csSingleIndexVertexSet::GetTexel (int n) const
{
  return Texels->Get (n);
}

// ---------------------------------------------------------------------------
// Some helper declarations
// ---------------------------------------------------------------------------

SCF_DECLARE_FAST_INTERFACE (iModelDataTexture);
SCF_DECLARE_FAST_INTERFACE (iModelDataMaterial);
SCF_DECLARE_FAST_INTERFACE (iModelDataObject);
SCF_DECLARE_FAST_INTERFACE (iModelDataPolygon);
SCF_DECLARE_FAST_INTERFACE (iModelDataVertices);
SCF_DECLARE_FAST_INTERFACE (iModelDataAction);
SCF_DECLARE_FAST_INTERFACE (iModelData);

CS_DECLARE_OBJECT_ITERATOR (csModelDataObjectIterator, iModelDataObject);
CS_DECLARE_OBJECT_ITERATOR (csModelDataPolygonIterator, iModelDataPolygon);
CS_DECLARE_OBJECT_ITERATOR (csModelDataActionIterator, iModelDataAction);
CS_DECLARE_OBJECT_ITERATOR (csModelDataTextureIterator, iModelDataTexture);
CS_DECLARE_OBJECT_ITERATOR (csModelDataMaterialIterator, iModelDataMaterial);

CS_TYPEDEF_GROWING_ARRAY (csVector3Array, csVector3);
CS_TYPEDEF_GROWING_ARRAY (csVector2Array, csVector2);
CS_TYPEDEF_GROWING_ARRAY (csColorArray, csColor);
CS_DECLARE_TYPED_VECTOR (csIntArrayVector, csIntArray);

CS_DECLARE_OBJECT_VECTOR (csModelDataActionVector, iModelDataAction);
CS_DECLARE_OBJECT_VECTOR (csModelDataMaterialVector, iModelDataMaterial);
CS_DECLARE_OBJECT_VECTOR (csModelDataObjectVector, iModelDataObject);
CS_DECLARE_OBJECT_VECTOR (csModelDataPolygonVector, iModelDataPolygon);
CS_DECLARE_OBJECT_VECTOR (csModelDataVerticesVector, iModelDataVertices);

// ---------------------------------------------------------------------------
// Some helper functions
// ---------------------------------------------------------------------------

/*
 * ExtractObjects (): Takes a model data scene and extracts all mesh objects
 * from it into the given vector.
 */
static void ExtractObjects (iModelData *Parent, csModelDataObjectVector &vec)
{
  while (1)
  {
    iModelDataObject *obj = CS_GET_CHILD_OBJECT_FAST ((Parent->QueryObject ()),
      iModelDataObject);
    if (!obj) break;
    vec.Push (obj);
    Parent->QueryObject ()->ObjRemove (obj->QueryObject ());
    obj->DecRef ();
  }
}

/*
 * DumpVertices (): Copy all elements of a vertex frame into growing arrays.
 */
static void DumpVertices (iModelDataVertices *vframe,
  csVector3Array *Vertices, csVector3Array *Normals,
  csColorArray *Colors, csVector2Array *Texels)
{
  int i;
  for (i=0; i<vframe->GetVertexCount (); i++)
    Vertices->Push (vframe->GetVertex (i));
  for (i=0; i<vframe->GetNormalCount (); i++)
    Normals->Push (vframe->GetNormal (i));
  for (i=0; i<vframe->GetColorCount (); i++)
    Colors->Push (vframe->GetColor (i));
  for (i=0; i<vframe->GetTexelCount (); i++)
    Texels->Push (vframe->GetTexel (i));
}

/*
 * InterpolateVertices (): Return a new vertex frame whose elements are the
 * linear interpolation of the given two frames at the given position [0..1].
 * The input vertex frames must contain the same amount elements of each
 * type (vertices, normals, colors, texels).
 */
static iModelDataVertices *InterpolateVertices (iModelDataVertices *v1,
  iModelDataVertices *v2, float Position)
{
  iModelDataVertices *ver = new csModelDataVertices ();
  int i;

  for (i=0; i<v1->GetVertexCount (); i++)
    ver->AddVertex (v1->GetVertex (i) +
      (v2->GetVertex (i) - v1->GetVertex (i)) * Position);
  for (i=0; i<v1->GetNormalCount (); i++)
    ver->AddNormal (v1->GetNormal (i) +
      (v2->GetNormal (i) - v1->GetNormal (i)) * Position);
  for (i=0; i<v1->GetColorCount (); i++)
    ver->AddColor (v1->GetColor (i) +
      (v2->GetColor (i) - v1->GetColor (i)) * Position);
  for (i=0; i<v1->GetTexelCount (); i++)
    ver->AddTexel (v1->GetTexel (i) +
      (v2->GetTexel (i) - v1->GetTexel (i)) * Position);
  
  return ver;
}

/*
 * MergeAction(): Merge all frames of the input action with the given frame and
 * store the result in the output action (which is returned). If 'Swap' is true
 * then the merging order of the frames is reversed.
 */
static iModelDataAction *MergeAction (iModelDataAction *In1,
  iModelDataVertices *In2, bool Swap)
{
  iModelDataAction *Out = new csModelDataAction ();
  int i;
  for (i=0; i<In1->GetFrameCount (); i++)
  {
    iModelDataVertices *ver = SCF_QUERY_INTERFACE_FAST (In1->GetState (i),
      iModelDataVertices);
    if (ver)
    {
      iModelDataVertices *NewVertices = Swap ?
        new csModelDataVertices (In2, ver) : new csModelDataVertices (ver, In2);
      Out->AddFrame (In1->GetTime (i), NewVertices->QueryObject ());
      NewVertices->DecRef ();
      ver->DecRef ();
    }
  }
  return Out;
}

/*
 * MergeAction(): Create an action of the given time in which the given
 * actions are looped and the frames are merged. This function currently
 * assumes that only vertex frames are used in the action! Note: To change
 * that, it is not enough to make the SCF_QUERY_INTERFACE calls safe. There
 * has to be additional test, for example for the case that the action
 * doesn't contain vertex frames at all (only other frame types).
 */
static iModelDataAction *MergeAction (iModelDataAction *In1,
  iModelDataAction *In2, float TotalTime)
{
  // copy the first action
  iModelDataAction *Out = new csModelDataAction ();

  // next frame to process
  int n1 = 0, n2 = 0;
  // base time value for the current animation cycle
  float tbase1 = 0, tbase2 = 0;

  while (1)
  {
    // index of the previous frame
    int prev1 = (n1 == 0) ? (In1->GetFrameCount () - 1) : (n1 - 1);
    int prev2 = (n2 == 0) ? (In2->GetFrameCount () - 1) : (n2 - 1);

    // time value of the next frame, relative to current base time
    float ft1 = In1->GetTime (n1);
    float ft2 = In2->GetTime (n2);

    // absolute time value of the next frame
    float nf1 = tbase1 + ft1;
    float nf2 = tbase2 + ft2;

    // check if finished
    if (nf1 > TotalTime && nf2 > TotalTime) break;

    // absolute time value of the previous frame (can be negative for the
    // very first frame)
    float pf1 = tbase1 + In1->GetTime (prev1);
    if (n1 == 0) pf1 -= In1->GetTotalTime ();
    float pf2 = tbase2 + In2->GetTime (prev2);
    if (n2 == 0) pf2 -= In2->GetTotalTime ();

    // current frame states
    iModelDataVertices *Frame1, *Frame2;
    bool Hit1, Hit2;
    float CurrentTime;

    if (ABS (nf2 - nf1) < EPSILON) {
      Frame1 = SCF_QUERY_INTERFACE_FAST (In1->GetState (n1), iModelDataVertices);
      Frame2 = SCF_QUERY_INTERFACE_FAST (In2->GetState (n2), iModelDataVertices);
      CurrentTime = nf1;
      Hit1 = Hit2 = true;
    } else {
      iModelDataVertices *DirectFrame, *InterpFrameStart, *InterpFrameEnd;
      float InterpAmount;
      bool Swap;

      if (nf1 < nf2) {
        DirectFrame = SCF_QUERY_INTERFACE_FAST (In1->GetState (n1), iModelDataVertices);
        InterpFrameStart = SCF_QUERY_INTERFACE_FAST (In2->GetState (prev2), iModelDataVertices);
        InterpFrameEnd = SCF_QUERY_INTERFACE_FAST (In2->GetState (n2), iModelDataVertices);
        InterpAmount = (nf1-pf2) / (nf2-pf2);
        Swap = false;

        CurrentTime = nf1;
	Hit1 = true;
	Hit2 = false;
      } else {
        DirectFrame = SCF_QUERY_INTERFACE_FAST (In2->GetState (n2), iModelDataVertices);
        InterpFrameStart = SCF_QUERY_INTERFACE_FAST (In1->GetState (prev1), iModelDataVertices);
        InterpFrameEnd = SCF_QUERY_INTERFACE_FAST (In1->GetState (n1), iModelDataVertices);
        InterpAmount = (nf2-pf1) / (nf1-pf1);
        Swap = true;

        CurrentTime = nf2;
	Hit1 = false;
	Hit2 = true;
      }

      iModelDataVertices *InterpFrame =
        InterpolateVertices (InterpFrameStart, InterpFrameEnd, InterpAmount);
      InterpFrameStart->DecRef ();
      InterpFrameEnd->DecRef ();

      if (Swap == false) {
        Frame1 = DirectFrame;
	Frame2 = InterpFrame;
      } else {
	Frame1 = InterpFrame;
        Frame2 = DirectFrame;
      }
    }

    iModelDataVertices *MergedVertices = 
        new csModelDataVertices (Frame1, Frame2);
    Frame1->DecRef ();
    Frame2->DecRef ();

    Out->AddFrame (CurrentTime, MergedVertices->QueryObject ());
    MergedVertices->DecRef ();

    if (Hit1) {
      n1++;
      if (n1 == In1->GetFrameCount ())
      { n1 = 0; tbase1 += In1->GetTotalTime (); }
    }

    if (Hit2) {
      n2++;
      if (n2 == In2->GetFrameCount ())
      { n2 = 0; tbase2 += In2->GetTotalTime (); }
    }
  }

  return Out;
}

/*
 * BuildMappedVertexFrame (): Create a vertex frame by copying vertices from
 * the given frame, as described by the given map.
 */
static iModelDataVertices *BuildMappedVertexFrame (iModelDataVertices *Orig,
  const csModelDataVertexMap *Map)
{
  iModelDataVertices *ver = new csModelDataVertices ();
  int i;

  for (i=0; i<Map->VertexCount; i++)
    ver->AddVertex (Orig->GetVertex (Map->Vertices [i]));

  for (i=0; i<Map->NormalCount; i++)
    ver->AddNormal (Orig->GetNormal (Map->Normals [i]));

  for (i=0; i<Map->ColorCount; i++)
    ver->AddColor (Orig->GetColor (Map->Colors [i]));

  for (i=0; i<Map->TexelCount; i++)
    ver->AddTexel (Orig->GetTexel (Map->Texels [i]));

  return ver;
}

/*
 * CheckMaterialConflict (one object): Returns true if the given
 * object uses more than one material, false otherwise.
 */
static bool CheckMaterialConflict (iModelDataObject *obj1)
{
  iModelDataMaterial *mat = NULL;

  csModelDataPolygonIterator it = (obj1->QueryObject ());
  while (!it.IsFinished ())
  {
    iModelDataMaterial *mat2 = it.Get ()->GetMaterial ();

    if (mat2)
    {
      if (!mat) mat = mat2;
      else {
        if (mat != mat2)
          return true; 
      }
    }

    it.Next ();
  }

  return false;
}

/*
 * Helper function for MergeTimes () which assumes that in1<in2
 */
static float MergeTimesHelper (float in1, float in2)
{
  if (in1 < EPSILON)
    return in2;

  if (in2 < EPSILON)
    return in1;

  float a = in2 / in1;
  if (ABS (a-1) < EPSILON) {
    return in2;
  } else if (ABS (a-2) < EPSILON) {
    return in2;
  } else if (ABS (a-3) < EPSILON) {
    return in2;
  } else if (ABS (a-1.5) < EPSILON) {
    return in2 * 2;
  }
  return -1;
}

/*
 * Take two time values ('in1' and 'in2') and find a total time value which is
 * a multiple of both values, which is returned. A negative value is returned
 * if this is impossible or if one of the multipliers is >3 (@@@ find a better
 * way to detect actions that would end up in too many frames when merging!).
 * If one of the time values is zero then the other time value is returned.
 */
inline float MergeTimes (float in1, float in2)
{
  if (in1 < in2) {
    return MergeTimesHelper (in1, in2);
  } else {
    return MergeTimesHelper (in2, in1);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void csModelDataTools::MergeCopyObject (iModelDataObject *dest, iModelDataObject *src)
{
  // store vertex, normal, texel and color offset
  iModelDataVertices *DefaultVertices = dest->GetDefaultVertices ();
  SCF_INC_REF (DefaultVertices);
  int VertexOffset = DefaultVertices ? DefaultVertices->GetVertexCount () : 0;
  int NormalOffset = DefaultVertices ? DefaultVertices->GetNormalCount () : 0;
  int TexelOffset = DefaultVertices ? DefaultVertices->GetTexelCount () : 0;
  int ColorOffset = DefaultVertices ? DefaultVertices->GetColorCount () : 0;
  int i;

  // copy the default vertices
  iModelDataVertices *ver = new csModelDataVertices (DefaultVertices, src->GetDefaultVertices ());
  dest->SetDefaultVertices (ver);
  ver->DecRef ();

  // copy all polygons
  csModelDataPolygonIterator *polyit = new csModelDataPolygonIterator (src->QueryObject ());
  while (!polyit->IsFinished ())
  {
    iModelDataPolygon *poly = polyit->Get ();
    iModelDataPolygon *NewPoly = new csModelDataPolygon ();
    dest->QueryObject ()->ObjAdd (NewPoly->QueryObject ());

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
  csModelDataActionVector ActionMap1, ActionMap2;

  while (1)
  {
    iModelDataAction *Action = CS_GET_CHILD_OBJECT_FAST (dest->QueryObject (), iModelDataAction);
    if (!Action) break;

    ActionMap1.Push (Action);
    ActionMap2.Push (NULL);
    dest->QueryObject ()->ObjRemove (Action->QueryObject ());
  }

  while (1)
  {
    iModelDataAction *Action = CS_GET_CHILD_OBJECT_FAST (src->QueryObject (), iModelDataAction);
    if (!Action) break;

    int n = ActionMap1.GetIndexByName (Action->QueryObject ()->GetName ());
    if (n == -1) {
      ActionMap1.Push (NULL);
      ActionMap2.Push (Action);
    } else {
      ActionMap2.Replace (n, Action);
    }
    src->QueryObject ()->ObjRemove (Action->QueryObject ());
  }

  // merge the actions
  for (i=0; i<ActionMap1.Length (); i++)
  {
    iModelDataAction *Action1 = ActionMap1.Get (i),
                     *Action2 = ActionMap2.Get (i),
		     *NewAction;
    if (Action1 && Action1->GetTotalTime () > EPSILON) {
      if (Action2 && Action2->GetTotalTime () > EPSILON) {
        float total = MergeTimes (Action1->GetTotalTime (), Action2->GetTotalTime ());
        
	if (total<0)
	{
          // this should not happen
	  CS_ASSERT ("Action conflict detection missed a conflict!!!" && false);
	  NewAction = NULL;
	}

	NewAction = MergeAction (Action1, Action2, total);
      } else {
        // merge action 1 and the default frame of object 2
	NewAction = MergeAction (Action1, src->GetDefaultVertices (), false);
      }
    } else {
      // merge action 2 and the default frame of object 1
      NewAction = MergeAction (Action2, DefaultVertices, true);
    }

    NewAction->QueryObject ()->SetName (Action1 ?
      Action1->QueryObject ()->GetName () :
      Action2->QueryObject ()->GetName ());
    dest->QueryObject ()->ObjAdd (NewAction->QueryObject ());
    NewAction->DecRef ();
  }
  SCF_DEC_REF (DefaultVertices);
}

void csModelDataTools::CopyVerticesMapped (iModelDataObject *dest,
  iModelDataObject *src, const csModelDataVertexMap *Map)
{
  iModelDataVertices *ver =
    BuildMappedVertexFrame (src->GetDefaultVertices (), Map);
  dest->SetDefaultVertices (ver);
  ver->DecRef ();

  csModelDataActionIterator it (src->QueryObject ());
  while (!it.IsFinished ())
  {
    iModelDataAction *OldAction = it.Get ();
    iModelDataAction *NewAction = CS_GET_NAMED_CHILD_OBJECT (dest->QueryObject (),
      iModelDataAction, OldAction->QueryObject ()->GetName ());
    if (!NewAction) {
      NewAction = new csModelDataAction ();
      dest->QueryObject ()->ObjAdd (NewAction->QueryObject ());
      NewAction->QueryObject ()->SetName (OldAction->QueryObject ()->GetName ());
    } else {
      while (NewAction->GetFrameCount () > 0)
        NewAction->DeleteFrame (0);
    }

	int i;
    for (i=0; i<OldAction->GetFrameCount (); i++)
    {
      iModelDataVertices *oldver = SCF_QUERY_INTERFACE_FAST (OldAction->GetState (i),
        iModelDataVertices);
      if (oldver)
      {
        ver = BuildMappedVertexFrame (oldver, Map);
	NewAction->AddFrame (OldAction->GetTime (i), ver->QueryObject ());
        oldver->DecRef ();
	ver->DecRef ();
      }
    }
    NewAction->DecRef ();
    it.Next ();
  }
}

static bool CheckActionConflict (iModelDataObject *obj1, iModelDataObject *obj2)
{
  csModelDataActionIterator it (obj1->QueryObject ());
  while (!it.IsFinished ())
  {
    iModelDataAction *Action = it.Get ();

    iModelDataAction *Action2 = CS_GET_NAMED_CHILD_OBJECT_FAST (obj2->QueryObject (),
      iModelDataAction, Action->QueryObject ()->GetName ());

    if (Action2)
    {
      float time2 = Action2->GetTotalTime ();
      Action2->DecRef ();
      if (MergeTimes (Action->GetTotalTime (), time2) < 0)
        return true;
    }
    it.Next ();
  }

  return false;
}

static bool CheckMaterialConflict (iModelDataObject *obj1, iModelDataObject *obj2)
{
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

  int i, j;
  for (i=0; i<mat1.Length (); i++)
  {
    for (j=0; j<mat2.Length (); j++)
    {
      if (mat1.Get (i) != mat2.Get (j))
        return true;
    }
  }

  return false;
}

void csModelDataTools::MergeObjects (iModelData *Scene, bool MultiTexture)
{
  csModelDataObjectVector OldObjects, NewObjects;

  ExtractObjects (Scene, OldObjects);

  while (OldObjects.Length () > 0)
  {
    iModelDataObject *obj = OldObjects.Pop ();

    // look if we can merge this object with an existing one
    int i;
    for (i=0; i<NewObjects.Length (); i++)
    {
      iModelDataObject *obj2 = NewObjects.Get (i);
      if ((MultiTexture || !CheckMaterialConflict (obj, obj2)) &&
        !CheckActionConflict (obj, obj2))
      {
        MergeCopyObject (obj2, obj);
	break;
      }
    }
    if (i == NewObjects.Length ())
    {
      Scene->QueryObject ()->ObjAdd (obj->QueryObject ());
      NewObjects.Push (obj);
    }
    obj->DecRef ();
  }
}

void csModelDataTools::SplitObjectsByMaterial (iModelData *Scene)
{
  csModelDataObjectVector OldObjects;

  ExtractObjects (Scene, OldObjects);

  while (OldObjects.Length () > 0)
  {
    iModelDataObject *obj = OldObjects.Pop ();

    if (!CheckMaterialConflict (obj)) {
      Scene->QueryObject ()->ObjAdd (obj->QueryObject ());
    } else {
      csModelDataPolygonVector Polygons;
      csModelDataMaterialVector Materials;
      csIntArray PolygonToNewObject;
      csModelDataObjectVector NewObjects;

      // build the Polygons, Materials and PolygonToNewObject lists
      csModelDataPolygonIterator it (obj->QueryObject ());
      while (!it.IsFinished ())
      {
        iModelDataPolygon *poly = it.Get ();

	Polygons.Push (poly);

        int n = Materials.Find (poly->GetMaterial ());
        if (n == -1) {
          PolygonToNewObject.Push (NewObjects.Length ());
          Materials.Push (poly->GetMaterial ());

	  iModelDataObject *obj = new csModelDataObject ();
	  NewObjects.Push (obj);
	  Scene->QueryObject ()->ObjAdd (obj->QueryObject ());
	  obj->DecRef ();
        } else {
          PolygonToNewObject.Push (n);
        }

        it.Next ();
      }

      // build the vertex mapping table and move the polygons
      int i;
      csIntArray *VertexMap = new csIntArray [Materials.Length ()];
      csIntArray *NormalMap = new csIntArray [Materials.Length ()];
      csIntArray *ColorMap = new csIntArray [Materials.Length ()];
      csIntArray *TexelMap = new csIntArray [Materials.Length ()];
      for (i=0; i<Polygons.Length (); i++)
      {
        iModelDataPolygon *Polygon = Polygons.Get (i);
        int NewObjIndex = PolygonToNewObject.Get (i);
	csIntArray *VertexIndexTable = &(VertexMap [NewObjIndex]);
	csIntArray *NormalIndexTable = &(NormalMap [NewObjIndex]);
	csIntArray *ColorIndexTable = &(ColorMap [NewObjIndex]);
	csIntArray *TexelIndexTable = &(TexelMap [NewObjIndex]);
	iModelDataObject *Object = NewObjects.Get (NewObjIndex);

        Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
	Scene->QueryObject ()->ObjRemove (Polygon->QueryObject ());

	int j;
	for (j=0; j<Polygon->GetVertexCount (); j++)
	{

#define CS_MDLTOOL_HELPER(obj)						\
  n = obj##IndexTable->Find (Polygon->Get##obj (j));			\
  if (n != -1) {							\
    Polygon->Set##obj (j, n);						\
  } else {								\
    obj##IndexTable->Push (Polygon->Get##obj (j));			\
    Polygon->Set##obj (j, obj##IndexTable->Length () - 1);		\
  }
        
          int n;
          CS_MDLTOOL_HELPER (Vertex);
          CS_MDLTOOL_HELPER (Normal);
          CS_MDLTOOL_HELPER (Color);
          CS_MDLTOOL_HELPER (Texel);

	}
      }

#undef CS_MDLTOOL_HELPER

      // copy the vertices themselves
      for (i=0; i<NewObjects.Length (); i++)
      {
        csModelDataVertexMap Map;
	Map.VertexCount = VertexMap [i].Length ();
	Map.NormalCount = NormalMap [i].Length ();
	Map.ColorCount = ColorMap [i].Length ();
	Map.TexelCount = TexelMap [i].Length ();
	Map.Vertices = VertexMap [i].GetArray ();
	Map.Normals = NormalMap [i].GetArray ();
	Map.Colors = ColorMap [i].GetArray ();
	Map.Texels = TexelMap [i].GetArray ();
        CopyVerticesMapped (NewObjects.Get (i), obj, &Map);
      }

      delete [] VertexMap;
      delete [] NormalMap;
      delete [] ColorMap;
      delete [] TexelMap;
    }
    obj->DecRef ();
  }
}

// ---------------------------------------------------------------------------
// Debug output functions
// ---------------------------------------------------------------------------

#define CS_MDLTOOL_TRY_BEGIN(obj,t)				\
  {								\
    t *Object = SCF_QUERY_INTERFACE_FAST (obj, t);		\
    if (Object) {						\
      typestring = #t;

#define CS_MDLTOOL_TRY_END					\
      Object->DecRef ();					\
    }								\
  }

void csModelDataTools::Describe (iObject *obj, csString &out)
{
  static int Indent = 0;
  static char *Indention = NULL;
  int i;
  if (!Indention) {
    Indention = new char [2000];
    memset (Indention, ' ', 2000);
    Indention[0] = 0;
  }

  Indention [Indent] = ' ';
  Indent += 2;
  Indention [Indent] = 0;

  csString typestring, contents;

  CS_MDLTOOL_TRY_BEGIN (obj, iModelData)
  CS_MDLTOOL_TRY_END

  CS_MDLTOOL_TRY_BEGIN (obj, iModelDataObject)
    contents << Indention << "DefaultVertices :\n";
    Describe (Object->GetDefaultVertices ()->QueryObject (), contents);
  CS_MDLTOOL_TRY_END

  CS_MDLTOOL_TRY_BEGIN (obj, iModelDataPolygon)
    for (i=0; i<Object->GetVertexCount (); i++)
    {
      contents << Indention << "Vertex <v" <<
        Object->GetVertex (i) << ",n" <<
        Object->GetNormal (i) << ",t" <<
        Object->GetTexel (i) << ",c" <<
        Object->GetColor (i) << ">\n";
    }
  CS_MDLTOOL_TRY_END

  CS_MDLTOOL_TRY_BEGIN (obj, iModelDataAction)
    for (i=0; i<Object->GetFrameCount (); i++)
    {
      contents << Indention << "Frame <" << Object->GetTime (i) <<
        "> :\n";
      Describe (Object->GetState (i), contents);
    }
  CS_MDLTOOL_TRY_END

  CS_MDLTOOL_TRY_BEGIN (obj, iModelDataVertices)
    int i;
    for (i=0; i<Object->GetVertexCount (); i++)
    { csVector3 v = Object->GetVertex (i); contents << Indention <<
      "Vertex <" << v.x << ',' << v.y << ',' << v.z << ">\n"; }
    for (i=0; i<Object->GetNormalCount (); i++)
    { csVector3 v = Object->GetNormal (i); contents << Indention <<
      "Normal <" << v.x << ',' << v.y << ',' << v.z << ">\n"; }
    for (i=0; i<Object->GetTexelCount (); i++)
    { csVector2 v = Object->GetTexel (i); contents << Indention <<
      "Texel <" << v.x << ',' << v.y << ">\n"; }
    for (i=0; i<Object->GetColorCount (); i++)
    { csColor v = Object->GetColor (i); contents << Indention <<
      "Color <" << v.red << ',' << v.green << ',' << v.blue << ">\n"; }
  CS_MDLTOOL_TRY_END

  CS_MDLTOOL_TRY_BEGIN (obj, iModelDataTexture)
  CS_MDLTOOL_TRY_END

  CS_MDLTOOL_TRY_BEGIN (obj, iModelDataMaterial)
  CS_MDLTOOL_TRY_END

  iObjectIterator *it = obj->GetIterator ();
  while (!it->IsFinished ())
  {
    Describe (it->GetObject (), contents);
    it->Next ();
  }

  it->DecRef ();
  Indent -= 2;
  Indention [Indent] = 0;

  csString s;
  s << Indention << "object '" << obj->GetName () << "' [" << typestring <<
    "] [" << obj->GetRefCount () << "] (\n";
  s << contents;
  s << Indention << ") \n";
  out << s;
}

#undef CS_MDLTOOL_TRY_BEGIN
#undef CS_MDLTOOL_TRY_END

void csModelDataTools::CompressVertices (iModelData *Scene)
{
  csModelDataObjectIterator it (Scene->QueryObject ());
  while (!it.IsFinished ())
  {
    CompressVertices (it.Get ());
    it.Next ();
  }
}

void csModelDataTools::CompressVertices (iModelDataObject *Object)
{
  csModelDataVerticesVector VertexFrames;
  csModelDataPolygonVector Polygons;
  int i,j,k;

  // collect all vertex frames and polygons
  VertexFrames.Push (Object->GetDefaultVertices ());
  iObjectIterator *it = Object->QueryObject ()->GetIterator ();
  while (!it->IsFinished ())
  {
    iModelDataPolygon *Polygon =
      SCF_QUERY_INTERFACE_FAST (it->GetObject (), iModelDataPolygon);
    if (Polygon)
    {
      Polygons.Push (Polygon);
      Polygon->DecRef ();
    }

    iModelDataAction *Action =
      SCF_QUERY_INTERFACE_FAST (it->GetObject (), iModelDataAction);
    if (Action)
    {
      for (i=0; i<Action->GetFrameCount (); i++)
      {
        iModelDataVertices *ver =
	  SCF_QUERY_INTERFACE_FAST (Action->GetState (i), iModelDataVertices);
	if (ver)
	{
	  VertexFrames.PushSmart (ver);
	  ver->DecRef ();
	}
      }
    }
    it->Next ();
  }
  it->DecRef ();

  // extract the data from one of the vertex frames into growing arrays
  csVector3Array VertexList;
  csVector3Array NormalList;
  csColorArray ColorList;
  csVector2Array TexelList;

  iModelDataVertices *ver = VertexFrames[0];
  DumpVertices (ver, &VertexList, &NormalList, &ColorList, &TexelList);

  // build the initial 'potential mergeable vertices' list
  csIntArray *VertexListIndices = csIntArray::CreateIdentityMapping (VertexList.Length ());
  csIntArray *NormalListIndices = csIntArray::CreateIdentityMapping (NormalList.Length ());
  csIntArray *ColorListIndices = csIntArray::CreateIdentityMapping (ColorList.Length ());
  csIntArray *TexelListIndices = csIntArray::CreateIdentityMapping (TexelList.Length ());
  csIntArrayVector VertexSets;
  csIntArrayVector NormalSets;
  csIntArrayVector ColorSets;
  csIntArrayVector TexelSets;
  csIntArray UniqueVertexList;
  csIntArray UniqueNormalList;
  csIntArray UniqueColorList;
  csIntArray UniqueTexelList;

#define CS_MDLTOOL_HELPER(type,obj,comp)				\
  while (obj##List.Length () > 0)					\
  {									\
    csIntArray *Set = new csIntArray ();				\
    type o1 = obj##List [0];						\
    int Index1 = obj##ListIndices->Get (0);				\
    Set->Push (Index1);							\
    obj##List.Delete (0);						\
    obj##ListIndices->Delete (0);					\
									\
    for (i=0; i<obj##List.Length (); i++)				\
    {									\
      type o2 = obj##List [i];						\
      if (comp)								\
      {									\
        Set->Push (obj##ListIndices->Get (i));				\
	obj##List.Delete (i);						\
	obj##ListIndices->Delete (i);					\
	i--;								\
      }									\
    }									\
									\
    if (Set->Length () > 1)						\
      obj##Sets.Push (Set);						\
    else								\
      { Unique##obj##List.Push (Index1); delete Set; }			\
  }

  CS_MDLTOOL_HELPER (csVector3, Vertex, (o1 - o2 < EPSILON));
  CS_MDLTOOL_HELPER (csVector3, Normal, (o1 - o2 < EPSILON));
  CS_MDLTOOL_HELPER (csColor, Color,
    ABS (o1.red - o2.red) < SMALL_EPSILON &&
    ABS (o1.green - o2.green) < SMALL_EPSILON &&
    ABS (o1.blue - o2.blue) < SMALL_EPSILON );
  CS_MDLTOOL_HELPER (csVector2, Texel, (o1 - o2 < EPSILON));
#undef CS_MDLTOOL_HELPER

  // now look which parts of these sets are valid for all frames
#define CS_MDLTOOL_HELPER(type,obj,comp)				\
  for (i=0; i<obj##Sets.Length (); i++)					\
  {									\
    csIntArray *Set = obj##Sets [i];					\
    csIntArray *Removed = NULL;						\
    type o1 = ver->Get##obj (Set->Get (0));				\
    for (j=1; j<Set->Length (); j++)					\
    {									\
      type o2 = ver->Get##obj (Set->Get (j));				\
      if (!(comp)) {							\
        if (!Removed) Removed = new csIntArray ();			\
        Removed->Push (Set->Get (j));					\
        Set->Delete (j); j--;						\
      }									\
    }									\
    if (Removed) {							\
      if (Removed->Length () < 2)					\
      { Unique##obj##List.Push (Removed->Get (0)); delete Removed; }	\
      else obj##Sets.Push (Removed);					\
    }									\
    if (Set->Length () < 2) {obj##Sets.Delete (i); i--;}		\
  }

  for (k=1; k<VertexFrames.Length (); k++)
  {
    ver = VertexFrames [k];

    CS_MDLTOOL_HELPER (csVector3, Vertex, (o1 - o2 < EPSILON));
    CS_MDLTOOL_HELPER (csVector3, Normal, (o1 - o2 < EPSILON));
    CS_MDLTOOL_HELPER (csColor, Color,
      ABS (o1.red - o2.red) < SMALL_EPSILON &&
      ABS (o1.green - o2.green) < SMALL_EPSILON &&
      ABS (o1.blue - o2.blue) < SMALL_EPSILON );
    CS_MDLTOOL_HELPER (csVector2, Texel, (o1 - o2 < EPSILON));
  }
#undef CS_MDLTOOL_HELPER

  // build the final mapping tables
  csIntArray VertexMapN2O, NormalMapN2O, ColorMapN2O, TexelMapN2O;
  csIntArray VertexMapO2N, NormalMapO2N, ColorMapO2N, TexelMapO2N;
  VertexMapO2N.SetLength (VertexFrames[0]->GetVertexCount ());
  NormalMapO2N.SetLength (VertexFrames[0]->GetNormalCount ());
  ColorMapO2N.SetLength (VertexFrames[0]->GetColorCount ());
  TexelMapO2N.SetLength (VertexFrames[0]->GetTexelCount ());

#define CS_MDLTOOL_HELPER(obj)						\
  for (i=0; i<Unique##obj##List.Length (); i++)				\
  {									\
    obj##MapN2O.Push (Unique##obj##List [i]);				\
    obj##MapO2N [Unique##obj##List [i]] = i;				\
  }									\
  for (i=0; i<obj##Sets.Length (); i++)					\
  {									\
    obj##MapN2O.Push (obj##Sets [i]->Get (0));				\
    for (j=0; j<obj##Sets [i]->Length (); j++)				\
    {									\
      obj##MapO2N [obj##Sets [i]->Get (j)] = Unique##obj##List.Length () + i; \
    }									\
  }

  CS_MDLTOOL_HELPER (Vertex);
  CS_MDLTOOL_HELPER (Normal);
  CS_MDLTOOL_HELPER (Color);
  CS_MDLTOOL_HELPER (Texel);
#undef CS_MDLTOOL_HELPER

  // apply the mapping to all vertex frames
#define CS_MDLTOOL_HELPER(obj)						\
  while (ver->Get##obj##Count () > 0)					\
    ver->Delete##obj (0);						\
  for (i=0; i<obj##MapN2O.Length (); i++)				\
    ver->Add##obj (obj##List [obj##MapN2O [i]]);

  while (VertexFrames.Length () > 0)
  {
    ver = VertexFrames.Pop ();
    VertexList.SetLength (0);
    NormalList.SetLength (0);
    ColorList.SetLength (0);
    TexelList.SetLength (0);
    DumpVertices (ver, &VertexList, &NormalList, &ColorList, &TexelList);
    CS_MDLTOOL_HELPER (Vertex);
    CS_MDLTOOL_HELPER (Normal);
    CS_MDLTOOL_HELPER (Color);
    CS_MDLTOOL_HELPER (Texel);
    ver->DecRef ();
  }
#undef CS_MDLTOOL_HELPER

  // apply the mapping to all polygon
#define CS_MDLTOOL_HELPER(obj)						\
  for (i=0; i<poly->GetVertexCount (); i++)				\
    Orig[i] = poly->Get##obj (i);					\
  for (i=0; i<poly->GetVertexCount (); i++)				\
    poly->Set##obj (i, obj##MapO2N [Orig [i]]);				\

  while (Polygons.Length () > 0)
  {
    iModelDataPolygon *poly = Polygons.Pop ();
    int *Orig = new int [poly->GetVertexCount ()];
    CS_MDLTOOL_HELPER (Vertex);
    CS_MDLTOOL_HELPER (Normal);
    CS_MDLTOOL_HELPER (Color);
    CS_MDLTOOL_HELPER (Texel);
    poly->DecRef ();
    delete[] Orig;
  }
}

void csModelDataTools::BuildVertexArray (
	iModelDataPolygon* poly,
	csIntArray* SpriteVertices,
	csIntArray* SpriteNormals,
	csIntArray* SpriteColors,
	csIntArray* SpriteTexels,
	csIntArray* PolyVertices)
{
  int i;
  PolyVertices->SetLength (0);
  csSingleIndexVertexSet set (SpriteVertices,
    SpriteNormals, SpriteColors, SpriteTexels, false);

  for (i=0; i<poly->GetVertexCount (); i++)
  {
    int idx = set.Add (poly->GetVertex (i), poly->GetNormal (i),
      poly->GetColor (i), poly->GetTexel (i));
    PolyVertices->Push (idx);
  }
}
