/*
    Copyright (C) 1999,2000 by Jorrit Tyberghein
  
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

#include "csengine/terrddg.h"
#include "csengine/pol2d.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/engine.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csterr/struct/ddgcntxt.h"
#include "csterr/struct/ddgtmesh.h"
#include "csterr/struct/ddgbtree.h"
#include "csterr/struct/ddgvarr.h"
#include "igraph3d.h"
#include "itxtmgr.h"

IMPLEMENT_CSOBJTYPE (csDDGTerrain, csTerrain);

csDDGTerrain::csDDGTerrain () : csTerrain ()
{
  heightMap = NULL;
  mesh = NULL;
  vbuf = NULL;
  _materialMap = NULL;
  do_dirlight = false;
}

csDDGTerrain::~csDDGTerrain ()
{
  delete mesh;
  delete heightMap;
  delete vbuf;
  delete _materialMap;
}

void csDDGTerrain::SetDetail (unsigned int detail)
{
  mesh->minDetail (detail);
  mesh->maxDetail ((unsigned int)(detail*1.1));
  mesh->absMaxDetail ((unsigned int)(detail * 1.25));
  mesh->nearClip (.1);
  mesh->farClip (1000.0);
}

void csDDGTerrain::SetDirLight (const csVector3& dirl, const csColor& dirc)
{
  dirlight = dirl;
  dircolor = dirc;
  do_dirlight = true;
}

int csDDGTerrain::GetNumMaterials ()
{
  return mesh->getBinTreeNo ()/2;
}

static ddgControl control;
bool csDDGTerrain::Initialize (const void* heightMapFile, unsigned long size)
{
  heightMap = new ddgHeightMap ();
  if (heightMap->readTGN (heightMapFile, size))
    return false;

  mesh = new ddgTBinMesh (heightMap);
  context = new ddgContext ();
  context->control (&control);
  context->clipbox ()->min.set (0, 0, 0.1);
  int fw = csEngine::current_engine->G3D->GetWidth ();
  int fh = csEngine::current_engine->G3D->GetHeight ();
  context->clipbox ()->max.set (fw, fh, 1000);

  vbuf = new ddgVArray ();

  mesh->init (context);

  vbuf->size ((mesh->absMaxDetail ()*4*12)/10);
  vbuf->renderMode (true, false, false);
  vbuf->init ();
  vbuf->reset ();

  // We are going to get texture coords from the terrain engine
  // ranging from 0 to rows and 0 to cols.
  // CS wants them to range from 0 to 1.
  _pos = csVector3 (0,0,0);
  _size = csVector3 (heightMap->cols (),
      mesh->wheight (mesh->absMaxHeight ()), heightMap->rows ());

  // This is  code that allocates the texture array for the terrain.
  _materialMap = new csMaterialWrapper* [GetNumMaterials ()];
  return true;
}

/// Number of entries in the Most Recently Used cache.
#define	_MRUsize 12
/// Vertices cached.
unsigned int	_MRUvertex[_MRUsize];
/// Buffer indexes corresponding to those vertices.
int		_MRUindex[_MRUsize];
/// Position of last entry added into the cache.
unsigned int	_MRUcursor;
/// Number or items currently in the cache.
unsigned int	_MRUinuse;

static int lut[24] = {0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11};
#define ddgInvalidBufferIndex	0xFFFF

/**
 *  Retrieve info for a single triangle.
 *  Returns true if triangle should be rendered at all.
 */
bool csDDGTerrain::drawTriangle (ddgTBinTree *bt, ddgVBIndex tvc, ddgVArray *vbuf)
{
  if (!bt->visible (tvc))
    return ddgFailure;

  static ddgVector3 p[3];
  static ddgVector2 t[3];
  ddgVBIndex bufindex[3] = {0,0,0};

  ddgTriIndex tv[3];
  tv[0] = bt->parent (tvc),
  tv[2] = mesh->v0 (tvc),
  tv[1] = mesh->v1 (tvc);

  int i, j;
  for (i = 0; i < 3; i++)
  {
    bufindex[i] = ddgInvalidBufferIndex;
    j = _MRUsize+_MRUcursor;
    unsigned int end = j - _MRUinuse;
    // See if the current entry is in the MRUCache.
    while (j > (int)end)
    {
      ddgAssert (j>=0 && j < _MRUsize*2);
      if (_MRUvertex[lut[j]] == tv[i])
      {
        bufindex[i] = _MRUindex[lut[j]];
        break;
      }
      j--;
    }
    if (ddgInvalidBufferIndex == bufindex[i])
    {
      // We could just call.
      bt->vertex (tv[i],&p[i]);
      bt->textureC (tv[i],&(t[i]));
      // Push the vertex.
      bufindex[i] = vbuf->pushVT (&p[i],&t[i]);
      if (_MRUinuse < _MRUsize) _MRUinuse++;

      if (_MRUcursor==_MRUsize-1)
	_MRUcursor = 0;
      else
	_MRUcursor++;

      // Record that these vertices are in the buffer.
      _MRUindex[_MRUcursor] = bufindex[i];
      _MRUvertex[_MRUcursor] = tv[i];
    }
  }

  // Record that these vertices are in the buffer.
  vbuf->pushTriangle (bufindex[0], bufindex[1], bufindex[2]);

  return ddgSuccess;
}

static DECLARE_GROWING_ARRAY (fog_verts, G3DFogInfo);
static DECLARE_GROWING_ARRAY (color_verts, csColor);

void csDDGTerrain::Draw (csRenderView& rview, bool use_z_buf)
{
  bool modified = true;

  unsigned int i = 0, s = 0, d = 0, n = 0, nd = 0;
  ddgTBinTree *bt;

  // Currently the CS version of the terrain engine uses a clipping
  // wedge base on the position, the field of view, the forward vector and
  // the far clip distance.  It is a 2D clipping in the XZ plane.
  //
  // The DDG engine uses clipping agains 2 or 5 clipping planes
  // which match the true clipping planes as set by the projection
  // matrix.

  // Position of camera.
  const csVector3& translation = rview.GetO2TTranslation ();
  ddgVector3 p (translation.x, translation.y, translation.z);

  // Compute the camera's foward facing vector in world space.
  const csVector3 cforward (0,0,-1);
  const csVector3 wforward = rview.This2OtherRelative (cforward);
  ddgVector3 f (wforward.x,wforward.y,wforward.z);
  f.normalize ();
  // Compute the camera's up facing vector in world space.
  const csVector3 cup (0,1,0);
  const csVector3 wup = rview.This2OtherRelative (cup);
  ddgVector3 u (wup.x,wup.y,wup.z);
  u.normalize ();
  // Compute the camera's right facing vector in world space.
  const csVector3 cright (-1,0,0);
  const csVector3 wright = rview.This2OtherRelative (cright);
  ddgVector3 r (wright.x,wright.y,wright.z);
  r.normalize ();
 
  // Update the DDG context object.
  ddgControl *control = context->control();
  control->position (p.v[0], p.v[1], p.v[2]);
  context->forward (&f);
  context->up (&u);
  context->right (&r);

  // Get the FOV in angles.
  context->fov (rview.GetFOVAngle ());
  context->aspect (float (rview.GetG3D ()->GetWidth ()) / (float)rview.GetG3D ()->GetHeight ());

  // Construct some clipping planes.
  context->extractPlanes (context->frustrum ());

  // Optimize the mesh w.r.t. the current viewing location.
  modified = mesh->calculate (context);

  // If our orientation has changed, reload the buffer.
  if (modified)
  {
    vbuf->reset ();
    // Update the vertex buffers.
    i = 0;
    while (i < mesh->getBinTreeNo ())
    {
      if ((bt = mesh->getBinTree (i)))
      {
	unsigned int v = 0;
	n = 0;

	if (bt && bt->treeVis () != ddgOUT)
	{
	  // Position of last entry added into the cache.
	  _MRUcursor = _MRUsize-1;
	  // Number or items currently in the cache.
	  _MRUinuse = 0;
	  // Find 1st triangle in the mesh.
	  ddgTriIndex tindex = bt->firstInMesh ();
	  ddgTriIndex end = 0;
	  n = vbuf->num();
	  do 
	  {
 	    if (drawTriangle (bt, tindex, vbuf) == ddgSuccess)
	      v++;
	    tindex = bt->nextInMesh (tindex, &end);
	  }
	  while (end);

	  n = vbuf->num () - n;
	}
	bt->visTriangle (v);
	bt->uniqueVertex (n);
      }
      i++;
    }

    // Ugly hack to help software renderer, reindex the triangles per block.
    i = 0;
    unsigned int j;
    s = 0;

    n = 0, nd = 0;
    while (i < mesh->getBinTreeNo ())
    {
      d = mesh->getBinTree (i)->visTriangle () + mesh->getBinTree (i+1)->visTriangle ();
      if (d > 0)
      {
	nd = mesh->getBinTree (i)->uniqueVertex () + mesh->getBinTree (i+1)->uniqueVertex ();
        for (j = s*3; j < 3*(s+d); j++)
          vbuf->ibuf[j] -= n;
        s += d;
        n += nd;
      }
      i = i+2;
    }

  } // end modified.

  rview.GetG3D ()->SetObjectToCamera (&rview);
  rview.GetG3D ()->SetClipper (rview.GetView ()->GetClipPoly (), rview.GetView ()->GetNumVertices ());
  // @@@ This should only be done when aspect changes...
  rview.GetG3D ()->SetPerspectiveAspect (rview.GetFOV ());
  rview.GetG3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE,
      use_z_buf ? CS_ZBUF_USE : CS_ZBUF_FILL);

  // Setup the structure for DrawTriangleMesh.
  static G3DTriangleMesh g3dmesh;
  static bool init = false;
  if (!init)
  {
    g3dmesh.vertex_colors[0] = NULL;
    g3dmesh.morph_factor = 0;
    g3dmesh.num_vertices_pool = 1;
    g3dmesh.num_materials = 1;
    g3dmesh.use_vertex_color = false;
    g3dmesh.do_mirror = rview.IsMirrored ();
    g3dmesh.do_morph_texels = false;
    g3dmesh.do_morph_colors = false;
    g3dmesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    g3dmesh.fxmode = 0;//CS_FX_GOURAUD;
    init = true;
  }

  // Render the vertex buffer piece by piece (per texture).
  i = 0;
  s = 0;
  n = 0;
  nd = 0;
  while (i < mesh->getBinTreeNo ())
  {
    d = mesh->getBinTree (i)->visTriangle () + mesh->getBinTree (i+1)->visTriangle ();
    if (d > 0)
    {
      nd = mesh->getBinTree (i)->uniqueVertex () + mesh->getBinTree (i+1)->uniqueVertex ();
      if (_materialMap && _materialMap[i/2])
      {
	_materialMap[i/2]->Visit ();
	g3dmesh.mat_handle[0] = _materialMap[i/2]->GetMaterialHandle ();
      }
      // Render this block.
      // For software renderer we need to pass in a little bit at a time.
      g3dmesh.num_vertices = nd;  // Number of shared vertices for all triangles
      g3dmesh.vertices[0] = (csVector3*) &(vbuf->vbuf[n]);  // Vertex array
      g3dmesh.texels[0][0] = (csVector2*) &(vbuf->tbuf[n]); // Uv coordinates
      g3dmesh.num_triangles = d; // Number of triangles
      g3dmesh.triangles = (csTriangle *) &(vbuf->ibuf[s*3]);
      // Enable clipping for blocks that are not entirely within the
      // view frustrum.
      g3dmesh.do_clip =
		mesh->getBinTree (i)->treeVis () != ddgIN ||
	        mesh->getBinTree (i+1)->treeVis () != ddgIN;

      // @@@ This is not the good way to do this as it is slow.
      // The lighting information must be recalculated only when needed.
      // Here we calculate it again every time.
      //if (do_dirlight)
      //{
        //if (nd > (unsigned int)color_verts.Limit ())
          //color_verts.SetLimit (nd);
        //g3dmesh.vertex_colors[0] = color_verts.GetArray ();
        //g3dmesh.fxmode = CS_FX_GOURAUD;
      //}
      //else
      //{
        //g3dmesh.vertex_colors[0] = NULL;
        //g3dmesh.fxmode = 0;
      //}

      if (nd > (unsigned int)fog_verts.Limit ())
        fog_verts.SetLimit (nd);
      g3dmesh.vertex_fog = fog_verts.GetArray ();

      rview.CalculateFogMesh (rview, g3dmesh);

      if (rview.GetCallback)
        rview.CallCallback (CALLBACK_MESH, (void*)&g3dmesh);
      else
        rview.GetG3D ()->DrawTriangleMesh (g3dmesh);
      // Increment the starting offset by the number of triangles that
      // were in this block.
      s += d;
      n += nd;
    }
    i = i+2;
  }
}

// If we hit this terrain adjust our position to be on top of it.
int csDDGTerrain::CollisionDetect (csTransform *transform)
{
  float h;
  // Translate us into terrain coordinate space.
  csVector3 p = transform->GetOrigin () - _pos;
  // If our location is above or outside the terrain then we cannot hit is.
  if (p[0] < 0 || p[2] < 0 || p[0] > _size[0] || p[2] > _size[2] ||
      p[1]>_size[1])
    return 0;

  // Return height of terrain at this location in Y coord.
  h = mesh->height (p[0],p[2])+2;
  if (h < p[1])
    return 0;
  p[1] = h;
  // Translate us back.
  p = p + _pos;
  transform->SetOrigin (p);
  return 1;
}
