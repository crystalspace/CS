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
#include "cssys/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/box.h"
#include "csgeom/math3d.h"
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
}

csExactCuller::~csExactCuller ()
{
  delete[] scr_buffer;
  delete[] z_buffer;
  delete[] objects;
}

void csExactCuller::InsertPolygon (csVector2* tr_verts, float* tr_invz,
	int* vi, int num_verts, uint32 obj_number, int& totpix)
{
  totpix = 0;

  int i;
  int min_i, max_i;
  min_i = max_i = 0;
  float min_y, max_y;
  min_y = max_y = tr_verts[vi[0]].y;
  // count 'real' number of vertices
  int real_num_verts = 1;
  for (i = 1 ; i < num_verts ; i++)
  {
    if (tr_verts[vi[i]].y > max_y)
    {
      max_y = tr_verts[vi[i]].y;
      max_i = i;
    }
    else if (tr_verts[vi[i]].y < min_y)
    {
      min_y = tr_verts[vi[i]].y;
      min_i = i;
    }
    if ((ABS (tr_verts [vi[i]].x - tr_verts [vi[i - 1]].x)
       + ABS (tr_verts [vi[i]].y - tr_verts [vi[i - 1]].y))
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
  sy = fyL = fyR = QRound (tr_verts [vi[scanL2]].y);
  float sinvzL, dinvzL;
  float sinvzR, dinvzR;
  float invz;

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
        fyR = QRound (tr_verts [vi[scanR2]].y);
        if (sy <= fyR)
          continue;

        float dyR = (tr_verts [vi[scanR1]].y - tr_verts [vi[scanR2]].y);
        if (dyR)
        {
          sxR = tr_verts [vi[scanR1]].x;
          dxR = (tr_verts [vi[scanR2]].x - sxR) / dyR;
	  sinvzR = tr_invz [vi[scanR1]];
          dinvzR = (tr_invz [vi[scanR2]] - sinvzR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (tr_verts [vi[scanR1]].y - (float (sy) - 0.5));
        }
      }
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = num_verts - 1;

        leave = false;
        fyL = QRound (tr_verts [vi[scanL2]].y);
        if (sy <= fyL)
          continue;

        float dyL = (tr_verts [vi[scanL1]].y - tr_verts [vi[scanL2]].y);
        if (dyL)
        {
          sxL = tr_verts [vi[scanL1]].x;
          dxL = (tr_verts [vi[scanL2]].x - sxL) / dyL;
	  sinvzL = tr_invz [vi[scanL1]];
          dinvzL = (tr_invz [vi[scanL2]] - sinvzL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (tr_verts [vi[scanL1]].y - (float (sy) - 0.5));
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
        xL = QRound (sxL);
        xR = QRound (sxR);
	if (xR >= 0 && xL < width && xR != xL)
	{
	  invz = sinvzL;
	  float dinvz = (sinvzR - sinvzL) / (sxR - sxL);
	  if (xL < 0) { invz += dinvz * (-xL); xL = 0; }
	  if (xR >= width) xR = width-1;
          uint32* scr_buf = scr_buffer + width * screenY + xL;
          float* z_buf = z_buffer + width * screenY + xL;

	  int xx = xR - xL;
	  if (xx > 0)
	  {
	    do
  	    {
	      if (*scr_buf != obj_number) totpix++;
	      float z = 1.0 / invz;
	      if (z < *z_buf)
	      {
	        *scr_buf = obj_number;
		*z_buf = z;
	      }
	      z_buf++;
	      scr_buf++;
	      invz += dinvz;
	      xx--;
	    }
	    while (xx);
	  }
        }
      }

      sxL += dxL;
      sxR += dxR;
      sinvzL += dinvzL;
      sinvzR += dinvzR;
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
  // First transform all vertices.
  csVector2* tr_verts = new csVector2[vertex_count];
  float* tr_invz = new float[vertex_count];
  bool* vis = new bool[vertex_count];
  for (i = 0 ; i < vertex_count ; i++)
  {
    csVector3 camv = trans.Other2This (verts[i]);
    if (camv.z > 0.0001)
    {
      Perspective (camv, tr_verts[i], fov, sx, sy);
      tr_invz[i] = 1.0 / camv.z;
      vis[i] = true;
    }
    else
      vis[i] = false;
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
    bool use = true;
    for (j = 0 ; j < num_verts ; j++)
    {
      if (!vis[vi[j]])
      {
        // @@@ Later we should clamp instead of ignoring this polygon.
	use = false;
	break;
      }
    }
    if (use)
    {
      int totpix;
      InsertPolygon (tr_verts, tr_invz, vi, num_verts,
        num_objects-1, totpix);
      objects[num_objects-1].totpix += totpix;
    }
  }

  delete[] vis;
  delete[] tr_invz;
  delete[] tr_verts;
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

