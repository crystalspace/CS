/*
    Copyright (C) 1999 by Jorrit Tyberghein
  
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
#include "csengine/terrain.h"
#include "csengine/pol2d.h"
#include "csengine/texture.h"
#include "csterr/ddgtmesh.h"
#include "csterr/ddgbtree.h"
#include "csterr/ddgvbuf.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "igraph3d.h"


//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csTerrain,csObject);

csTerrain::csTerrain () : csObject()
{
  clipbox = NULL;
  height = NULL;
  mesh = NULL;
  vbuf = NULL;
}

csTerrain::~csTerrain ()
{
  CHK (delete mesh);
  CHK (delete height);
  CHK (delete clipbox);
  CHK (delete vbuf);
}

void csTerrain::classify( csVector3 *p, ddgColor3 *c)
{
	// Steep normal
    if (p->y < _beachalt)
		c->set(_beach); // Beach
    else if (p->y < _grassalt)
		c->set(_grass); // Grass
    else if (p->y < _treealt)
		c->set(_trees); // Trees
    else if (p->y < _rockalt)
		c->set(_rock);  // Rock
    else
		c->set(_snow);  // Snow
}

static csRenderView *grview = NULL;
// Perform coord transformation using CS instead of DDG.
void transformer(csVector3 vin, csVector3 *vout)
{
	if (grview)
	{
		csVector3 csw1 = grview->World2Camera( vin );
		vout->x = csw1.x;
		vout->y = csw1.y;
		vout->z = csw1.z;
	}
	else
	{
		vout->x = vin.x;
		vout->y = vin.y;
		vout->z = vin.z;
	}
}

bool csTerrain::Initialize (char* heightmap)
{
  grview = NULL;
  CHK (height = new ddgHeightMap ());
  if (height->readTGN (heightmap))
	  height->generateHeights(257,257,5);
  CHK (mesh = new ddgTBinMesh (height));
  CHK (clipbox = new ddgBBox (csVector3(0,0,3),csVector3(640, 480, 15000)));

  CHK (vbuf = new ddgVBuffer());

  vbuf->size(25000);
  vbuf->renderMode(true,false,true);
  vbuf->init();
  float fov = 90.0;
  mesh->init (NULL, clipbox, fov);

	_cliff.set(175,175,175);  // Cliff
	_beach.set(200,200,50);  // Beach.
	_grass.set(95,145,70);    // Grass
	_trees.set(25,50,25);     // Trees
	_rock.set(125,125,125);      // Rock
	_snow.set(250,250,250);      // Snow

	_cliffangle = 0.2; // up component of normal vector.
	_beachalt = 0;
	_grassalt = 3;
	_treealt = 5;
	_rockalt = 7;
	_snowalt = 999;


  return true;
}

void csTerrain::Draw (csRenderView& rview, bool /*use_z_buf*/)
{
  G3DPolygonDPFX poly;

  bool moved = false;
  static bool modified = true;
 
  memset (&poly, 0, sizeof(poly));
  poly.inv_aspect = rview.inv_aspect;
  poly.flat_color_r = 1;
  poly.flat_color_g = 1;
  poly.flat_color_b = 1;
  poly.txt_handle = _textureMap->GetTextureHandle ();
  // We are going to get texture coords from the terrain engine ranging from 0 to rows and 0 to cols
  // CS wants them to range from 0 to 1.
  float texscale  = 10.0 / height->rows();
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, false /*use_z_buf*/);
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  rview.g3d->StartPolygonFX (poly.txt_handle, CS_FX_GOURAUD);
  grview = &rview;

  // See if viewpoint changed.
  {
	  static csVector3 p1, p2, p3, po1, po2, po3, pt1, pt2, pt3;
	  p1.Set(1,0,0);
	  transformer(p1,&pt1);
	  p2.Set(0,1,0);
	  transformer(p2,&pt2);
	  p3.Set(0,0,1);
	  transformer(p3,&pt3);
	  if (pt1.x != po1.x || pt1.y != po1.y || pt1.z != po1.z
		||pt2.x != po2.x || pt2.y != po2.y || pt2.z != po2.z
		||pt3.x != po3.x || pt3.y != po3.y || pt3.z != po3.z)
	  {
		  moved = true;
		  po1.x = pt1.x; po1.y = pt1.y; po1.z = pt1.z;
		  po2.x = pt2.x; po2.y = pt2.y; po2.z = pt2.z;
		  po3.x = pt3.x; po3.y = pt3.y; po3.z = pt3.z;
	  }
	  mesh->dirty(moved);
  }
  modified = mesh->calculate ();
  // For each frame.
  if ( modified )
  {
		// Something changed, update the vertex buffer.
		vbuf->reset();

		// Get all the visible triangles.
		mesh->qsi()->reset();
		while (!mesh->qsi()->end())
		{
			ddgTriIndex tvc = mesh->indexSQ(mesh->qsi()), tva, tv0, tv1;

			ddgTBinTree *bt = mesh->treeSQ(mesh->qsi());
			static csVector3 p1, p2, p3;
			static csVector3 *cp1, *cp2, *cp3;
			static csVector3 n1, n2, n3;
			static ddgVector2 t1, t2, t3;
//			static ddgColor3  c1, c2, c3;
			unsigned int i1 = 0, i2 = 0, i3 = 0;
			// If triangle is visible.
			if (!bt->tri (tvc)->vis ().flags.allout)
			{
				tva = ddgTBinTree::parent(tvc);
				tv0 = bt->v0(tvc);
				tv1 = bt->v1(tvc);

				// Get the camera space coords.
				cp1 =	bt->pos(tva);
				cp2 =	bt->pos(tv0);
   				cp3 =	bt->pos(tv1);

				i1 = bt->vertex(tva,&p1);
				i2 = bt->vertex(tv0,&p2);
				i3 = bt->vertex(tv1,&p3);

//				if (vbuf->normalOn())
				{
//					if (!i1) bt->normal(tva,&n1);
//					if (!i2) bt->normal(tv0,&n2);
//					if (!i3) bt->normal(tv1,&n3);
				}

				if (vbuf->textureOn())
				{
					if (!i1) {bt->textureC(tva,t1); t1.multiply( texscale); t1.v[0] -= ((int)t1[0]);  t1.v[1] -= ((int)t1[1]);}
					if (!i2) {bt->textureC(tv0,t2); t2.multiply( texscale); t2.v[0] -= ((int)t2[0]);  t2.v[1] -= ((int)t2[1]); }
					if (!i3) {bt->textureC(tv1,t3); t3.multiply( texscale); t3.v[0] -= ((int)t3[0]);  t3.v[1] -= ((int)t3[1]); }
				}
//				if (vbuf->colorOn())
				{
//					if (!i1) classify(&p1,&c1);
//					if (!i2) classify(&p2,&c2);
//					if (!i3) classify(&p3,&c3);
				}
				// Push them into the buffer.
				if (!i1) i1 = vbuf->pushVT(cp1,&t1);
				if (!i2) i2 = vbuf->pushVT(cp2,&t2);
				if (!i3) i3 = vbuf->pushVT(cp3,&t3);
 
				// Record that these vertices are in the buffer.
				bt->tri(tva)->vbufindex(i1); bt->tri(tva)->setvbufflag();
				bt->tri(tv0)->vbufindex(i2); bt->tri(tv0)->setvbufflag();
				bt->tri(tv1)->vbufindex(i3); bt->tri(tv1)->setvbufflag();
				vbuf->pushIndex(i1,i2,i3);
			}
			mesh->qsi ()->next ();
		}
		// Perform Qsort on VBuffer->_ibuf.
		vbuf->sort();
  }
   // Render
  int i;
  csVector3 *p1, *p2, *p3;
  ddgVector2 t1, t2, t3;
//  float  *c1, *c2, *c3;
  ddgVBIndex i1, i2, i3;
  csVector2 clipped_triangle [10];	//@@@BAD HARCODED!
  for (i = 0; i < vbuf->size(); i++)
  {
      int rescount;

	  i3 = vbuf->ibuf[i*3];
	  i2 = vbuf->ibuf[i*3+1];
	  i1 = vbuf->ibuf[i*3+2];
	  // Camera space coords.
	  p1 =	&(vbuf->vbuf[i1]);
	  p2 =	&(vbuf->vbuf[i2]);
	  p3 =	&(vbuf->vbuf[i3]);
//	  c1 =	&(vbuf->cbuf[i1]);
//	  c2 =	&(vbuf->cbuf[i2]);
//	  c3 =	&(vbuf->cbuf[i3]);
	  t1 =	&(vbuf->tbuf[i1]);
	  t2 =	&(vbuf->tbuf[i2]);
	  t3 =	&(vbuf->tbuf[i3]);

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

      if (!rview.view->Clip (triangle, clipped_triangle, 3, rescount))
		  continue;
      poly.num = rescount;

      poly.vertices[0].z = p1->z;
      poly.vertices[0].u = t1[0];
      poly.vertices[0].v = t1[1];
      poly.vertices[0].r = 1;//c1[0];
      poly.vertices[0].g = 1;//c1[1];
      poly.vertices[0].b = 1;//c1[2];

      poly.vertices[1].z = p2->z;
      poly.vertices[1].u = t2[0];
      poly.vertices[1].v = t2[1];
      poly.vertices[1].r = 1;//c2[0];
      poly.vertices[1].g = 1;//c2[1];
      poly.vertices[1].b = 1;//c2[2];

      poly.vertices[2].z = p3->z;
      poly.vertices[2].u = t3[0];
      poly.vertices[2].v = t3[1];
      poly.vertices[2].r = 1;//c3[0];
      poly.vertices[2].g = 1;//c3[1];
      poly.vertices[2].b = 1;//c3[2];
      
      PreparePolygonFX (&poly, clipped_triangle, rescount, triangle, true);
      rview.g3d->DrawPolygonFX (poly);
  }

  rview.g3d->FinishPolygonFX ();
}

