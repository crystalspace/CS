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
#include "csutil/csendian.h"
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
#include "csgfx/shadervarcontext.h"
#include "csgeom/subrec.h"
#include "csgeom/subrec2.h"

#ifdef CS_DEBUG
  //#define LIGHTMAP_DEBUG
#endif

CS_IMPLEMENT_PLUGIN

int csThing::lightmap_quality = 3;
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
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingStatic::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE(iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

#ifdef CS_USE_NEW_RENDERER
csStringID csThingStatic::texLightmapName = csInvalidStringID;
#endif

csThingStatic::csThingStatic (iBase* parent, csThingObjectType* thing_type) :
	last_range (0, -1),
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

  if ((texLightmapName == csInvalidStringID))
  {
    csRef<iStringSet> strings = 
      CS_QUERY_REGISTRY_TAG_INTERFACE (thing_type->object_reg,
        "crystalspace.shared.stringset", iStringSet);

    texLightmapName = strings->Request ("tex lightmap");
  }
#endif
}

csThingStatic::~csThingStatic ()
{
  delete[] obj_verts;
  delete[] obj_normals;

  UnprepareLMLayout ();

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_IBASE ();
}

void csThingStatic::Prepare (iBase* thing_logparent)
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
      if (!sp->Finish (thing_logparent))
	prepared = false;
    }
    static_polygons.ShrinkBestFit ();
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
    if ((lmi != 0) && (sp->flags.Check (CS_POLY_LIGHTING)))
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

  for (i = 0 ; i < litPolys.Length () ; i++)
  {
    StaticSuperLM* slm = litPolys[i]->staticSLM;
    delete slm->rects;
    slm->rects = 0;
  }

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

    csPolyTextureMapping* lm = sp->GetTextureMapping ();
    if ((lm == 0) || (!sp->flags.Check (CS_POLY_LIGHTING)))
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
      int nidx = superLMs.InsertSorted (slm, CompareStaticSuperLM);
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

	if ((((neww * newh) - usedLumels) >= inputQueues[curQueue].totalLumels) &&
	  (((float)(usedLumels + inputQueues[curQueue].totalLumels) / 
	  (float)(neww * newh)) > (1.0f - thing_type->maxSLMSpaceWaste)))
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

  for (i = 0; i < litPolys.Length(); i++)
  {
    StaticSuperLM* slm = litPolys[i]->staticSLM;
    for (int j = 0; j < litPolys[i]->polys.Length(); j++)
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
    int *idx = p->GetVertexIndices ();
    for (j = 0; j < p->GetVertexCount (); j++) idx[j] = vt[idx[j]].new_idx;
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
  for (i = 0; i < static_polygons.Length (); i++)
  {
    csPolygon3DStatic *p = static_polygons.Get (i);
    int *idx = p->GetVertexIndices ();
    for (j = 0; j < p->GetVertexCount (); j++) idx[j] = relocate[idx[j]];
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

int csThingStatic::AddPolygon (csPolygon3DStatic* spoly)
{
  spoly->SetParent (this);
  spoly->EnableTextureMapping (true);
  int idx = static_polygons.Push (spoly);
  scfiObjectModel.ShapeChanged ();
  UnprepareLMLayout ();
  return idx;
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

csPtr<csThingStatic> csThingStatic::CloneStatic ()
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
    csPolygon3DStatic* p = static_polygons.Get (i)->Clone (clone);
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
void csThingStatic::FillRenderMeshes (
	csDirtyAccessArray<csRenderMesh*>& rmeshes,
	const csArray<RepMaterial>& repMaterials,
	uint mixmode)
{
//@@@@ ADD support for repMaterials!!!
  //polyRenderers.DeleteAll ();

  for (int i = 0; i < (litPolys.Length () + unlitPolys.Length ()); i++)
  {
    const csStaticPolyGroup& pg = 
      (i < litPolys.Length ()) ? *(litPolys[i]) : 
        *(unlitPolys[i - litPolys.Length ()]) ;
    csRenderMesh* rm = new csRenderMesh;

    csRef<iPolygonRenderer> polyRenderer;
    if (polyRenderers.Length () <= i)
    {
      polyRenderer = r3d->CreatePolygonRenderer ();
      polyRenderers.Push (polyRenderer);

      int j;
      for (j = 0; j< pg.polys.Length(); j++)
      {
	polyRenderer->AddPolygon (&static_polygons[pg.polys[j]]->polygon_data);
      }
    }
    else
      polyRenderer = polyRenderers[i];

    rm->mixmode = mixmode; 
    rm->material = pg.material;
    rm->meshtype = CS_MESHTYPE_POLYGON;
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
#endif // CS_USE_NEW_RENDERER

int csThingStatic::FindPolygonByName (const char* name)
{
  return static_polygons.FindKey (static_polygons.KeyCmp(name));
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
  if (end >= static_polygons.Length ())
    end = static_polygons.Length ()-1;
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

void csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
  	const csMatrix3& m, const csVector3& v)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
    static_polygons[i]->SetTextureSpace (m, v);
}

void csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector2& uv1, const csVector2& uv2, const csVector2& uv3)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->SetTextureSpace (
    	sp->Vobj (0), uv1,
    	sp->Vobj (1), uv2,
    	sp->Vobj (2), uv3);
  }
}

void csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector3& p1, const csVector2& uv1,
  	const csVector3& p2, const csVector2& uv2,
  	const csVector3& p3, const csVector2& uv3)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->SetTextureSpace (p1, uv1, p2, uv2, p3, uv3);
  }
}

void csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector3& v_orig, const csVector3& v1, float len1)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->SetTextureSpace (v_orig, v1, len1);
  }
}

void csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range, float len1)
{
  int i, start, end;
  GetRealRange (range, start, end);
  for (i = start ; i <= end ; i++)
  {
    csPolygon3DStatic* sp = static_polygons[i];
    sp->SetTextureSpace (sp->Vobj (0), sp->Vobj (1), len1);
  }
}

void csThingStatic::SetPolygonTextureMapping (const csPolygonRange& range,
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
}

void csThingStatic::GetPolygonTextureMapping (int polygon_idx,
  	csMatrix3& m, csVector3& v)
{
  static_polygons[GetRealIndex (polygon_idx)]->GetTextureSpace (m, v);
}

void csThingStatic::SetPolygonTextureMappingEnabled (const csPolygonRange& range,
  	bool enabled)
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
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshObject);
  DG_TYPE (this, "csThing");

  csThing::static_data = static_data;
  polygons.SetThingType (static_data->thing_type);
  polygon_world_planes = 0;
  polygon_world_planes_num = -1;	// -1 means not checked yet, 0 means no planes.

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

#ifdef __USE_MATERIALS_REPLACEMENT__
  replaceMaterialChanged = false;
#endif

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

  polygons.DeleteAll ();
  delete[] polygon_world_planes;

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiThingState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMeshObject);
  SCF_DESTRUCT_IBASE ();
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

void csThing::LightChanged (iLight*)
{
  MarkLightmapsDirty ();
}

void csThing::LightDisconnect (iLight* light)
{
  MarkLightmapsDirty ();
  int dt = light->GetDynamicType ();
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
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
          for (i = 0; i < static_data->num_vertices; i++)
            wor_verts[i] = movtrans.This2Other (static_data->obj_verts[i]);
	  if (!polygon_world_planes || polygon_world_planes_num < polygons.Length ())
	  {
	    delete[] polygon_world_planes;
	    polygon_world_planes_num = polygons.Length ();
	    polygon_world_planes = new csPlane3[polygon_world_planes_num];
	  }
          for (i = 0; i < polygons.Length (); i++)
          {
	    csPolygon3DStatic* sp = static_data->GetPolygon3DStatic (i);
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
  prepared = false;
}

void csThing::PreparePolygons ()
{
  csPolygon3DStatic *ps;
  csPolygon3D *p;
  polygons.DeleteAll ();
  delete[] polygon_world_planes;
  polygon_world_planes = 0;
  polygon_world_planes_num = -1;	// Not checked!

  int i;
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

#ifdef __USE_MATERIALS_REPLACEMENT__

void csThing::ReplaceMaterial (iMaterialWrapper* oldmat,
	iMaterialWrapper* newmat)
{
  //
  //Remove the binding of oldmat, if it exists.
  int i;
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
  
  replaceMaterialChanged = true;
}

#else

void csThing::ReplaceMaterial (iMaterialWrapper* oldmat,
	iMaterialWrapper* newmat)
{
  replace_materials.Push (RepMaterial (oldmat, newmat));
  prepared = false;
}

#endif // __USE_MATERIALS_REPLACEMENT__


void csThing::ClearReplacedMaterials ()
{
  replace_materials.DeleteAll ();
  prepared = false;
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
	&scfiMeshObject,
	polygon_idx));
}

const csPlane3& csThing::GetPolygonWorldPlane (int polygon_idx)
{
  CS_ASSERT (polygon_idx >= 0);
  if (polygon_world_planes_num == -1)
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

static int ComparePointer (iMaterialWrapper* const& item1,
			   iMaterialWrapper* const& item2)
{
  if (item1 < item2) return -1;
  if (item1 > item2) return 1;
  return 0;
}

#ifndef __USE_MATERIALS_REPLACEMENT__

void csThing::PreparePolygonBuffer ()
{
#ifndef CS_USE_NEW_RENDERER
  if (polybuf) return;

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
      csPolygon3DStatic *spoly = poly->GetStaticPoly ();
      csPolyTextureMapping *tmapping = spoly->GetTextureMapping ();

      int v;
      for (v = 0; v < spoly->GetVertexCount (); v++)
      {
	const int vnum = spoly->GetVertexIndices ()[v];
	verts.Push (vnum);
      }

      polybuf->AddPolygon (spoly->GetVertexCount (), verts.GetArray (), 
	tmapping, spoly->GetObjectPlane (), 
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
      csPolygon3DStatic *spoly = poly->GetStaticPoly ();
      csPolyTextureMapping *tmapping = spoly->GetTextureMapping ();

      int v;
      for (v = 0; v < spoly->GetVertexCount (); v++)
      {
	const int vnum = spoly->GetVertexIndices ()[v];
	verts.Push (vnum);
      }

      polybuf->AddPolygon (spoly->GetVertexCount (), verts.GetArray (), 
	tmapping, spoly->GetObjectPlane (), 
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

#else // if __USE_MATERIALS_REPLACEMENT__

void csThing::PreparePolygonBuffer ()
{
#ifndef CS_USE_NEW_RENDERER
  //
  //If 'polybuf' is null, create it.
  if (!polybuf)
  {    
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
      csRef<iMaterialWrapper> l_mat = FindRealMaterial (litPolys[i]->material);
      if (l_mat == 0)
        l_mat = litPolys[i]->material;
      polybuf_materials.Push (l_mat);
      if (l_mat->IsVisitRequired ())
        materials_to_visit.Push (l_mat);
      polybuf->AddMaterial (l_mat->GetMaterialHandle ());
      
      int j;
      for (j = 0; j < litPolys[i]->polys.Length (); j++)
      {
        verts.DeleteAll ();
        
        csPolygon3D *poly = litPolys[i]->polys[j];
        csPolygon3DStatic *spoly = poly->GetStaticPoly ();
        csPolyTextureMapping *tmapping = spoly->GetTextureMapping ();
        
        int v;
        for (v = 0; v < spoly->GetVertexCount (); v++)
        {
          const int vnum = spoly->GetVertexIndices ()[v];
          verts.Push (vnum);
        }
        
        polybuf->AddPolygon (spoly->GetVertexCount (), verts.GetArray (), 
          tmapping, spoly->GetObjectPlane (), 
          mi, litPolys[i]->lightmaps[j]);
      }
    }
    
    for (i = 0; i < unlitPolys.Length (); i++)
    {
      int mi = polybuf->GetMaterialCount ();
      csRef<iMaterialWrapper> l_mat = FindRealMaterial (unlitPolys[i]->material);
      if (l_mat == 0)
        l_mat = unlitPolys[i]->material;
      polybuf_materials.Push (l_mat);
      if (l_mat->IsVisitRequired ())
        materials_to_visit.Push (l_mat);
      polybuf->AddMaterial (l_mat->GetMaterialHandle ());
      int j;
      for (j = 0; j < unlitPolys[i]->polys.Length (); j++)
      {
        verts.DeleteAll ();
        
        csPolygon3D *poly = unlitPolys[i]->polys[j];
        csPolygon3DStatic *spoly = poly->GetStaticPoly ();
        csPolyTextureMapping *tmapping = spoly->GetTextureMapping ();
        
        int v;
        for (v = 0; v < spoly->GetVertexCount (); v++)
        {
          const int vnum = spoly->GetVertexIndices ()[v];
          verts.Push (vnum);
        }
        
        polybuf->AddPolygon (spoly->GetVertexCount (), verts.GetArray (), 
          tmapping, spoly->GetObjectPlane (), 
          mi, 0);
      }//for
    }//for

    //
    //
    replaceMaterialChanged = false;
  }//if
  //
  //If the 'polybuf' has not been recreated, then we check
  //if the replace_materials array has been modified. In this case
  //the updating of the materials in the 'polybuf' structure it's needed.
  else if (replaceMaterialChanged)
  {
    int i;
    for (i = 0; i < static_data->litPolys.Length (); i++)
    {
      csRef<iMaterialWrapper> l_mW1 = static_data->litPolys.Get (i)->material;
      csRef<iMaterialWrapper> l_mW2 = FindRealMaterial (l_mW1);
      if (l_mW2 != 0)
      {
        polybuf->SetMaterial (i, l_mW2->GetMaterialHandle ());
      }//if
      else
      {
        polybuf->SetMaterial (i, l_mW1->GetMaterialHandle ());
      }//else
    }//for

    for (i = 0; i < static_data->unlitPolys.Length (); i++)
    {
      csRef<iMaterialWrapper> l_mW1 = static_data->unlitPolys.Get (i)->material;
      csRef<iMaterialWrapper> l_mW2 = FindRealMaterial (l_mW1);
      if (l_mW2 != 0)
      {
        polybuf->SetMaterial (i, l_mW2->GetMaterialHandle ());
      }//if
      else
      {
        polybuf->SetMaterial (i, l_mW1->GetMaterialHandle ());
      }//else
    }//for

    replaceMaterialChanged = false;
  }//else if

  //
  // Optimize the array of materials to visit.
  materials_to_visit.Sort (ComparePointer);	// Sort on pointer.
  int i = 0;
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

#endif // if __USE_MATERIALS_REPLACEMENT__

void csThing::DrawPolygonArrayDPM (
  iRenderView *rview,
  iMovable *movable,
  csZBufMode zMode)
{
#ifndef CS_USE_NEW_RENDERER
  PreparePolygonBuffer ();

  iCamera *icam = rview->GetCamera ();
  csReversibleTransform tr_o2c = icam->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  G3DPolygonMesh mesh;
  mesh.clip_portal = clip_portal;
  mesh.clip_plane = clip_plane;
  mesh.clip_z_plane = clip_z_plane;
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

  rview->CalculateFogMesh (tr_o2c,mesh);
  rview->GetGraphics3D ()->DrawPolygonMesh (mesh);
#endif
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
    p = &polygons.Get (i);

    //if (p->GetPlane ()->VisibleFromPoint (origin) != cw) continue;
    const csPlane3& world_plane = GetPolygonWorldPlaneNoCheck (i);
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

bool csThing::DrawTest (iRenderView *rview, iMovable *movable,
	uint32 frustum_mask)
{
  Prepare ();

  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

#ifndef CS_USE_NEW_RENDERER
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
  	clip_z_plane);
#endif

  return true;
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

  int i;

  iFrustumViewUserdata* fvud = lview->GetUserdata ();
  iLightingProcessInfo* lpi = (iLightingProcessInfo*)fvud;
  bool dyn = lpi->IsDynamic ();

  csRef<csLightingPolyTexQueue> lptq;
  if (!dyn)
  {
    csRef<iLightingProcessData> lpd = lpi->QueryUserdata (
    	scfInterface<csLightingPolyTexQueue>::GetID (),
	scfInterface<csLightingPolyTexQueue>::GetVersion());
    lptq = (csLightingPolyTexQueue*)(iLightingProcessData*)lpd;
    if (!lptq)
    {
      lptq = csPtr<csLightingPolyTexQueue> (new csLightingPolyTexQueue (
    	  lpi->GetLight ()));
      lpi->AttachUserdata (lptq);
    }
  }
  lpi->GetLight ()->AddAffectedLightingInfo (&scfiLightingInfo);

  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D* poly = GetPolygon3D (i);
    csPolygon3DStatic* spoly = static_data->GetPolygon3DStatic (i);
    const csPlane3& world_plane = GetPolygonWorldPlaneNoCheck (i);
    if (dyn)
      poly->CalculateLightingDynamic (lview, movable, world_plane, spoly);
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
      poly->CalculateLightingStatic (lview, movable, lptq, true,
      	m_world2tex, v_world2tex, world_plane, spoly);
    }
  }
}

void csThing::InitializeDefault (bool clear)
{
  if (clear) Unprepare();
  Prepare ();

  int i;
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
    int i;
    for (i = 0; i < polygons.Length (); i++)
    {
      csPolygon3D& p = polygons.Get (i);
      csPolygon3DStatic* sp = static_data->GetPolygon3DStatic (i);
      const char* error = p.ReadFromCache (&mf, sp);
      if (error != 0)
      {
        rc = false;
        if (csThingObjectType::do_verbose)
	{
	  printf ("  Thing '%s' Poly '%s': %s\n",
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
  {
    csPolygon3D& p = polygons.Get (i);
    csPolygon3DStatic* sp = static_data->GetPolygon3DStatic (i);
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
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D& p = polygons.Get (i);
    csLightMap* lm = p.GetPolyTexture ()->GetLightMap ();
    if (lm)
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

#ifdef CS_USE_NEW_RENDERER

void csThing::PrepareRenderMeshes (
  csDirtyAccessArray<csRenderMesh*>& renderMeshes)
{
  int i;

  for (i = 0; i < renderMeshes.Length () ; i++)
  {
    // @@@ Is this needed?
    if (renderMeshes[i]->variablecontext != 0)
      renderMeshes[i]->variablecontext->DecRef ();
    delete renderMeshes[i];
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
}

#endif

csRenderMesh **csThing::GetRenderMeshes (int &num, iRenderView* rview, 
                                         iMovable* movable, uint32 frustum_mask)
{
#ifdef CS_USE_NEW_RENDERER
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

  //@@@
  int i;
  csReversibleTransform tr_o2c = camtrans;
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();
  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
  	clip_z_plane);
  csVector3 camera_origin = tr_o2c.GetT2OTranslation ();

  const uint currentFrame = rview->GetCurrentFrameNumber ();
  csDirtyAccessArray<csRenderMesh*>& renderMeshes =
    rmHolder.GetUnusedMeshes (currentFrame);
  
  if (renderMeshes.Length() == 0)
  {
    PrepareRenderMeshes (renderMeshes);
  }

  for (i = 0; i < renderMeshes.Length(); i++)
  {
    csRenderMesh* rm = renderMeshes[i];
    rm->object2camera = tr_o2c;
    rm->camera_origin = camera_origin;
    rm->clip_portal = clip_portal;
    rm->clip_plane = clip_plane;
    rm->clip_z_plane = clip_z_plane;
    rm->do_mirror = icam->IsMirrored ();
    rm->lastFrame = currentFrame;

    rm->variablecontext->GetVariable (static_data->texLightmapName)->
      SetValue (i < litPolys.Length() ? litPolys[i]->SLM->GetTexture() : 0);
  }

  UpdateDirtyLMs (); // @@@ Here?

  num = renderMeshes.Length ();
  for (i = 0; i < materials_to_visit.Length (); i++)
  {
    materials_to_visit[i]->Visit ();
  }

  return renderMeshes.GetArray ();
#else
  return 0;
#endif
}

void csThing::PrepareLMs ()
{
  if (lightmapsPrepared) return;

  csThingObjectType* thing_type = static_data->thing_type;
  iTextureManager* txtmgr = thing_type->G3D->GetTextureManager ();

  csHash<csRef<iSuperLightmap>, csThingStatic::StaticSuperLM*> superLMs;

  int i;
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

      int j;
      pg->polys.SetLength (slpg.polys.Length ());
      for (j = 0; j < slpg.polys.Length(); j++)
      {
	pg->polys.Put (j, &polygons[slpg.polys[j]]);
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
      lpg->lightmaps.SetLength (slpg.polys.Length ());
      for (j = 0; j < slpg.polys.Length(); j++)
      {
	csPolygon3D* poly = &polygons[slpg.polys[j]];

	lpg->polys.Push (poly);
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

    int j;
    pg->polys.SetLength (spg.polys.Length ());
    for (j = 0; j < spg.polys.Length(); j++)
    {
      pg->polys.Put (j, &polygons[spg.polys[j]]);
    }

    unlitPolys.Push (pg);
  }

  litPolys.ShrinkBestFit ();
  unlitPolys.ShrinkBestFit ();

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

  WorUpdate ();
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

  int i;
  for (i = 0; i < litPolys.Length (); i++)
  {
    int j;
    for (j = 0; j < litPolys[i]->polys.Length (); j++)
    {
      csPolygon3D *poly = litPolys[i]->polys[j];
      csPolyTexture* lmi = poly->GetPolyTexture ();
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
      if (lmi->GetLightVersion () != GetLightVersion ())
      {
        const csPlane3& world_plane = GetPolygonWorldPlaneNoCheck (
		poly->GetPolyIdx ());
        if (lmi->RecalculateDynamicLights (m_world2tex, v_world2tex, poly,
		world_plane))
        {
	  litPolys[i]->lightmaps[j]->SetData (
	    lmi->GetLightMap ()->GetMapData ());
        }
      }
    }
  }

  lightmapsDirty = false;
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
	blk_polygon3dstatic (2000),
	blk_texturemapping (2000),
	blk_lightmap (2000),
	blk_polidx3 (1000),
	blk_polidx4 (2000)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiThingEnvironment);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
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
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiConfig);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  SCF_DESTRUCT_IBASE ();
}

bool csThingObjectType::Initialize (iObjectRegistry *object_reg)
{
  csThingObjectType::object_reg = object_reg;			
  csRef<iEngine> e = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = e;	// We don't want a real ref here to avoid circular refs.
  csRef<iGraphics3D> g = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  G3D = g;
  if (!g) return false;

  lightpatch_pool = new csLightPatchPool ();

  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);
  if (cmdline)
  {
    csThingObjectType::do_verbose = cmdline->GetOption ("verbose") != 0;
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
  maxSLMSpaceWaste =
    cfg->GetFloat ("Mesh.Thing.MaxSuperlightmapWaste", 0.6f);
  csThing::lightmap_quality = cfg->GetInt (
      "Mesh.Thing.LightmapQuality", 3);
  if (csThingObjectType::do_verbose)
    Notify ("Lightmap quality=%d\n", csThing::lightmap_quality);

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
  { 1, "lightqual", "Lighting quality", CSVAR_LONG }
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
    case 1:
      csThing::lightmap_quality = value->GetLong ();
      if (csThingObjectType::do_verbose)
	scfParent->Notify ("Lightmap quality=%d\n", csThing::lightmap_quality);
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
    case 1:   value->SetLong (csThing::lightmap_quality); break;
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
