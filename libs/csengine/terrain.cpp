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

#include "csengine/terrain.h"
#include "csengine/pol2d.h"
#include "csengine/texture.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csterr/struct/ddgcntxt.h"
#include "csterr/struct/ddgtmesh.h"
#include "csterr/struct/ddgbtree.h"
#include "csterr/struct/ddgvarr.h"
#include "igraph3d.h"

IMPLEMENT_CSOBJTYPE (csTerrain,csObject);

csTerrain::csTerrain () : csObject()
{
  clipbox = NULL;
  heightMap = NULL;
  mesh = NULL;
  vbuf = NULL;
  _textureMap = NULL;
}

csTerrain::~csTerrain ()
{
  delete mesh;
  delete heightMap;
  delete clipbox;
  delete vbuf;
  delete _textureMap;
}

void csTerrain::SetDetail( unsigned int detail)
{
  mesh->minDetail(detail);
  mesh->maxDetail((unsigned int)(detail*1.1));
  mesh->absMaxDetail((unsigned int)(detail * 1.25));
  mesh->nearClip(1.0);
  mesh->farClip(150.0);
}

int csTerrain::GetNumTextures ()
{
  return mesh->getBinTreeNo ()/2;
}

static ddgControl control;
bool csTerrain::Initialize (const void* heightMapFile, unsigned long size)
{
  heightMap = new ddgHeightMap ();
  if (heightMap->readTGN (heightMapFile, size))
    return false;

  mesh = new ddgTBinMesh (heightMap);
  clipbox = new ddgBBox (ddgVector3(0,0,3),ddgVector3(640, 480, 15000));
  context = new ddgContext ();
  context->control( &control );

  vbuf = new ddgVArray ();

  mesh->init (context);

  vbuf->size((mesh->absMaxDetail()*3*11)/10);
  vbuf->renderMode(true,false,false);
  vbuf->init ();
  vbuf->reset ();

  // We are going to get texture coords from the terrain engine
  // ranging from 0 to rows and 0 to cols.
  // CS wants them to range from 0 to 1.
  _pos = csVector3(0,0,0);
  _size = csVector3(heightMap->cols(),mesh->wheight(mesh->absMaxHeight()),heightMap->rows());

  // This is  code that allocates the texture array for the terrain.
  _textureMap = new csTextureHandle* [GetNumTextures ()];
  return true;
}


/**
 *  Retrieve info for a single triangle.
 *  Returns true if triangle should be rendered at all.
 */
bool csTerrain::drawTriangle( ddgTBinTree *bt, ddgVBIndex tvc, ddgVArray *vbuf )
{
	if ( !bt->visible(tvc))
		return ddgFailure;

    static ddgVector3 p[3];
    static ddgColor3 c[3];
    static ddgVector2 t[3];
    ddgVBIndex i[3] = {0,0,0};

    ddgTriIndex tv[3];
	tv[0] = bt->parent(tvc),
	tv[2] = mesh->v0(tvc),
	tv[1] = mesh->v1(tvc);

	int cnt;
	for (cnt = 0; cnt < 3; cnt++)
	{
		bt->vertex(tv[cnt],&p[cnt]);
		bt->textureC(tv[cnt],&t[cnt]);
		i[cnt] = vbuf->pushVT(&p[cnt],&t[cnt]);
	}
    // Record that these vertices are in the buffer.
    vbuf->pushTriangle(i[0],i[1],i[2]);

    return ddgSuccess;
}

void csTerrain::Draw (csRenderView& rview, bool /*use_z_buf*/)
{
  bool modified = true;
  // Get matrices in OpenGL form

  unsigned int i = 0, s = 0, d = 0;
  ddgTBinTree *bt;

  // Currently the CS version of the terrain engine uses a clipping
  // wedge base on the position, the field of view, the forward vector and
  // the far clip distance.  It is a 2D clipping in the XZ plane.
  //
  // The DDG engine uses clipping agains 2 or 5 clipping planes
  // which match the true clipping planes as set by the projection
  // matrix.
  const csMatrix3& orientation = rview.GetO2T ();

  const csVector3& translation = rview.GetO2TTranslation();
  // JORRIT: I need the camera's foward facing vector in world space.
  // I dont think this is working...
  const csVector3 wforward(1,0,0);
  const csVector3 cforward = wforward * rview;
  ddgVector3 f(cforward.x,cforward.y,cforward.z);
  // Hard code for now to show initial view.
  f.set(0,0,1);
  f.normalize();

  ddgControl *control = context->control();
  control->position(translation.x, translation.y, translation.z);
 
  context->forward(&f);
  // JORRIT: rview.GetFOV() returns 480 I need the real field of view.
  context->fov(90);
  // context->extractPlanes(context->frustrum());
  // Optimize the mesh w.r.t. the current viewing location.
  modified = mesh->calculate(context);

  // If our orientation has changed, reload the buffer.
  if (modified)
  {
	vbuf->reset();
	// Update the vertex buffers.
	ddgCacheIndex ci = 0;
	while (i < mesh->getBinTreeNo())
	{
	  if ((bt = mesh->getBinTree(i)))
	  {
	    s = 0;
		// Render each triangle.
		ci = bt->chain();
		// Render each triangle.
		while (ci)
		{
		  ddgTNode *tn = (ddgTNode*) mesh->tcache()->get(ci);
 		  if (drawTriangle(bt, tn->tindex(), vbuf) == ddgSuccess)
		    s++;
		  ci = tn->next();
		}

		bt->visTriangle(s);
	  }
	  i++;
	}
	// Ugly hack to help software renderer, reindex the triangles per block.
	  i = 0;
	  int j;
	  s = 0;
	  while (i < mesh->getBinTreeNo())
	  {
		d = mesh->getBinTree(i)->visTriangle() + mesh->getBinTree(i+1)->visTriangle();
		if (d > 0)
		{
		  for (j = s*3; j < 3*(s+d); j++)
			  vbuf->ibuf[j] -= s*3;
		  s = s+d;
		}
		i = i+2;
	  }

  }

  rview.g3d->SetObjectToCamera (&rview);
  rview.g3d->SetClipper (rview.view->GetClipPoly (), rview.view->GetNumVertices ());
  // @@@ This should only be done when aspect changes...
  rview.g3d->SetPerspectiveAspect (rview.aspect);
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);

  // Setup the structure for DrawTriangleMesh.
  static G3DTriangleMesh g3dmesh;
  static bool init = false;
  if (!init)
  {
	g3dmesh.vertex_colors[0] = NULL;			 // pointer to array of csColor for color information.
	g3dmesh.morph_factor = 0;
	g3dmesh.num_vertices_pool = 1;
	g3dmesh.num_textures = 1;
	g3dmesh.use_vertex_color = false;
	g3dmesh.do_clip = false;	// DEBUG THIS LATER
	g3dmesh.do_mirror = rview.IsMirrored ();
	g3dmesh.do_morph_texels = false;
	g3dmesh.do_morph_colors = false;
	g3dmesh.vertex_fog = NULL;
	g3dmesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
	g3dmesh.fxmode = 0;//CS_FX_GOURAUD;
	init = true;
  }
  // Cant dothis in one step for software renderer.
//  g3dmesh.num_vertices = vbuf->num();	  // number of shared vertices for all triangles
  // All the three below arrays have num_vertices elements.
//  g3dmesh.vertices[0] = (csVector3*) vbuf->vbuf; // pointer to array of csVector3 for all those verts
//  g3dmesh.texels[0][0] = (csVector2*) vbuf->tbuf;	 // pointer to array of csVector2 for uv coordinates

  // Render the vertex buffer piece by piece (per texture).
  i = 0;
  s = 0;
  while (i < mesh->getBinTreeNo())
  {
	d = mesh->getBinTree(i)->visTriangle() + mesh->getBinTree(i+1)->visTriangle();
	if (d > 0)
	{
      if (_textureMap && _textureMap[i/2])
	    g3dmesh.txt_handle[0] = _textureMap[i/2]->GetTextureHandle ();
	  // Render this block.
	  // For software renderer we need to pass in a little bit at a time.
	  g3dmesh.num_vertices = d*3;	  // number of shared vertices for all triangles
	  g3dmesh.vertices[0] = (csVector3*) &(vbuf->vbuf[s*3]); // pointer to array of csVector3 for all those verts
	  g3dmesh.texels[0][0] = (csVector2*) &(vbuf->tbuf[s*3]);	 // pointer to array of csVector2 for uv coordinates
      g3dmesh.num_triangles = d; // number of triangles
      g3dmesh.triangles = (csTriangle *) &(vbuf->ibuf[s*3]);	// pointer to array of csTriangle for all triangles

      rview.g3d->DrawTriangleMesh (g3dmesh);
	  // Increment the starting offset by the number of triangles that were in this block.
	  s = s+d;
	}
	i = i+2;
  }
}

// If we hit this terrain adjust our position to be on top of it.
int csTerrain::CollisionDetect( csTransform *transform )
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
