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

bool csTerrain::Initialize (char* heightmap)
{
  CHK (height = new ddgHeightMap ());
  if (height->readTGN (heightmap))
	  height->generateHeights(257,257,5);
  CHK (mesh = new ddgTBinMesh (height));
  CHK (clipbox = new ddgBBox (ddgVector3(0,0,1),ddgVector3(640, 480, 15000)));

  float fov = 90.0;
  mesh->settransform(transformer); 
  mesh->init (wtoc, clipbox, fov);
  return true;
}

void csTerrain::Draw (csRenderView& rview, bool use_z_buf)
{
  // Initialize.
  const csMatrix3& m_o2t = rview.GetO2T ();
  const csVector3& v_o2t = rview.GetO2TTranslation ();

  G3DPolygonDPFX poly;
  memset (&poly, 0, sizeof(poly));
  poly.inv_aspect = rview.inv_aspect;
  poly.flat_color_r = 1;
  poly.flat_color_g = 1;
  poly.flat_color_b = 1;
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, use_z_buf);
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  rview.g3d->StartPolygonFX (poly.txt_handle, CS_FX_GOURAUD);
  grview = &rview;
  // For each frame.
  mesh->calculate ();
  // Render
  mesh->qsi()->reset ();
  while (!mesh->qsi ()->end ())
  {
    ddgTriIndex tvc = mesh->qs ()->index (mesh->qsi ());
    ddgTBinTree *bt = mesh->qs ()->tree (mesh->qsi ());

    if (!bt->tri (tvc)->vis ().flags.none)
    {

      ddgVector3 *p1, *p2, *p3;
/*
		ddgVector3 w1, w2, w3;
		bt->vertex(bt->v0 (tvc),&w1);
		bt->vertex(bt->v1 (tvc),&w2);
		bt->vertex(bt->parent (tvc),&w3);
		p1 = &w1;
		p2 = &w2;
		p3 = &w3;
*/

	  p3 =	bt->tri(bt->parent (tvc))->pos();
	  p2 =	bt->tri(bt->v1 (tvc))->pos();
	  p1 =	bt->tri(bt->v0 (tvc))->pos();
	  
      // DDG vectors work with negative Z pointing forwards.
      float iz;
      float pz[3];
      csVector2 triangle[3];
      if (p1->v[2] < SMALL_Z) goto not_vis;
      pz[0] = 1 / -p1->v[2];
      iz = rview.aspect * pz[0];
      triangle[0].x = p1->v[0] * iz + rview.shift_x;
      triangle[0].y = p1->v[1] * iz + rview.shift_x;
      if (p2->v[2] < SMALL_Z) goto not_vis;
      pz[1] = 1 / -p2->v[2];
      iz = rview.aspect * pz[1];
      triangle[1].x = p2->v[0] * iz + rview.shift_x;
      triangle[1].y = p2->v[1] * iz + rview.shift_x;
      if (p3->v[2] < SMALL_Z) goto not_vis;
      pz[2] = 1 / -p3->v[2];
      iz = rview.aspect * pz[2];
      triangle[2].x = p3->v[0] * iz + rview.shift_x;
      triangle[2].y = p3->v[1] * iz + rview.shift_x;

      csVector2 clipped_triangle [10];	//@@@BAD HARCODED!
      int rescount;
      if (!rview.view->Clip (triangle, clipped_triangle, 3, rescount)) goto not_vis;
      poly.num = rescount;

      poly.vertices[0].z = 1.0/pz[0];
      poly.vertices[0].u = 0;
      poly.vertices[0].v = 0;
      poly.vertices[0].r = 1;
      poly.vertices[0].g = 1;
      poly.vertices[0].b = 1;

      poly.vertices[1].z = 1.0/pz[1];
      poly.vertices[1].u = 1;
      poly.vertices[1].v = 0;
      poly.vertices[1].r = 1;
      poly.vertices[1].g = 0;
      poly.vertices[1].b = 0;

      poly.vertices[2].z = 1.0/pz[2];
      poly.vertices[2].u = 0;
      poly.vertices[2].v = 1;
      poly.vertices[2].r = 0;
      poly.vertices[2].g = 0;
      poly.vertices[2].b = 1;
      
      PreparePolygonFX (&poly, clipped_triangle, rescount, triangle,
      	true);
      rview.g3d->DrawPolygonFX (poly);
    }

    not_vis:
    mesh->qsi ()->next ();
  }
	{
	// Dummy triangle.
      csVector2 triangle[3];
      csVector2 clipped_triangle [10];	//@@@BAD HARCODED!
	  int rescount = 3;

      triangle[0].x = 0;
      triangle[0].y = 0;
      clipped_triangle[0] = triangle[0];
      poly.vertices[0].z = 1;
      poly.vertices[0].u = 0;
      poly.vertices[0].v = 0;
      poly.vertices[0].r = 1;
      poly.vertices[0].g = 1;
      poly.vertices[0].b = 1;

      triangle[1].x = 0;
      triangle[1].y = 30;
      clipped_triangle[1] = triangle[1];
      poly.vertices[1].z = 1;
      poly.vertices[1].u = 1;
      poly.vertices[1].v = 0;
      poly.vertices[1].r = 1;
      poly.vertices[1].g = 0;
      poly.vertices[1].b = 0;

      triangle[2].x = 30;
      triangle[2].y = 0;
      clipped_triangle[2] = triangle[2];
      poly.vertices[2].z = 1;
      poly.vertices[2].u = 0;
      poly.vertices[2].v = 1;
      poly.vertices[2].r = 0;
      poly.vertices[2].g = 0;
      poly.vertices[2].b = 1;
      poly.num = 3;

      PreparePolygonFX (&poly, clipped_triangle, rescount, triangle,
      	true);
      rview.g3d->DrawPolygonFX (poly);
	}
  rview.g3d->FinishPolygonFX ();
}

