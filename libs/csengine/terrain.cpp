/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

bool csTerrain::Initialize (char* heightmap)
{
  CHK (height = new ddgHeightMap ());
  if (height->readTGN (heightmap)) return false;
  CHK (mesh = new ddgTBinMesh (height));
  CHK (clipbox = new ddgBBox (ddgVector3(0,0,1),ddgVector3(640, 480, 15000)));
  wtoc[0] = 1;
  wtoc[1] = 0;
  wtoc[2] = 0;
  wtoc[3] = 0;
  wtoc[4] = 0;
  wtoc[5] = 1;
  wtoc[6] = 0;
  wtoc[7] = 0;
  wtoc[8] = 0;
  wtoc[9] = 0;
  wtoc[10] = 1;
  wtoc[11] = 0;
  wtoc[12] = 0;
  wtoc[13] = 0;
  wtoc[14] = 0;
  wtoc[15] = 1;  float fov = 90.0;
  mesh->init (wtoc, clipbox, fov);
  return true;
}

void csTerrain::Draw (csRenderView& rview, bool use_z_buf)
{
  // Initialize.
  const csMatrix3& m_o2t = rview.GetO2T ();
  const csVector3& v_o2t = rview.GetO2TTranslation ();
  wtoc[0] = m_o2t.m11;
  wtoc[4] = m_o2t.m12;
  wtoc[8] = m_o2t.m13;
  wtoc[12] = v_o2t.x;
  wtoc[1] = m_o2t.m21;
  wtoc[5] = m_o2t.m22;
  wtoc[9] = m_o2t.m23;
  wtoc[13] = v_o2t.y;
  wtoc[2] = m_o2t.m31;
  wtoc[6] = m_o2t.m32;
  wtoc[10] = m_o2t.m33;
  wtoc[14] = v_o2t.z;
  wtoc[3] = 0;
  wtoc[7] = 0;
  wtoc[11] = 0;
  wtoc[15] = 1;
  float fov = 90.0;

  G3DPolygonDPFX poly;
  memset (&poly, 0, sizeof(poly));
  poly.inv_aspect = rview.inv_aspect;
  poly.flat_color_r = 1;
  poly.flat_color_g = 1;
  poly.flat_color_b = 1;
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, true);
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, use_z_buf);
  rview.g3d->StartPolygonFX (poly.txt_handle, CS_FX_GOURAUD);

  // For each frame.
  // Update the wtoc.
  mesh->calculate ();
  // Render
  mesh->qsi()->reset ();
  while (!mesh->qsi ()->end ())
  {
    ddgTriIndex tvc = mesh->qs ()->index (mesh->qsi ());
    ddgTBinTree *bt = mesh->qs ()->tree (mesh->qsi ());

    if (!bt->tri (tvc)->vis ().flags.none)
    {
      ddgVector3 p1, p2, p3;
      ddgVector3 n1, n2, n3;
      ddgVector2 t1, t2, t3;
      unsigned int i1, i2, i3;

      ddgTriIndex tva = bt->parent (tvc);
      ddgTriIndex tv1 = bt->v0 (tvc);
      ddgTriIndex tv0 = bt->v1 (tvc);
      i1 = bt->vertex (tva, &p1);
      i2 = bt->vertex (tv0, &p2);
      i3 = bt->vertex (tv1, &p3);

      float iz;
      float pz[3];
      csVector2 triangle[3];
      if (p1.v[2] < SMALL_Z) continue;
      pz[0] = 1 / p1.v[2];
      iz = rview.aspect * pz[0];
      triangle[0].x = p1.v[0] * iz + rview.shift_x;
      triangle[0].y = p1.v[1] * iz + rview.shift_x;
      if (p2.v[2] < SMALL_Z) continue;
      pz[1] = 1 / p2.v[2];
      iz = rview.aspect * pz[1];
      triangle[1].x = p2.v[0] * iz + rview.shift_x;
      triangle[1].y = p2.v[1] * iz + rview.shift_x;
      if (p3.v[2] < SMALL_Z) continue;
      pz[2] = 1 / p3.v[2];
      iz = rview.aspect * pz[2];
      triangle[2].x = p3.v[0] * iz + rview.shift_x;
      triangle[2].y = p3.v[1] * iz + rview.shift_x;

      csVector2 clipped_triangle [10];	//@@@BAD HARCODED!
      int rescount;
      if (!rview.view->Clip (triangle, clipped_triangle, 3, rescount)) continue;

      poly.vertices[0].z = pz[0];
      poly.vertices[0].u = 0;
      poly.vertices[0].v = 0;
      poly.vertices[0].r = 0;
      poly.vertices[0].g = 0;
      poly.vertices[0].b = 0;

      poly.vertices[1].z = pz[1];
      poly.vertices[1].u = 1;
      poly.vertices[1].v = 0;
      poly.vertices[1].r = 1;
      poly.vertices[1].g = 0;
      poly.vertices[1].b = 0;

      poly.vertices[2].z = pz[2];
      poly.vertices[2].u = 0;
      poly.vertices[2].v = 1;
      poly.vertices[2].r = 0;
      poly.vertices[2].g = 0;
      poly.vertices[2].b = 1;
      
      PreparePolygonFX (&poly, clipped_triangle, rescount, triangle,
      	true);
      rview.g3d->DrawPolygonFX (poly);
    }

    mesh->qsi ()->next ();
  }

  rview.g3d->FinishPolygonFX ();
}

