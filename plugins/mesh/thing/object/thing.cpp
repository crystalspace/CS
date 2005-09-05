/*
    Copyright (C) 1998-2002 by Jorrit Tyberghein

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

//#define __USE_MATERIALS_REPLACEMENT__

#include "cssysdef.h"
#include <limits.h>
#include "csgeom/frustum.h"
#include "csgeom/math3d.h"
#include "csgeom/poly3d.h"
#include "csgeom/polypool.h"
#include "csgeom/sphere.h"
#include "csgeom/subrec.h"
#include "csgfx/memimage.h"
#include "csgfx/shadervarcontext.h"
#include "csqint.h"
#include "csqsqrt.h"
#include "csutil/array.h"
#include "csutil/cfgacc.h"
#include "csutil/csendian.h"
#include "csutil/csmd5.h"
#include "csutil/csstring.h"
#include "csutil/debug.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/hash.h"
#include "csutil/memfile.h"
#include "csutil/timer.h"
#include "csutil/weakref.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/fview.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/shadcast.h"
#include "iengine/shadows.h"
#include "iengine/texture.h"
#include "igraphic/imageio.h"
#include "iutil/cache.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/strset.h"
#include "iutil/verbositymanager.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/polyrender.h"
#include "ivideo/rendermesh.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "lppool.h"
#include "polygon.h"
#include "polytext.h"
#include "thing.h"

#ifdef CS_DEBUG
  //#define LIGHTMAP_DEBUG
#endif

CS_IMPLEMENT_PLUGIN

CS_LEAKGUARD_IMPLEMENT (csThingStatic);
CS_LEAKGUARD_IMPLEMENT (csThing);

int csThing::lightmap_quality = 3;
bool csThing::lightmap_enabled = true;
bool csThingObjectType::do_verbose = false;

//---------------------------------------------------------------------------

class csPolygonHandle : public iPolygonHandle
{
private:
  csWeakRef<iThingFactoryState> factstate;
  csWeakRef<iMeshObjectFactory> factory;
  csWeakRef<iThingState> objstate;
  csWeakRef<iMeshObject> obj;
  int index;

public:
  csPolygonHandle (
        iThingFactoryState* factstate, iMeshObjectFactory* factory,
        iThingState* objstate, iMeshObject* obj,
        int index)
  {
    SCF_CONSTRUCT_IBASE (0);
    csPolygonHandle::factstate = factstate;
    csPolygonHandle::factory = factory;
    csPolygonHandle::objstate = objstate;
    csPolygonHandle::obj = obj;
    csPolygonHandle::index = index;
  }
  virtual ~csPolygonHandle ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;

  virtual iThingFactoryState* GetThingFactoryState () const
  {
    return factstate;
  }
  virtual iMeshObjectFactory* GetMeshObjectFactory () const
  {
    return factory;
  }
  virtual iThingState* GetThingState () const
  {
    return objstate;
  }
  virtual iMeshObject* GetMeshObject () const
  {
    return obj;
  }
  virtual int GetIndex () const
  {
    return index;
  }
};

SCF_IMPLEMENT_IBASE(csPolygonHandle)
  SCF_IMPLEMENTS_INTERFACE(iPolygonHandle)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csThing)
  SCF_IMPLEMENTS_INTERFACE(iLightingInfo)
  SCF_IMPLEMENTS_INTERFACE(iShadowReceiver)
  SCF_IMPLEMENTS_INTERFACE(iMeshObject)
  SCF_IMPLEMENTS_INTERFACE(iShadowCaster)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iThingState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::ThingState)
  SCF_IMPLEMENTS_INTERFACE(iThingState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


int csThing:: last_thing_id = 0;

//----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csThingStatic)
  SCF_IMPLEMENTS_INTERFACE(iThingFactoryState)
  SCF_IMPLEMENTS_INTERFACE(iMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE(iObjectModel)
SCF_IMPLEMENT_IBASE_END

csStringID csThingStatic::texLightmapName = csInvalidStringID;

csThingStatic::csThingStatic (iBase* parent, csThingObjectType* thing_type) :
        last_range (0, -1),
        static_polygons (32, 64),
        scfiPolygonMesh (0),
        scfiPolygonMeshCD (CS_POLY_COLLDET),
        scfiPolygonMeshLOD (CS_POLY_VISCULL)
{
  SCF_CONSTRUCT_IBASE (parent);
  csThingStatic::thing_type = thing_type;
  static_polygons.SetThingType (thing_type);

  scfiPolygonMesh.SetThing (this);
  scfiPolygonMeshCD.SetThing (this);
  scfiPolygonMeshLOD.SetThing (this);
  SetPolygonMeshBase (&scfiPolygonMesh);
  SetPolygonMeshColldet (&scfiPolygonMeshCD);
  SetPolygonMeshViscull (&scfiPolygonMeshLOD);
  SetPolygonMeshShadows (&scfiPolygonMeshLOD);

  max_vertices = num_vertices = 0;
  obj_verts = 0;
  obj_normals = 0;

  cosinus_factor = -1;
  logparent = 0;
  thingmesh_type = thing_type;

  mixmode = (uint)~0;   // Just a marker meaning not set.

  r3d = CS_QUERY_REGISTRY (thing_type->object_reg, iGraphics3D);

  if ((texLightmapName == csInvalidStringID))
  {
    texLightmapName = thing_type->stringset->Request ("tex lightmap");
  }
}

csThingStatic::~csThingStatic ()
{
  delete[] obj_verts;
  delete[] obj_normals;

  UnprepareLMLayout ();

  SCF_DESTRUCT_IBASE ();
}

void csThingStatic::Prepare (iBase* thing_logparent)
{
  if (!IsPrepared())
  {
    SetPrepared (true);

    if (!flags.Check (CS_THING_NOCOMPRESS))
    {
      CompressVertices ();
      RemoveUnusedVertices ();
    }

    if (IsSmoothed())
      CalculateNormals();

    size_t i;
    csPolygon3DStatic* sp;
    for (i = 0; i < static_polygons.Length (); i++)
    {
      sp = static_polygons.Get (i);
      // If a Finish() call returns false this means the textures are not
      // completely ready yet. In that case we set 'prepared' to false
      // again so that we force a new prepare later.
      if (!sp->Finish (thing_logparent))
        SetPrepared (false);
    }
    static_polygons.ShrinkBestFit ();
  }

  if (IsPrepared())
  {
    PrepareLMLayout ();
  }
}

static int CompareStaticPolyGroups (
  csThingStatic::csStaticPolyGroup* const& pg1,
  csThingStatic::csStaticPolyGroup* const& pg2)
{
  float r1 = (float)pg1->totalLumels / (float)pg1->numLitPolys;
  float r2 = (float)pg2->totalLumels / (float)pg2->numLitPolys;

  float rd = r2 - r1;
  if (rd > EPSILON)
  {
    return 1;
  }
  else if (rd < -EPSILON)
  {
    return -1;
  }
  else
  {
    return ((uint8*)pg2 - (uint8*)pg1);
  }
}

void csThingStatic::PrepareLMLayout ()
{
  if (IsLmPrepared()) return;

  csHash<csStaticPolyGroup*, csPtrKey<iMaterialWrapper> > polysSorted;

  int i;
  for (i = 0; i < (int)static_polygons.Length (); i++)
  {
    int polyIdx = i;
    csPolygon3DStatic* sp = static_polygons.Get (polyIdx);

    iMaterialWrapper* mat = sp->GetMaterialWrapper ();

    csStaticPolyGroup* lp = polysSorted.Get (mat, 0);
    if (lp == 0)
    {
      lp = new csStaticPolyGroup;
      lp->material = mat;
      lp->numLitPolys = 0;
      lp->totalLumels = 0;
      polysSorted.Put (mat, lp);
    }

    csPolyTextureMapping* lmi = sp->GetTextureMapping ();
    if ((lmi != 0) &&
        (csThing::lightmap_enabled && sp->flags.Check (CS_POLY_LIGHTING)))
    {
      lp->numLitPolys++;

      int lmw = (csLightMap::CalcLightMapWidth (lmi->GetLitOriginalWidth ()));
      int lmh = (csLightMap::CalcLightMapHeight (lmi->GetLitHeight ()));
      lp->totalLumels += lmw * lmh;
    }

    lp->polys.Push (polyIdx);
  }

  /*
   * Presort polys.
   */
  csArray<csStaticPolyGroup*> polys;
  {
    csHash<csStaticPolyGroup*, csPtrKey<iMaterialWrapper> >::GlobalIterator
      polyIt = polysSorted.GetIterator ();

    while (polyIt.HasNext ())
    {
      csStaticPolyGroup* lp = polyIt.Next ();
      polys.InsertSorted (lp, CompareStaticPolyGroups);
    }
  }

  csStaticPolyGroup* rejectedPolys = new csStaticPolyGroup;
  for (i = 0; i < (int)polys.Length (); i++)
  {
    csStaticPolyGroup* lp = polys[i];
    lp->polys.ShrinkBestFit ();

    if (lp->numLitPolys == 0)
    {
      unlitPolys.Push (lp);
    }
    else
    {
      DistributePolyLMs (*lp, litPolys, rejectedPolys);
      if (rejectedPolys->polys.Length () > 0)
      {
        unlitPolys.Push (rejectedPolys);
        rejectedPolys = new csStaticPolyGroup;
      }

      delete lp;
    }
  }
  delete rejectedPolys;

  litPolys.ShrinkBestFit ();
  unlitPolys.ShrinkBestFit ();

  for (i = 0 ; i < (int)litPolys.Length () ; i++)
  {
    StaticSuperLM* slm = litPolys[i]->staticSLM;
    delete slm->rects;
    slm->rects = 0;
  }

  SetLmPrepared (true);
}

#ifdef LIGHTMAP_DEBUG
#define LM_BORDER 1
#else
#define LM_BORDER 0
#endif

static int CompareStaticSuperLM (csThingStatic::StaticSuperLM* const& slm1,
                                 csThingStatic::StaticSuperLM* const& slm2)
{
  int d = slm2->freeLumels - slm1->freeLumels;
  if (d != 0) return d;
  return ((uint8*)slm2 - (uint8*)slm1);
}

// @@@ urg
static csPolygonStaticArray* static_poly_array = 0;

static int CompareStaticPolys (int const& i1, int const& i2)
{
  csPolygon3DStatic* const poly1 = (*static_poly_array)[i1];
  csPolygon3DStatic* const poly2 = (*static_poly_array)[i2];
  csPolyTextureMapping* lm1 = poly1->GetTextureMapping ();
  csPolyTextureMapping* lm2 = poly2->GetTextureMapping ();

  int maxdim1, mindim1, maxdim2, mindim2;

  maxdim1 = MAX (
    csLightMap::CalcLightMapWidth (lm1->GetLitOriginalWidth ()),
    csLightMap::CalcLightMapHeight (lm1->GetLitHeight ()));
  mindim1 = MIN (
    csLightMap::CalcLightMapWidth (lm1->GetLitOriginalWidth ()),
    csLightMap::CalcLightMapHeight (lm1->GetLitHeight ()));
  maxdim2 = MAX (
    csLightMap::CalcLightMapWidth (lm2->GetLitOriginalWidth ()),
    csLightMap::CalcLightMapHeight (lm2->GetLitHeight ()));
  mindim2 = MIN (
    csLightMap::CalcLightMapWidth (lm2->GetLitOriginalWidth ()),
    csLightMap::CalcLightMapHeight (lm2->GetLitHeight ()));

  if (maxdim1 == maxdim2)
  {
    return (mindim1 - mindim2);
  }

  return (maxdim1 - maxdim2);
}

void csThingStatic::DistributePolyLMs (
        const csStaticPolyGroup& inputPolys,
        csPDelArray<csStaticLitPolyGroup>& outputPolys,
        csStaticPolyGroup* rejectedPolys)
{
  struct internalPolyGroup : public csStaticPolyGroup
  {
    int totalLumels;
    int maxlmw, maxlmh;
    int minLMArea;
  };

  // Polys that couldn't be fit onto a SLM are processed again.
  internalPolyGroup inputQueues[2];
  int curQueue = 0;

  size_t i;

  static_poly_array = &static_polygons;

  rejectedPolys->material = inputPolys.material;
  inputQueues[0].material = inputPolys.material;
  inputQueues[0].totalLumels = 0;
  inputQueues[0].maxlmw = 0;
  inputQueues[0].maxlmh = 0;
  inputQueues[0].minLMArea = INT_MAX;
  inputQueues[1].material = inputPolys.material;
  // Sort polys and filter out oversized polys on the way
  for (i = 0; i < inputPolys.polys.Length(); i++)
  {
    int polyIdx  = inputPolys.polys[i];
    csPolygon3DStatic* sp = static_polygons[polyIdx];

    csPolyTextureMapping* lm = sp->GetTextureMapping ();
    if ((lm == 0) || (!csThing::lightmap_enabled) ||
        !sp->flags.Check (CS_POLY_LIGHTING))
    {
      sp->polygon_data.useLightmap = false;
      rejectedPolys->polys.Push (polyIdx);
      continue;
    }

    int lmw = (csLightMap::CalcLightMapWidth (lm->GetLitOriginalWidth ())
        + LM_BORDER);
    int lmh = (csLightMap::CalcLightMapHeight (lm->GetLitHeight ())
        + LM_BORDER);

    if ((lmw > thing_type->maxLightmapW) ||
      (lmh > thing_type->maxLightmapH))
    {
      sp->polygon_data.useLightmap = false;
      rejectedPolys->polys.Push (polyIdx);
    }
    else
    {
      inputQueues[0].totalLumels += (lmw * lmh);
      inputQueues[0].maxlmw = MAX (inputQueues[0].maxlmw, lmw);
      inputQueues[0].maxlmh = MAX (inputQueues[0].maxlmh, lmh);
      inputQueues[0].minLMArea = MIN(inputQueues[0].minLMArea, lmw * lmh);
      inputQueues[0].polys.InsertSorted (polyIdx, CompareStaticPolys);
    }
  }

  csStaticLitPolyGroup* curOutputPolys = new csStaticLitPolyGroup;
  while (inputQueues[curQueue].polys.Length () > 0)
  {
    // Try to fit as much polys as possible into the SLMs.
    size_t s = 0;
    while ((s<superLMs.Length ()) && (inputQueues[curQueue].polys.Length ()>0))
    {
      StaticSuperLM* slm = superLMs[s];

      /*
       * If the number of free lumels is less than the number of lumels in
       * the smallest LM, we can break testing SLMs. SLMs are sorted by free
       * lumels, so subsequent SLMs won't have any more free space.
       */
      if (slm->freeLumels < inputQueues[curQueue].minLMArea)
      {
        break;
      }

      curOutputPolys->staticSLM = slm;
      curOutputPolys->material = inputQueues[curQueue].material;

      inputQueues[curQueue ^ 1].totalLumels = 0;
      inputQueues[curQueue ^ 1].maxlmw = 0;
      inputQueues[curQueue ^ 1].maxlmh = 0;
      inputQueues[curQueue ^ 1].minLMArea = INT_MAX;

      while (inputQueues[curQueue].polys.Length () > 0)
      {
        bool stuffed = false;
        csSubRect* slmSR;
        int polyIdx = inputQueues[curQueue].polys.Pop ();
        csPolygon3DStatic* sp = static_polygons[polyIdx];

        csPolyTextureMapping* lm = sp->GetTextureMapping ();

        int lmw = (csLightMap::CalcLightMapWidth (lm->GetLitOriginalWidth ())
                + LM_BORDER);
        int lmh = (csLightMap::CalcLightMapHeight (lm->GetLitHeight ())
                + LM_BORDER);

        csRect r;
        if ((lmw * lmh) <= slm->freeLumels)
        {
          if ((slmSR = slm->GetRects ()->Alloc (lmw, lmh, r)) != 0)
          {
            r.xmax -= LM_BORDER;
            r.ymax -= LM_BORDER;
            stuffed = true;
            slm->freeLumels -= (lmw * lmh);
          }
        }

        if (stuffed)
        {
          curOutputPolys->polys.Push (polyIdx);
          curOutputPolys->lmRects.Push (r);
        }
        else
        {
          inputQueues[curQueue ^ 1].polys.InsertSorted (
            polyIdx, CompareStaticPolys);
          inputQueues[curQueue ^ 1].totalLumels += (lmw * lmh);
          inputQueues[curQueue ^ 1].maxlmw =
            MAX (inputQueues[curQueue ^ 1].maxlmw, lmw);
          inputQueues[curQueue ^ 1].maxlmh =
            MAX (inputQueues[curQueue ^ 1].maxlmh, lmh);
          inputQueues[curQueue ^ 1].minLMArea =
            MIN(inputQueues[curQueue ^ 1].minLMArea, lmw * lmh);
        }
      }
      superLMs.DeleteIndex (s);
      size_t nidx = superLMs.InsertSorted (slm, CompareStaticSuperLM);
      if (nidx <= s + 1)
      {
        s++;
      }

      if (curOutputPolys->polys.Length () > 0)
      {
        curOutputPolys->lmRects.ShrinkBestFit ();
        curOutputPolys->polys.ShrinkBestFit ();
        outputPolys.Push (curOutputPolys);
        curOutputPolys = new csStaticLitPolyGroup;
      }

      curQueue ^= 1;
    }

    // Not all polys could be stuffed away, so we possibly need more space.
    if (inputQueues[curQueue].polys.Length () > 0)
    {
      // Try if enlarging an existing SLM suffices.
      bool foundNew = false;
      s = superLMs.Length ();
      while (s > 0)
      {
        s--;

        StaticSuperLM* slm = superLMs[s];
        int usedLumels = (slm->width * slm->height) - slm->freeLumels;

        int neww = (slm->width > slm->height) ? slm->width : slm->width*2;
        int newh = (slm->width > slm->height) ? slm->height*2 : slm->height;

        if ((((neww*newh) - usedLumels) >= inputQueues[curQueue].totalLumels) &&
          (((float)(usedLumels + inputQueues[curQueue].totalLumels) /
          (float)(neww * newh)) > (1.0f - thing_type->maxSLMSpaceWaste)) &&
          (neww <= thing_type->maxLightmapW) &&
          (newh <= thing_type->maxLightmapH))
        {
          superLMs.DeleteIndex (s);
          slm->Grow (neww, newh);
          superLMs.InsertSorted (slm, CompareStaticSuperLM);
          foundNew = true;
          break;
        }
      }

      // Otherwise, add a new empty SLM.
      if (!foundNew)
      {
        int lmW = csFindNearestPowerOf2 (inputQueues[curQueue].maxlmw);
        int lmH = csFindNearestPowerOf2 (inputQueues[curQueue].maxlmh);

        while (inputQueues[curQueue].totalLumels > (lmW * lmH))
        {
          if (lmH < lmW)
            lmH *= 2;
          else
            lmW *= 2;
        }
        StaticSuperLM* newslm = new StaticSuperLM (lmW, lmH);
        superLMs.InsertSorted (newslm, CompareStaticSuperLM);
      }
    }
  }
  delete curOutputPolys;

  //superLMs.ShrinkBestFit ();

  for (i = 0; i < litPolys.Length(); i++)
  {
    StaticSuperLM* slm = litPolys[i]->staticSLM;
    for (size_t j = 0; j < litPolys[i]->polys.Length(); j++)
    {
      csPolygon3DStatic* sp = static_polygons[litPolys[i]->polys[j]];
      const csRect& r = litPolys[i]->lmRects[j];

      sp->polygon_data.useLightmap = true;
      float lmu1, lmv1, lmu2, lmv2;
      thing_type->G3D->GetTextureManager ()->GetLightmapRendererCoords (
        slm->width, slm->height,
        r.xmin, r.ymin, r.xmax, r.ymax,
        lmu1, lmv1, lmu2, lmv2);
      sp->polygon_data.tmapping->SetCoordsOnSuperLM (
        lmu1, lmv1, lmu2, lmv2);
    }
  }
}

void csThingStatic::UnprepareLMLayout ()
{
  if (!IsLmPrepared()) return;
  litPolys.DeleteAll ();
  unlitPolys.DeleteAll ();

  size_t i;
  for (i = 0; i < superLMs.Length (); i++)
  {
    StaticSuperLM* sslm = superLMs[i];
    delete sslm;
  }
  superLMs.DeleteAll ();
  SetLmPrepared (false);
}

int csThingStatic::AddVertex (float x, float y, float z)
{
  if (!obj_verts)
  {
    max_vertices = 10;
    obj_verts = new csVector3[max_vertices];
  }

  while (num_vertices >= max_vertices)
  {
    if (max_vertices < 10000)
      max_vertices *= 2;
    else
      max_vertices += 10000;

    csVector3 *new_obj_verts = new csVector3[max_vertices];
    memcpy (new_obj_verts, obj_verts, sizeof (csVector3) * num_vertices);
    delete[] obj_verts;
    obj_verts = new_obj_verts;
  }

  obj_verts[num_vertices].Set (x, y, z);
  num_vertices++;
  ShapeChanged ();
  return num_vertices - 1;
}

void csThingStatic::SetVertex (int idx, const csVector3 &vt)
{
  CS_ASSERT (idx >= 0 && idx < num_vertices);
  obj_verts[idx] = vt;
  ShapeChanged ();
}

void csThingStatic::DeleteVertex (int idx)
{
  CS_ASSERT (idx >= 0 && idx < num_vertices);

  int copysize = sizeof (csVector3) * (num_vertices - idx - 1);
  memmove (obj_verts + idx, obj_verts + idx + 1, copysize);
  ShapeChanged ();
}

void csThingStatic::DeleteVertices (int from, int to)
{
  if (from <= 0 && to >= num_vertices - 1)
  {
    // Delete everything.
    delete[] obj_verts;
    max_vertices = num_vertices = 0;
    obj_verts = 0;
  }
  else
  {
    if (from < 0) from = 0;
    if (to >= num_vertices) to = num_vertices - 1;

    int rangelen = to - from + 1;
    int copysize = sizeof (csVector3) * (num_vertices - from - rangelen);
    memmove (obj_verts + from,
        obj_verts + from + rangelen, copysize);
    num_vertices -= rangelen;
  }

  ShapeChanged ();
}

void csThingStatic::CompressVertices ()
{
  csVector3* new_obj;
  size_t count_unique;
  csCompressVertex* vt = csVector3Array::CompressVertices (
        obj_verts, num_vertices, new_obj, count_unique);
  if (vt == 0) return;

  // Replace the old vertex tables.
  delete[] obj_verts;
  obj_verts = new_obj;
  num_vertices = max_vertices = (int)count_unique;

  // Now we can remap the vertices in all polygons.
  size_t i;
  int j;
  for (i = 0; i < static_polygons.Length (); i++)
  {
    csPolygon3DStatic *p = static_polygons.Get (i);
    int *idx = p->GetVertexIndices ();
    for (j = 0; j < p->GetVertexCount (); j++) idx[j] = (int)vt[idx[j]].new_idx;
  }

  delete[] vt;
  ShapeChanged ();
}

void csThingStatic::RemoveUnusedVertices ()
{
  if (num_vertices <= 0) return ;

  // Copy all the vertices that are actually used by polygons.
  bool *used = new bool[num_vertices];
  int i, j;
  size_t k;
  for (i = 0; i < num_vertices; i++) used[i] = false;

  // Mark all vertices that are used as used.
  for (k = 0; k < static_polygons.Length (); k++)
  {
    csPolygon3DStatic *p = static_polygons.Get (k);
    int *idx = p->GetVertexIndices ();
    for (j = 0; j < p->GetVertexCount (); j++) used[idx[j]] = true;
  }

  // Count relevant values.
  int count_relevant = 0;
  for (i = 0; i < num_vertices; i++)
  {
    if (used[i]) count_relevant++;
  }

  // If all vertices are relevant then there is nothing to do.
  if (count_relevant == num_vertices)
  {
    delete[] used;
    return ;
  }

  // Now allocate and fill new vertex tables.
  // Also fill the 'relocate' table.
  csVector3 *new_obj = new csVector3[count_relevant];
  int *relocate = new int[num_vertices];
  j = 0;
  for (i = 0; i < num_vertices; i++)
  {
    if (used[i])
    {
      new_obj[j] = obj_verts[i];
      relocate[i] = j;
      j++;
    }
    else
      relocate[i] = -1;
  }

  // Replace the old vertex tables.
  delete[] obj_verts;
  obj_verts = new_obj;
  num_vertices = max_vertices = count_relevant;

  // Now we can remap the vertices in all polygons.
  for (k = 0; k < static_polygons.Length (); k++)
  {
    csPolygon3DStatic *p = static_polygons.Get (k);
    int *idx = p->GetVertexIndices ();
    for (j = 0; j < p->GetVertexCount (); j++) idx[j] = relocate[idx[j]];
  }

  delete[] relocate;
  delete[] used;

  SetObjBboxValid (false);
  ShapeChanged ();
}

struct PolygonsForVertex
{
  csArray<int> poly_indices;
};

void csThingStatic::CalculateNormals ()
{
  int polyCount = (int)static_polygons.Length();
  int i, k;

  delete[] obj_normals;
  obj_normals = new csVector3[num_vertices];
  memset (obj_normals, 0, sizeof (csVector3)*num_vertices);

  for (i = 0 ; i < polyCount ; i++)
  {
    csPolygon3DStatic* p = static_polygons.Get (i);
    const csVector3& normal = p->GetObjectPlane ().Normal();
    int* vtidx = p->GetVertexIndices ();
    for (k = 0 ; k < p->GetVertexCount () ; k++)
    {
      CS_ASSERT (vtidx[k] >= 0 && vtidx[k] < num_vertices);
      obj_normals[vtidx[k]] += normal;
    }
  }

  // Now calculate normals.
  for (i = 0 ; i < num_vertices ; i++)
  {
    obj_normals[i].Normalize ();
  }
}

int csThingStatic::AddPolygon (csPolygon3DStatic* spoly)
{
  spoly->SetParent (this);
  spoly->EnableTextureMapping (true);
  int idx = (int)static_polygons.Push (spoly);
  ShapeChanged ();
  UnprepareLMLayout ();
  return idx;
}

void csThingStatic::RemovePolygon (int idx)
{
  static_polygons.FreeItem (static_polygons.Get (idx));
  static_polygons.DeleteIndex (idx);
  ShapeChanged ();
  UnprepareLMLayout ();
}

void csThingStatic::RemovePolygons ()
{
  static_polygons.FreeAll ();
  ShapeChanged ();
  UnprepareLMLayout ();
}

int csThingStatic::IntersectSegmentIndex (
  const csVector3 &start, const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  size_t i;
  float r, best_r = 2000000000.;
  csVector3 cur_isect;
  int best_p = -1;

  // @@@ This routine is not very optimal. Especially for things
  // with large number of polygons.
  for (i = 0; i < static_polygons.Length (); i++)
  {
    csPolygon3DStatic *p = static_polygons.Get (i);
    if (p->IntersectSegment (start, end, cur_isect, &r))
    {
      if (r < best_r)
      {
        best_r = r;
        best_p = (int)i;
        isect = cur_isect;
      }
    }
  }

  if (pr) *pr = best_r;
  return best_p;
}

csPtr<csThingStatic> csThingStatic::CloneStatic ()
{
  csThingStatic* clone = new csThingStatic (scfParent, thing_type);
  clone->flags.SetAll (GetFlags ().Get ());
  clone->SetSmoothed (IsSmoothed());
  clone->obj_bbox = obj_bbox;
  clone->SetObjBboxValid (IsObjBboxValid ());
  clone->obj_radius = obj_radius;
  clone->max_obj_radius = max_obj_radius;
  clone->SetPrepared (IsPrepared());
  clone->SetShapeNumber (GetShapeNumber ());
  clone->cosinus_factor = cosinus_factor;

  clone->num_vertices = num_vertices;
  clone->max_vertices = max_vertices;
  if (obj_verts)
  {
    clone->obj_verts = new csVector3[max_vertices];
    memcpy (clone->obj_verts, obj_verts, sizeof (csVector3)*num_vertices);
  }
  else
  {
    clone->obj_verts = 0;
  }
  if (obj_normals)
  {
    clone->obj_normals = new csVector3[max_vertices];
    memcpy (clone->obj_normals, obj_normals, sizeof (csVector3)*num_vertices);
  }
  else
  {
    clone->obj_normals = 0;
  }

  size_t i;
  for (i = 0 ; i < static_polygons.Length () ; i++)
  {
    csPolygon3DStatic* p = static_polygons.Get (i)->Clone (clone);
    clone->static_polygons.Push (p);
  }

  return csPtr<csThingStatic> (clone);
}

csPtr<iMeshObjectFactory> csThingStatic::Clone ()
{
  csRef<csThingStatic> clone=CloneStatic ();
  return csPtr<iMeshObjectFactory> (clone);
}

void csThingStatic::HardTransform (const csReversibleTransform &t)
{
  int i;

  for (i = 0; i < num_vertices; i++)
  {
    obj_verts[i] = t.This2Other (obj_verts[i]);
  }

  //-------
  // Now transform the polygons.
  //-------
  for (int j = 0; j < (int)static_polygons.Length (); j++)
  {
    csPolygon3DStatic *p = GetPolygon3DStatic (j);
    p->HardTransform (t);
  }

  ShapeChanged ();
  SetObjBboxValid (false);
}

csPtr<iMeshObject> csThingStatic::NewInstance ()
{
  csThing *thing = new csThing ((iBase*)(iThingFactoryState*)this, this);
  if (mixmode != (uint)~0)
    thing->SetMixMode (mixmode);
  return csPtr<iMeshObject> ((iMeshObject*)thing);
}

void csThingStatic::SetBoundingBox (const csBox3 &box)
{
  SetObjBboxValid (true);
  obj_bbox = box;
  ShapeChanged ();
}

void csThingStatic::GetBoundingBox (csBox3 &box)
{
  int i;

  if (IsObjBboxValid())
  {
    box = obj_bbox;
    return ;
  }

  SetObjBboxValid (true);

  if (!obj_verts)
  {
    obj_bbox.Set (0, 0, 0, 0, 0, 0);
    box = obj_bbox;
    return ;
  }

  if (num_vertices > 0)
  {
    obj_bbox.StartBoundingBox (obj_verts[0]);
    for (i = 1; i < num_vertices; i++)
    {
      obj_bbox.AddBoundingVertexSmart (obj_verts[i]);
    }
  }

  obj_radius = (obj_bbox.Max () - obj_bbox.Min ()) * 0.5f;
  max_obj_radius = csQsqrt (csSquaredDist::PointPoint (
        obj_bbox.Max (), obj_bbox.Min ())) * 0.5f;
  box = obj_bbox;
}

void csThingStatic::GetRadius (csVector3 &rad, csVector3 &cent)
{
  csBox3 b;
  GetBoundingBox (b);
  rad = obj_radius;
  cent = b.GetCenter ();
}

void csThingStatic::FillRenderMeshes (
        csDirtyAccessArray<csRenderMesh*>& rmeshes,
        const csArray<RepMaterial>& repMaterials,
        uint mixmode)
{
  for (size_t i = 0; i < (litPolys.Length () + unlitPolys.Length ()); i++)
  {
    const csStaticPolyGroup& pg =
      (i < litPolys.Length ()) ? *(litPolys[i]) :
        *(unlitPolys[i - litPolys.Length ()]) ;
    csRenderMesh* rm = thing_type->blk_rendermesh.Alloc();

    csRef<csPolygonRenderer> polyRenderer;
    if (polyRenderers.Length () <= i)
    {
      polyRenderer.AttachNew (new csPolygonRenderer (thing_type));
      polyRenderers.Push (polyRenderer);

      size_t j;
      for (j = 0; j< pg.polys.Length(); j++)
      {
        polyRenderer->AddPolygon (&static_polygons[pg.polys[j]]->polygon_data,
          static_polygons[pg.polys[j]]->polyBuffers.GetBuffers());
      }
    }
    else
      polyRenderer = polyRenderers[i];

    rm->mixmode = mixmode;
    iMaterialWrapper* material = pg.material;
    for (size_t m = 0; m < repMaterials.Length(); m++)
    {
      if (repMaterials[m].old_mat == material)
      {
        material = repMaterials[m].new_mat;
        break;
      }
    }
    rm->material = material;
    CS_ASSERT (material != 0);
    rm->meshtype = CS_MESHTYPE_TRIANGLES;
    rm->variablecontext.AttachNew (new csShaderVariableContext ());
    csRef<csShaderVariable> sv (
      csPtr<csShaderVariable> (new csShaderVariable (texLightmapName)));
    rm->variablecontext->AddVariable (sv);

    /*csShaderVariable* sv;
    sv = dynDomain->GetVariableAdd (index_name);
    sv->SetValue (factory->GetRenderBuffer (factory->index_name));
    sv = dynDomain->GetVariableAdd (vertex_name);
    sv->SetValue (factory->GetRenderBuffer (factory->vertex_name));
    sv = dynDomain->GetVariableAdd (texel_name);
    sv->SetValue (factory->GetRenderBuffer (factory->texel_name));
    sv = dynDomain->GetVariableAdd (normal_name);
    sv->SetValue (factory->GetRenderBuffer (factory->normal_name));
    sv = dynDomain->GetVariableAdd (color_name);
    sv->SetValue (factory->GetRenderBuffer (factory->color_name));*/

    /*rm->buffersource = polyRenderer->GetBufferSource (rm->indexstart,
      rm->indexend);*/
    polyRenderer->PrepareRenderMesh (*rm);

    rmeshes.Push (rm);
  }
}

int csThingStatic::FindPolygonByName (const char* name)
{
  return (int)static_polygons.FindKey (static_polygons.KeyCmp(name));
}

int csThingStatic::GetRealIndex (int requested_index) const
{
  if (requested_index == -1)
  {
    CS_ASSERT (last_range.end != -1);
    return last_range.end;
  }
  return requested_index;
}

void csThingStatic::GetRealRange (const csPolygonRange& requested_range,
        int& start, int& end)
{
  if (requested_range.start == -1)
  {
    start = last_range.start;
    end = last_range.end;
    CS_ASSERT (end != -1);
    return;
  }
  start = requested_range.start;
  end = requested_range.end;
  if (start < 0) start = 0;
  if ((size_t)end >= static_polygons.Length ())
    end = (int)static_polygons.Length ()-1;
}

int csThingStatic::AddEmptyPolygon ()
{
  csPolygon3DStatic* sp = thing_type->blk_polygon3dstatic.Alloc ();
  int idx = AddPolygon (sp);
  last_range.Set (idx);
  return idx;
}

int csThingStatic::AddTriangle (const csVector3& v1, const csVector3& v2,
        const csVector3& v3)
{
  int idx = AddEmptyPolygon ();
  csPolygon3DStatic* sp = static_polygons[idx];
  sp->SetNumVertices (3);
  sp->SetVertex (0, v1);
  sp->SetVertex (1, v2);
  sp->SetVertex (2, v3);
  last_range.Set (idx);
  sp->SetTextureSpace (v1, v2, 1);
  SetObjBboxValid(false);
  return idx;
}

int csThingStatic::AddQuad (const csVector3& v1, const csVector3& v2,
        const csVector3& v3, const csVector3& v4)
{
  int idx = AddEmptyPolygon ();
  csPolygon3DStatic* sp = static_polygons[idx];
  sp->SetNumVertices (4);
  sp->SetVertex (0, v1);
  sp->SetVertex (1, v2);
  sp->SetVertex (2, v3);
  sp->SetVertex (3, v4);
  last_range.Set (idx);
  sp->SetTextureSpace (v1, v2, 1);
  SetObjBboxValid(false);
  return idx;
}

int csThingStatic::AddPolygon (csVector3* vertices, int num)
{
  int idx = AddEmptyPolygon ();
  csPolygon3DStatic* sp = static_polygons[idx];
  sp->SetNumVertices (num);
  int i;
  for (i = 0 ; i < num ; i++)
  {
    sp->SetVertex (i, vertices[i]);
  }
  last_range.Set (idx);
  sp->SetTextureSpace (vertices[0], vertices[1], 1);
  SetObjBboxValid(false);
  return idx;
}

int csThingStatic::AddPolygon (int num, ...)
{
  int idx = AddEmptyPolygon ();
  csPolygon3DStatic* sp = static_polygons[idx];
  sp->SetNumVertices (num);
  va_list arg;
  va_start (arg, num);
  int i;
  for (i = 0 ; i < num ; i++)
  {
    int v = va_arg (arg, int);
    sp->SetVertex (i, v);
  }
  va_end (arg);
  last_range.Set (idx);
  sp->SetTextureSpace (sp->Vobj (0), sp->Vobj (1), 1);
  return idx;
}

int csThingStatic::AddOutsideBox (const csVector3& bmin, const csVector3& bmax)
{
  csBox3 box (bmin, bmax);
  int firstidx = AddQuad (
        box.GetCorner (CS_BOX_CORNER_xYz),
        box.GetCorner (CS_BOX_CORNER_XYz),
        box.GetCorner (CS_BOX_CORNER_Xyz),
        box.GetCorner (CS_BOX_CORNER_xyz));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_XYz),
        box.GetCorner (CS_BOX_CORNER_XYZ),
        box.GetCorner (CS_BOX_CORNER_XyZ),
        box.GetCorner (CS_BOX_CORNER_Xyz));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_XYZ),
        box.GetCorner (CS_BOX_CORNER_xYZ),
        box.GetCorner (CS_BOX_CORNER_xyZ),
        box.GetCorner (CS_BOX_CORNER_XyZ));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_xYZ),
        box.GetCorner (CS_BOX_CORNER_xYz),
        box.GetCorner (CS_BOX_CORNER_xyz),
        box.GetCorner (CS_BOX_CORNER_xyZ));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_xyz),
        box.GetCorner (CS_BOX_CORNER_Xyz),
        box.GetCorner (CS_BOX_CORNER_XyZ),
        box.GetCorner (CS_BOX_CORNER_xyZ));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_xYZ),
        box.GetCorner (CS_BOX_CORNER_XYZ),
        box.GetCorner (CS_BOX_CORNER_XYz),
        box.GetCorner (CS_BOX_CORNER_xYz));

  last_range.Set (firstidx, firstidx+5);
  SetObjBboxValid(false);
  return firstidx;
}

int csThingStatic::AddInsideBox (const csVector3& bmin, const csVector3& bmax)
{
  csBox3 box (bmin, bmax);
  int firstidx = AddQuad (
        box.GetCorner (CS_BOX_CORNER_xyz),
        box.GetCorner (CS_BOX_CORNER_Xyz),
        box.GetCorner (CS_BOX_CORNER_XYz),
        box.GetCorner (CS_BOX_CORNER_xYz));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_Xyz),
        box.GetCorner (CS_BOX_CORNER_XyZ),
        box.GetCorner (CS_BOX_CORNER_XYZ),
        box.GetCorner (CS_BOX_CORNER_XYz));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_XyZ),
        box.GetCorner (CS_BOX_CORNER_xyZ),
        box.GetCorner (CS_BOX_CORNER_xYZ),
        box.GetCorner (CS_BOX_CORNER_XYZ));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_xyZ),
        box.GetCorner (CS_BOX_CORNER_xyz),
        box.GetCorner (CS_BOX_CORNER_xYz),
        box.GetCorner (CS_BOX_CORNER_xYZ));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_xyZ),
        box.GetCorner (CS_BOX_CORNER_XyZ),
        box.GetCorner (CS_BOX_CORNER_Xyz),
        box.GetCorner (CS_BOX_CORNER_xyz));
  AddQuad (
        box.GetCorner (CS_BOX_CORNER_xYz),
        box.GetCorner (CS_BOX_CORNER_XYz),
        box.GetCorner (CS_BOX_CORNER_XYZ),
        box.GetCorner (CS_BOX_CORNER_xYZ));

  last_range.Set (firstidx, firstidx+5);
  SetObjBboxValid(false);
  return firstidx;
}

void csThingStatic::SetPolygonName (const csPolygonRange& range,
        const char* name)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
    static_polygons[i]->SetName (name);
}

const char* csThingStatic::GetPolygonName (int polygon_idx)
{
  return static_polygons[GetRealIndex (polygon_idx)]->GetName ();
}

csPtr<iPolygonHandle> csThingStatic::CreatePolygonHandle (int polygon_idx)
{
  return csPtr<iPolygonHandle> (new csPolygonHandle (
        (iThingFactoryState*)this,
        (iMeshObjectFactory*)this,
        0, 0,
        GetRealIndex (polygon_idx)));
}

void csThingStatic::SetPolygonMaterial (const csPolygonRange& range,
        iMaterialWrapper* material)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
    static_polygons[i]->SetMaterial (material);
#ifdef __USE_MATERIALS_REPLACEMENT__
  UnprepareLMLayout ();
#endif // __USE_MATERIALS_REPLACEMENT__
}

iMaterialWrapper* csThingStatic::GetPolygonMaterial (int polygon_idx)
{
  return static_polygons[GetRealIndex (polygon_idx)]->GetMaterialWrapper ();
}

void csThingStatic::AddPolygonVertex (const csPolygonRange& range,
        const csVector3& vt)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
    static_polygons[i]->AddVertex (vt);
}

void csThingStatic::AddPolygonVertex (const csPolygonRange& range, int vt)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
    static_polygons[i]->AddVertex (vt);
}

void csThingStatic::SetPolygonVertexIndices (const csPolygonRange& range,
        int num, int* indices)
{
  int i, start, end;
  int j;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->SetNumVertices (num);
    for (j = 0 ; j < num ; j++)
      sp->SetVertex (j, indices[j]);
  }
}

int csThingStatic::GetPolygonVertexCount (int polygon_idx)
{
  return static_polygons[GetRealIndex (polygon_idx)]->GetVertexCount ();
}

const csVector3& csThingStatic::GetPolygonVertex (int polygon_idx,
        int vertex_idx)
{
  return static_polygons[GetRealIndex (polygon_idx)]->Vobj (vertex_idx);
}

int* csThingStatic::GetPolygonVertexIndices (int polygon_idx)
{
  return static_polygons[GetRealIndex (polygon_idx)]->GetVertexIndices ();
}

bool csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
        const csMatrix3& m, const csVector3& v)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
    static_polygons[i]->SetTextureSpace (m, v);
  return true;
}

bool csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
        const csVector2& uv1, const csVector2& uv2, const csVector2& uv3)
{
  int i, start, end;
  GetRealRange (range, start, end);
  bool error = false;
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    if (!sp->SetTextureSpace (
        sp->Vobj (0), uv1,
        sp->Vobj (1), uv2,
        sp->Vobj (2), uv3))
      error = true;
  }
  return !error;
}

bool csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
        const csVector3& p1, const csVector2& uv1,
        const csVector3& p2, const csVector2& uv2,
        const csVector3& p3, const csVector2& uv3)
{
  int i, start, end;
  GetRealRange (range, start, end);
  bool error = false;
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    if (!sp->SetTextureSpace (p1, uv1, p2, uv2, p3, uv3))
      error = true;
  }
  return !error;
}

bool csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
        const csVector3& v_orig, const csVector3& v1, float len1)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->SetTextureSpace (v_orig, v1, len1);
  }
  return true;
}

bool csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
        float len1)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->SetTextureSpace (sp->Vobj (0), sp->Vobj (1), len1);
  }
  return true;
}

bool csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
        const csVector3& v_orig,
        const csVector3& v1, float len1,
        const csVector3& v2, float len2)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->SetTextureSpace (v_orig, v1, len1, v2, len2);
  }
  return true;
}

void csThingStatic::GetPolygonTextureMapping (int polygon_idx,
        csMatrix3& m, csVector3& v)
{
  static_polygons[GetRealIndex (polygon_idx)]->GetTextureSpace (m, v);
}

void csThingStatic::SetPolygonTextureMappingEnabled (
        const csPolygonRange& range, bool enabled)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->EnableTextureMapping (enabled);
  }
}

bool csThingStatic::IsPolygonTextureMappingEnabled (int polygon_idx) const
{
  return static_polygons[GetRealIndex (polygon_idx)]->IsTextureMappingEnabled ();
}

void csThingStatic::SetPolygonFlags (const csPolygonRange& range, uint32 flags)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->flags.Set (flags);
  }
}

void csThingStatic::SetPolygonFlags (const csPolygonRange& range, uint32 mask, uint32 flags)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->flags.Set (mask, flags);
  }
}

void csThingStatic::ResetPolygonFlags (const csPolygonRange& range, uint32 flags)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->flags.Reset (flags);
  }
}

csFlags& csThingStatic::GetPolygonFlags (int polygon_idx)
{
  return static_polygons[GetRealIndex (polygon_idx)]->flags;
}

const csPlane3& csThingStatic::GetPolygonObjectPlane (int polygon_idx)
{
  return static_polygons[GetRealIndex (polygon_idx)]->GetObjectPlane ();
}

bool csThingStatic::IsPolygonTransparent (int polygon_idx)
{
  return static_polygons[GetRealIndex (polygon_idx)]->IsTransparent ();
}

bool csThingStatic::AddPolygonRenderBuffer (int polygon_idx, const char* name,
                                            iRenderBuffer* buffer)
{
  csStringID nameID = thing_type->stringset->Request (name);
  iRenderBuffer* Template;
  if ((Template = polyBufferTemplates.GetRenderBuffer (nameID)) != 0)
  {
    if ((Template->GetComponentType() != buffer->GetComponentType())
      || (Template->GetComponentCount() != buffer->GetComponentCount()))
      return false;
  }
  else
    polyBufferTemplates.AddRenderBuffer (nameID, buffer);
  csPolygon3DStatic* sp = static_polygons[GetRealIndex (polygon_idx)];
  return sp->polyBuffers.AddRenderBuffer (nameID, buffer);
}

bool csThingStatic::PointOnPolygon (int polygon_idx, const csVector3& v)
{
  return static_polygons[GetRealIndex (polygon_idx)]->PointOnPolygon (v);
}

//----------------------------------------------------------------------------

csThing::csThing (iBase *parent, csThingStatic* static_data) :
        polygons(32, 64),
        scfiPolygonMesh (0),
        scfiPolygonMeshCD (CS_POLY_COLLDET),
        scfiPolygonMeshLOD (CS_POLY_VISCULL)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiThingState);
  DG_TYPE (this, "csThing");

  csThing::static_data = static_data;
  polygons.SetThingType (static_data->thing_type);
  polygon_world_planes = 0;
  polygon_world_planes_num = (size_t)-1;        // -1 means not checked yet, 0 means no planes.

  scfiPolygonMesh.SetThing (static_data);
  scfiPolygonMeshCD.SetThing (static_data);
  scfiPolygonMeshLOD.SetThing (static_data);

  last_thing_id++;
  thing_id = last_thing_id;
  logparent = 0;

  wor_verts = 0;

  dynamic_ambient.Set (0, 0, 0);
  dynamic_ambient_version = 0;
  light_version = 1;

  mixmode = CS_FX_COPY;

  movablenr = -1;
  wor_bbox_movablenr = -1;
  cached_movable = 0;

  cfg_moving = CS_THING_MOVE_NEVER;

  static_data_nr = 0xfffffffd;  // (static_nr of csThingStatic is init to -1)

  current_visnr = 1;

  SetLmDirty (true);
}

csThing::~csThing ()
{
  ClearLMs ();

  if (wor_verts != static_data->obj_verts)
  {
    delete[] wor_verts;
  }

  polygons.DeleteAll ();
  delete[] polygon_world_planes;

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiThingState);
  SCF_DESTRUCT_IBASE ();
}

char* csThing::GenerateCacheName ()
{
  csBox3 b;
  static_data->GetBoundingBox (b);

  csMemFile mf;
  int32 l;
  l = csConvertEndian ((int32)static_data->num_vertices);
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)polygons.Length ());
  mf.Write ((char*)&l, 4);

  if (logparent)
  {
    csRef<iMeshWrapper> mw (SCF_QUERY_INTERFACE (logparent, iMeshWrapper));
    if (mw)
    {
      if (mw->QueryObject ()->GetName ())
        mf.Write (mw->QueryObject ()->GetName (),
                strlen (mw->QueryObject ()->GetName ()));
      iSector* sect = mw->GetMovable ()->GetSectors ()->Get (0);
      if (sect && sect->QueryObject ()->GetName ())
        mf.Write (sect->QueryObject ()->GetName (),
                strlen (sect->QueryObject ()->GetName ()));
    }
  }

  l = csConvertEndian ((int32)csQint ((b.MinX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MinY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MinZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MaxX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MaxY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MaxZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());
  csString hex(digest.HexString());
  return hex.Detach();
}

void csThing::MarkLightmapsDirty ()
{
  SetLmDirty (true);
  light_version++;
}

void csThing::LightChanged (iLight*)
{
  MarkLightmapsDirty ();
}

void csThing::LightDisconnect (iLight* light)
{
  MarkLightmapsDirty ();
  int dt = light->GetDynamicType ();
  int i;
  for (i = 0 ; i < (int)polygons.Length () ; i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    if (dt == CS_LIGHT_DYNAMICTYPE_DYNAMIC)
      p->DynamicLightDisconnect (light);
    else
      p->StaticLightDisconnect (light);
  }
}

void csThing::SetMovingOption (int opt)
{
  cfg_moving = opt;
  switch (cfg_moving)
  {
    case CS_THING_MOVE_NEVER:
      if (wor_verts != static_data->obj_verts) delete[] wor_verts;
      wor_verts = static_data->obj_verts;
      break;

    case CS_THING_MOVE_OCCASIONAL:
      if ((wor_verts == 0 || wor_verts == static_data->obj_verts)
        && static_data->max_vertices)
      {
        wor_verts = new csVector3[static_data->max_vertices];
        memcpy (wor_verts, static_data->obj_verts,
                static_data->max_vertices * sizeof (csVector3));
      }

      //cached_movable = 0;
      movablenr--;
      break;
  }

  movablenr = -1;                 // @@@ Is this good?
}

void csThing::WorUpdate ()
{
  size_t i;
  int j;
  switch (cfg_moving)
  {
    case CS_THING_MOVE_NEVER:
      if (cached_movable && cached_movable->GetUpdateNumber () != movablenr)
      {
        if (!cached_movable->IsFullTransformIdentity ())
        {
          // If the movable is no longer the identity transform we
          // have to change modes to moveable.
          SetMovingOption (CS_THING_MOVE_OCCASIONAL);
          WorUpdate ();
          break;
        }
        movablenr = cached_movable->GetUpdateNumber ();
        delete[] polygon_world_planes;
        polygon_world_planes = 0;
        polygon_world_planes_num = 0;
      }
      return ;

    case CS_THING_MOVE_OCCASIONAL:
      if (cached_movable && cached_movable->GetUpdateNumber () != movablenr)
      {
        movablenr = cached_movable->GetUpdateNumber ();

        if (cached_movable->IsFullTransformIdentity ())
        {
          memcpy (wor_verts, static_data->obj_verts,
                static_data->num_vertices * (sizeof (csVector3)));
          delete[] polygon_world_planes;
          polygon_world_planes = 0;
          polygon_world_planes_num = 0;
        }
        else
        {
          csReversibleTransform movtrans = cached_movable->GetFullTransform ();
          for (j = 0; j < static_data->num_vertices; j++)
            wor_verts[j] = movtrans.This2Other (static_data->obj_verts[j]);
          if (!polygon_world_planes || polygon_world_planes_num < polygons.Length () ||
            polygon_world_planes_num == (size_t)-1)
          {
            delete[] polygon_world_planes;
            polygon_world_planes_num = polygons.Length ();
            polygon_world_planes = new csPlane3[polygon_world_planes_num];
          }
          for (i = 0; i < polygons.Length (); i++)
          {
            csPolygon3DStatic* sp = static_data->GetPolygon3DStatic ((int)i);
            movtrans.This2Other (sp->polygon_data.plane_obj,
                Vwor (sp->GetVertexIndices ()[0]),
                polygon_world_planes[i]);
            polygon_world_planes[i].Normalize ();
          }
        }
      }
      break;
  }
}

void csThing::HardTransform (const csReversibleTransform& t)
{
  csRef<csThingStatic> new_static_data = static_data->CloneStatic ();
  static_data = new_static_data;
  static_data->HardTransform (t);
  scfiPolygonMesh.SetThing (static_data);
  scfiPolygonMeshCD.SetThing (static_data);
  scfiPolygonMeshLOD.SetThing (static_data);
}

void csThing::Unprepare ()
{
  SetPrepared (false);
}

void csThing::PreparePolygons ()
{
  csPolygon3DStatic *ps;
  csPolygon3D *p;
  polygons.DeleteAll ();
  delete[] polygon_world_planes;
  polygon_world_planes = 0;
  polygon_world_planes_num = (size_t)-1;        // Not checked!

  size_t i;
  polygons.SetLength (static_data->static_polygons.Length ());
  for (i = 0; i < static_data->static_polygons.Length (); i++)
  {
    p = &polygons.Get (i);
    ps = static_data->static_polygons.Get (i);
    p->SetParent (this);
    p->Finish (ps);
  }
  polygons.ShrinkBestFit ();
}

void csThing::Prepare ()
{
  static_data->Prepare (logparent);

  if (IsPrepared())
  {
    if (static_data_nr != static_data->GetShapeNumber ())
    {
      static_data_nr = static_data->GetShapeNumber ();

      if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
      {
        if (wor_verts != static_data->obj_verts)
          delete[] wor_verts;
        wor_verts = new csVector3[static_data->max_vertices];
      }
      else
      {
        wor_verts = static_data->obj_verts;
      }
      if (cached_movable) movablenr = cached_movable->GetUpdateNumber ()-1;
      else movablenr--;

      meshesHolder.Clear();

      materials_to_visit.DeleteAll ();

      ClearLMs ();
      PreparePolygons ();

      MarkLightmapsDirty ();
      ClearLMs ();
      PrepareLMs ();
    }
    return;
  }

  SetPrepared (true);

  static_data_nr = static_data->GetShapeNumber ();

  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
  {
    if (wor_verts != static_data->obj_verts)
      delete[] wor_verts;
    wor_verts = new csVector3[static_data->max_vertices];
  }
  else
  {
    wor_verts = static_data->obj_verts;
  }
  if (cached_movable) movablenr = cached_movable->GetUpdateNumber ()-1;
  else movablenr--;

  meshesHolder.Clear();

  materials_to_visit.DeleteAll ();

  PreparePolygons ();

  // don't prepare lightmaps yet - the LMs may still be unlit,
  // as this function is called from within 'ReadFromCache()'.
}

iMaterialWrapper* csThing::FindRealMaterial (iMaterialWrapper* old_mat)
{
  size_t i;
  for (i = 0 ; i < replace_materials.Length () ; i++)
  {
    if (replace_materials[i].old_mat == old_mat)
      return replace_materials[i].new_mat;
  }
  return 0;
}

#ifdef __USE_MATERIALS_REPLACEMENT__

void csThing::ReplaceMaterial (iMaterialWrapper* oldmat,
        iMaterialWrapper* newmat)
{
  //
  //Remove the binding of oldmat, if it exists.
  size_t i;
  for (i = 0 ; i < replace_materials.Length () ; i++)
  {
    if (replace_materials[i].old_mat == oldmat)
    {
      replace_materials.DeleteIndex (i);
      break;
    }//if
  }//for

  //
  //If newmat == 0 then it means the caller want to use the standard
  //material given by the factory mesh object. Otherwise the caller
  //want to create a new binding.
  if (newmat != 0)
  {
    //Create the binding of the 'oldmat' material with a new one.
    replace_materials.Push (RepMaterial (oldmat, newmat));
  }//if

  SetReplaceMaterialChanged (true);
}

#else

void csThing::ReplaceMaterial (iMaterialWrapper* oldmat,
        iMaterialWrapper* newmat)
{
  replace_materials.Push (RepMaterial (oldmat, newmat));
  SetPrepared (false);
}

#endif // __USE_MATERIALS_REPLACEMENT__


void csThing::ClearReplacedMaterials ()
{
  replace_materials.DeleteAll ();
  SetPrepared (false);
}

csPolygon3D *csThing::GetPolygon3D (const char *name)
{
  int idx = static_data->FindPolygonByName (name);
  return idx >= 0 ? &polygons.Get (idx) : 0;
}

csPtr<iPolygonHandle> csThing::CreatePolygonHandle (int polygon_idx)
{
  CS_ASSERT (polygon_idx >= 0);
  return csPtr<iPolygonHandle> (new csPolygonHandle (
        (iThingFactoryState*)(csThingStatic*)static_data,
        (iMeshObjectFactory*)(csThingStatic*)static_data,
        &scfiThingState,
        (iMeshObject*)this,
        polygon_idx));
}

const csPlane3& csThing::GetPolygonWorldPlane (int polygon_idx)
{
  CS_ASSERT (polygon_idx >= 0);
  if (polygon_world_planes_num == (size_t)-1)
  {
    WorUpdate ();
  }
  return GetPolygonWorldPlaneNoCheck (polygon_idx);
}

const csPlane3& csThing::GetPolygonWorldPlaneNoCheck (int polygon_idx) const
{
  if (polygon_world_planes)
    return polygon_world_planes[polygon_idx];
  else
    return static_data->static_polygons[polygon_idx]->GetObjectPlane ();
}

void csThing::InvalidateThing ()
{
  materials_to_visit.DeleteAll ();

  SetPrepared (false);
  static_data->SetObjBboxValid (false);

  delete [] static_data->obj_normals; static_data->obj_normals = 0;
  static_data->ShapeChanged ();
}

iPolygonMesh* csThing::GetWriteObject ()
{
  return &scfiPolygonMeshLOD;
}

bool csThing::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  size_t i;

  // @@@ This routine is not very optimal. Especially for things
  // with large number of polygons.
  for (i = 0; i < static_data->static_polygons.Length (); i++)
  {
    csPolygon3DStatic *p = static_data->static_polygons.Get (i);
    if (p->IntersectSegment (start, end, isect, pr))
    {
      return true;
    }
  }

  return false;
}

bool csThing::HitBeamObject (const csVector3& start,
  const csVector3& end, csVector3& isect, float *pr, int* polygon_idx)
{
  int idx = static_data->IntersectSegmentIndex (start, end, isect, pr);
  if (polygon_idx) *polygon_idx = idx;
  if (idx == -1) return false;
  return true;
}

struct MatPol
{
  iMaterialWrapper *mat;
  int mat_index;
  csPolygon3DStatic *spoly;
  csPolygon3D *poly;
};

void csThing::PreparePolygonBuffer ()
{
}

void csThing::DrawPolygonArrayDPM (
  iRenderView *rview,
  iMovable *movable,
  csZBufMode zMode)
{

}

void csThing::InvalidateMaterialHandles ()
{

}

// @@@ We need a better algorithm here. We should try
// to recognize convex sub-parts of a polygonset and return
// convex shadow frustums for those. This will significantly
// reduce the number of shadow frustums. There are basically
// two ways to do this:
//      - Split object into convex sub-parts in 3D.
//      - Split object into convex sub-parts in 2D.
// The first way is probably best because it is more efficient
// at runtime (important if we plan to use dynamic shadows for things)
// and also more correct in that a convex 3D object has no internal
// shadowing while a convex outline may have no correspondance to internal
// shadows.
void csThing::AppendShadows (
  iMovable* movable,
  iShadowBlockList *shadows,
  const csVector3 &origin)
{
  Prepare ();
  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

  iShadowBlock *list = shadows->NewShadowBlock (
      (int)polygons.Length ());
  csFrustum *frust;
  size_t i;
  int j;
  csPolygon3DStatic *sp;
  csPolygon3D *p;
  bool cw = true;                   //@@@ Use mirroring parameter here!
  for (i = 0; i < static_data->static_polygons.Length (); i++)
  {
    sp = static_data->static_polygons.Get (i);
    p = &polygons.Get (i);

    //if (p->GetPlane ()->VisibleFromPoint (origin) != cw) continue;
    const csPlane3& world_plane = GetPolygonWorldPlaneNoCheck ((int)i);
    float clas = world_plane.Classify (origin);
    if (ABS (clas) < EPSILON) continue;
    if ((clas <= 0) != cw) continue;

    csPlane3 pl = world_plane;
    pl.DD += origin * pl.norm;
    pl.Invert ();
    frust = list->AddShadow (
        origin,
        (void *)p,
        sp->GetVertexCount (),
        pl);
    int* p_vt_idx = sp->GetVertexIndices ();
    for (j = 0; j < sp->GetVertexCount (); j++)
      frust->GetVertex (j).Set (Vwor (p_vt_idx[j]) - origin);
  }
}

void csThing::GetBoundingBox (iMovable *movable, csBox3 &box)
{
  if (wor_bbox_movablenr != movable->GetUpdateNumber ())
  {
    // First make sure obj_bbox is valid.
    static_data->GetBoundingBox (box);
    wor_bbox_movablenr = movable->GetUpdateNumber ();
    csBox3& obj_bbox = static_data->obj_bbox;

    // @@@ Maybe it would be better to really calculate the bounding box
    // here instead of just transforming the object space bounding box?
    if (movable->IsFullTransformIdentity ())
    {
      wor_bbox = obj_bbox;
    }
    else
    {
      csReversibleTransform mt = movable->GetFullTransform ();
      wor_bbox.StartBoundingBox (mt.This2Other (obj_bbox.GetCorner (0)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (1)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (2)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (3)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (4)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (5)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (6)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (7)));
    }
  }

  box = wor_bbox;
}

//-------------------------------------------------------------------------

struct PolyMeshTimerEvent : public iTimerEvent
{
  csWeakRef<PolyMeshHelper> pmh;
  PolyMeshTimerEvent (PolyMeshHelper* pmh)
  {
    SCF_CONSTRUCT_IBASE (0);
    PolyMeshTimerEvent::pmh = pmh;
  }
  virtual ~PolyMeshTimerEvent ()
  {
    SCF_DESTRUCT_IBASE ();
  }
  SCF_DECLARE_IBASE;
  virtual bool Perform (iTimerEvent*)
  {
    if (pmh) pmh->Cleanup ();
    return false;
  }
};

SCF_IMPLEMENT_IBASE(PolyMeshTimerEvent)
  SCF_IMPLEMENTS_INTERFACE(iTimerEvent)
SCF_IMPLEMENT_IBASE_END

//-------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (PolyMeshHelper)
  SCF_IMPLEMENTS_INTERFACE(iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

void PolyMeshHelper::SetThing (csThingStatic* thing)
{
  PolyMeshHelper::thing = thing;
  static_data_nr = thing->GetStaticDataNumber ()-1;
  num_poly = -1;
}

void PolyMeshHelper::Setup ()
{
  thing->Prepare (0);
  if (static_data_nr != thing->GetStaticDataNumber ())
  {
    static_data_nr = thing->GetStaticDataNumber ();
    ForceCleanup ();
  }

  if (polygons || num_poly == 0)
  {
    // Already set up. First we check if the object vertex array
    // is still valid.
    if (vertices == thing->obj_verts) return ;
  }

  vertices = 0;

  // Count the number of needed polygons and vertices.
  num_verts = thing->GetVertexCount ();
  num_poly = 0;

  size_t i;
  const csPolygonStaticArray &pol = thing->static_polygons;
  for (i = 0 ; i < pol.Length () ; i++)
  {
    csPolygon3DStatic *p = pol.Get (i);
    if (p->flags.CheckAll (poly_flag)) num_poly++;
  }

  // Allocate the arrays and the copy the data.
  if (num_verts)
  {
    vertices = thing->obj_verts;
  }

  if (num_poly)
  {
    polygons = new csMeshedPolygon[num_poly];
    num_poly = 0;
    for (i = 0 ; i < pol.Length () ; i++)
    {
      csPolygon3DStatic *p = pol.Get (i);
      if (p->flags.CheckAll (poly_flag))
      {
        polygons[num_poly].num_vertices = p->GetVertexCount ();
        polygons[num_poly].vertices = p->GetVertexIndices ();
        num_poly++;
      }
    }
  }

  csRef<iEventTimer> timer = csEventTimer::GetStandardTimer (
        thing->thing_type->object_reg);
  PolyMeshTimerEvent* te = new PolyMeshTimerEvent (this);
  timer->AddTimerEvent (te, 9000+(rand ()%2000));
  te->DecRef ();
}

void PolyMeshHelper::Unlock ()
{
  locked--;
  CS_ASSERT (locked >= 0);
  if (locked <= 0)
  {
    csRef<iEventTimer> timer = csEventTimer::GetStandardTimer (
        thing->thing_type->object_reg);
    PolyMeshTimerEvent* te = new PolyMeshTimerEvent (this);
    timer->AddTimerEvent (te, 9000+(rand ()%2000));
    te->DecRef ();
  }
}

void PolyMeshHelper::Cleanup ()
{
  if (locked) return;
  ForceCleanup ();
}

void PolyMeshHelper::ForceCleanup ()
{
  delete[] polygons;
  polygons = 0;
  vertices = 0;
  delete[] triangles;
  triangles = 0;
  num_poly = -1;
}

//----------------------------------------------------------------------

void csThing::CastShadows (iMovable* movable, iFrustumView *lview)
{
  Prepare ();
  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();
  bool ident;
  csReversibleTransform o2c;
  if (cached_movable->IsFullTransformIdentity ())
  {
    ident = true;
  }
  else
  {
    ident = false;
    o2c = cached_movable->GetFullTransform ();
  }

  csMatrix3 m_world2tex;
  csVector3 v_world2tex;

  size_t i;

  iFrustumViewUserdata* fvud = lview->GetUserdata ();
  iLightingProcessInfo* lpi = (iLightingProcessInfo*)fvud;
  bool dyn = lpi->IsDynamic ();

  csRef<csLightingPolyTexQueue> lptq;
  if (!dyn)
  {
    csRef<iLightingProcessData> lpd = lpi->QueryUserdata (
        scfInterfaceTraits<csLightingPolyTexQueue>::GetID (),
        scfInterfaceTraits<csLightingPolyTexQueue>::GetVersion());
    lptq = (csLightingPolyTexQueue*)(iLightingProcessData*)lpd;
    if (!lptq)
    {
      lptq = csPtr<csLightingPolyTexQueue> (new csLightingPolyTexQueue (
          lpi->GetLight ()));
      lpi->AttachUserdata (lptq);
    }
  }

  bool affect = false;
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D* poly = GetPolygon3D ((int)i);
    csPolygon3DStatic* spoly = static_data->GetPolygon3DStatic ((int)i);
    const csPlane3& world_plane = GetPolygonWorldPlaneNoCheck ((int)i);
    if (dyn)
    {
      if (poly->CalculateLightingDynamic (lview, movable, world_plane, spoly))
        affect = true;
    }
    else
    {
      if (ident)
      {
        poly->GetStaticPoly ()->MappingGetTextureSpace (m_world2tex,
                v_world2tex);
      }
      else
      {
        csMatrix3 m_obj2tex;
        csVector3 v_obj2tex;
        poly->GetStaticPoly ()->MappingGetTextureSpace (m_obj2tex,
                v_obj2tex);
        csPolyTexture* lmi = poly->GetPolyTexture ();
        lmi->ObjectToWorld (m_obj2tex, v_obj2tex,
                o2c, m_world2tex, v_world2tex);
      }
      if (poly->CalculateLightingStatic (lview, movable, lptq, true,
                m_world2tex, v_world2tex, world_plane, spoly))
        affect = true;
    }
  }
  if (affect)
    lpi->GetLight ()->AddAffectedLightingInfo ((iLightingInfo*)this);
}

void csThing::InitializeDefault (bool clear)
{
  if (clear) Unprepare();
  Prepare ();

  size_t i;
  for (i = 0; i < polygons.Length (); i++)
    polygons.Get (i).InitializeDefault (clear);
}

bool csThing::ReadFromCache (iCacheManager* cache_mgr)
{
  Prepare ();
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  // For error reporting.
  const char* thing_name = 0;
  if (csThingObjectType::do_verbose && logparent)
  {
    csRef<iMeshWrapper> mw (SCF_QUERY_INTERFACE (logparent, iMeshWrapper));
    if (mw) thing_name = mw->QueryObject ()->GetName ();
  }

  bool rc = true;
  csRef<iDataBuffer> db = cache_mgr->ReadCache ("thing_lm", 0, (uint32) ~0);
  if (db)
  {
    csMemFile mf ((const char*)(db->GetData ()), db->GetSize ());
    size_t i;
    for (i = 0; i < polygons.Length (); i++)
    {
      csPolygon3D& p = polygons.Get (i);
      csPolygon3DStatic* sp = static_data->GetPolygon3DStatic ((int)i);
      const char* error = p.ReadFromCache (&mf, sp);
      if (error != 0)
      {
        rc = false;
        if (csThingObjectType::do_verbose)
        {
          csPrintf ("  Thing '%s' Poly '%s': %s\n",
            thing_name, sp->GetName (), error);
          fflush (stdout);
        }
      }
    }
  }
  else
  {
    if (csThingObjectType::do_verbose)
    {
      csPrintf ("  Thing '%s': Could not find cached lightmap file for thing!\n",
        thing_name);
      fflush (stdout);
    }
    rc = false;
  }

  cache_mgr->SetCurrentScope (0);
  return rc;
}

bool csThing::WriteToCache (iCacheManager* cache_mgr)
{
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  size_t i;
  bool rc = false;
  csMemFile mf;
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D& p = polygons.Get (i);
    csPolygon3DStatic* sp = static_data->GetPolygon3DStatic ((int)i);
    if (!p.WriteToCache (&mf, sp)) goto stop;
  }
  if (!cache_mgr->CacheData ((void*)(mf.GetData ()), mf.GetSize (),
        "thing_lm", 0, (uint32) ~0))
    goto stop;

  rc = true;

stop:
  cache_mgr->SetCurrentScope (0);
  return rc;
}

void csThing::PrepareLighting ()
{
  csColor ambient;
  static_data->thing_type->engine->GetAmbientLight (ambient);
  size_t i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D& p = polygons.Get (i);
    csLightMap* lm = p.GetPolyTexture ()->GetLightMap ();
    if (lm && lm->GetStaticMap ())
    {
      lm->CalcMaxStatic (
          int(ambient.red * 255.0f),
          int(ambient.green * 255.0f),
          int(ambient.blue * 255.0f));
    }
  }
  ClearLMs ();
  PrepareLMs ();
}

void csThing::PrepareRenderMeshes (
  csDirtyAccessArray<csRenderMesh*>& renderMeshes)
{
  size_t i;

  for (i = 0; i < renderMeshes.Length () ; i++)
  {
    // @@@ Is this needed?
    //if (renderMeshes[i]->variablecontext != 0)
      //renderMeshes[i]->variablecontext->DecRef ();
    static_data->thing_type->blk_rendermesh.Free (renderMeshes[i]);
  }
  renderMeshes.DeleteAll ();
  static_data->FillRenderMeshes (renderMeshes, replace_materials, mixmode);
  renderMeshes.ShrinkBestFit ();
  materials_to_visit.DeleteAll ();
  for (i = 0 ; i < renderMeshes.Length () ; i++)
  {
    if (renderMeshes[i]->material->IsVisitRequired ())
      materials_to_visit.Push (renderMeshes[i]->material);
  }
  materials_to_visit.ShrinkBestFit ();

  for (i = 0; i < renderMeshes.Length(); i++)
  {
    csRenderMesh* rm = renderMeshes[i];
    rm->variablecontext->GetVariable (static_data->texLightmapName)->
      SetValue (i < litPolys.Length() ? litPolys[i]->SLM->GetTexture() : 0);
  }

}

void csThing::PrepareForUse ()
{
  Prepare ();
  PreparePolygonBuffer ();
  PrepareLMs ();

  WorUpdate ();
  UpdateDirtyLMs ();

  bool meshesCreated;
  csDirtyAccessArray<csRenderMesh*>& renderMeshes =
    meshesHolder.GetUnusedData (meshesCreated, 0);
  if (renderMeshes.Length() == 0)
  {
    PrepareRenderMeshes (renderMeshes);
  }
}

csRenderMesh **csThing::GetRenderMeshes (int &num, iRenderView* rview,
                                         iMovable* movable, uint32 frustum_mask)
{
  Prepare ();

  iCamera *icam = rview->GetCamera ();

  // Only get the transformation if this thing can move.
  bool can_move = false;
  if (movable && cfg_moving != CS_THING_MOVE_NEVER)
  {
    can_move = true;
  }

  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

  csBox3 b;
  static_data->GetBoundingBox (b);

  csSphere sphere;
  sphere.SetCenter (b.GetCenter ());
  sphere.SetRadius (static_data->max_obj_radius);

  size_t i;

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
        clip_z_plane);

  const uint currentFrame = rview->GetCurrentFrameNumber ();
  bool meshesCreated;
  csDirtyAccessArray<csRenderMesh*>& renderMeshes =
    meshesHolder.GetUnusedData (meshesCreated, currentFrame);

  if (renderMeshes.Length() == 0)
  {
    PrepareRenderMeshes (renderMeshes);
  }

  const csVector3& wo = movable->GetFullPosition ();
  const csReversibleTransform& t = movable->GetFullTransform ();

  for (i = 0; i < renderMeshes.Length(); i++)
  {
    csRenderMesh* rm = renderMeshes[i];
    rm->worldspace_origin = wo;
    rm->clip_portal = clip_portal;
    rm->clip_plane = clip_plane;
    rm->clip_z_plane = clip_z_plane;
    rm->do_mirror = icam->IsMirrored ();
    rm->object2world = t;

    // Jorrit: Moved the code below to PrepareRenderMeshes().
    //rm->variablecontext->GetVariable (static_data->texLightmapName)->
      //SetValue (i < litPolys.Length() ? litPolys[i]->SLM->GetTexture() : 0);
  }

  UpdateDirtyLMs (); // @@@ Here?

  num = (int)renderMeshes.Length ();
  for (i = 0; i < materials_to_visit.Length (); i++)
  {
    materials_to_visit[i]->Visit ();
  }

  return renderMeshes.GetArray ();
}

void csThing::PrepareLMs ()
{
  if (IsLmPrepared()) return;

  csThingObjectType* thing_type = static_data->thing_type;
  iTextureManager* txtmgr = thing_type->G3D->GetTextureManager ();

  csHash<csRef<iSuperLightmap>,
    csPtrKey<csThingStatic::StaticSuperLM> > superLMs;

  size_t i;
  for (i = 0; i < static_data->litPolys.Length(); i++)
  {
    const csThingStatic::csStaticLitPolyGroup& slpg =
      *(static_data->litPolys[i]);

    const csRef<iSuperLightmap>* SLMptr =
      superLMs.GetElementPointer (slpg.staticSLM);
    csRef<iSuperLightmap> SLM;

    if (SLMptr == 0)
    {
      SLM = txtmgr->CreateSuperLightmap (slpg.staticSLM->width,
        slpg.staticSLM->height);
      superLMs.Put (slpg.staticSLM, SLM);
    }
    else
      SLM = *SLMptr;

    // SLM creation failed for some reason. The polys will be drawn unlit.
    if (SLM == 0)
    {
      csPolyGroup* pg = new csPolyGroup;
      pg->material = FindRealMaterial (slpg.material);
      if (pg->material == 0) pg->material = slpg.material;

      size_t j;
      pg->polys.SetLength (slpg.polys.Length ());
      for (j = 0; j < slpg.polys.Length(); j++)
      {
        pg->polys.Put (j, slpg.polys[j]);
      }
      //pg->polys.ShrinkBestFit();

      unlitPolys.Push (pg);
    }
    else
    {
      csLitPolyGroup* lpg = new csLitPolyGroup;
      lpg->material = FindRealMaterial (slpg.material);
      if (lpg->material == 0) lpg->material = slpg.material;
      lpg->SLM = SLM;

      size_t j;
      lpg->lightmaps.SetLength (slpg.polys.Length ());
      lpg->polys.SetLength (slpg.polys.Length ());
      for (j = 0; j < slpg.polys.Length(); j++)
      {
        csPolygon3D* poly = &polygons[slpg.polys[j]];

        lpg->polys.Put (j, slpg.polys[j]);
        const csRect& r = slpg.lmRects[j];
        csRef<iRendererLightmap> rlm =
          SLM->RegisterLightmap (r.xmin, r.ymin, r.Width (), r.Height ());

        csPolyTexture* polytxt = poly->GetPolyTexture ();
        rlm->SetLightCellSize (polytxt->GetLightCellSize ());
        polytxt->SetRendererLightmap (rlm);

        lpg->lightmaps.Put (j, rlm);
      }

      litPolys.Push (lpg);
    }
  }

  for (i = 0; i < static_data->unlitPolys.Length(); i++)
  {
    const csThingStatic::csStaticPolyGroup& spg =
      *(static_data->unlitPolys[i]);
    csPolyGroup* pg = new csPolyGroup;
    pg->material = FindRealMaterial (spg.material);
    if (pg->material == 0) pg->material = spg.material;

    size_t j;
    pg->polys.SetLength (spg.polys.Length ());
    for (j = 0; j < spg.polys.Length(); j++)
    {
      pg->polys.Put (j, spg.polys[j]);
    }
    //pg->polys.ShrinkBestFit();

    unlitPolys.Push (pg);
  }

  litPolys.ShrinkBestFit ();
  unlitPolys.ShrinkBestFit ();

  SetLmDirty (true);
  SetLmPrepared (true);
}

void csThing::ClearLMs ()
{
  if (!IsLmPrepared()) return;

  litPolys.DeleteAll ();
  unlitPolys.DeleteAll ();

  SetLmDirty (true);
  SetLmPrepared (false);
}

void csThing::UpdateDirtyLMs ()
{
  csColor amb = dynamic_ambient;
  if (cached_movable)
  {
    // First check if dynamic ambient has changed.
    iSector* s = cached_movable->GetSectors ()->Get (0);
    amb += s->GetDynamicAmbientLight ();
    if (dynamic_ambient_version != s->GetDynamicAmbientVersion ())
    {
      dynamic_ambient_version = s->GetDynamicAmbientVersion ();
      MarkLightmapsDirty ();
    }
  }

  if (!IsLmDirty()) return;

  bool ident;
  csReversibleTransform o2c;
  if (!cached_movable || cached_movable->IsFullTransformIdentity ())
  {
    ident = true;
  }
  else
  {
    ident = false;
    o2c = cached_movable->GetFullTransform ();
  }

  csMatrix3 m_world2tex;
  csVector3 v_world2tex;

  size_t i;
  for (i = 0; i < litPolys.Length (); i++)
  {
    size_t j;
    for (j = 0; j < litPolys[i]->polys.Length (); j++)
    {
      csPolygon3D& poly = polygons[litPolys[i]->polys[j]];
      csPolyTexture* lmi = poly.GetPolyTexture ();
      if (ident)
      {
        poly.GetStaticPoly ()->MappingGetTextureSpace (m_world2tex,
                v_world2tex);
      }
      else
      {
        csMatrix3 m_obj2tex;
        csVector3 v_obj2tex;
        poly.GetStaticPoly ()->MappingGetTextureSpace (m_obj2tex,
                v_obj2tex);
        csPolyTexture* lmi = poly.GetPolyTexture ();
        lmi->ObjectToWorld (m_obj2tex, v_obj2tex,
                o2c, m_world2tex, v_world2tex);
      }
      if (lmi->GetLightVersion () != GetLightVersion ())
      {
        const csPlane3& world_plane = GetPolygonWorldPlaneNoCheck (
                poly.GetPolyIdx ());
        csLightingScratchBuffer& scratch = static_data->thing_type->lightingScratch;
        if (lmi->RecalculateDynamicLights (m_world2tex, v_world2tex, &poly,
                world_plane, amb, scratch))
        {
          litPolys[i]->lightmaps[j]->SetData (
            /*lmi->GetLightMap ()->GetMapData ()*/scratch.GetArray ());
        }
      }
    }
  }

  SetLmDirty (false);
}

//---------------------------------------------------------------------------

iMeshObjectFactory *csThing::GetFactory () const
{
  return (iMeshObjectFactory*)static_data;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csThingObjectType)
  SCF_IMPLEMENTS_INTERFACE(iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iThingEnvironment)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPluginConfig)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiThingEnvironment)
  SCF_IMPLEMENTS_INTERFACE(iThingEnvironment)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiPluginConfig)
  SCF_IMPLEMENTS_INTERFACE(iPluginConfig)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiDebugHelper)
  SCF_IMPLEMENTS_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csThingObjectType)


csThingObjectType::csThingObjectType (iBase *pParent) :
        blk_polygon3dstatic (2000),
        blk_texturemapping (2000),
        blk_lightmap (2000),
        blk_polidx3 (1000),
        blk_polidx4 (2000)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiThingEnvironment);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPluginConfig);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  lightpatch_pool = 0;
  blk_polidx5 = 0;
  blk_polidx6 = 0;
  blk_polidx20 = 0;
  blk_polidx60 = 0;
}

csThingObjectType::~csThingObjectType ()
{
  delete lightpatch_pool;
  delete blk_polidx5;
  delete blk_polidx6;
  delete blk_polidx20;
  delete blk_polidx60;

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiThingEnvironment);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiPluginConfig);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  SCF_DESTRUCT_IBASE ();
}

bool csThingObjectType::Initialize (iObjectRegistry *object_reg)
{
  csThingObjectType::object_reg = object_reg;
  csRef<iEngine> e = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = e;   // We don't want a real ref here to avoid circular refs.
  csRef<iGraphics3D> g = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  G3D = g;
  if (!g) return false;

  lightpatch_pool = new csLightPatchPool ();

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr)
    csThingObjectType::do_verbose = verbosemgr->Enabled ("thing");

  csRef<iTextureManager> txtmgr = g->GetTextureManager ();

  int maxTW, maxTH, maxTA;
  txtmgr->GetMaxTextureSize (maxTW, maxTH, maxTA);

  csConfigAccess cfg (object_reg, "/config/thing.cfg");

  int maxLightmapSize = cfg->GetInt ("Mesh.Thing.MaxSuperlightmapSize",
    /*256*/MIN (maxTW, maxTH));
  maxLightmapW =
    cfg->GetInt ("Mesh.Thing.MaxSuperlightmapWidth", maxLightmapSize);
  maxLightmapW = MIN (maxLightmapW, maxTW);
  maxLightmapH =
    cfg->GetInt ("Mesh.Thing.MaxSuperlightmapHeight", maxLightmapSize);
  maxLightmapH = MIN (maxLightmapH, maxTH);
  maxSLMSpaceWaste =
    cfg->GetFloat ("Mesh.Thing.MaxSuperlightmapWaste", 0.6f);
  csThing::lightmap_quality = cfg->GetInt (
      "Mesh.Thing.LightmapQuality", 3);
  csThing::lightmap_enabled = cfg->GetBool (
      "Mesh.Thing.EnableLightmaps", true);
  if (csThingObjectType::do_verbose)
  {
    Notify ("Lightmap quality=%d", csThing::lightmap_quality);
    Notify ("Lightmapping enabled=%d", (int)csThing::lightmap_enabled);
  }

  stringset = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  shadermgr = csQueryRegistry<iShaderManager> (object_reg);

  return true;
}

void csThingObjectType::Clear ()
{
  delete lightpatch_pool;
  lightpatch_pool = new csLightPatchPool ();
}

csPtr<iMeshObjectFactory> csThingObjectType::NewFactory ()
{
  csThingStatic *cm = new csThingStatic (this, this);
  csRef<iMeshObjectFactory> ifact (SCF_QUERY_INTERFACE (
      cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csThingObjectType::DebugCommand (const char* cmd)
{
  return false;
}

void csThingObjectType::Warn (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  csReportV (object_reg,
    CS_REPORTER_SEVERITY_WARNING,
    "crystalspace.mesh.object.thing",
    description,
    arg);

  va_end (arg);
}

void csThingObjectType::Bug (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  csReportV (object_reg,
    CS_REPORTER_SEVERITY_BUG,
    "crystalspace.mesh.object.thing",
    description,
    arg);

  va_end (arg);
}

void csThingObjectType::Error (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  csReportV (object_reg,
    CS_REPORTER_SEVERITY_ERROR,
    "crystalspace.mesh.object.thing",
    description,
    arg);

  va_end (arg);
}

void csThingObjectType::Notify (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  csReportV (object_reg,
    CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.mesh.object.thing",
    description,
    arg);

  va_end (arg);
}

//---------------------------------------------------------------------------

static const csOptionDescription
  config_options[] =
{
  { 0, "cosfact", "Cosinus factor for lighting", CSVAR_FLOAT },
  { 1, "lightqual", "Lighting quality", CSVAR_LONG },
  { 2, "lightmapping", "Enable/disable lightmapping", CSVAR_BOOL }
};
const int NUM_OPTIONS =
  (
    sizeof (config_options) /
    sizeof (config_options[0])
  );

bool csThingObjectType::eiPluginConfig::SetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:
      csPolyTexture::cfg_cosinus_factor = value->GetFloat ();
      break;
    case 1:
      csThing::lightmap_quality = value->GetLong ();
      if (csThingObjectType::do_verbose)
        scfParent->Notify ("Lightmap quality=%d", csThing::lightmap_quality);
      break;
    case 2:
      csThing::lightmap_enabled = value->GetBool ();
      if (csThingObjectType::do_verbose)
        scfParent->Notify ("Lightmapping enabled=%d",
                (int)csThing::lightmap_enabled);
      break;
    default:
      return false;
  }

  return true;
}

bool csThingObjectType::eiPluginConfig::GetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:   value->SetFloat (csPolyTexture::cfg_cosinus_factor); break;
    case 1:   value->SetLong (csThing::lightmap_quality); break;
    case 2:   value->SetBool (csThing::lightmap_enabled); break;
    default:  return false;
  }

  return true;
}

bool csThingObjectType::eiPluginConfig::GetOptionDescription (
  int idx,
  csOptionDescription *option)
{
  if (idx < 0 || idx >= NUM_OPTIONS) return false;
  *option = config_options[idx];
  return true;
}

//---------------------------------------------------------------------------

