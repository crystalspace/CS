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

void csTerrain::Draw (csRenderView& /*rview*/, bool /*use_z_buf*/)
{
#if 0
//////////////////// OLD FUNCTION INITIALIZATION.
  G3DPolygonDPFX poly;

  bool moved = false;
  static bool modified = true;
 
  memset (&poly, 0, sizeof(poly));
  poly.inv_aspect = rview.inv_aspect;
  poly.flat_color_r = 255;
  poly.flat_color_g = 255;
  poly.flat_color_b = 255;
  poly.txt_handle = _textureMap->GetTextureHandle ();
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE,
    false ? CS_ZBUF_USE : CS_ZBUF_FILL);
  rview.g3d->StartPolygonFX (poly.txt_handle, CS_FX_GOURAUD)
#endif

  ////////////// HERE IS HOW DDG RENDERS THE TRIANGLE MESH PER TEXTURE
	bool modified = true;
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
				unsigned int v = 0;
				// Render each triangle.
				ci = bt->chain();
				// Render each triangle.
				while (ci)
				{
					ddgTNode *tn = (ddgTNode*) mesh->tcache()->get(ci);
 					if (drawTriangle(bt, tn->tindex(), vbuf) == ddgSuccess)
						v++;
					ci = tn->next();
				}
				bt->visTriangle(v);
			}
			i++;
		}

	}

	// Render the vertex buffer piece by piece.
	i = 0;
	while (i < mesh->getBinTreeNo())
	{
//		JORRIT: Switch textures here.
//
//			if (_textureMap && (i%2 == 0) && _textureMap[i/2])
//				_textureMap[i/2]->activate();

		if ((bt = mesh->getBinTree(i)) && (bt->visTriangle() > 0))
		{
			// Render this bintree.

			ddgAssert(s >= 0 && s + bt->visTriangle() <= vbuf->inum());
/*

	ALEX: Here you see how you can render a mesh of triangles
	in CS. Note that a mesh can only have one texture so you
	need to call DrawTriangleMesh for every texture you have.
	It is best that you can collect as many triangles with that
	texture together as you can. I don't know how your code
	returns the triangles? Grouped per texture would be best.

  // Setup the structure for DrawTriangleMesh.
  G3DTriangleMesh mesh;
  mesh.txt_handle[0] = _textureMap[i/2];
  mesh.num_vertices = ALEX: number of shared vertices for all triangles
  // All the three below arrays have num_vertices elements.
  mesh.vertices[0] = ALEX: pointer to array of csVector3 for all those verts
  mesh.texels[0][0] = ALEX: pointer to array of csVector2 for uv coordinates
  mesh.vertex_colors[0] = ALEX: pointer to array of csColor for color information.
  mesh.morph_factor = 0;
  mesh.num_vertices_pool = 1;
  mesh.num_textures = 1;

  mesh.num_triangles = ALEX: number of triangles
  mesh.triangles = ALEX: pointer to array of csTriangle for all triangles

  mesh.use_vertex_color = true;
  mesh.do_clip = true;	// DEBUG THIS LATER
  mesh.do_mirror = rview.IsMirrored ();
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_fog = NULL;

  mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  mesh.fxmode = CS_FX_GOURAUD;
  rview.g3d->DrawTriangleMesh (mesh);

  //else
	JORRIT: Here is how I render triangles in DDG.

				Really render the vertex array.  Open GL example
			if (s == 0)
			{
				glEnableClientState (GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer (2, GL_FLOAT, 0, vbuf->tbuf);
			}
			// Vertex array for rendering.
			glEnableClientState (GL_VERTEX_ARRAY);
			glVertexPointer ( 3, GL_FLOAT, 12, vbuf->vbuf); // 3 floats = 16 bytes.
			glDrawElements(GL_TRIANGLES, bt->visTriangle()*3, GL_UNSIGNED_INT, &(vbuf->ibuf[s*3]));
			if (bt->visTriangle() + s == vbuf->inum()/3)
			{
				glDisableClientState (GL_VERTEX_ARRAY);
				glDisableClientState (GL_TEXTURE_COORD_ARRAY);
			}
*/
			s = s+bt->visTriangle();
		}
		i++;
	}

#if 0
  ///////////////////// OLD CS TERRAIN CODE ///////////////////////
  // Render
  csVector3 *p1, *p2, *p3;
  ddgVector2 t1, t2, t3;
  // float  *c1, *c2, *c3;
  ddgVBIndex i1, i2, i3;
  csVector2 clipped_triangle [10];	//@@@BAD HARCODED!
  unsigned int i = vbuf->size();
  while (i)
  {
    int rescount;
    i--;

    i3 = vbuf->ibuf[i*3];
    i2 = vbuf->ibuf[i*3+1];
    i1 = vbuf->ibuf[i*3+2];
    // Camera space coords.
    p1 = &(vbuf->vbuf[i1]);
    p2 = &(vbuf->vbuf[i2]);
    p3 = &(vbuf->vbuf[i3]);
    // c1 = &(vbuf->cbuf[i1]);
    // c2 = &(vbuf->cbuf[i2]);
    // c3 = &(vbuf->cbuf[i3]);
    t1 = &(vbuf->tbuf[i1]);
    t2 = &(vbuf->tbuf[i2]);
    t3 = &(vbuf->tbuf[i3]);

    float iz;
    float pz[3];
    csVector2 triangle[3];
    if (p1->z < SMALL_Z) continue;
    pz[0] = 1 / p1->z;
    iz = rview.aspect * pz[0];
    triangle[0].x = p1->x * iz + rview.shift_x;
    triangle[0].y = p1->y * iz + rview.shift_y;
    if (p2->z < SMALL_Z) continue;
    pz[1] = 1 / p2->z;
    iz = rview.aspect * pz[1];
    triangle[1].x = p2->x * iz + rview.shift_x;
    triangle[1].y = p2->y * iz + rview.shift_y;
    if (p3->z < SMALL_Z) continue;
    pz[2] = 1 / p3->z;
    iz = rview.aspect * pz[2];
    triangle[2].x = p3->x * iz + rview.shift_x;
    triangle[2].y = p3->y * iz + rview.shift_y;

    if (!rview.view->Clip (triangle, 3, clipped_triangle, rescount))
      continue;
    poly.num = rescount;

    poly.vertices[0].z = pz[0]; //p1->z;
    poly.vertices[0].u = t1[0];
    poly.vertices[0].v = t1[1];
    poly.vertices[0].r = 1;//c1[0];
    poly.vertices[0].g = 1;//c1[1];
    poly.vertices[0].b = 1;//c1[2];

    poly.vertices[1].z = pz[1]; //p2->z;
    poly.vertices[1].u = t2[0];
    poly.vertices[1].v = t2[1];
    poly.vertices[1].r = 1;//c2[0];
    poly.vertices[1].g = 1;//c2[1];
    poly.vertices[1].b = 1;//c2[2];

    poly.vertices[2].z = pz[2]; //p3->z;
    poly.vertices[2].u = t3[0];
    poly.vertices[2].v = t3[1];
    poly.vertices[2].r = 1;//c3[0];
    poly.vertices[2].g = 1;//c3[1];
    poly.vertices[2].b = 1;//c3[2];
      
    PreparePolygonFX (&poly, clipped_triangle, rescount, triangle, true);
    rview.g3d->DrawPolygonFX (poly);
  }

  rview.g3d->FinishPolygonFX ();
#endif
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
