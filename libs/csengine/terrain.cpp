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
#include "sysdef.h"

#define DDG_FIXME 0

#include "csengine/terrain.h"
#include "csengine/pol2d.h"
#include "csengine/texture.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "igraph3d.h"

#if DDG_FIXME
#include "csterr/struct/ddgcntxt.h"
#include "csterr/struct/ddgtmesh.h"
#include "csterr/struct/ddgbtree.h"
#include "csterr/struct/ddgvarr.h"
#endif

IMPLEMENT_CSOBJTYPE (csTerrain,csObject);

csTerrain::csTerrain () : csObject()
{
  clipbox = NULL;
  heightMap = NULL;
  mesh = NULL;
  vbuf = NULL;
}

csTerrain::~csTerrain ()
{
#if DDG_FIXME
  delete mesh;
  delete heightMap;
  delete clipbox;
  delete vbuf;
#endif
}

void csTerrain::SetDetail( unsigned int detail)
{
#if DDG_FIXME
  mesh->minDetail(detail);
  mesh->maxDetail((unsigned int)(detail*1.1));
  mesh->absMaxDetail((unsigned int)(detail * 1.25));
  mesh->nearClip(1.0);
  mesh->farClip(150.0);
#endif
}

#if DDG_FIXME
static csRenderView *grview = NULL;
#endif

bool csTerrain::Initialize (const void* heightMapFile, unsigned long size)
{
#if DDG_FIXME
  grview = NULL;
  heightMap = new ddgHeightMap ();
  if (heightMap->readTGN (heightMapFile, size))
    return false;

  mesh = new ddgTBinMesh (heightMap);
  clipbox = new ddgBBox (ddgVector3(0,0,3),ddgVector3(640, 480, 15000));
  context = new ddgContext ();

  vbuf = new ddgVArray ();

  vbuf->size (25000);
  vbuf->renderMode (true, false, true);
  vbuf->init ();
  vbuf->reset ();
  mesh->init (context);

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
	 openGL specific:  unsigned int i, r = 0, c = 0, k;
  _texturescale = 8;			// Texture is 8x the resolution of terrain
  unsigned int	width = heightMap->rows()*_texturescale;
  unsigned int height = heightMap->cols()*_texturescale;
  // Input texture is size 2^n+1 x 2^m+1
  // Assumption:
  // bintree pairs each form a square region on the mother texture.
  // Eg. 0/1  is bottom left n-1/n is top right of mother texture.
  // each bintree pair can be mapped left to right, top to bottom
  // onto the mother texture.

  k = width * height;
  // Calculate size of split textures.
  k = 2*k / mesh->getBinTreeNo();
  k = sqrt(k);



  char fileNameBuf[32];
  ddgStr	tbasename("terrainTexture.tga");
  for (i = 0; i < mesh->getBinTreeNo()/2; i++)
  {
    ostrstream msg(fileNameBuf,32);
    msg << tbasename.s << i << ".tga" << '\0';
    ddgStr sname;
    sname.assign(fileNameBuf);
    ddgImage *simg = new ddgImage();
    // Load the file if it exists, otherwise create if from the texture.
    if (simg->readFile(sname)== ddgFailure)
      return false;			// Failed to load texture file.
		_texture[i] = new ddgTexture(simg);
		_texture[i]->linear(true);
		_texture[i]->repeat(false);
		_texture[i]->scale( k, k);
		_texture[i]->mode(_meshcolor ? ddgTexture::MODULATE : ddgTexture::DECAL);
		_texture[i]->init();

    c = c + k;
    if (c + 1 >= width)
	{
      r += k;
      c = 0;
	}
  }

  // We are going to get texture coords from the terrain engine
  // ranging from 0 to rows and 0 to cols.
  // CS wants them to range from 0 to 1.
  _pos = csVector3(0,0,0);
  _size = csVector3(heightMap->cols(),mesh->wheight(mesh->absMaxHeight()),heightMap->rows());
#endif
  return true;
}


/**
 *  Retrieve info for a single triangle.
 *  Returns true if triangle should be rendered at all.
 */
#if !DDG_FIXME
bool csTerrain::drawTriangle(ddgTBinTree *bt, unsigned int tvc, ddgVArray *vbuf)
  { return false; }
#else
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

	if (!i1) bt->textureC(tva,t1);
	if (!i2) bt->textureC(tv0,t2);
	if (!i3) bt->textureC(tv1,t3);

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
#endif

void csTerrain::Draw (csRenderView& rview, bool /*use_z_buf*/)
{
#if DDG_FIXME
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
  rview.g3d->StartPolygonFX (poly.txt_handle, CS_FX_GOURAUD);
  grview = &rview;

  // See if viewpoint changed.
  {
    static csVector3 po1 (0, 0, 0), po2 (0, 0, 0), po3 (0, 0, 0);
    csVector3 pt1, pt2, pt3;
    pt1 = transformer (csVector3 (1, 0, 0));
    pt2 = transformer (csVector3 (0, 1, 0));
    pt3 = transformer (csVector3 (0, 0, 1));
    if (pt1 != po1 || pt2 != po2 || pt3 != po3)
    {
      moved = true;
      po1 = pt1;
      po2 = pt2;
      po3 = pt3;
    }
    mesh->dirty (moved);
  }

  ddgTBinTree::initWtoC(clipbox);
  modified = mesh->calculate ();
  // For each frame.
  if (true)//modified)
  {
    // Something changed, update the vertex buffer.
    vbuf->reset();
    // Get all the visible triangles.
    mesh->qsi()->reset();
    while (!mesh->qsi()->end())
    {
      ddgTriIndex tvc = mesh->indexSQ(mesh->qsi());
      ddgTBinTree *bt = mesh->treeSQ(mesh->qsi());
      PushTriangle(bt, tvc, vbuf);
      mesh->qsi ()->next ();
    }
    // Render all the leaf nodes.
    mesh->qmi()->reset();
    unsigned int nearLeaf = mesh->leafTriNo()/2;
    while (!mesh->qmi()->end() )
    {
      ddgTriIndex tvc = mesh->indexSQ(mesh->qmi());
      if (tvc >= nearLeaf)
      {
        ddgTBinTree *bt = mesh->treeSQ(mesh->qmi());
        PushTriangle(bt, ddgTBinTree::left(tvc), vbuf);
        PushTriangle(bt, ddgTBinTree::right(tvc), vbuf);
        ddgTriIndex n = bt->neighbour(tvc);
        if (n)
        {
          PushTriangle(bt->neighbourTree(n), ddgTBinTree::left(n), vbuf);
          PushTriangle(bt->neighbourTree(n), ddgTBinTree::right(n), vbuf);
        }
      }
      mesh->qmi()->next();
    }

    // Perform Qsort on VBuffer->_ibuf.
    vbuf->sort ();
  }


  ////////////// HERE IS HOW DDG RENDERS THE TRIANGLE MESH PER TEXTURE

  		unsigned int i = 0, s = 0;
		ddgTBinTree *bt;

		// If our orientation has changed, reload the buffer.
		if (modified)
		{
			ddgVBuffer::reset();
			// Update the vertex buffers.
			ddgCacheIndex ci = 0;
			while (i < _mesh->getBinTreeNo())
			{
				if (bt = _mesh->getBinTree(i))
				{
					unsigned int v = 0;
					// Render each triangle.
					ci = bt->chain();
					// Render each triangle.
					while (ci)
					{
						ddgTNode *tn = (ddgTNode*) _mesh->tcache()->get(ci);
 						if (drawTriangle(bt, tn->tindex(), ctx) == ddgSuccess)
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
		while (i < _mesh->getBinTreeNo())
		{
			if (mode.flags.texture && _texture && (i%2 == 0) && _texture[i/2])
				_texture[i/2]->activate();

			if ((bt = _mesh->getBinTree(i)) && (bt->visTriangle() > 0))
			{
				// Render this bintree.
				range(s,bt->visTriangle());

				super::draw(ctx);
				s = s+bt->visTriangle();
			}
			i++;
		}

	}
	if (mode.flags.texture && _texture )
		_texture[0]->deactivate();

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

#if DDG_FIXME
// Define a player object which can handle collision detection against
// the terrain.
class ddgPlayer : public ddgControl
{
	typedef	ddgControl super;
public:
	ddgPlayer( ddgVector3 *p, ddgVector3 *o) : super(p,o) {}
	///  Update the current position and orientation.
	bool update(void)
	{
		if( super::update())
		{
			float terrainHeight;
			// Keep camera above the terrain and within bounds.
			ddgVector3 campos(position());

			// Stay within x,z limits.
			if (campos[0] < 0) campos[0] = 0;
			if (campos[2] < 0) campos[2] = 0;
			if (campos[0] > tsize) campos[0] = tsize;
			if (campos[2] > tsize) campos[2] = tsize;

			// Follow terrain.
			terrainHeight = mesh ? mesh->height(campos[0],campos[2]) : 0.0;
			// Don't go below sea level.
			if (terrainHeight < sealevel)
				terrainHeight = sealevel;

			terrainHeight +=  groundHeight;
			if (campos[1] < terrainHeight)
				campos[1] = terrainHeight;
			position()->set(campos[0],campos[1],campos[2]);
			return true;
		}
		else
			return false;
	}
};
static ddgPlayer *player = 0;
#endif

// If we hit this terrain adjust our position to be on top of it.
int csTerrain::CollisionDetect( csTransform *transform )
{
  return 0;
#if DDG_FIXME
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
#endif
}
