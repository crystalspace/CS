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

bool csTerrain::Initialize (const void* heightMapFile, unsigned long size)
{
  heightMap = new ddgHeightMap ();
  if (heightMap->readTGN (heightMapFile, size))
    return false;

  mesh = new ddgTBinMesh (heightMap);
  clipbox = new ddgBBox (ddgVector3(0,0,3),ddgVector3(640, 480, 15000));
  context = new ddgContext ();

  vbuf = new ddgVArray ();

  mesh->init (context);

  vbuf->size((mesh->absMaxDetail()*3*11)/10);
  vbuf->init ();
  vbuf->reset ();
/*
JORRIT:  Create mesh->getBinTreeNo()/2  CS textures
         and put them in an array so I can reach them in the 
		 Draw method.
		 This is probably not even the right place do this, perhaps
		 it is better done in csLoader.
		 I would suggest that in the world file syntax we derive the
		 the texture name from the heightMap file name eg.
		 myTerrain.TGN
		 has textures named:
		 myTerrain0.jpg
		 myTerrain1.jpg 
		 ...
		 myTerrain128.jpg
		 
	 Here is how DDG does it, but I use the ddgTexture class which is
	 openGL specific:
*/


  // We are going to get texture coords from the terrain engine
  // ranging from 0 to rows and 0 to cols.
  // CS wants them to range from 0 to 1.
  _pos = csVector3(0,0,0);
  _size = csVector3(heightMap->cols(),mesh->wheight(mesh->absMaxHeight()),heightMap->rows());

  // (15 May 2000) To Alex: This is new code that allocates the
  // texture array for the terrain.
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

    static ddgVector3 p1, p2, p3;
    static ddgColor3 c1, c2, c3;
    static ddgVector2 t1, t2, t3;
    unsigned int i1 = 0, i2 = 0, i3 = 0;

    ddgTriIndex 
		tva = bt->parent(tvc),
		tv1 = bt->v0(tvc),
		tv0 = bt->v1(tvc);

    i1 = bt->vertex(tva,&p1);
    i2 = bt->vertex(tv0,&p2);
    i3 = bt->vertex(tv1,&p3);

	if (!i1) bt->textureC(tva,&t1);
	if (!i2) bt->textureC(tv0,&t2);
	if (!i3) bt->textureC(tv1,&t3);

    if (!i1) i1 = vbuf->pushVT(&p1,&t1);
    if (!i2) i2 = vbuf->pushVT(&p2,&t2);
    if (!i3) i3 = vbuf->pushVT(&p3,&t3);

    // Record that these vertices are in the buffer.
    bt->vbufferIndex(tva,i1);
    bt->vbufferIndex(tv0,i2);
    bt->vbufferIndex(tv1,i3);
    vbuf->pushTriangle(i1,i2,i3);

    return ddgSuccess;
}

void csTerrain::Draw (csRenderView& rview, bool /*use_z_buf*/)
{
  bool modified = true;
  /* Get matrices in OpenGL form
  JORRIT:
     this code is copied from the ogl_g3d renderer.
  csMatrix3 orientation = o2c.GetO2T();
    
  // set up coordinate transform
  GLfloat matrixholder[16];

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  // this zeroing is probably not needed, but I'm playing it safe in case
  // this code is changed later... GJH
  for (i=0; i<16; i++) matrixholder[i] = 0.0;

  matrixholder[0] = 1.0;
  matrixholder[5] = 1.0;
  matrixholder[10] = 1.0;

  matrixholder[0] = orientation.m11;
  matrixholder[1] = orientation.m21;
  matrixholder[2] = orientation.m31;

  matrixholder[4] = orientation.m12;
  matrixholder[5] = orientation.m22;
  matrixholder[6] = orientation.m32;

  matrixholder[8] = orientation.m13;
  matrixholder[9] = orientation.m23;
  matrixholder[10] = orientation.m33;

  matrixholder[15] = 1.0;

  csVector3 translation = o2c.GetO2TTranslation();

  glMultMatrixf(matrixholder);

  glTranslatef(-translation.x, -translation.y, -translation.z);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glOrtho (0., (GLdouble) width, 0., (GLdouble) height, -1.0, 10.0);

  glTranslatef(width2,height2,0);

  for (i = 0 ; i < 16 ; i++)
    matrixholder[i] = 0.0;
  
  matrixholder[0] = matrixholder[5] = matrixholder[10] = matrixholder[15] = 1.0;
  
  matrixholder[10] = 0.0;
  matrixholder[11] = +1.0/aspect;
  matrixholder[14] = -1.0/aspect;
  matrixholder[15] = 0.0;

  glMultMatrixf(matrixholder);

*/
	context->extractPlanes(context->frustrum());
	// Optimize the mesh w.r.t. the current viewing location.
	modified = mesh->calculate(context);

  	unsigned int i = 0, s = 0;
	ddgTBinTree *bt;

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

	}

    // Setup the structure for DrawTriangleMesh.
    static G3DTriangleMesh g3dmesh;
	static bool init = false;
	if (!init)
	{
		g3dmesh.vertex_colors[0] = NULL;			 // pointer to array of csColor for color information.
		g3dmesh.morph_factor = 0;
		g3dmesh.num_vertices_pool = 1;
		g3dmesh.num_textures = 1;
		g3dmesh.use_vertex_color = true;
		g3dmesh.do_clip = true;	// DEBUG THIS LATER
		g3dmesh.do_mirror = rview.IsMirrored ();
		g3dmesh.do_morph_texels = false;
		g3dmesh.do_morph_colors = false;
		g3dmesh.vertex_fog = NULL;
		g3dmesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
		g3dmesh.fxmode = CS_FX_GOURAUD;
		init = true;
	}
    g3dmesh.num_vertices = vbuf->num();	  // number of shared vertices for all triangles
    // All the three below arrays have num_vertices elements.
    g3dmesh.vertices[0] = (csVector3*) vbuf->vbuf; // pointer to array of csVector3 for all those verts
    g3dmesh.texels[0][0] = (csVector2*) vbuf->tbuf;	 // pointer to array of csVector2 for uv coordinates

	// Render the vertex buffer piece by piece (per texture).
	i = 0;
	s = 0;
	while (i < mesh->getBinTreeNo())
	{
//	  if (_textureMap && (i%2 == 0) && _textureMap[i/2])
// JORRIT:       g3dmesh.txt_handle[0] = _textureMap[i/2];

		if ((bt = mesh->getBinTree(i)) && (bt->visTriangle() > 0))
		{
			// Render this bintree.
            g3dmesh.num_triangles = bt->visTriangle(); // number of triangles
            g3dmesh.triangles = (csTriangle *) &(vbuf->ibuf[s*3]);	// pointer to array of csTriangle for all triangles

            rview.g3d->DrawTriangleMesh (g3dmesh);

			s = s+bt->visTriangle();
		}
		i++;
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
