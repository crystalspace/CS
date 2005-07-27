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
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "cstool/sprbuild.h"
#include "cstool/mdltool.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/objiter.h"

#include "imesh/mdldata.h"
#include "imesh/sprite3d.h"
#include "ivideo/graph3d.h"

typedef csArray<iModelDataVertices*> csModelFrameVector;

struct UsedVertexInfo
{
  int normal;
  int texel;
  int vidx;
};

typedef csDirtyAccessArray<UsedVertexInfo> UsedVerticesInfo;

bool csSpriteBuilder::Build (iModelDataObject *Object)
{
  size_t i,j;
  csRef<iObjectIterator> it1;
  iModelDataMaterial *Material = 0;

  //--- preparation stage: arrange and validate incoming data locally --------

  // build a list of all frames and merge duplicate frames
  csModelFrameVector Frames;

  it1 = Object->QueryObject ()->GetIterator ();
  while (it1->HasNext ())
  {
    csRef<iModelDataAction> ac (SCF_QUERY_INTERFACE (it1->Next (),
        iModelDataAction));
    if (ac)
    {
      for (i=0; i<ac->GetFrameCount (); i++)
      {
        csRef<iModelDataVertices> ver (
                SCF_QUERY_INTERFACE (ac->GetState (i), iModelDataVertices));
        if (ver)
          Frames.PushSmart (ver);
      }
    }
  }

  // we need at least one vertex frame
  if (Frames.Length () == 0)
  {
    iModelDataVertices *BaseVertices = Object->GetDefaultVertices ();
    if (!BaseVertices) return false;
    Frames.Push (BaseVertices);
  }

  //--- building stage -------------------------------------------------------

  Begin ();

  /* These lists are filled up with (sprite vertex) to (model data vertex),
   * (model data normal) and (model data texel) mappings. This means that
   * they are indexed by the sprite vertex and must be of the same size.
   */
  csDirtyAccessArray<int> SpriteVertices;
  csDirtyAccessArray<int> SpriteNormals;
  csDirtyAccessArray<int> SpriteTexels;

#if 0
  int vertices=0;
  // count polygons
  it1 = Object->QueryObject ()->GetIterator ();
  while (it1->HasNext ())
  {
    iModelDataPolygon *poly =
      SCF_QUERY_INTERFACE (it1->Next (), iModelDataPolygon);
    if (poly)
    {
      vertices += poly->GetVertexCount ();
    }
  }

  // reserve memory
  SpriteVertices.SetLimit (vertices+1);
  SpriteNormals.SetLimit (vertices+1);
  SpriteTexels.SetLimit (vertices+1);

  // copy polygon data (split polygons into triangles)
  it1 = Object->QueryObject ()->GetIterator ();
  while (it1->HasNext ())
  {
    csRef<iModelDataPolygon> poly (
        SCF_QUERY_INTERFACE (it1->Next (), iModelDataPolygon));
    if (poly)
    {
      // build the vertex array
      csDirtyAccessArray<int> PolyVertices;
      csModelDataTools::BuildVertexArray (poly, &SpriteVertices,
        &SpriteNormals, 0, &SpriteTexels, &PolyVertices);

      // split the polygon into triangles and copy them
      for (i=2; i<PolyVertices.Length (); i++)
        StoreTriangle (PolyVertices[0], PolyVertices[i-1], PolyVertices[i]);

      // store the material if we don't have any yet
      if (!Material && poly->GetMaterial ())
        Material = poly->GetMaterial ();
    }
  }

#else
/*
  Every output vertex must have exactly on texture coordinate and one normal.
  However, it can happen that actually more than one normal/texcoord
  occurs for the same vertex (for example by hard edges in 3ds files).
  So we make sure that every vertex/texcoord/normal combo gets its
  own output vertex, but without the waste of vertices when simply every
  point in every polygon gets its own output vertex.

  the following is done:
   - for every vertex a table with texcoord/normal combos and its output
     vertex index is created
   - for every vertex we encounter in the polys we check this table if that
     texcoord/normal combo appeared. if yes take the output vertex index,
     otherwise add this combo.
*/

  size_t vertices=Frames[0]->GetVertexCount();
  UsedVerticesInfo *usedvertices = new UsedVerticesInfo [vertices];

  it1 = Object->QueryObject ()->GetIterator ();
  while (it1->HasNext ())
  {
    csRef<iModelDataPolygon> poly (
      SCF_QUERY_INTERFACE (it1->Next (), iModelDataPolygon));
    if (poly)
    {
      csDirtyAccessArray<int> PolyVertices;
      for (i=0; i<poly->GetVertexCount(); i++)
      {
        int vertex = poly->GetVertex (i);
        int normal = poly->GetNormal (i);
        int texel = poly->GetTexel (i);

        UsedVerticesInfo *vinfo = &usedvertices[vertex];
        int index = -1;

        for (j=0; j<vinfo->Length(); j++)
        {
          if ((vinfo->Get(j).normal == normal) && (vinfo->Get(j).texel == texel))
          {
            index = vinfo->Get(j).vidx;
            break;
          }
        }
        if (index == -1)
        {
          index = (int)SpriteVertices.Push (vertex);
          SpriteNormals.Push (normal);
          SpriteTexels.Push (texel);
          UsedVertexInfo vtxinfo;
          vtxinfo.normal = normal;
          vtxinfo.texel = texel;
          vtxinfo.vidx = index;
          vinfo->Push (vtxinfo);
        }
        PolyVertices.Push (index);
      }

      // split the polygon into triangles and copy them
      for (i=2; i<PolyVertices.Length (); i++)
      {
        StoreTriangle (PolyVertices[0], PolyVertices[i-1], PolyVertices[i]);
      }

      // store the material if we don't have any yet
      if (!Material && poly->GetMaterial ())
        Material = poly->GetMaterial ();
    }
  }

  delete[] usedvertices;
#endif

  // copy the first valid material
  if (Material) StoreMaterial (Material);

  /* create all frames in the target factory. This is done separately because
   * adding the vertices fails if no frames exist.
   */
  int NumPreviousFrames = StoreFrameInfo ((int)Frames.Length (), 
    (int)SpriteVertices.Length ());

  // create all frames in the target factory
  bool NeedTiling = false;
  for (i=0; i<Frames.Length (); i++)
  {
    int FrameIndex = NumPreviousFrames + (int)i;
    BeginFrame (FrameIndex);
    iModelDataVertices *Vertices = Frames.Get (i);

    for (j=0; j<SpriteVertices.Length (); j++)
    {
      csVector2 t = Vertices->GetTexel (SpriteTexels [j]);
      // @@@ hmmm... isn't 1 actually already wrapped around to 0?
      // should be '>= 1', not just '> 1' then
      if (t.x < 0 || t.y < 0 || t.x > 1 || t.y > 1)
        NeedTiling = true;

      AddVertex (Vertices->GetVertex (SpriteVertices [j]),
                 Vertices->GetNormal (SpriteNormals [j]), t);
    }
    FinishFrame ();
  }

  // enable texture tiling if required
  if (NeedTiling) EnableTiling ();

  /* Create all actions in the target factory. We also build a default
   * action (named 'default') if no action with this name exists. The
   * default action shows the first frame all the time.
   */
  bool FoundDefault = false;

  it1 = Object->QueryObject ()->GetIterator ();
  while (it1->HasNext ())
  {
    csRef<iModelDataAction> ac (
        SCF_QUERY_INTERFACE (it1->Next (), iModelDataAction));
    if (ac)
    {
      const char *name = ac->QueryObject ()->GetName ();
      BeginAction (name);
      if (name && !strcmp (name, "default"))
        FoundDefault = true;

      float LastTime = 0;
      for (i=0; i<ac->GetFrameCount (); i++)
      {
        /* It might seem strange to store the nth frame time value with the
         * (n-1)th frame state. This difference is due to the different
         * meaning of the time values in the model data structures and
         * in 3d sprites.
         */
        int FrameIndex = (int)((i == 0) ? (ac->GetFrameCount ()-1) : (i-1));
        csRef<iModelDataVertices> ver (SCF_QUERY_INTERFACE (
                ac->GetState (FrameIndex), iModelDataVertices));
        if (ver)
        {
          float ThisTime = ac->GetTime (i);
          float Delay = ThisTime - LastTime;
          LastTime = ThisTime;

	  size_t FrameIndex = Frames.Find (ver);
	  CS_ASSERT (FrameIndex != csArrayItemNotFound);
          StoreActionFrame ((int)FrameIndex, int(Delay * 1000),0);
        }
      }
      FinishAction ();
    }
  }

  if (!FoundDefault)
  {
    BeginAction ("default");
    StoreActionFrame (0, 1000,0);
    FinishAction ();
  }

  Finish ();

  return true;
}

// --- file version ----------------------------------------------------------

void csSpriteBuilderFile::Begin ()
{
  char *name = "obj";
  Out << "MESHOBJ '" << name << "' (\n";
  Out << "  PLUGIN ('crystalspace.mesh.loader.factory.sprite.3d')\n";
  Out << "  PARAMS (\n";
}

void csSpriteBuilderFile::Finish ()
{
  Out << "  )\n";
  Out << ")\n";
}

void csSpriteBuilderFile::StoreTriangle (int a, int b, int c)
{
  Out << "    TRIANGLE (" << a << ',' << b << ',' << c << ")\n";
}

void csSpriteBuilderFile::StoreMaterial (iModelDataMaterial * /*mat*/)
{
  char *name = "obj";
  Out << "    MATERIAL ('" << name << "skin')\n";
}

int csSpriteBuilderFile::StoreFrameInfo (int /*FrameCount*/, int /*VertexCount*/)
{
  return 0;
}

void csSpriteBuilderFile::EnableTiling ()
{
  Out << "    MIXMODE (TILING ())\n";
}

void csSpriteBuilderFile::BeginFrame (int Num)
{
  Out << "    FRAME 'frame" << Num << "' (\n";
}

void csSpriteBuilderFile::FinishFrame ()
{
  Out << "    )\n";
}

void csSpriteBuilderFile::AddVertex (const csVector3 &pos,
        const csVector3 & /*nrm*/, const csVector2 &tex)
{
  Out << "      V (" << pos.x << ',' << pos.y << ',' << pos.z;
  Out << ':' << tex.x << ',' << tex.y << ")\n";
}

void csSpriteBuilderFile::BeginAction (const char *Name)
{
  Out << "    ACTION '" << Name << "' (\n";
}

void csSpriteBuilderFile::FinishAction ()
{
  Out << "    )\n";
}

void csSpriteBuilderFile::StoreActionFrame (int Frame, csTicks Delay, float displacement)
{
  Out << "      F ('frame" << Frame << "', " << Delay << "," << displacement << ")\n";
}

csPtr<iDataBuffer> csSpriteBuilderFile::Build (iModelDataObject *Input)
{
  Out.Clear ();
  csSpriteBuilder::Build (Input);
  size_t Size = Out.Length () + 1;
  return csPtr<iDataBuffer> (new csDataBuffer (Out.Detach (), Size));
}

// --- mesh version ----------------------------------------------------------

void csSpriteBuilderMesh::Begin ()
{
}

void csSpriteBuilderMesh::Finish ()
{
}

void csSpriteBuilderMesh::StoreTriangle (int a, int b, int c)
{
  Out->AddTriangle (a, b, c);
}

void csSpriteBuilderMesh::StoreMaterial (iModelDataMaterial *mat)
{
  Out->SetMaterialWrapper (mat->GetMaterialWrapper ());
}

int csSpriteBuilderMesh::StoreFrameInfo (int FrameCount, int VertexCount)
{
  int NumPreviousFrames = Out->GetFrameCount ();

  int i;
  for (i=0; i<FrameCount; i++) Out->AddFrame ();
  Out->AddVertices (VertexCount);
  return NumPreviousFrames;
}

void csSpriteBuilderMesh::EnableTiling ()
{
  Out->SetMixMode (Out->GetMixMode () | CS_FX_TILING);
}

void csSpriteBuilderMesh::BeginFrame (int Num)
{
  csString name;
  name << "frame" << Num;

  CurrentFrame = Out->GetFrame (Num);
  CurrentFrame->SetName (name);
  CurrentFrameNum = Num;
  CurrentVertexNum = 0;
}

void csSpriteBuilderMesh::FinishFrame ()
{
}

void csSpriteBuilderMesh::AddVertex (const csVector3 &pos,
        const csVector3 &nrm, const csVector2 &tex)
{
  Out->SetVertex (CurrentFrameNum, CurrentVertexNum, pos);
  Out->SetNormal (CurrentFrameNum, CurrentVertexNum, nrm);
  Out->SetTexel (CurrentFrameNum, CurrentVertexNum, tex);
  CurrentVertexNum++;
}

void csSpriteBuilderMesh::BeginAction (const char *Name)
{
  CurrentAction = Out->AddAction ();
  CurrentAction->SetName (Name);
}

void csSpriteBuilderMesh::FinishAction ()
{
}

void csSpriteBuilderMesh::StoreActionFrame (int Frame, csTicks Delay, float displacement)
{
  CurrentAction->AddFrame (Out->GetFrame (Frame), Delay, displacement);
}

bool csSpriteBuilderMesh::Build (iModelDataObject *Input,
        iSprite3DFactoryState *Output)
{
  Out = Output;
  return csSpriteBuilder::Build (Input);
}
