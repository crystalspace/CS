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
}

csTerrain::~csTerrain ()
{
  CHK (delete mesh);
  CHK (delete height);
  CHK (delete clipbox);
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
	
	static int b=0,g=0,t=0,r=0,s=0;
    if (p->y < _beachalt)
		b++;
    else if (p->y < _grassalt)
		g++;
    else if (p->y < _treealt)
		t++;
    else if (p->y < _rockalt)
		r++;
    else
		s++;
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
  CHK (clipbox = new ddgBBox (csVector3(0,0,1),csVector3(640, 480, 15000)));

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

  memset (&poly, 0, sizeof(poly));
  poly.inv_aspect = rview.inv_aspect;
  poly.flat_color_r = 1;
  poly.flat_color_g = 1;
  poly.flat_color_b = 1;
  poly.txt_handle = _textureMap->GetTextureHandle ();
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, false /*use_z_buf*/);
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  rview.g3d->StartPolygonFX (poly.txt_handle, CS_FX_GOURAUD);
  grview = &rview;
  // For each frame.
  mesh->calculate ();

	// If we are not using Z-buffer...
    // For all the triangle in the visible queue, insert them in z-order.

	mesh->qzi()->reset();
	// Clear queue.
	mesh->qz()->clear();
	mesh->qsi()->reset();
	while (!mesh->qsi()->end())
	{
		ddgTriIndex tvc = mesh->indexSQ(mesh->qsi()), tvp, tv0, tv1;
        float d;
		ddgTBinTree *bt = mesh->treeSQ(mesh->qsi());

        tvp = ddgTBinTree::parent(tvc);
        tv0 = mesh->stri[tvc].v0;
        tv1 = mesh->stri[tvc].v1;
		csVector3 *p = bt->pos(tv0);
    	// Use the nearest coord of the triangle.
        d = p->z;
		p = bt->pos(tv1);
        if (p->z < d)
            d = p->z;
		p = bt->pos(tvp);
        if (p->z < d)
            d = p->z;
        mesh->qz()->ddgSplayTree::insert (bt->index(),tvc,(unsigned int)d);
		mesh->qsi ()->next ();
	}

   // Render
  mesh->qzi()->reset (true);
  while (!mesh->qzi()->end ())
  {
    ddgTriIndex tvc = mesh->indexZQ (mesh->qzi());
    ddgTBinTree *bt = mesh->treeZQ (mesh->qzi());

    if (!bt->tri (tvc)->vis ().flags.none)
    {

      csVector3 *p1, *p2, *p3;
      csVector3 wp1, wp2, wp3;
      ddgVector2 t1, t2, t3;
      ddgColor3  c1, c2, c3;
	  unsigned int tvp = bt->parent (tvc),
		  tv0 = bt->v0 (tvc),
		  tv1 = bt->v1 (tvc);
	  // Camera space coords.
	  p3 =	bt->pos(tvp);
	  p2 =	bt->pos(tv0);
	  p1 =	bt->pos(tv1);
	  // World space coords.
      bt->vertex(tvp,&wp1);
      bt->vertex(tv0,&wp2);
      bt->vertex(tv1,&wp3);
	  // Normals.
      // Texture coords from 0 - N where N is size of texture.
	  bt->textureC(tvp,t1);
	  bt->textureC(tv0,t2);
	  bt->textureC(tv1,t3);
	  // Create some color scheme.
	  classify(&wp1,&c1);
	  classify(&wp2,&c2);
	  classify(&wp3,&c3);

      float iz;
      float pz[3];
      csVector2 triangle[3];
      if (p1->z < SMALL_Z) goto not_vis;
      pz[0] = 1 / p1->z;
      iz = rview.aspect * pz[0];
      triangle[0].x = p1->x * iz + rview.shift_x;
      triangle[0].y = p1->y * iz + rview.shift_y;
      if (p2->z < SMALL_Z) goto not_vis;
      pz[1] = 1 / p2->z;
      iz = rview.aspect * pz[1];
      triangle[1].x = p2->x * iz + rview.shift_x;
      triangle[1].y = p2->y * iz + rview.shift_y;
      if (p3->z < SMALL_Z) goto not_vis;
      pz[2] = 1 / p3->z;
      iz = rview.aspect * pz[2];
      triangle[2].x = p3->x * iz + rview.shift_x;
      triangle[2].y = p3->y * iz + rview.shift_y;

      csVector2 clipped_triangle [10];	//@@@BAD HARCODED!
      int rescount;
      if (!rview.view->Clip (triangle, clipped_triangle, 3, rescount)) goto not_vis;
      poly.num = rescount;

      poly.vertices[0].z = pz[0];
      poly.vertices[0].u = 0;
      poly.vertices[0].v = 0;
      poly.vertices[0].r = (float)c1.v[0]/255.0;
      poly.vertices[0].g = (float)c1.v[1]/255.0;
      poly.vertices[0].b = (float)c1.v[2]/255.0;

      poly.vertices[1].z = pz[1];
      poly.vertices[1].u = 1;
      poly.vertices[1].v = 0;
      poly.vertices[1].r = (float)c2.v[0]/255.0;
      poly.vertices[1].g = (float)c2.v[1]/255.0;
      poly.vertices[1].b = (float)c2.v[2]/255.0;

      poly.vertices[2].z = pz[2];
      poly.vertices[2].u = 0;
      poly.vertices[2].v = 1;
      poly.vertices[2].r = (float)c3.v[0]/255.0;
      poly.vertices[2].g = (float)c3.v[1]/255.0;
      poly.vertices[2].b = (float)c3.v[2]/255.0;
      
      PreparePolygonFX (&poly, clipped_triangle, rescount, triangle,
      	true);
      rview.g3d->DrawPolygonFX (poly);
    }

    not_vis:
    mesh->qzi() ->prev ();
  }

  rview.g3d->FinishPolygonFX ();
}

