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

CSOBJTYPE_IMPL(csTerrain,csObject);

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

void csTerrain::classify( ddgVector3 *p, ddgVector3 *n, ddgColor3 *c)
{
	// Steep normal
    if (fabs(n->v[1]) < _cliffangle)
		c->set(_cliff); // Cliff
    else if (p->v[1] < _beachalt)
		c->set(_beach); // Beach
    else if (p->v[1] < _grassalt)
		c->set(_grass); // Grass
    else if (p->v[1] < _treealt)
		c->set(_trees); // Trees
    else if (p->v[1] < _rockalt)
		c->set(_rock);  // Rock
    else
		c->set(_snow);  // Snow
	
	static int cl = 0, b=0,g=0,t=0,r=0,s=0;
    if (fabs(n->v[1]) < _cliffangle)
		cl++;
    else if (p->v[1] < _beachalt)
		b++;
    else if (p->v[1] < _grassalt)
		g++;
    else if (p->v[1] < _treealt)
		t++;
    else if (p->v[1] < _rockalt)
		r++;
    else
		s++;
}

static csRenderView *grview = NULL;
// Perform coord transformation using CS instead of DDG.
static void transformer(ddgVector3 vin, ddgVector3 *vout)
{
	if (grview)
	{
		csVector3 csw1 = grview->World2Camera( csVector3(vin.v[0],vin.v[1],vin.v[2]) );
		vout->v[0] = csw1.x;
		vout->v[1] = csw1.y;
		vout->v[2] = csw1.z;
	}
	else
	{
		vout->v[0] = vin.v[0];
		vout->v[1] = vin.v[1];
		vout->v[2] = vin.v[2];
	}
}

// Z depth queue iterator
static ddgSplayIterator		*qri;
// Z depth queue.
static ddgQueue			*qr;

bool csTerrain::Initialize (char* heightmap)
{

  CHK (height = new ddgHeightMap ());
  if (height->readTGN (heightmap))
	  height->generateHeights(257,257,5);
  CHK (mesh = new ddgTBinMesh (height));
  CHK (clipbox = new ddgBBox (ddgVector3(0,0,1),ddgVector3(640, 480, 15000)));

  float fov = 90.0;
  mesh->settransform(transformer); 
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

void csTerrain::Draw (csRenderView& rview, bool use_z_buf)
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

  	if ( 1 /*!use_z_buf*/ )
	{
		mesh->qzi()->reset();
		// Clear queue.
		mesh->qz()->clear();
		mesh->qsi()->reset();
		while (!mesh->qsi()->end())
		{
			ddgTriIndex tvc = mesh->qs ()->index (mesh->qsi ()), tvp, tv0, tv1;
            float d;
			ddgTBinTree *bt = mesh->qs ()->tree (mesh->qsi ());

            tvp = ddgTBinTree::parent(tvc);
            tv0 = mesh->stri[tvc].v0;
            tv1 = mesh->stri[tvc].v1;
			ddgVector3 *p = bt->tri(tv0)->pos();
    		// Use the nearest coord of the triangle.
            d = p->v[2];
			p = bt->tri(tv1)->pos();
            if (p->v[2] < d)
                d = p->v[2];
			p = bt->tri(tvp)->pos();
            if (p->v[2] < d)
                d = p->v[2];
            mesh->qz()->ddgSplayTree::insert (bt->index(),tvc,(unsigned int)d);
			mesh->qsi ()->next ();
		}
		qri = mesh->qzi();
		qr  = mesh->qz();
	}
	else
	{
		qri = mesh->qsi ();
		qr  = mesh->qs ();
	}

   // Render
  qri->reset ();
  while (!qri->end ())
  {
    ddgTriIndex tvc = qr->index (qri );
    ddgTBinTree *bt = qr->tree (qri );

    if (!bt->tri (tvc)->vis ().flags.none)
    {

      ddgVector3 *p1, *p2, *p3;
      ddgVector3 wp1, wp2, wp3;
      ddgVector3 n1, n2, n3;
      ddgVector2 t1, t2, t3;
      ddgColor3  c1, c2, c3;
	  unsigned int tvp = bt->parent (tvc),
		  tv0 = bt->v0 (tvc),
		  tv1 = bt->v1 (tvc);
	  // Camera space coords.
	  p3 =	bt->tri(tvp)->pos();
	  p2 =	bt->tri(tv0)->pos();
	  p1 =	bt->tri(tv1)->pos();
	  // World space coords.
      bt->vertex(tvp,&wp1);
      bt->vertex(tv0,&wp2);
      bt->vertex(tv1,&wp3);
	  // Normals.
	  bt->normal(tvp,&n1);
	  bt->normal(tv0,&n2);
	  bt->normal(tv1,&n3);
      // Texture coords from 0 - N where N is size of texture.
	  bt->textureC(tvp,t1);
	  bt->textureC(tv0,t2);
	  bt->textureC(tv1,t3);
	  // Create some color scheme.
	  classify(wp1,&n1,&c1);
	  classify(wp2,&n2,&c2);
	  classify(wp3,&n3,&c3);

      float iz;
      float pz[3];
      csVector2 triangle[3];
      if (p1->v[2] < SMALL_Z) goto not_vis;
      pz[0] = 1 / p1->v[2];
      iz = rview.aspect * pz[0];
      triangle[0].x = p1->v[0] * iz + rview.shift_x;
      triangle[0].y = p1->v[1] * iz + rview.shift_x;
      if (p2->v[2] < SMALL_Z) goto not_vis;
      pz[1] = 1 / p2->v[2];
      iz = rview.aspect * pz[1];
      triangle[1].x = p2->v[0] * iz + rview.shift_x;
      triangle[1].y = p2->v[1] * iz + rview.shift_x;
      if (p3->v[2] < SMALL_Z) goto not_vis;
      pz[2] = 1 / p3->v[2];
      iz = rview.aspect * pz[2];
      triangle[2].x = p3->v[0] * iz + rview.shift_x;
      triangle[2].y = p3->v[1] * iz + rview.shift_x;

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
    qri ->next ();
  }

  rview.g3d->FinishPolygonFX ();
}

