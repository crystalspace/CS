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

#include "cssysdef.h"
#include <limits.h>
#include "cssys/csendian.h"
#include "qint.h"
#include "thing.h"
#include "polygon.h"
#include "polytext.h"
#include "lppool.h"
#include "csgeom/polypool.h"
#include "csgeom/poly3d.h"
#include "csgeom/frustum.h"
#include "iengine/light.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/camera.h"
#include "iengine/statlght.h"
#include "iengine/dynlight.h"
#include "iengine/shadows.h"
#include "csgeom/sphere.h"
#include "csgeom/math3d.h"
#include "csutil/csstring.h"
#include "csutil/memfile.h"
#include "csutil/hash.h"
#include "csutil/hashmap.h"
#include "csutil/debug.h"
#include "csutil/csmd5.h"
#include "csutil/array.h"
#include "csutil/garray.h"
#include "csutil/cfgacc.h"
#include "csutil/timer.h"
#include "csutil/weakref.h"
#include "ivideo/txtmgr.h"
#include "ivideo/vbufmgr.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "iengine/shadcast.h"
#include "iutil/vfs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/cache.h"
#include "iutil/cmdline.h"
#include "iutil/strset.h"
#include "iengine/rview.h"
#include "iengine/fview.h"
#include "qsqrt.h"
#include "ivideo/graph3d.h"
#include "ivideo/polyrender.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"
#include "igraphic/imageio.h"
#include "csgfx/memimage.h"
#include "csgeom/subrec.h"
#include "csgeom/subrec2.h"

#ifdef CS_DEBUG
  //#define LIGHTMAP_DEBUG
#endif

CS_IMPLEMENT_PLUGIN

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csThing)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iThingState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iLightingInfo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iShadowCaster)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iShadowReceiver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMeshObject)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::ThingState)
  SCF_IMPLEMENTS_INTERFACE(iThingState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::LightingInfo)
  SCF_IMPLEMENTS_INTERFACE(iLightingInfo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::ShadowCaster)
  SCF_IMPLEMENTS_INTERFACE(iShadowCaster)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::ShadowReceiver)
  SCF_IMPLEMENTS_INTERFACE(iShadowReceiver)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::MeshObject)
  SCF_IMPLEMENTS_INTERFACE(iMeshObject)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


int csThing:: last_thing_id = 0;

//----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csThingStatic)
  SCF_IMPLEMENTS_INTERFACE(iThingFactoryState)
  SCF_IMPLEMENTS_INTERFACE(iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iObjectModel)
#ifdef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_INTERFACE(iRenderBufferSource)
#endif
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingStatic::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE(iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

#ifdef CS_USE_NEW_RENDERER
/*csStringID csThingStatic::vertex_name = csInvalidStringID;
csStringID csThingStatic::texel_name = csInvalidStringID;
csStringID csThingStatic::normal_name = csInvalidStringID;
csStringID csThingStatic::color_name = csInvalidStringID;
csStringID csThingStatic::index_name = csInvalidStringID;
csStringID csThingStatic::tangent_name = csInvalidStringID;
csStringID csThingStatic::binormal_name = csInvalidStringID;*/
#endif

csThingStatic::csThingStatic (iBase* parent, csThingObjectType* thing_type) :
	static_polygons (32, 64),
	scfiPolygonMesh (0),
	scfiPolygonMeshCD (CS_POLY_COLLDET),
	scfiPolygonMeshLOD (CS_POLY_VISCULL)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  csThingStatic::thing_type = thing_type;
  static_polygons.SetThingType (thing_type);

  scfiPolygonMesh.SetThing (this);
  scfiPolygonMeshCD.SetThing (this);
  scfiPolygonMeshLOD.SetThing (this);
  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMeshCD);
  scfiObjectModel.SetPolygonMeshViscull (&scfiPolygonMeshLOD);
  scfiObjectModel.SetPolygonMeshShadows (&scfiPolygonMeshLOD);

  max_vertices = num_vertices = 0;
  obj_verts = 0;
  obj_normals = 0;
  smoothed = false;

  obj_bbox_valid = false;

  prepared = false;
  lmprepared = false;
  cosinus_factor = -1;
  logparent = 0;

#ifdef CS_USE_NEW_RENDERER
  r3d = CS_QUERY_REGISTRY (thing_type->object_reg, iGraphics3D);

/*  if ((vertex_name == csInvalidStringID) ||
    (texel_name == csInvalidStringID) ||
    (normal_name == csInvalidStringID) ||
    (color_name == csInvalidStringID) ||
    (index_name == csInvalidStringID) ||
    (tangent_name == csInvalidStringID) ||
    (binormal_name == csInvalidStringID))
  {
    csRef<iStringSet> strings = 
      CS_QUERY_REGISTRY_TAG_INTERFACE (thing_type->object_reg,
        "crystalspace.renderer.stringset", iStringSet);

    vertex_name = strings->Request ("vertices");
    texel_name = strings->Request ("texture coordinates");
    normal_name = strings->Request ("normals");
    color_name = strings->Request ("colors");
    index_name = strings->Request ("indices");
    tangent_name = strings->Request ("tangents");
    binormal_name = strings->Request ("binormals");
  }*/
#endif
}

csThingStatic::~csThingStatic ()
{
  delete[] obj_verts;
  delete[] obj_normals;

  UnprepareLMLayout ();
}

void csThingStatic::Prepare ()
{
  if (!prepared) 
  {
    prepared = true;

    if (!flags.Check (CS_THING_NOCOMPRESS))
    {
      CompressVertices ();
      RemoveUnusedVertices ();
    }

    if (smoothed)
      CalculateNormals();

    int i;
    csPolygon3DStatic* sp;
    for (i = 0; i < static_polygons.Length (); i++)
    {
      sp = static_polygons.Get (i);
      // If a Finish() call returns false this means the textures are not
      // completely ready yet. In that case we set 'prepared' to false
      // again so that we force a new prepare later.
      if (!sp->Finish ())
	prepared = false;
    }
  }
  
  if (prepared)
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
  if (lmprepared) return;

  csHash<csStaticPolyGroup*, iMaterialWrapper*> polysSorted;

  int i;
  for (i = 0; i < static_polygons.Length (); i++)
  {
    int polyIdx = i;
    csPolygon3DStatic* sp = static_polygons.Get (polyIdx);

    iMaterialWrapper* mat = sp->GetMaterialWrapper ();

    csStaticPolyGroup* lp = polysSorted.Get (mat);
    if (lp == 0)
    {
      lp = new csStaticPolyGroup;
      lp->material = mat;
      lp->numLitPolys = 0;
      lp->totalLumels = 0;
      polysSorted.Put (mat, lp);
    }

    csPolyLightMapMapping* lm = sp->GetLightMapMapping ();
    if ((lm != 0) && (sp->flags.Check (CS_POLY_LIGHTING)))
    {
      lp->numLitPolys++;

      int lmw = (csLightMap::CalcLightMapWidth (lm->GetOriginalWidth ()));
      int lmh = (csLightMap::CalcLightMapHeight (lm->GetHeight ()));
      lp->totalLumels += lmw * lmh;
    }

    lp->polys.Push (polyIdx);
  }

  /*
   * Presort polys.
   */
  csArray<csStaticPolyGroup*> polys;
  {
    csHash<csStaticPolyGroup*, iMaterialWrapper*>::GlobalIterator polyIt = 
      polysSorted.GetIterator ();

    while (polyIt.HasNext ())
    {
      csStaticPolyGroup* lp = polyIt.Next ();
      polys.InsertSorted (lp, CompareStaticPolyGroups);
    }
  }

  csStaticPolyGroup* rejectedPolys = new csStaticPolyGroup;
  for (i = 0; i < polys.Length (); i++)
  {				      
    csStaticPolyGroup* lp = polys[i];

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

  lmprepared = true;
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
  csPolyLightMapMapping* lm1 = poly1->GetLightMapMapping ();
  csPolyLightMapMapping* lm2 = poly2->GetLightMapMapping ();

  int maxdim1, mindim1, maxdim2, mindim2;

  maxdim1 = MAX (
    csLightMap::CalcLightMapWidth (lm1->GetOriginalWidth ()), 
    csLightMap::CalcLightMapHeight (lm1->GetHeight ()));
  mindim1 = MIN (
    csLightMap::CalcLightMapWidth (lm1->GetOriginalWidth ()), 
    csLightMap::CalcLightMapHeight (lm1->GetHeight ()));
  maxdim2 = MAX (
    csLightMap::CalcLightMapWidth (lm2->GetOriginalWidth ()), 
    csLightMap::CalcLightMapHeight (lm2->GetHeight ()));
  mindim2 = MIN (
    csLightMap::CalcLightMapWidth (lm2->GetOriginalWidth ()), 
    csLightMap::CalcLightMapHeight (lm2->GetHeight ()));

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

  int i;

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

    csPolyLightMapMapping* lm = sp->GetLightMapMapping ();
    if ((lm == 0) || (!sp->flags.Check (CS_POLY_LIGHTING)))
    {
      rejectedPolys->polys.Push (polyIdx);
      continue;
    }

    int lmw = (csLightMap::CalcLightMapWidth (lm->GetOriginalWidth ())
    	+ LM_BORDER);
    int lmh = (csLightMap::CalcLightMapHeight (lm->GetHeight ())
    	+ LM_BORDER);

    if ((lmw > thing_type->maxLightmapW) || 
      (lmh > thing_type->maxLightmapH)) 
    {
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
    int s = 0;
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
	csSubRect2* slmSR;
	int polyIdx = inputQueues[curQueue].polys.Pop ();
	csPolygon3DStatic* sp = static_polygons[polyIdx];

	csPolyLightMapMapping* lm = sp->GetLightMapMapping ();

	int lmw = (csLightMap::CalcLightMapWidth (lm->GetOriginalWidth ())
		+ LM_BORDER);
	int lmh = (csLightMap::CalcLightMapHeight (lm->GetHeight ())
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
	  curOutputPolys->slmSubrects.Push (slmSR);
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
      int nidx = superLMs.InsertSorted (slm, CompareStaticSuperLM);
      if (nidx <= s + 1)
      {
	s++;
      }

      if (curOutputPolys->polys.Length () > 0)
      {
	outputPolys.Push (curOutputPolys);
	curOutputPolys = new csStaticLitPolyGroup;
      }
    
      curQueue ^= 1;
    }

    // Add a new empty SLM. Not all polys could be stuffed away, 
    // so we possibly need more space.
    if (inputQueues[curQueue].polys.Length () > 0)
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
  delete curOutputPolys;

}

void csThingStatic::UnprepareLMLayout ()
{
  if (!lmprepared) return;
  litPolys.DeleteAll ();
  unlitPolys.DeleteAll ();

  int i;
  for (i = 0; i < superLMs.Length (); i++)
  {
    StaticSuperLM* sslm = superLMs[i];
    delete sslm;
  }
  superLMs.DeleteAll ();
  lmprepared = false;
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
  scfiObjectModel.ShapeChanged ();
  return num_vertices - 1;
}

void csThingStatic::SetVertex (int idx, const csVector3 &vt)
{
  CS_ASSERT (idx >= 0 && idx < num_vertices);
  obj_verts[idx] = vt;
  scfiObjectModel.ShapeChanged ();
}

void csThingStatic::DeleteVertex (int idx)
{
  CS_ASSERT (idx >= 0 && idx < num_vertices);

  int copysize = sizeof (csVector3) * (num_vertices - idx - 1);
  memmove (obj_verts + idx, obj_verts + idx + 1, copysize);
  scfiObjectModel.ShapeChanged ();
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

  scfiObjectModel.ShapeChanged ();
}

void csThingStatic::CompressVertices ()
{
  csVector3* new_obj;
  int count_unique;
  csCompressVertex* vt = csVector3Array::CompressVertices (
  	obj_verts, num_vertices, new_obj, count_unique);
  if (vt == 0) return;

  // Replace the old vertex tables.
  delete[] obj_verts;
  obj_verts = new_obj;
  num_vertices = max_vertices = count_unique;

  // Now we can remap the vertices in all polygons.
  int i, j;
  for (i = 0; i < static_polygons.Length (); i++)
  {
    csPolygon3DStatic *p = static_polygons.Get (i);
    csPolyIndexed &pi = p->GetVertices ();
    int *idx = pi.GetVertexIndices ();
    for (j = 0; j < pi.GetVertexCount (); j++) idx[j] = vt[idx[j]].new_idx;
  }

  delete[] vt;
  scfiObjectModel.ShapeChanged ();
}

void csThingStatic::RemoveUnusedVertices ()
{
  if (num_vertices <= 0) return ;

  // Copy all the vertices that are actually used by polygons.
  bool *used = new bool[num_vertices];
  int i, j;
  for (i = 0; i < num_vertices; i++) used[i] = false;

  // Mark all vertices that are used as used.
  for (i = 0; i < static_polygons.Length (); i++)
  {
    csPolygon3DStatic *p = static_polygons.Get (i);
    csPolyIndexed &pi = p->GetVertices ();
    int *idx = pi.GetVertexIndices ();
    for (j = 0; j < pi.GetVertexCount (); j++) used[idx[j]] = true;
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
  for (i = 0; i < static_polygons.Length (); i++)
  {
    csPolygon3DStatic *p = static_polygons.Get (i);
    csPolyIndexed &pi = p->GetVertices ();
    int *idx = pi.GetVertexIndices ();
    for (j = 0; j < pi.GetVertexCount (); j++) idx[j] = relocate[idx[j]];
  }

  delete[] relocate;
  delete[] used;

  obj_bbox_valid = false;
  scfiObjectModel.ShapeChanged ();
}

struct PolygonsForVertex
{
  csArray<int> poly_indices;
};

void csThingStatic::CalculateNormals ()
{
  int polyCount = static_polygons.Length();
  int i, j;

  delete[] obj_normals;
  obj_normals = new csVector3[num_vertices];

  // First build a table so that we can find all polygons that connect
  // to a vertex easily.
  PolygonsForVertex* pvv = new PolygonsForVertex[num_vertices];
  for (i = 0 ; i < polyCount ; i++)
  {
    csPolygon3DStatic* p = static_polygons.Get (i);
    int* vtidx = p->GetVertexIndices ();
    for (j = 0 ; j < p->GetVertexCount () ; j++)
    {
      CS_ASSERT (vtidx[j] >= 0 && vtidx[j] < num_vertices);
      pvv[vtidx[j]].poly_indices.Push (i);
    }
  }

  // Now calculate normals.
  for (i = 0 ; i < num_vertices ; i++)
  {
    csVector3 n (0);
    for (j = 0 ; j < pvv[i].poly_indices.Length () ; j++)
    {
      csPolygon3DStatic* p = static_polygons.Get (pvv[i].poly_indices[j]);
      const csVector3& normal = p->GetObjectPlane ().Normal();
      n += normal;
    }
    float norm = n.Norm ();
    if (norm) n /= norm;
    obj_normals[i] = n;
  }

  delete[] pvv;
}

int csThingStatic::FindPolygonIndex (iPolygon3DStatic *polygon) const
{
  csPolygon3DStatic *p = polygon->GetPrivateObject ();
  return static_polygons.Find (p);
}

iPolygon3DStatic *csThingStatic::GetPolygon (int idx)
{
  return &(static_polygons.Get (idx)->scfiPolygon3DStatic);
}

iPolygon3DStatic *csThingStatic::GetPolygon (const char* name)
{
  int idx = static_polygons.FindKey ((void*)name, static_polygons.CompareKey);
  return idx >= 0 ? &(static_polygons.Get (idx)->scfiPolygon3DStatic) : 0;
}

void csThingStatic::AddPolygon (csPolygon3DStatic* spoly)
{
  spoly->SetParent (this);
  spoly->EnableTextureMapping (true);
  static_polygons.Push (spoly);
  scfiObjectModel.ShapeChanged ();
  UnprepareLMLayout ();
}

iPolygon3DStatic *csThingStatic::CreatePolygon (const char *name)
{
  csPolygon3DStatic* sp = thing_type->blk_polygon3dstatic.Alloc ();
  if (name) sp->SetName (name);
  AddPolygon (sp);
  return &(sp->scfiPolygon3DStatic);
}

void csThingStatic::RemovePolygon (int idx)
{
  static_polygons.FreeItem (static_polygons.Get (idx));
  static_polygons.DeleteIndex (idx);
  scfiObjectModel.ShapeChanged ();
  UnprepareLMLayout ();
}

void csThingStatic::RemovePolygons ()
{
  static_polygons.FreeAll ();
  scfiObjectModel.ShapeChanged ();
  UnprepareLMLayout ();
}

int csThingStatic::IntersectSegmentIndex (
  const csVector3 &start, const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  int i;
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
        best_p = i;
        isect = cur_isect;
      }
    }
  }

  if (pr) *pr = best_r;
  return best_p;
}

csPtr<csThingStatic> csThingStatic::Clone ()
{
  csThingStatic* clone = new csThingStatic (scfParent, thing_type);
  clone->flags.SetAll (GetFlags ().Get ());
  clone->smoothed = smoothed;
  clone->obj_bbox = obj_bbox;
  clone->obj_bbox_valid = obj_bbox_valid;
  clone->obj_radius = obj_radius;
  clone->max_obj_radius = max_obj_radius;
  clone->prepared = prepared;
  clone->scfiObjectModel.SetShapeNumber (scfiObjectModel.GetShapeNumber ());
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

  int i;
  for (i = 0 ; i < static_polygons.Length () ; i++)
  {
    csPolygon3DStatic* p = static_polygons.Get (i)->Clone ();
    p->SetParent (clone);
    clone->static_polygons.Push (p);
  }

  return csPtr<csThingStatic> (clone);
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
  for (i = 0; i < static_polygons.Length (); i++)
  {
    csPolygon3DStatic *p = GetPolygon3DStatic (i);
    p->HardTransform (t);
  }

  scfiObjectModel.ShapeChanged ();
}

csPtr<iMeshObject> csThingStatic::NewInstance ()
{
  csThing *thing = new csThing ((iBase*)(iThingFactoryState*)this, this);
  return csPtr<iMeshObject> (&thing->scfiMeshObject);
}

void csThingStatic::GetBoundingBox (csBox3 &box)
{
  int i;

  if (obj_bbox_valid)
  {
    box = obj_bbox;
    return ;
  }

  obj_bbox_valid = true;

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
  max_obj_radius = qsqrt (csSquaredDist::PointPoint (
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

#ifdef CS_USE_NEW_RENDERER
iRenderBuffer* csThingStatic::GetRenderBuffer (csStringID name)
{
/*  if (name == vertex_name)
  {
    return vertex_buffer;
  } 
  else if (name == texel_name)
  {
    return texel_buffer;
  }
  else if (name == normal_name)
  {
    return normal_buffer;
  }
  else if (name == color_name)
  {
    return color_buffer;
  }
  else if (name == index_name)
  {
    return index_buffer;
  }
  else if (name == tangent_name)
  {
    return tangent_buffer;
  }
  else if (name == binormal_name)
  {
    return binormal_buffer;
  }
  else*/
  {
    return 0;
  }
}

void csThingStatic::FillRenderMeshes (
	csDirtyAccessArray<csRenderMesh*>& rmeshes,
	const csArray<RepMaterial>& repMaterials,
	uint mixmode)
{
  polyRenderers.DeleteAll ();

/*  int num_verts = 0, num_indices = 0, max_vc = 0;
  int i;

  for (i = 0; i < static_polygons.Length(); i++)
  {
    csPolygon3DStatic* poly = static_polygons.Get (i);

    int pvc = poly->GetVertexCount ();

    num_verts += pvc;
    num_indices += (pvc - 2) * 3;
    max_vc = MAX (max_vc, pvc);
  }

  if (!vertex_buffer)
    vertex_buffer = r3d->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
  csVector3* vertices = (csVector3*)vertex_buffer->Lock (CS_BUF_LOCK_NORMAL);

  if (!normal_buffer)
    normal_buffer = r3d->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
  csVector3* normals = (csVector3*)normal_buffer->Lock (CS_BUF_LOCK_NORMAL);

  if (!texel_buffer)
    texel_buffer = r3d->CreateRenderBuffer (num_verts * sizeof (csVector2), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2, false);
  csVector2* texels = (csVector2*)texel_buffer->Lock (CS_BUF_LOCK_NORMAL);

  if (!index_buffer)
    index_buffer = r3d->CreateRenderBuffer (num_indices  * sizeof (int), 
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 1, true);
  int* indices = (int*)index_buffer->Lock (CS_BUF_LOCK_NORMAL);

  if (!tangent_buffer)
    tangent_buffer = r3d->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
  csVector3* tangents = (csVector3*)tangent_buffer->Lock (CS_BUF_LOCK_NORMAL);

  if (!binormal_buffer)
    binormal_buffer = r3d->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
  csVector3* binormals = (csVector3*)binormal_buffer->Lock (CS_BUF_LOCK_NORMAL);

  CS_ALLOC_STACK_ARRAY (int, pvIndices, max_vc);

  int vindex = 0, iindex = 0;*/

  for (int i = 0; i < litPolys.Length (); i++)
  {
    const csStaticPolyGroup& pg = *(litPolys[i]);
    csRenderMesh* rm = new csRenderMesh;

    csRef<iPolygonRenderer> polyRenderer = r3d->CreatePolygonRenderer ();
    polyRenderers.Push (polyRenderer);

    rm->buffersource = this;
    rm->z_buf_mode = CS_ZBUF_USE;
    rm->mixmode = mixmode; 
    rm->material = pg.material;
    //rm->meshtype = CS_MESHTYPE_TRIANGLES;
    rm->meshtype = CS_MESHTYPE_POLYGON;

    //rm->indexstart = iindex;

    int j;
    for (j = 0; j< pg.polys.Length(); j++)
    {
      csRef<iPolygon3DStatic> iStatic = 
        SCF_QUERY_INTERFACE(static_polygons[pg.polys[j]], iPolygon3DStatic);
      polyRenderer->AddPolygon (iStatic);

#if 0
      csPolygon3DStatic* static_data = static_polygons[pg.polys[j]];
      int* poly_indices = static_data->GetVertexIndices ();

      csVector3 polynormal;
      if (!smoothed)
      {
	// hmm... It seems that both polynormal and obj_normals[] need to be inverted.
	//  Don't know why, just found it out empirical.
	polynormal = -static_data->GetObjectPlane().Normal();
      }

      /*
	To get the texture coordinates of a vertex, the coordinates
	in object space have to be transformed to the texture space.
	The Z part is simply dropped then.
      */
      csMatrix3 t_m;
      csVector3 t_v;
      static_data->MappingGetTextureSpace (t_m, t_v);
      csTransform object2texture (t_m, t_v);

      /*
        Calculate the 'tangent' vector of this poly, needed for dot3.
        It is "a tangent to the surface which represents the direction 
        of increase of the t texture coordinate." (Quotation from
         http://www.ati.com/developer/sdk/rage128sdk/Rage128BumpTutorial.html)
         Conveniently, all polys have a object->texture space transformatin
         associated with them.

         @@@ Ignores the fact things can be smooth.
         But it's simpler for now :)
       */
      csTransform tangentTF (t_m.GetInverse (), csVector3 (0));
      csVector3 tangent = tangentTF.Other2This (csVector3 (1, 0, 0));
      tangent.Normalize ();

      /*
      Calculate the 'binormal' vector of this poly, needed for dot3.
      */
      csVector3 binormal = tangentTF.Other2This (csVector3 (0, -1, 0));
      binormal.Normalize ();

      // First, fill the normal/texel/vertex buffers.
      int j, vc = static_data->GetVertexCount();
      for (j = 0; j < vc; j++)
      {
	int vidx = *poly_indices++;
	*vertices++ = obj_verts[vidx];
	if (smoothed)
	{
	  CS_ASSERT (obj_normals != 0);
	  *normals++ = -obj_normals[vidx];
	}
	else
	  *normals++ = polynormal;
	csVector3 t = object2texture.Other2This (obj_verts[vidx]);
	*texels++ = csVector2 (t.x, t.y);
        *tangents++ = tangent;
        *binormals++ = binormal;

	pvIndices[j] = vindex++;
      }

      // Triangulate poly.
      for (j = 2; j < vc; j++)
      {
	*indices++ = pvIndices[0];
	iindex++;
	*indices++ = pvIndices[j - 1];
	iindex++;
	*indices++ = pvIndices[j];
	iindex++;
      }
#endif
    }
    //rm->indexend = iindex;
    rm->buffersource = polyRenderer->GetBufferSource (rm->indexstart,
      rm->indexend);

    rmeshes.Push (rm);
  }

/*  index_buffer->Release ();
  texel_buffer->Release ();
  normal_buffer->Release ();
  vertex_buffer->Release ();
  tangent_buffer->Release ();
  binormal_buffer->Release ();*/
}
#endif // CS_USE_NEW_RENDERER

//----------------------------------------------------------------------------

csThing::csThing (iBase *parent, csThingStatic* static_data) :
	polygons(32, 64),
	scfiPolygonMesh (0),
	scfiPolygonMeshCD (CS_POLY_COLLDET),
	scfiPolygonMeshLOD (CS_POLY_VISCULL)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiThingState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshObject);
  DG_TYPE (this, "csThing");

  csThing::static_data = static_data;
  polygons.SetThingType (static_data->thing_type);

  scfiPolygonMesh.SetThing (static_data);
  scfiPolygonMeshCD.SetThing (static_data);
  scfiPolygonMeshLOD.SetThing (static_data);

  last_thing_id++;
  thing_id = last_thing_id;
  logparent = 0;

  wor_verts = 0;

  dynamic_ambient.Set (0,0,0);
  light_version = 1;

  mixmode = CS_FX_COPY;

  ParentTemplate = 0;

  movablenr = -1;
  wor_bbox_movablenr = -1;
  cached_movable = 0;

  cfg_moving = CS_THING_MOVE_NEVER;

  prepared = false;
  static_data_nr = 0xfffffffd;	// (static_nr of csThingStatic is init to -1)

  current_lod = 1;
  current_features = 0;

#ifndef CS_USE_NEW_RENDERER
  polybuf = 0;
#endif // CS_USE_NEW_RENDERER
  current_visnr = 1;

  lightmapsPrepared = false;
  lightmapsDirty = true;
}

csThing::~csThing ()
{
#ifndef CS_USE_NEW_RENDERER
  if (polybuf) polybuf->DecRef ();
#endif // CS_USE_NEW_RENDERER

  ClearLMs ();

  if (wor_verts != static_data->obj_verts)
  {
    delete[] wor_verts;
  }

  polygons.FreeAll ();

#ifdef CS_USE_NEW_RENDERER
  ClearRenderMeshes ();
#endif
}

char* csThing::GenerateCacheName ()
{
  csBox3 b;
  static_data->GetBoundingBox (b);

  csMemFile mf;
  int32 l;
  l = convert_endian ((int32)static_data->num_vertices);
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)polygons.Length ());
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

  l = convert_endian ((int32)QInt ((b.MinX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((b.MinY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((b.MinZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((b.MaxX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((b.MaxY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((b.MaxZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());
  csString hex(digest.HexString());
  return hex.Detach();
}

void csThing::MarkLightmapsDirty ()
{
#ifndef CS_USE_NEW_RENDERER
  if (polybuf)
    polybuf->MarkLightmapsDirty ();
#endif // CS_USE_NEW_RENDERER
  lightmapsDirty = true;
  light_version++;
}

void csThing::DynamicLightChanged (iDynLight* /*dynlight*/)
{
  MarkLightmapsDirty ();
}

void csThing::DynamicLightDisconnect (iDynLight* dynlight)
{
  MarkLightmapsDirty ();
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    p->DynamicLightDisconnect (dynlight);
  }
}

void csThing::StaticLightChanged (iStatLight* /*statlight*/)
{
  MarkLightmapsDirty ();
}

void csThing::StaticLightDisconnect (iStatLight* statlight)
{
  MarkLightmapsDirty ();
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    p->StaticLightDisconnect (statlight);
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

      cached_movable = 0;
      break;
  }

  movablenr = -1;                 // @@@ Is this good?
}

void csThing::WorUpdate ()
{
  int i;
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
	  csReversibleTransform movtrans;	// Identity.
	  // @@@ It is possible to optimize the below too. Don't know
	  // if it is worth it though.
	  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
          for (i = 0; i < polygons.Length (); i++)
          {
            csPolygon3D *p = GetPolygon3D (i);
            p->ObjectToWorld (movtrans, p->Vwor (0));
          }
	}
	else
	{
          csReversibleTransform movtrans = cached_movable->GetFullTransform ();
          for (i = 0; i < static_data->num_vertices; i++)
            wor_verts[i] = movtrans.This2Other (static_data->obj_verts[i]);
          for (i = 0; i < polygons.Length (); i++)
          {
            csPolygon3D *p = GetPolygon3D (i);
            p->ObjectToWorld (movtrans, p->Vwor (0));
          }
	}
      }
      break;
  }
}

void csThing::HardTransform (const csReversibleTransform& t)
{
  csRef<csThingStatic> new_static_data = static_data->Clone ();
  static_data = new_static_data;
  static_data->HardTransform (t);
  scfiPolygonMesh.SetThing (static_data);
  scfiPolygonMeshCD.SetThing (static_data);
  scfiPolygonMeshLOD.SetThing (static_data);
}

void csThing::Unprepare ()
{
  prepared = false;
}

void csThing::PreparePolygons ()
{
  csPolygon3DStatic *ps;
  csPolygon3D *p;
  polygons.FreeAll ();

  int i;
  for (i = 0; i < static_data->static_polygons.Length (); i++)
  {
    p = static_data->thing_type->blk_polygon3d.Alloc ();
    ps = static_data->static_polygons.Get (i);
    p->SetStaticData (ps);
    p->SetParent (this);
    polygons.Push (p);
    p->SetMaterial (FindRealMaterial (ps->GetMaterialWrapper ()));
    p->Finish ();
  }
}

void csThing::Prepare ()
{
  static_data->Prepare ();

  if (prepared)
  {
    if (static_data_nr != static_data->scfiObjectModel.GetShapeNumber ())
    {
      static_data_nr = static_data->scfiObjectModel.GetShapeNumber ();

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

#ifndef CS_USE_NEW_RENDERER
      if (polybuf)
      {
	polybuf->DecRef ();
	polybuf = 0;
      }
#endif // CS_USE_NEW_RENDERER

      polybuf_materials.DeleteAll ();
      materials_to_visit.DeleteAll ();

      ClearLMs ();
      PreparePolygons ();

      MarkLightmapsDirty ();
      ClearLMs ();
      PrepareLMs ();
    }
    return;
  }

  prepared = true;

  static_data_nr = static_data->scfiObjectModel.GetShapeNumber ();

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

#ifndef CS_USE_NEW_RENDERER
  if (polybuf)
  {
    polybuf->DecRef ();
    polybuf = 0;
  }
#endif // CS_USE_NEW_RENDERER

  polybuf_materials.DeleteAll ();
  materials_to_visit.DeleteAll ();

  PreparePolygons ();

  // don't prepare lightmaps yet - the LMs may still be unlit,
  // as this function is called from within 'ReadFromCache()'.
}

iMaterialWrapper* csThing::FindRealMaterial (iMaterialWrapper* old_mat)
{
  int i;
  for (i = 0 ; i < replace_materials.Length () ; i++)
  {
    if (replace_materials[i].old_mat == old_mat)
      return replace_materials[i].new_mat;
  }
  return 0;
}

void csThing::ReplaceMaterial (iMaterialWrapper* oldmat,
	iMaterialWrapper* newmat)
{
  replace_materials.Push (RepMaterial (oldmat, newmat));
  prepared = false;
}

void csThing::ClearReplacedMaterials ()
{
  replace_materials.DeleteAll ();
  prepared = false;
}

csPolygon3D *csThing::GetPolygon3D (const char *name)
{
  int idx = polygons.FindKey ((void*)name, polygons.CompareKey);
  return idx >= 0 ? polygons.Get (idx) : 0;
}

int csThing::FindPolygonIndex (iPolygon3D *polygon) const
{
  csPolygon3D *p = polygon->GetPrivateObject ();
  return polygons.Find (p);
}

void csThing::InvalidateThing ()
{
#ifndef CS_USE_NEW_RENDERER
  if (polybuf)
  {
    polybuf->DecRef ();
    polybuf = 0;
  }
#endif // CS_USE_NEW_RENDERER

  polybuf_materials.DeleteAll ();
  materials_to_visit.DeleteAll ();
  prepared = false;
  static_data->obj_bbox_valid = false;

  delete [] static_data->obj_normals; static_data->obj_normals = 0;
  static_data->scfiObjectModel.ShapeChanged ();
}

iPolygonMesh* csThing::GetWriteObject ()
{
  return &scfiPolygonMeshLOD;
}

bool csThing::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  int i;

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

static int ComparePointer (void const* item1,
	void const* item2)
{
  if (item1 < item2) return -1;
  if (item1 > item2) return 1;
  return 0;
}

void csThing::PreparePolygonBuffer ()
{
#ifndef CS_USE_NEW_RENDERER
  if (polybuf) return ;

  struct csPolyLMCoords
  {
    float u1, v1, u2, v2;
  };

  iVertexBufferManager *vbufmgr = static_data->thing_type->G3D->
    GetVertexBufferManager ();
  polybuf = vbufmgr->CreatePolygonBuffer ();

  csDirtyAccessArray<int> verts;

  polybuf->SetVertexArray (static_data->obj_verts, static_data->num_vertices);

  polybuf_materials.DeleteAll ();
  materials_to_visit.DeleteAll ();
  polybuf_materials.SetCapacity (
  	litPolys.Length () + unlitPolys.Length ());

  int i;
  for (i = 0; i < litPolys.Length (); i++)
  {
    int mi = polybuf->GetMaterialCount ();
    polybuf_materials.Push (litPolys[i]->material);
    if (litPolys[i]->material->IsVisitRequired ())
      materials_to_visit.Push (litPolys[i]->material);
    polybuf->AddMaterial (litPolys[i]->material->GetMaterialHandle ());
    int j;
    for (j = 0; j < litPolys[i]->polys.Length (); j++)
    {
      verts.DeleteAll ();

      csPolygon3D *poly = litPolys[i]->polys[j];
      csPolygon3DStatic *spoly = poly->GetStaticData ();
      csPolyLightMapMapping *mapping = spoly->GetLightMapMapping ();
      csPolyTextureMapping *tmapping = spoly->GetTextureMapping ();

      int v;
      for (v = 0; v < spoly->GetVertexCount (); v++)
      {
	const int vnum = spoly->GetVertices ().GetVertex (v);
	verts.Push (vnum);
      }

      polybuf->AddPolygon (spoly->GetVertexCount (), verts.GetArray (), 
	tmapping, mapping, 
	spoly->GetObjectPlane (), 
	mi, litPolys[i]->lightmaps[j]);
    }
  }

  for (i = 0; i < unlitPolys.Length (); i++)
  {
    int mi = polybuf->GetMaterialCount ();
    polybuf_materials.Push (unlitPolys[i]->material);
    if (unlitPolys[i]->material->IsVisitRequired ())
      materials_to_visit.Push (unlitPolys[i]->material);
    polybuf->AddMaterial (unlitPolys[i]->material->GetMaterialHandle ());
    int j;
    for (j = 0; j < unlitPolys[i]->polys.Length (); j++)
    {
      verts.DeleteAll ();

      csPolygon3D *poly = unlitPolys[i]->polys[j];
      csPolygon3DStatic *spoly = poly->GetStaticData ();
      csPolyTextureMapping *tmapping = spoly->GetTextureMapping ();

      int v;
      for (v = 0; v < spoly->GetVertexCount (); v++)
      {
	const int vnum = spoly->GetVertices ().GetVertex (v);
	verts.Push (vnum);
      }

      polybuf->AddPolygon (spoly->GetVertexCount (), verts.GetArray (), 
	tmapping, 0, spoly->GetObjectPlane (), 
	mi, 0);
    }
  }

  // Optimize the array of materials to visit.
  materials_to_visit.Sort (ComparePointer);	// Sort on pointer.
  i = 0;
  int ni = 0;
  iMaterialWrapper* prev = 0;
  for (i = 0 ; i < materials_to_visit.Length () ; i++)
  {
    if (materials_to_visit[i] != prev)
    {
      prev = materials_to_visit[i];
      materials_to_visit[ni++] = prev;
    }
  }
  materials_to_visit.Truncate (ni);

  polybuf->Prepare ();
#endif // CS_USE_NEW_RENDERER
}

void csThing::DrawPolygonArrayDPM (
  iRenderView *rview,
  iMovable *movable,
  csZBufMode zMode)
{
  PreparePolygonBuffer ();

  iCamera *icam = rview->GetCamera ();
  csReversibleTransform tr_o2c = icam->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  G3DPolygonMesh mesh;
  csVector3 radius;
  csSphere sphere;
  static_data->GetRadius (radius, sphere.GetCenter ());
  sphere.SetRadius (static_data->max_obj_radius);
  if (rview->ClipBSphere (tr_o2c, sphere, mesh.clip_portal, mesh.clip_plane,
  	mesh.clip_z_plane) == false)
    return;	// Not visible.

  rview->GetGraphics3D ()->SetObjectToCamera (&tr_o2c);
  rview->GetGraphics3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zMode);

  mesh.polybuf = polybuf;
  int i;
  for (i = 0 ; i < materials_to_visit.Length () ; i++)
  {
    materials_to_visit[i]->Visit ();
  }

  mesh.do_fog = false;
  mesh.do_mirror = icam->IsMirrored ();
  mesh.vertex_mode = G3DPolygonMesh::VM_WORLDSPACE;
  mesh.vertex_fog = 0;
  mesh.mixmode = mixmode;

  rview->CalculateFogMesh(tr_o2c,mesh);
  rview->GetGraphics3D ()->DrawPolygonMesh (mesh);
}

void csThing::InvalidateMaterialHandles ()
{
#ifndef CS_USE_NEW_RENDERER
  int i;
  for (i = 0; i < polybuf_materials.Length (); i++)
  {
    polybuf->SetMaterial (i, polybuf_materials[i]->GetMaterialHandle ());
  }
#endif
}

// @@@ We need a better algorithm here. We should try
// to recognize convex sub-parts of a polygonset and return
// convex shadow frustums for those. This will significantly
// reduce the number of shadow frustums. There are basically
// two ways to do this:
//	- Split object into convex sub-parts in 3D.
//	- Split object into convex sub-parts in 2D.
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
      polygons.Length ());
  csFrustum *frust;
  int i, j;
  csPolygon3DStatic *sp;
  csPolygon3D *p;
  bool cw = true;                   //@@@ Use mirroring parameter here!
  for (i = 0; i < static_data->static_polygons.Length (); i++)
  {
    sp = static_data->static_polygons.Get (i);
    p = polygons.Get (i);

    //if (p->GetPlane ()->VisibleFromPoint (origin) != cw) continue;
    float clas = p->GetWorldPlane ().Classify (origin);
    if (ABS (clas) < EPSILON) continue;
    if ((clas <= 0) != cw) continue;

    csPlane3 pl = p->GetWorldPlane ();
    pl.DD += origin * pl.norm;
    pl.Invert ();
    frust = list->AddShadow (
        origin,
        (void *)p,
        sp->GetVertices ().GetVertexCount (),
        pl);
    for (j = 0; j < sp->GetVertices ().GetVertexCount (); j++)
      frust->GetVertex (j).Set (p->Vwor (j) - origin);
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
  virtual ~PolyMeshTimerEvent () { }
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
  thing->Prepare ();
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

  int i;
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

//-------------------------------------------------------------------------

bool csThing::DrawTest (iRenderView *rview, iMovable *movable)
{
  Prepare ();

  iCamera *icam = rview->GetCamera ();
  const csReversibleTransform &camtrans = icam->GetTransform ();

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

#ifdef CS_USE_NEW_RENDERER
  //@@@
  int i;
  tr_o2c = camtrans;
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();
  int clip_portal, clip_plane, clip_z_plane;
  if (rview->ClipBSphere (tr_o2c, sphere, clip_portal, clip_plane,
  	clip_z_plane) == false)
    return false;

  if (renderMeshes.Length() == 0)
  {
    PrepareRenderMeshes ();
  }

  for (i = 0; i < renderMeshes.Length(); i++)
  {
    csRenderMesh* rm = renderMeshes[i];
    rm->object2camera = tr_o2c;
    rm->clip_portal = clip_portal;
    rm->clip_plane = clip_plane;
    rm->clip_z_plane = clip_z_plane;
    rm->do_mirror = icam->IsMirrored ();  
  }

  UpdateDirtyLMs (); // @@@ Here?

  return true;
#else
  if (can_move)
  {
    csReversibleTransform tr_o2c = camtrans;
    if (!movable->IsFullTransformIdentity ())
      tr_o2c /= movable->GetFullTransform ();
    bool rc = rview->TestBSphere (tr_o2c, sphere);
    return rc;
  }
  else
  {
    bool rc = rview->TestBSphere (camtrans, sphere);
    return rc;
  }
#endif
}

bool csThing::Draw (iRenderView *rview, iMovable *movable, csZBufMode zMode)
{
  PrepareLMs ();
  UpdateDirtyLMs ();

  DrawPolygonArrayDPM (rview, movable, zMode);

  return true;                                  // @@@@ RETURN correct vis info
}

//----------------------------------------------------------------------

void csThing::CastShadows (iFrustumView *lview, iMovable *movable)
{
  Prepare ();
  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

  int i;

  iFrustumViewUserdata* fvud = lview->GetUserdata ();
  iLightingProcessInfo* lpi = (iLightingProcessInfo*)fvud;
  bool dyn = lpi->IsDynamic ();

  csRef<csLightingPolyTexQueue> lptq;
  if (!dyn)
  {
    csRef<iLightingProcessData> lpd = lpi->QueryUserdata (
    	csLightingPolyTexQueue_scfGetID (),
	csLightingPolyTexQueue_VERSION);
    lptq = (csLightingPolyTexQueue*)(iLightingProcessData*)lpd;
    if (!lptq)
    {
      lptq = csPtr<csLightingPolyTexQueue> (new csLightingPolyTexQueue (
    	  lpi->GetLight ()));
      lpi->AttachUserdata (lptq);
    }
    csRef<iStatLight> sl = SCF_QUERY_INTERFACE (lpi->GetLight (),
    	iStatLight);
    sl->AddAffectedLightingInfo (&scfiLightingInfo);
  }
  else
  {
    csRef<iDynLight> dl = SCF_QUERY_INTERFACE (lpi->GetLight (),
    	iDynLight);
    dl->AddAffectedLightingInfo (&scfiLightingInfo);
  }

  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D* poly = GetPolygon3D (i);
    if (dyn)
      poly->CalculateLightingDynamic (lview, movable);
    else
      poly->CalculateLightingStatic (lview, movable, lptq, true);
  }
}

void csThing::InitializeDefault (bool clear)
{
  Prepare ();

  int i;
  for (i = 0; i < polygons.Length (); i++)
    polygons.Get (i)->InitializeDefault (clear);
}

bool csThing::ReadFromCache (iCacheManager* cache_mgr)
{
  Prepare ();
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  // For error reporting.
  const char* thing_name = 0;
  if (static_data->thing_type->do_verbose && logparent)
  {
    csRef<iMeshWrapper> mw (SCF_QUERY_INTERFACE (logparent, iMeshWrapper));
    if (mw) thing_name = mw->QueryObject ()->GetName ();
  }

  bool rc = true;
  csRef<iDataBuffer> db = cache_mgr->ReadCache ("thing_lm", 0, (uint32) ~0);
  if (db)
  {
    csMemFile mf ((const char*)(db->GetData ()), db->GetSize ());
    int i;
    for (i = 0; i < polygons.Length (); i++)
    {
      const char* error = polygons.Get (i)->ReadFromCache (&mf);
      if (error != 0)
      {
        rc = false;
        if (static_data->thing_type->do_verbose)
	{
	  printf ("  Thing '%s' Poly '%s': %s\n",
	  	thing_name, static_data->static_polygons.Get (i)->GetName (),
		error);
	  fflush (stdout);
        }
      }
    }
  }
  else
  {
    if (static_data->thing_type->do_verbose)
    {
      printf ("  Thing '%s': Could not find cached lightmap file for thing!\n",
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

  int i;
  bool rc = false;
  csMemFile mf;
  for (i = 0; i < polygons.Length (); i++)
    if (!polygons.Get (i)->WriteToCache (&mf)) goto stop;
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
  int i;
  for (i = 0; i < polygons.Length (); i++)
    polygons.Get (i)->PrepareLighting ();

  PrepareLMs ();
}

void csThing::Merge (csThing *other)
{
  int i, j;
  int *merge_vertices = new int[other->static_data->GetVertexCount () + 1];
  for (i = 0; i < other->static_data->GetVertexCount (); i++)
    merge_vertices[i] = static_data->AddVertex (other->Vwor (i));

  for (i = 0; i < other->polygons.Length (); i++)
  {
    //csPolygon3D *p = other->GetPolygon3D (i);
    csPolygon3DStatic *sp = other->static_data->GetPolygon3DStatic (i);
    int *idx = sp->GetVertices ().GetVertexIndices ();
    for (j = 0; j < sp->GetVertices ().GetVertexCount (); j++)
      idx[j] = merge_vertices[idx[j]];
    static_data->AddPolygon (sp);
    other->polygons[i] = 0;
  }

  delete[] merge_vertices;
}

#ifdef CS_USE_NEW_RENDERER

void csThing::PrepareRenderMeshes ()
{
  static_data->FillRenderMeshes (renderMeshes, replace_materials, mixmode);
  int i;
  materials_to_visit.DeleteAll ();
  for (i = 0 ; i < renderMeshes.Length () ; i++)
  {
    if (renderMeshes[i]->material->IsVisitRequired ())
      materials_to_visit.Push (renderMeshes[i]->material);
  }
}

void csThing::ClearRenderMeshes ()
{
  for (int i = 0; i < renderMeshes.Length(); i++)
  {
    delete renderMeshes[i];
  }
  renderMeshes.DeleteAll ();
}

#endif

csRenderMesh **csThing::GetRenderMeshes (int &num)
{
#ifdef CS_USE_NEW_RENDERER
  // @@@ "dirtiness" check
  if (renderMeshes.Length() == 0)
  {
    PrepareRenderMeshes ();
  }

  //PrepareLMs (); // @@@ Maybe more here ?

  num = renderMeshes.Length ();
  for (int i = 0; i < materials_to_visit.Length (); i++)
  {
    materials_to_visit[i]->Visit ();
  }
  return renderMeshes.GetArray ();
#else
  return 0;
#endif
}

struct PolyGroupSLM
{
  csRef<iSuperLightmap> SLM;
  bool slmCreated;

  // ctor has a parameter so csHash<> can construct this struct from 0
  PolyGroupSLM (int x = 0) { (void)x; slmCreated = false; }
};

void csThing::PrepareLMs ()
{
  if (lightmapsPrepared) return;

  csThingObjectType* thing_type = static_data->thing_type;
  iTextureManager* txtmgr = thing_type->G3D->GetTextureManager ();

  csHash<PolyGroupSLM, csThingStatic::StaticSuperLM*> superLMs;

  int i;
  for (i = 0; i < static_data->litPolys.Length(); i++)
  {
    const csThingStatic::csStaticLitPolyGroup& slpg = 
      *(static_data->litPolys[i]);

    csRef<iSuperLightmap> SLM;

    PolyGroupSLM pgSLM = superLMs.Get (slpg.staticSLM);

    if (!pgSLM.slmCreated)
    {
      pgSLM.SLM = txtmgr->CreateSuperLightmap (slpg.staticSLM->width, 
        slpg.staticSLM->height);
      pgSLM.slmCreated = true;
      superLMs.Put (slpg.staticSLM, pgSLM);
    }

    // SLM creation failed for some reason. The polys will be drawn unlit.
    if ((SLM = pgSLM.SLM) == 0)
    {
      csPolyGroup* pg = new csPolyGroup;
      pg->material = FindRealMaterial (slpg.material);
      if (pg->material == 0) pg->material = slpg.material;

      int j;
      for (j = 0; j < slpg.polys.Length(); j++)
      {
	pg->polys.Push (polygons[slpg.polys[j]]);
      }

      unlitPolys.Push (pg);
    }
    else
    {
      csLitPolyGroup* lpg = new csLitPolyGroup;
      lpg->material = FindRealMaterial (slpg.material);
      if (lpg->material == 0) lpg->material = slpg.material;
      lpg->SLM = SLM;

      int j;
      for (j = 0; j < slpg.polys.Length(); j++)
      {
	csPolygon3D* poly = polygons[slpg.polys[j]];

	lpg->polys.Push (poly);
	const csRect& r = slpg.lmRects[j];
	csRef<iRendererLightmap> rlm = 
	  SLM->RegisterLightmap (r.xmin, r.ymin, r.Width (), r.Height ());
        
	csPolyTexture* polytxt = poly->GetPolyTexture ();
	rlm->SetLightCellSize (polytxt->GetLightCellSize ());
	polytxt->SetRendererLightmap (rlm);

	lpg->lightmaps.Push (rlm);
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

    int j;
    for (j = 0; j < spg.polys.Length(); j++)
    {
      pg->polys.Push (polygons[spg.polys[j]]);
    }

    unlitPolys.Push (pg);
  }

  lightmapsPrepared = true;
  lightmapsDirty = true;
}

void csThing::ClearLMs ()
{
  if (!lightmapsPrepared) return;

  litPolys.DeleteAll ();
  unlitPolys.DeleteAll ();

  lightmapsPrepared = false;
  lightmapsDirty = true;
}

void csThing::UpdateDirtyLMs ()
{
  if (!lightmapsDirty) return;

  int i;
  for (i = 0; i < litPolys.Length (); i++)
  {
    int j;
    for (j = 0; j < litPolys[i]->polys.Length (); j++)
    {
      csPolygon3D *poly = litPolys[i]->polys[j];
      csPolyTexture* lmi = poly->GetPolyTexture ();
      if (lmi->DynamicLightsDirty () && lmi->RecalculateDynamicLights ())	
      {
	litPolys[i]->lightmaps[j]->SetData (
	  lmi->GetLightMapFast ()->GetMapDataFast ());
      }
    }
  }

  lightmapsDirty = false;
}

//---------------------------------------------------------------------------

iPolygon3D *csThing::ThingState::GetPolygon (int idx)
{
  csPolygon3D *p = scfParent->GetPolygon3D (idx);
  if (!p) return 0;
  return &(p->scfiPolygon3D);
}

iPolygon3D *csThing::ThingState::GetPolygon (const char *name)
{
  csPolygon3D *p = scfParent->GetPolygon3D (name);
  if (!p) return 0;
  return &(p->scfiPolygon3D);
}

//---------------------------------------------------------------------------
iMeshObjectFactory *csThing::MeshObject::GetFactory () const
{
  if (!scfParent->ParentTemplate) return 0;
  return (iMeshObjectFactory*)(scfParent->ParentTemplate->GetStaticData ());
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csThingObjectType)
  SCF_IMPLEMENTS_INTERFACE(iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iThingEnvironment)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iConfig)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiThingEnvironment)
  SCF_IMPLEMENTS_INTERFACE(iThingEnvironment)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiConfig)
  SCF_IMPLEMENTS_INTERFACE(iConfig)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiDebugHelper)
  SCF_IMPLEMENTS_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csThingObjectType)


csThingObjectType::csThingObjectType (iBase *pParent) :
	blk_polygon3d (2000),
	blk_polygon3dstatic (2000),
	blk_lightmapmapping (2000),
	blk_texturemapping (2000),
	blk_polytex (2000),
	blk_lightmap (2000)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiThingEnvironment);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  lightpatch_pool = 0;
  do_verbose = false;
}

csThingObjectType::~csThingObjectType ()
{
  delete lightpatch_pool;
}

bool csThingObjectType::Initialize (iObjectRegistry *object_reg)
{
  csThingObjectType::object_reg = object_reg;			
  csRef<iEngine> e = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = e;	// We don't want a real ref here to avoid circular refs.
  csRef<iGraphics3D> g = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  G3D = g;

  lightpatch_pool = new csLightPatchPool ();

  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);
  if (cmdline)
  {
    do_verbose = cmdline->GetOption ("verbose") != 0;
  }

  csRef<iTextureManager> txtmgr = g->GetTextureManager ();

  int maxTW, maxTH, maxTA;
  txtmgr->GetMaxTextureSize (maxTW, maxTH, maxTA);

  csConfigAccess cfg (object_reg, "/config/thing.cfg");

  int maxLightmapSize = cfg->GetInt ("Mesh.Thing.MaxSuperlightmapSize", 256);
  maxLightmapW = 
    cfg->GetInt ("Mesh.Thing.MaxSuperlightmapWidth", maxLightmapSize);
  maxLightmapW = MIN (maxLightmapW, maxTW);
  maxLightmapH = 
    cfg->GetInt ("Mesh.Thing.MaxSuperlightmapHeight", maxLightmapSize);
  maxLightmapH = MIN (maxLightmapH, maxTH);

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
  { 0, "cosfact", "Cosinus factor for lighting", CSVAR_FLOAT }
};
const int NUM_OPTIONS =
  (
    sizeof (config_options) /
    sizeof (config_options[0])
  );

bool csThingObjectType::eiConfig::SetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:
      csPolyTexture::cfg_cosinus_factor = value->GetFloat ();
      break;
    default:
      return false;
  }

  return true;
}

bool csThingObjectType::eiConfig::GetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:   value->SetFloat (csPolyTexture::cfg_cosinus_factor); break;
    default:  return false;
  }

  return true;
}

bool csThingObjectType::eiConfig::GetOptionDescription (
  int idx,
  csOptionDescription *option)
{
  if (idx < 0 || idx >= NUM_OPTIONS) return false;
  *option = config_options[idx];
  return true;
}

//---------------------------------------------------------------------------
