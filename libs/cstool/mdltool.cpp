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
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "cstool/mdldata.h"
#include "cstool/mdltool.h"

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

CS_TYPEDEF_GROWING_ARRAY (csIntArray, int);
CS_TYPEDEF_GROWING_ARRAY (csIntArrayArray, csIntArray);

CS_DECLARE_OBJECT_VECTOR (csModelDataActionVector, iModelDataAction);
CS_DECLARE_OBJECT_VECTOR (csModelDataMaterialVector, iModelDataMaterial);
CS_DECLARE_OBJECT_VECTOR (csModelDataObjectVector, iModelDataObject);
CS_DECLARE_OBJECT_VECTOR (csModelDataPolygonVector, iModelDataPolygon);

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
 * MergeVertices (): Concatenate the vertex, normal, color and texel lists of
 * the given vertex frames and build a new frame from them.
 */
static iModelDataVertices *MergeVertices (const iModelDataVertices *v1,
	const iModelDataVertices *v2)
{
#define CS_MERGE_VERTICES_HELPER(vnum,obj)		\
	for (i=0; i<vnum->Get##obj##Count (); i++)	\
	  ver->Add##obj (vnum->Get##obj (i));

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

#undef CS_MERGE_VERTICES_HELPER
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

  // copy the default vertices
  iModelDataVertices *ver = MergeVertices (DefaultVertices, src->GetDefaultVertices ());
  dest->SetDefaultVertices (ver);
  ver->DecRef ();

  // copy all polygons
  csModelDataPolygonIterator *polyit = new csModelDataPolygonIterator (src->QueryObject ());
  while (!polyit->IsFinished ())
  {
    iModelDataPolygon *poly = polyit->Get ();
    iModelDataPolygon *NewPoly = new csModelDataPolygon ();
    dest->QueryObject ()->ObjAdd (NewPoly->QueryObject ());

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
  csModelDataActionVector ActionMap1, ActionMap2;

  csModelDataActionIterator *actit = new csModelDataActionIterator (dest->QueryObject ());
  while (!actit->IsFinished ())
  {
    iModelDataAction *Action = actit->Get ();

    ActionMap1.Push (Action);
    ActionMap2.Push (NULL);
    dest->QueryObject ()->ObjRemove (Action->QueryObject ());

    actit->Next ();
  }
  delete actit;

  actit = new csModelDataActionIterator (src->QueryObject ());
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
		     *NewAction;
    if (Action1) {
      if (Action2) {
        // this should not happen
	CS_ASSERT (("Action conflict detection missed a conflict!!!", false));
	NewAction = NULL;
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

    for (int i=0; i<OldAction->GetFrameCount (); i++)
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
      Action2->DecRef ();
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

  for (int i=0; i<mat1.Length (); i++)
  {
    for (int j=0; j<mat2.Length (); j++)
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
      iModelDataObject *NewObject = new csModelDataObject ();
      Scene->QueryObject ()->ObjAdd (NewObject->QueryObject ());
      MergeCopyObject (NewObject, obj);
      NewObjects.Push (NewObject);
      NewObject->DecRef ();
    }
    obj->DecRef ();
  }
}

void csModelDataTools::SplitObjectsByMaterial (iModelData *Scene)
{
  csModelDataObjectVector OldObjects, NewObjects;

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

	for (int j=0; j<Polygon->GetVertexCount (); j++)
	{

#define CS_MDLTOOL_HELPER(obj)						\
  n = -1;								\
  for (k=0; k<obj##IndexTable->Length (); k++)				\
    if (obj##IndexTable->Get (k) == Polygon->Get##obj (j))		\
      { n = k; break; }							\
  if (n != -1) {							\
    Polygon->Set##obj (j, n);						\
  } else {								\
    obj##IndexTable->Push (Polygon->Get##obj (j));			\
    Polygon->Set##obj (j, obj##IndexTable->Length () - 1);		\
  }
        
          int n, k;
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
    }								\
  }

void csModelDataTools::Describe (iObject *obj, csString &out)
{
  static int Indent = 0;
  static char *Indention = NULL;
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
    for (int i=0; i<Object->GetVertexCount (); i++)
    {
      contents << Indention << "Vertex <v" <<
        Object->GetVertex (i) << ",n" <<
        Object->GetNormal (i) << ",t" <<
        Object->GetTexel (i) << ",c" <<
        Object->GetColor (i) << ">\n";
    }
  CS_MDLTOOL_TRY_END

  CS_MDLTOOL_TRY_BEGIN (obj, iModelDataAction)
    for (int i=0; i<Object->GetFrameCount (); i++)
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
  s << Indention << "object '" << obj->GetName () << "' [" << typestring << "] (\n";
  s << contents;
  s << Indention << ") \n";
  out << s;
}

#undef CS_MDLTOOL_TRY_BEGIN
#undef CS_MDLTOOL_TRY_END
