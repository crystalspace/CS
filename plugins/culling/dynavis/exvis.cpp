/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csutil/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "csqint.h"
#include "csqsqrt.h"
#include "csgeom/box.h"
#include "csgeom/math3d.h"
#include "csgeom/poly3d.h"
#include "csgeom/poly2d.h"
#include "csgeom/polyclip.h"
#include "igeom/polymesh.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "exvis.h"

//---------------------------------------------------------------------------

csExactCuller::csExactCuller (int w, int h)
{
  width = w;
  height = h;
  scr_buffer = new uint32[w*h];
  z_buffer = new float[w*h];
  for (int i = 0 ; i < w*h ; i++)
  {
    scr_buffer[i] = ~0;
    z_buffer[i] = 1000000000000.0f;
  }

  num_objects = 0;
  max_objects = 100;
  objects = new csExVisObj [max_objects];

  boxclip = new csBoxClipper (0, 0, float (w), float (h));
}

csExactCuller::~csExactCuller ()
{
  delete boxclip;
  delete[] scr_buffer;
  delete[] z_buffer;
  delete[] objects;
}

void csExactCuller::InsertPolygon (csVector2* tr_verts, int num_verts,
	float M, float N, float O, uint32 obj_number, int& totpix)
{
  totpix = 0;

  int i;
  int min_i, max_i;
  min_i = max_i = 0;
  float min_y, max_y;
  min_y = max_y = tr_verts[0].y;
  // count 'real' number of vertices
  int real_num_verts = 1;
  for (i = 1 ; i < num_verts ; i++)
  {
    if (tr_verts[i].y > max_y)
    {
      max_y = tr_verts[i].y;
      max_i = i;
    }
    else if (tr_verts[i].y < min_y)
    {
      min_y = tr_verts[i].y;
      min_i = i;
    }
    if ((ABS (tr_verts [i].x - tr_verts [i - 1].x)
       + ABS (tr_verts [i].y - tr_verts [i - 1].y))
       	> 0.001)
      real_num_verts++;
  }

  // If this is a 'degenerate' polygon, skip it.
  if (real_num_verts < 3)
    return;

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y l, final Y r
  int xL, xR;
  int screenY;

  sxL = sxR = dxL = dxR = 0;            // Avoid warnings about "uninit vars"
  scanL2 = scanR2 = max_i;
  sy = fyL = fyR = csQround (tr_verts [scanL2].y);

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
        if (scanR2 == min_i)
	  return;
        scanR1 = scanR2;
	if (++scanR2 >= num_verts)
	  scanR2 = 0;

        leave = false;
        fyR = csQround (tr_verts [scanR2].y);
        if (sy <= fyR)
          continue;

        float dyR = (tr_verts [scanR1].y - tr_verts [scanR2].y);
        if (dyR)
        {
          sxR = tr_verts [scanR1].x;
          dxR = (tr_verts [scanR2].x - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (tr_verts [scanR1].y - (float (sy) - 0.5));
        }
      }
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = num_verts - 1;

        leave = false;
        fyL = csQround (tr_verts [scanL2].y);
        if (sy <= fyL)
          continue;

        float dyL = (tr_verts [scanL1].y - tr_verts [scanL2].y);
        if (dyL)
        {
          sxL = tr_verts [scanL1].x;
          dxL = (tr_verts [scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (tr_verts [scanL1].y - (float (sy) - 0.5));
        }
      }
    } while (!leave);

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    screenY = height - sy;

    while (sy > fin_y)
    {
      if (screenY >= 0 && screenY < height)
      {
        // Compute the rounded screen coordinates of horizontal strip
        xL = csQround (sxL);
        xR = csQround (sxR);
	if (xR >= 0 && xL < width && xR != xL)
	{
	  if (xL < 0) xL = 0;
	  if (xR >= width) xR = width-1;
          uint32* scr_buf = scr_buffer + width * screenY + xL;
          float* z_buf = z_buffer + width * screenY + xL;
	  float invz = M * (xL-width/2) + N * (sy-height/2) + O;

	  int xx = xR - xL;
	  if (xx > 0)
	  {
	    do
  	    {
	      if (*scr_buf != obj_number) totpix++;
	      float z;
	      if (ABS (invz) > 0.001)
	        z = 1.0 / invz;
	      else
	        z = 9999999999999.0f;
	      if (z < *z_buf)
	      {
	        *scr_buf = obj_number;
		*z_buf = z;
	      }
	      z_buf++;
	      scr_buf++;
	      invz += M;
	      xx--;
	    }
	    while (xx);
	  }
        }
      }

      sxL += dxL;
      sxR += dxR;
      sy--;
      screenY++;
    }
  }
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

void csExactCuller::AddObject (void* obj,
	iPolygonMesh* polymesh, iMovable* movable, iCamera* camera,
	const csPlane3* planes)
{
  if (num_objects >= max_objects)
  {
    if (max_objects < 10000)
      max_objects += max_objects+1;
    else
      max_objects += 2000;
    csExVisObj* new_objects = new csExVisObj [max_objects];
    memcpy (new_objects, objects, sizeof (csExVisObj)*num_objects);
    delete[] objects;
    objects = new_objects;
  }

  objects[num_objects].obj = obj;
  objects[num_objects].totpix = 0;
  objects[num_objects].vispix = 0;
  num_objects++;

  const csVector3* verts = polymesh->GetVertices ();
  int vertex_count = polymesh->GetVertexCount ();
  int poly_count = polymesh->GetPolygonCount ();

  csReversibleTransform movtrans = movable->GetFullTransform ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  csReversibleTransform trans = camtrans / movtrans;
  float fov = camera->GetFOV ();
  float sx = camera->GetShiftX ();
  float sy = camera->GetShiftY ();

  // Calculate camera position in object space.
  csVector3 campos_object = movtrans.Other2This (camtrans.GetOrigin ());

  int i;
  // First check visibility of all vertices.
  bool* vis = new bool[vertex_count];
  for (i = 0 ; i < vertex_count ; i++)
  {
    csVector3 camv = trans.Other2This (verts[i]);
    vis[i] = (camv.z > 0.1);
  }

  // Then insert all polygons.
  csMeshedPolygon* poly = polymesh->GetPolygons ();
  for (i = 0 ; i < poly_count ; i++, poly++)
  {
    if (planes[i].Classify (campos_object) >= 0.0)
      continue;

    int num_verts = poly->num_vertices;
    int* vi = poly->vertices;
    int j;
    int cnt_vis = 0;
    for (j = 0 ; j < num_verts ; j++)
    {
      if (vis[vi[j]]) cnt_vis++;
    }
    if (cnt_vis > 0)
    {
      // Here we need to clip the polygon.
      csPoly3D clippoly;
      for (j = 0 ; j < num_verts ; j++)
      {
        csVector3 camv = trans.Other2This (verts[vi[j]]);
        clippoly.AddVertex (camv);
      }
      csPoly3D front, back;
      csPoly3D* spoly;
      if (cnt_vis < num_verts)
      {
        clippoly.SplitWithPlaneZ (front, back, 0.1f);
	spoly = &back;
      }
      else
      {
        spoly = &clippoly;
      }
      csVector2 clipped[100];
      size_t num_clipped = spoly->GetVertexCount ();
      csBox2 out_box;
      out_box.StartBoundingBox ();
      for (size_t k = 0 ; k < spoly->GetVertexCount () ; k++)
      {
        Perspective ((*spoly)[k], clipped[k], fov, sx, sy);
	out_box.AddBoundingVertex (clipped[k]);
      }
      if (boxclip->ClipInPlace (clipped, num_clipped, out_box)
      	!= CS_CLIP_OUTSIDE)
      {
	//csPlane3 camplane = trans.Other2This (planes[i]);
        csPlane3 camplane;
	trans.Other2This (planes[i], (*spoly)[0], camplane);
//printf ("    %g,%g,%g\n", (*spoly)[0].x, (*spoly)[0].y, (*spoly)[0].z);
//printf ("    planes[i] %g,%g,%g,%g\n",
//planes[i].A (), planes[i].B (), planes[i].C (), planes[i].D ());
//printf ("    camplane %g,%g,%g,%g\n",
//camplane.A (), camplane.B (), camplane.C (), camplane.D ());
	if (ABS (camplane.D ()) < 0.001)
	  continue;
	float M, N, O;
	float inv_D = 1.0 / camplane.D ();
	M = -camplane.A () * inv_D / fov;
	N = -camplane.B () * inv_D / fov;
	O = -camplane.C () * inv_D;
//printf ("    MNO %g,%g,%g\n", M, N, O);
//fflush (stdout);

        int totpix;
        InsertPolygon (clipped, num_clipped, M, N, O,
		num_objects-1, totpix);
        objects[num_objects-1].totpix += totpix;
      }
    }
  }

  delete[] vis;
}

void csExactCuller::VisTest ()
{
  int i;
  for (i = 0 ; i < num_objects ; i++)
    objects[i].vispix = 0;

  for (i = 0 ; i < width * height ; i++)
  {
    uint32 obj_num = scr_buffer[i];
    if (obj_num < (uint32)num_objects)
    {
      objects[obj_num].vispix++;
    }
  }
}

void csExactCuller::GetObjectStatus (void* obj, int& vispix, int& totpix)
{
  int i;
  for (i = 0 ; i < num_objects ; i++)
    if (objects[i].obj == obj)
    {
      vispix = objects[i].vispix;
      totpix = objects[i].totpix;
      return;
    }
  CS_ASSERT (false);
}

