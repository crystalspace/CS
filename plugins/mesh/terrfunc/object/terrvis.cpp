/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
#include "terrvis.h"
#include "csqint.h"
#include "csqsqrt.h"

//------------------------------------------------------------------------

int csTerrainQuad::global_visnr = 0;

csTerrainQuad::csTerrainQuad ()
{
  children[0] = 0;
  children[1] = 0;
  children[2] = 0;
  children[3] = 0;
  visnr = -1;
}

csTerrainQuad::~csTerrainQuad ()
{
  delete children[0];
  delete children[1];
  delete children[2];
  delete children[3];
}

void csTerrainQuad::Build (int depth)
{
  if (depth <= 0) return;
  children[0] = new csTerrainQuad ();
  children[0]->Build (depth-1);
  children[1] = new csTerrainQuad ();
  children[1]->Build (depth-1);
  children[2] = new csTerrainQuad ();
  children[2]->Build (depth-1);
  children[3] = new csTerrainQuad ();
  children[3]->Build (depth-1);
}

/// smallest value for horizon
#define MAXINF (99999999.0f)
#define MININF (-MAXINF)

void csTerrainQuad::InitHorizon (float *horizon, int horsize)
{
  int i;
  for(i=0; i<horsize; i++)
    horizon[i] = MININF;
}

static int horidx_table_horsize = 0;
static int horidx_table[259];
static void BuildHorIndexTable (int horsize)
{
  if (horidx_table_horsize == horsize) return;
  horidx_table_horsize = horsize;

  // Circle is split into horsize parts, each of size k (of 2*PI total)
  // the x-axis is right in the middle of parts 0 and horsize/2
  // thus precision around z==0 is not so important
  float k = TWO_PI / float(horsize);
  int i;
  for (i = -127 ; i <= 127 ; i++)
  {
    float cosinus = float (i)/127.0;
    if (cosinus < -1) cosinus = -1;
    else if (cosinus > 1) cosinus = 1;
    float invcos = acos(cosinus);
    int idx;
    if (invcos < k*0.5) idx = 0;
    else if (invcos > PI - k*0.5) idx = horsize/2;
    else idx = csQint ((invcos + k*0.5)/k);
    horidx_table[i+128] = idx;
    CS_ASSERT (idx >= 0 && idx < horsize);
  }
  // Important: the values at -128 and 128 are boundary
  // values to prevent overflow. They are set to the same
  // values as -127 and 127.
  horidx_table[-128+128] = horidx_table[-127+128];
  horidx_table[128+128] = horidx_table[127+128];
}

int csTerrainQuad::GetHorIndex(const csVector3& campos, float x, float z,
  int horsize)
{
  BuildHorIndexTable (horsize);

  x -= campos.x;
  z -= campos.z;
  float invlen = csQisqrt (x*x + z*z);
  float cosinus = x * invlen;
  int icos = csQint (cosinus * 127.0);
  CS_ASSERT (icos >= -128 && icos <= 128);
  int idx = horidx_table[icos+128];
  if (z < 0 && idx) idx = horsize - idx;
  return idx;
}

void csTerrainQuad::ComputeExtent(const csVector3& campos, const csBox3& bbox,
    int horsize, int& left, int& right)
{
  // First we check in which quadrant our camera is relative to the box
  // (as seen in 2D: only x and z components). Using that information we
  // know what vertex of the box is at the left side and what vertex is
  // at the right side of our extent. Then we can compute the angles of
  // those two vertices.

  // In the code below we compute the left index which is compatible
  // with CS_BOX_CORNER_xyz (for csBox3::GetCorner()).
  int left_idx;
  if (campos.x < bbox.MinX ())
  {
    if (campos.z > bbox.MaxZ ())
      left_idx = CS_BOX_CORNER_XyZ;
    else
      left_idx = CS_BOX_CORNER_xyZ;
  }
  else if (campos.x > bbox.MaxX ())
  {
    if (campos.z < bbox.MinZ ())
      left_idx = CS_BOX_CORNER_xyz;
    else
      left_idx = CS_BOX_CORNER_Xyz;
  }
  else
  {
    if (campos.z < bbox.MinZ ())
      left_idx = CS_BOX_CORNER_xyz;
    else
      left_idx = CS_BOX_CORNER_XyZ;
  }

  // In the code below we compute the right index which is compatible
  // with CS_BOX_CORNER_xyz (for csBox3::GetCorner()).
  int right_idx;
  if (campos.z < bbox.MinZ ())
  {
    if (campos.x < bbox.MaxX ())
      right_idx = CS_BOX_CORNER_Xyz;
    else
      right_idx = CS_BOX_CORNER_XyZ;
  }
  else if (campos.z > bbox.MaxZ ())
  {
    if (campos.x < bbox.MinX ())
      right_idx = CS_BOX_CORNER_xyz;
    else
      right_idx = CS_BOX_CORNER_xyZ;
  }
  else
  {
    if (campos.x < bbox.MinX ())
      right_idx = CS_BOX_CORNER_xyz;
    else
      right_idx = CS_BOX_CORNER_XyZ;
  }

  // left_idx and right_idx are now the left-most and right-most
  // indices in the bounding box. We compute angles for them.
  csVector3 left_corner = bbox.GetCorner (left_idx);
  csVector3 right_corner = bbox.GetCorner (right_idx);

  // Seems that right and left are swapped for the horizon buffer.
  right = GetHorIndex (campos, left_corner.x, left_corner.z, horsize);
  left = GetHorIndex (campos, right_corner.x, right_corner.z, horsize);

//printf ("left_idx=%d right_idx=%d\n", left_idx, right_idx);
//printf ("left=%d right=%d horsize=%d\n", left, right, horsize);
//printf ("campos=(%g,%g,%g) bbox=(%g,%g,%g)-(%g,%g,%g)\n", campos.x, campos.y, campos.z, bbox.MinX (), bbox.MinY (), bbox.MinZ (), bbox.MaxX (), bbox.MaxY (), bbox.MaxZ ());
//fflush (stdout);

  CS_ASSERT( (right - left + horsize)%horsize <= (horsize/2+1));
}

void csTerrainQuad::ComputeMinMaxDY(const csVector3& campos, const csBox3& bbox,
    float &mindy, float &maxdy)
{
  // idea is this:
  // dy (change of y) is the deltay / deltadistance.
  // maximum dy, is the max_height / minimumdistance
  // minimum dy, is the min_height / maximumdistance

  csBox2 tr_box (bbox.MinX ()-campos.x, bbox.MinZ ()-campos.z,
  		 bbox.MaxX ()-campos.x, bbox.MaxZ ()-campos.z);
  // Compute minimum and maximum squared distance to the box as seen
  // from the camera position.
  float mindist = tr_box.SquaredOriginDist ();
  float maxdist = tr_box.SquaredOriginMaxDist ();

  // Use difference between camera.y and min,maxheight
  // to get min and max height change
  float minh = min_height - campos.y;
  float maxh = max_height - campos.y;

  // Divide by distance, correctly using infinite if divbyzero.
  // like this:
  // if divbyzero, use infinity with the same sign.
  // see if negative values occur, distances get swapped in that case

  if(minh < 0.0)
  {
    /// below zero, smallest height at minimal distance is the steepest down
    if(mindist == 0.0f) mindy = MININF;
    else mindy = minh * csQisqrt(mindist);
  }
  else
  {
    /// above zero, the smallest height at maximal distance is the smallest
    /// upslope
    if(maxdist == 0.0f) mindy = MAXINF;
    else mindy = minh * csQisqrt(maxdist);
  }

  if(maxh < 0.0)
  {
    /// below zero, biggest height at maximal distance is the least downslope
    if(maxdist == 0.0f) maxdy = MININF;
    else maxdy = maxh * csQisqrt(maxdist);
  }
  else
  {
    /// above zero, the biggest height at minimum distance is steepest upslope
    if(mindist == 0.0f) maxdy = MAXINF;
    else maxdy = maxh * csQisqrt(mindist);
  }

//printf ("sqdist[0]=%g\n", sqdist[0]);
//printf ("sqdist[1]=%g\n", sqdist[1]);
//printf ("sqdist[2]=%g\n", sqdist[2]);
//printf ("sqdist[3]=%g\n", sqdist[3]);
//printf ("mindist=%g maxdist=%g\n", mindist, maxdist);
//printf ("min_height=%g max_height=%g\n", min_height, max_height);
//printf ("minh=%g maxh=%g\n", minh, maxh);
//fflush (stdout);
  /// we've been trying to do this all along here.
  CS_ASSERT( mindy <= maxdy );
}

bool csTerrainQuad::CheckIfAbove(float* horizon, int horsize, int left,
  int right, float dy)
{
  /// loop from left to right, but it can wrap by horsize
  int len = (right-left+horsize)%horsize;
  int idx = left;
  while(len--)
  {
    if( dy > horizon[idx] ) return true;
    idx = (idx+1)%horsize;
  }
  return false;
}

void csTerrainQuad::HeightenHorizon(float* horizon, int horsize, int left,
  int right, float dy)
{
  int len = (right-left+horsize)%horsize;
  int idx = left;
  while(len--)
  {
    if( dy > horizon[idx] ) horizon[idx] = dy;
    idx = (idx+1)%horsize;
  }
}


void csTerrainQuad::ComputeVisibility(const csVector3& campos,
  const csBox3& bbox, float* horizon, int horsize)
{
  // compute visibility for this node

  // compute min and max dy values.
  float mindy, maxdy;
  ComputeMinMaxDY(campos, bbox, mindy, maxdy);

  // start and end angles for projected bounding box on horizon
  int left = 0, right = 0;
  // visibility
  bool vis = false;
  // if camera is 'in' this node (disregarding height), vis always true
  if(  (bbox.MinX()-SMALL_EPSILON <= campos.x) &&
       (campos.x <= bbox.MaxX()+SMALL_EPSILON)
    && (bbox.MinZ()-SMALL_EPSILON <= campos.z) &&
       (campos.z <= bbox.MaxZ()+SMALL_EPSILON) )
  {
    // node spans the entire horizon
    left = 0;
    right = horsize-1;
    vis = true;
  }
  else {
    // see which piece of the horizon is spanned by the bbox
    ComputeExtent(campos, bbox, horsize, left, right);

    // check min and max dy against the horizon
    // if the max is bigger than any value -> the node could be visible
    vis = CheckIfAbove(horizon, horsize, left, right, maxdy);
  }

  if(!vis) return; // nothing to be done, nor for the children.

  // so this node is visible
  MarkVisible();
  // update horizon with minimum height
  HeightenHorizon(horizon, horsize, left, right, mindy);

  // for leaves, we are done now.
  if(IsLeaf()) return;

  // the children must be checked front-to-back...
  float midx = (bbox.MinX() + bbox.MaxX()) * 0.5;
  float midz = (bbox.MinZ() + bbox.MaxZ()) * 0.5;

  // first see where the camera lies:
  // TOP +z, BOT -z
  // LEFT -x, RIGHT +x
  // z splits top from bottom
  // x splits left from right
  int camchild = 0;
  if(campos.z < midz) camchild = CS_QUAD_TOPLEFT;
  else camchild = CS_QUAD_BOTLEFT;
  if(campos.x > midx) camchild++; // get the right quad

  // iterate front to back
  csBox3 cbox[4];
  cbox[CS_QUAD_TOPLEFT].Set (bbox.MinX(), 0, bbox.MinZ(), midx, 0, midz);
  cbox[CS_QUAD_TOPRIGHT].Set(midx, 0, bbox.MinZ(), bbox.MaxX(), 0, midz);
  cbox[CS_QUAD_BOTLEFT].Set (bbox.MinX(), 0, midz, midx, 0, bbox.MaxZ());
  cbox[CS_QUAD_BOTRIGHT].Set(midx, 0, midz, bbox.MaxX(), 0, bbox.MaxZ());

  // rect is set up as: 0 1
  //                    2 3
  // so, in bits, this is ab:  a is 0(left) 1(right), b is 0(top) 1(bot)
  // flip bits to get the other squares.
  int child = camchild;
  GetChild(child)->ComputeVisibility(campos, cbox[child], horizon, horsize);
  child = camchild^1;
  GetChild(child)->ComputeVisibility(campos, cbox[child], horizon, horsize);
  child = camchild^2;
  GetChild(child)->ComputeVisibility(campos, cbox[child], horizon, horsize);
  child = camchild^3;
  GetChild(child)->ComputeVisibility(campos, cbox[child], horizon, horsize);
}

//------------------------------------------------------------------------

