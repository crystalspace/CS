/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "csgfx/renderbuffer.h"
#include "csgeom/math3d.h"
#include "csgeom/poly2d.h"
#include "cstool/rbuflock.h"
#include "csutil/sysfunc.h"
#include "iutil/objreg.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "haze.h"
#include "csqsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csHazeHull)
  SCF_IMPLEMENTS_INTERFACE (iHazeHull)
SCF_IMPLEMENT_IBASE_END

csHazeHull::csHazeHull()
{
  SCF_CONSTRUCT_IBASE (0);
  total_poly = 0;
  total_vert = 0;
  total_edge = 0;
  verts = 0;
  edgept1 = 0;
  edgept2 = 0;
  pol_num = 0;
  pol_verts = 0;
  pol_edges = 0;
}

csHazeHull::~csHazeHull()
{
  /// delete and clear the object
  delete[] verts; verts = 0;
  delete[] edgept1; edgept1 = 0;
  delete[] edgept2; edgept2 = 0;
  int p;
  for(p=0; p<total_poly; p++)
  {
    delete[] pol_verts[p]; pol_verts[p] = 0;
    delete[] pol_edges[p]; pol_edges[p] = 0;
  }
  delete[] pol_verts; pol_verts = 0;
  delete[] pol_edges; pol_edges = 0;
  delete[] pol_num; pol_num = 0;
  total_poly = 0;
  total_vert = 0;
  total_edge = 0;
  SCF_DESTRUCT_IBASE ();
}


void csHazeHull::ComputeEdges()
{
  /// create table with edges.
  /// matrix: verticeidx x verticeidx
  int *matrix = new int [total_vert * total_vert];
  int i;
  for(i=0; i<total_vert*total_vert; i++)
    matrix[i] = 0;
  /// mark 1 where vertex goes to another vertex.
  /// (from smaller to larger vertex only)
  int p;
  for(p=0; p<total_poly; p++)
  {
    //csPrintf("poly %d: ", p);
    for(i=0; i<pol_num[p]; i++)
    {
      /// get two indices
      int idx1 = pol_verts[p][i];
      int idx2 = pol_verts[p][ (i+1) % pol_num[p] ];
      /// swap so idx1 <= idx2
      if(idx1>idx2)
      {
        int swp = idx1;
	idx1 = idx2;
	idx2 = swp;
      }
      /// mark
      //csPrintf("(%d-%d) ", idx1, idx2);
      matrix[ idx1*total_vert + idx2 ] = 1;
    }
    //csPrintf("\n");
  }
  /// use matrix to fill fields.

  total_edge = 0;
  /// count number of 1s
  int j;
  for(i=0; i< total_vert; i++)
    for(j=i; j<total_vert; j++)
    {
      /// i <= j
      if(matrix[ i*total_vert + j ] != 0)
        total_edge++;
    }
  /// fill edgept tables.
  delete[] edgept1;
  delete[] edgept2;
  edgept1 = new int[total_edge];
  edgept2 = new int[total_edge];
  int edge_now = 0;
  for(i=0; i< total_vert; i++)
    for(j=i; j<total_vert; j++)
    {
      /// i <= j
      if(matrix[ i*total_vert + j ] != 0)
      {
        edgept1[edge_now] = i;
        edgept2[edge_now] = j;
	/// fill matrix with edge_numbers of edge from i<->j
	matrix[ i*total_vert + j ] = edge_now;
	matrix[ j*total_vert + i ] = edge_now;
	/// next edge number
        edge_now ++;
      }
    }
  /// now use matrix filled with edge numbers to get edges for polygons
  /// and alloc pol_edges;
  if(pol_edges)
  {
    for(p=0; p<total_poly; p++)
      delete[] pol_edges[p];
    delete[] pol_edges;
  }
  pol_edges = new int* [total_poly];

  for(p=0; p<total_poly; p++)
  {
    pol_edges[p] = new int [pol_num[p]];
    for(i=0; i<pol_num[p]; i++)
    {
      /// get two vert indices
      int i1 = pol_verts[p][i];
      int i2 = pol_verts[p][ (i+1) % pol_num[p] ];
      /// get edge
      int edge_here = matrix[ i1*total_vert + i2 ];
      pol_edges[p][i] = edge_here;
    }
  }

  /// remove temp matrix
  delete[] matrix;
}


void csHazeHull::ComputeOutline(iHazeHull *hull, const csVector3& campos,
  int& numv, int*& pts)
{
  // create some temporary arrays
  numv = 0;
  int *use_edge = new int[hull->GetEdgeCount()];
  int *use_start = new int[hull->GetEdgeCount()];
  int *use_end = new int[hull->GetEdgeCount()];
  int *nextvert = new int[hull->GetVerticeCount()];

  /// no edges used
  int i, p;
  for(i=0; i<hull->GetEdgeCount(); i++)
    use_edge[i] = 0;

  /// mark number of times an edge is used by visible polygons
  for(p=0; p<hull->GetPolygonCount(); p++)
  {
    csVector3 v0,v1,v2;
    hull->GetVertex(v0, hull->GetPolVertex(p, 0));
    hull->GetVertex(v1, hull->GetPolVertex(p, 1));
    hull->GetVertex(v2, hull->GetPolVertex(p, 2));
    if(csMath3::WhichSide3D(campos-v0, v1-v0, v2-v0) > 0) {
      //csPrintf("polygon %d visible\n", p);
      for(i=0; i<hull->GetPolVerticeCount(p); i++)
      {
        int edge, i1, i2;
	edge = hull->GetPolEdge(p, i, i1, i2);
	//csPrintf("increasing edge %d (from %d - %d)\n", edge, i1, i2);
	use_edge[edge] ++;
	use_start[edge] = i1;
	use_end[edge] = i2;
      }
    }
  }
  /// connect use_edge where usage is once.
#ifdef CS_DEBUG
  for(i=0; i<hull->GetVerticeCount(); i++)
    nextvert[i] = -1;
#endif
  int startpt = -1;
  for(i=0; i<hull->GetEdgeCount(); i++)
  {
    if(use_edge[i] == 1)
    {
      if(startpt==-1) startpt = use_start[i];
      nextvert[use_start[i]] = use_end[i];
    }
  }

  if(startpt == -1)
  {
    /// no outline, no polygon.
    delete[] use_edge;
    delete[] use_start;
    delete[] use_end;
    delete[] nextvert;
    numv = 0;
    return;
  }

  pts = new int[hull->GetVerticeCount()];
  numv = 0;
  int pt = startpt;
  do
  {
    pts[numv++] = pt;
    pt = nextvert[pt];
    if(pt == -1)
    {
      csPrintf("Error: pt==-1 in Outline.\n");
      delete[] use_edge;
      delete[] use_start;
      delete[] use_end;
      delete[] nextvert;
      return;
    }
  }
  while( pt != startpt );

  delete[] use_edge;
  delete[] use_start;
  delete[] use_end;
  delete[] nextvert;
}

//------------ csHazeHullBox -----------------------------------

SCF_IMPLEMENT_IBASE_EXT (csHazeHullBox)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iHazeHullBox)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeHullBox::HazeHullBox)
  SCF_IMPLEMENTS_INTERFACE (iHazeHullBox)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csHazeHullBox::csHazeHullBox(const csVector3& a, const csVector3& b)
  : csHazeHull()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiHazeHullBox);
  min = a;
  max = b;
  /// fill with data
  total_vert = 8;
  total_poly = 12;
  verts = new csVector3 [total_vert];
  pol_num = new int [total_poly];
  pol_verts = new int* [total_poly];
  int i;
  for(i=0; i<total_poly; i++)
  {
    pol_num[i] = 3;
    pol_verts[i] = new int [ pol_num[i] ];
  }

  verts[0].Set (min.x, min.y, min.z);
  verts[1].Set (max.x, min.y, min.z);
  verts[2].Set (min.x, max.y, min.z);
  verts[3].Set (max.x, max.y, min.z);
  verts[4].Set (min.x, min.y, max.z);
  verts[5].Set (max.x, min.y, max.z);
  verts[6].Set (min.x, max.y, max.z);
  verts[7].Set (max.x, max.y, max.z);

  pol_verts[0][0] = 0; pol_verts[0][1] = 2; pol_verts[0][2] = 3;
  pol_verts[1][0] = 0; pol_verts[1][1] = 3; pol_verts[1][2] = 1;
  pol_verts[2][0] = 1; pol_verts[2][1] = 3; pol_verts[2][2] = 7;
  pol_verts[3][0] = 1; pol_verts[3][1] = 7; pol_verts[3][2] = 5;
  pol_verts[4][0] = 7; pol_verts[4][1] = 4; pol_verts[4][2] = 5;
  pol_verts[5][0] = 7; pol_verts[5][1] = 6; pol_verts[5][2] = 4;
  pol_verts[6][0] = 6; pol_verts[6][1] = 0; pol_verts[6][2] = 4;
  pol_verts[7][0] = 6; pol_verts[7][1] = 2; pol_verts[7][2] = 0;
  pol_verts[8][0] = 6; pol_verts[8][1] = 7; pol_verts[8][2] = 3;
  pol_verts[9][0] = 6; pol_verts[9][1] = 3; pol_verts[9][2] = 2;
  pol_verts[10][0] = 0; pol_verts[10][1] = 1; pol_verts[10][2] = 4;
  pol_verts[11][0] = 1; pol_verts[11][1] = 5; pol_verts[11][2] = 4;

  ComputeEdges();
}

csHazeHullBox::~csHazeHullBox()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiHazeHullBox);
}

//------------ csHazeHullCone -----------------------------------

SCF_IMPLEMENT_IBASE_EXT (csHazeHullCone)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iHazeHullCone)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeHullCone::HazeHullCone)
  SCF_IMPLEMENTS_INTERFACE (iHazeHullCone)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

/// static helper to fill
static void ConeFillVerts(csVector3 *verts, int nr_sides,
  const csVector3& pos, float radius)
{
  int i;
  for(i=0; i<nr_sides; i++)
  {
    float angle = (float(i)* TWO_PI) / float(nr_sides);
    verts[i] = pos;
    verts[i].x += sin(angle) * radius;
    verts[i].z += cos(angle) * radius;
  }
}

csHazeHullCone::csHazeHullCone(int nr_sides, const csVector3& start,
    const csVector3& end, float srad, float erad)
  : csHazeHull()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiHazeHullCone);
  csHazeHullCone::nr_sides = nr_sides;
  csHazeHullCone::start = start;
  csHazeHullCone::end = end;
  start_radius = srad;
  end_radius = erad;

  /// fill with data
  total_vert = nr_sides*2;
  total_poly = nr_sides + 2;
  verts = new csVector3 [total_vert];
  pol_num = new int [total_poly];
  pol_verts = new int* [total_poly];
  int i;

  pol_num[0] = nr_sides;
  pol_num[1] = nr_sides;
  for(i=2; i<total_poly; i++)
  {
    pol_num[i] = 4;
  }

  for(i=0; i<total_poly; i++)
  {
    pol_verts[i] = new int [ pol_num[i] ];
  }

  /// fill each circle in turn
  ConeFillVerts(&verts[0], nr_sides, start, start_radius);
  ConeFillVerts(&verts[nr_sides], nr_sides, end, end_radius);

  /// fill pol_verts
  // caps
  for(i=0; i<nr_sides; i++)
  {
    pol_verts[0][i] = nr_sides + i;
    pol_verts[1][i] = nr_sides-i-1;
  }
  // sides
  int pnum = 2;
  for(i=0; i<nr_sides; i++)
  {
    int nexti = (i+1)%nr_sides;

    pol_verts[pnum][0] = i;
    pol_verts[pnum][1] = nexti;
    pol_verts[pnum][2] = nexti + nr_sides;
    pol_verts[pnum][3] = i + nr_sides;
    pnum++;
    /*
    pol_verts[pnum][0] = i;
    pol_verts[pnum][1] = nexti + nr_sides;
    pol_verts[pnum][2] = i + nr_sides;
    pnum++;
    pol_verts[pnum][0] = i;
    pol_verts[pnum][1] = nexti;
    pol_verts[pnum][2] = nexti + nr_sides;
    pnum++;
    */
  }

  ComputeEdges();
}

csHazeHullCone::~csHazeHullCone()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiHazeHullCone);
}

//------------ csHazeMeshObject -------------------------------

SCF_IMPLEMENT_IBASE (csHazeMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iHazeState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeMeshObject::HazeState)
  SCF_IMPLEMENTS_INTERFACE (iHazeState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csStringID csHazeMeshObject::vertex_name = csInvalidStringID;
csStringID csHazeMeshObject::texel_name = csInvalidStringID;
csStringID csHazeMeshObject::index_name = csInvalidStringID;
csStringID csHazeMeshObject::string_object2world = csInvalidStringID;

csHazeMeshObject::csHazeMeshObject (csHazeMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiHazeState);
  csHazeMeshObject::factory = factory;
  logparent = 0;
  ifactory = SCF_QUERY_INTERFACE (factory, iMeshObjectFactory);
  material = factory->GetMaterialWrapper ();
  MixMode = factory->GetMixMode ();
  initialized = false;
  vis_cb = 0;
  current_lod = 1;
  current_features = 0;
  origin.Set(0,0,0);
  directional.Set(0,0,0);
  bbox.StartBoundingBox();

  /// copy the factory settings
  origin = factory->GetOrigin();
  directional = factory->GetDirectional();
  csPDelArray<csHazeLayer> *factlayers = factory->GetLayers();
  size_t i;
  for(i = 0; i < factlayers->Length(); i++)
  {
    csHazeLayer *p = new csHazeLayer (factlayers->Get(i)->hull,
      factlayers->Get(i)->scale);
    layers.Push(p);
  }

  csRef<iStringSet> strings;
  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (factory->object_reg,
    "crystalspace.shared.stringset", iStringSet);

  if ((vertex_name == csInvalidStringID) ||
    (texel_name == csInvalidStringID) ||
    (index_name == csInvalidStringID) ||
    (string_object2world == csInvalidStringID))
  {
    vertex_name = strings->Request ("vertices");
    texel_name = strings->Request ("texture coordinates");
    index_name = strings->Request ("indices");
    string_object2world = strings->Request ("object2world transform");
  }
}

csHazeMeshObject::~csHazeMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiHazeState);
  SCF_DESTRUCT_IBASE ();
}

void csHazeMeshObject::SetupObject ()
{
  if (!initialized)
  {
    bbox.StartBoundingBox( origin );
    csVector3 pos;
    size_t l;
    int i;
    for(l=0; l<layers.Length(); l++)
      for(i=0; i<layers[l]->hull->GetVerticeCount(); i++)
      {
	layers[l]->hull->GetVertex(pos, i);
        bbox.AddBoundingVertex( pos );
      }
    initialized = true;
  }
}


void csHazeMeshObject::GetTransformedBoundingBox (long cameranr,
        long movablenr, const csReversibleTransform& trans, csBox3& cbox)
{
  if (cur_cameranr == cameranr && cur_movablenr == movablenr)
  {
    cbox = camera_bbox;
    return;
  }
  cur_cameranr = cameranr;
  cur_movablenr = movablenr;

  camera_bbox.StartBoundingBox (trans * bbox.GetCorner(0));
  camera_bbox.AddBoundingVertexSmart (trans * bbox.GetCorner(1));
  camera_bbox.AddBoundingVertexSmart (trans * bbox.GetCorner(2));
  camera_bbox.AddBoundingVertexSmart (trans * bbox.GetCorner(3));
  camera_bbox.AddBoundingVertexSmart (trans * bbox.GetCorner(4));
  camera_bbox.AddBoundingVertexSmart (trans * bbox.GetCorner(5));
  camera_bbox.AddBoundingVertexSmart (trans * bbox.GetCorner(6));
  camera_bbox.AddBoundingVertexSmart (trans * bbox.GetCorner(7));

  cbox = camera_bbox;
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
        float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

float csHazeMeshObject::GetScreenBoundingBox (long cameranr,
      long movablenr, float fov, float sx, float sy,
      const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox)
{
  csVector2 oneCorner;

  GetTransformedBoundingBox (cameranr, movablenr, trans, cbox);

  // if the entire bounding box is behind the camera, we're done
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
  {
    return -1;
  }

  // Transform from camera to screen space.
  if (cbox.MinZ () <= 0)
  {
    // Sprite is very close to camera.
    // Just return a maximum bounding box.
    sbox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    Perspective (cbox.Max (), oneCorner, fov, sx, sy);
    sbox.StartBoundingBox (oneCorner);
    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    Perspective (cbox.Min (), oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}



CS_IMPLEMENT_STATIC_VAR(GetTempVertices, csDirtyAccessArray<csVector3>, ());
CS_IMPLEMENT_STATIC_VAR(GetTempTexels, csDirtyAccessArray<csVector2>, ());
CS_IMPLEMENT_STATIC_VAR(GetTempIndices, csDirtyAccessArray<uint>, ());

void csHazeMeshObject::GenGeometryAdapt (iRenderView *rview, iGraphics3D *g3d, 
					 int num_sides, csVector3* scrpts, 
					 csVector3* campts, csVector2* uvs,
					 float layer_scale, float quality, 
					 int depth, int maxdepth)
{
  /// only triangles
  (void)num_sides;
  CS_ASSERT(num_sides == 3);
  // check if the angle is OK
  csVector2 dir1;
  dir1.x = scrpts[1].x - scrpts[0].x;
  dir1.y = scrpts[1].y - scrpts[0].y;
  csVector2 dir2;
  dir2.x = scrpts[2].x - scrpts[0].x;
  dir2.y = scrpts[2].y - scrpts[0].y;
  csVector2 normdir1 = dir1 / dir1.Norm();
  csVector2 normdir2 = dir2 / dir2.Norm();
  float cosangle = normdir1 * normdir2;
  //csPrintf("cosangle %g, quality %g\n", cosangle, quality);
  if(cosangle > quality || depth >= maxdepth)
  {
    // emit geometry
    for (int i = 0; i < 3; i++)
    {
      GetTempIndices()->Push ((uint)GetTempVertices()->Length());
      GetTempVertices()->Push (campts[i]);
      GetTempTexels()->Push (uvs[i]);
    }
    return;
  }
  // split up
  csVector3 oldpos = scrpts[2];
  csVector3 oldcam = campts[2];
  csVector2 olduv = uvs[2];
  scrpts[2] = (scrpts[1] + scrpts[2])*0.5;
  campts[2] = (campts[1] + campts[2])*0.5;
  csVector2 newdir;
  newdir.x = scrpts[2].x - scrpts[0].x;
  newdir.y = scrpts[2].y - scrpts[0].y;
  newdir /= newdir.Norm();
  uvs[2].Set(0.5, 0.5);
  uvs[2] += newdir * layer_scale;
  GenGeometryAdapt (rview, g3d, 3, scrpts, campts, uvs, layer_scale, quality, 
    depth+1, maxdepth);
  // other half
  csVector3 oldpos1 = scrpts[1];
  csVector3 oldcam1 = campts[1];
  csVector2 olduv1 = uvs[1];
  scrpts[1] = scrpts[2];
  campts[1] = campts[2];
  uvs[1] = uvs[2];
  scrpts[2] = oldpos;
  campts[2] = oldcam;
  uvs[2] = olduv;
  GenGeometryAdapt (rview, g3d, 3, scrpts, campts, uvs, layer_scale, quality, 
    depth+1, maxdepth);
  scrpts[1] = oldpos1;
  campts[1] = oldcam1;
  uvs[1] = olduv1;
}

csRenderMesh** csHazeMeshObject::GetRenderMeshes (int &n, iRenderView* rview,
						  iMovable* movable, 
						  uint32 frustum_mask)
{ 
  SetupObject ();

  if(layers.Length() <= 0)
  {
    n = 0; 
    return 0; 
  }

  /// prepare to transform the points
  iGraphics3D* g3d = rview->GetGraphics3D ();
  iCamera* camera = rview->GetCamera ();
  csVector3 campos = camera->GetTransform().GetOrigin();
  if (!movable->IsFullTransformIdentity ())
    campos = movable->GetFullTransform() * campos;
  float fov = camera->GetFOV ();
  float shx = camera->GetShiftX ();
  float shy = camera->GetShiftY ();
  /// obj to camera space
  csReversibleTransform tr_o2c = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();


  // project origin to screenspace
  csVector2 center(0.5, 0.5);
  csVector3 scr_orig;
  csVector3 cam_orig;
  ProjectO2S(tr_o2c, fov, shx, shy, origin, scr_orig, &cam_orig);

  // get hull 0 outline in screenspace
  iHazeHull *hull = layers[0]->hull;
  float layer_scale = layers[0]->scale;
  int layer_num = 0;
  int *layer_poly = 0;
  csVector3* layer_pts = 0;
  csVector2* layer_uvs = 0;
  csVector3* cam_pts = 0;
  ComputeHullOutline(hull, layer_scale, campos, tr_o2c, fov, shx, shy,
    layer_num, layer_poly, layer_pts, &cam_pts, layer_uvs);
  if(layer_num <= 0)
  {
    n = 0; 
    return 0; 
  }

  int i;
  // additional test if origin inside the outline
  csVector2* incheck = new csVector2[layer_num];
  for(i=0; i<layer_num; i++)
  {
    incheck[i].x = layer_pts[layer_num-1 - i].x;
    incheck[i].y = layer_pts[layer_num-1 - i].y;
  }
  csVector2 checkpt( scr_orig.x, scr_orig.y );
  if(!csPoly2D::In (incheck, layer_num, checkpt))
  {
    // origin not inside outline.
    delete[] incheck;
    delete[] layer_poly;
    delete[] layer_pts;
    delete[] layer_uvs;
    delete[] cam_pts;
    n = 0; 
    return 0; 
  }
  delete[] incheck;

  GetTempVertices()->Empty();
  GetTempTexels()->Empty();
  GetTempIndices()->Empty();

  {
    // draw triangles from orig to layer 0
    csVector3 tri_pts[3];
    csVector3 tri_campts[3];
    csVector2 tri_uvs[3];
    tri_pts[0] = scr_orig;
    tri_campts[0] = cam_orig;
    tri_uvs[0] = center;
    for(i=0; i<layer_num; i++)
    {
      int nexti = (i+1)%layer_num;
      tri_pts[2] = layer_pts[i];
      tri_pts[1] = layer_pts[nexti];
      tri_campts[2] = cam_pts[i];
      tri_campts[1] = cam_pts[nexti];
      tri_uvs[2] = layer_uvs[i];
      tri_uvs[1] = layer_uvs[nexti];


      //csPrintf("drawing a polygon\n");
      //DrawPoly(rview, g3d, mat, 3, tri_pts, tri_uvs);

      float quality = 0.90f;
      int maxdepth = 10;
      GenGeometryAdapt (rview, g3d, 3, tri_pts, tri_campts, tri_uvs, 
	layer_scale, quality, 0, maxdepth);
    }
  }

  bool bufferCreated;
  const size_t vertCount = GetTempVertices()->Length();
  HazeRenderBuffer& vertices = renderBuffers.GetUnusedData (bufferCreated,
    rview->GetCurrentFrameNumber());
  if (bufferCreated || (vertices.count < vertCount))
  {
    vertices.buffer = csRenderBuffer::CreateRenderBuffer (vertCount,
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
  }
  vertices.buffer->CopyInto (GetTempVertices()->GetArray(), 
    vertCount);
  HazeRenderBuffer& texels = renderBuffers.GetUnusedData (bufferCreated,
    rview->GetCurrentFrameNumber());
  if (bufferCreated || (texels.count < vertCount))
  {
    texels.buffer = csRenderBuffer::CreateRenderBuffer (vertCount,
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 2);
  }
  texels.buffer->CopyInto (GetTempTexels()->GetArray(), 
    vertCount);
  HazeRenderBuffer& indices = indexBuffers.GetUnusedData (bufferCreated,
    rview->GetCurrentFrameNumber());
  if (bufferCreated || (indices.count < GetTempIndices()->Length()))
  {
    indices.buffer = csRenderBuffer::CreateIndexRenderBuffer (
      GetTempIndices()->Length(), CS_BUF_STREAM, 
      CS_BUFCOMP_UNSIGNED_INT, 0, vertCount);
  }
  indices.buffer->CopyInto (GetTempIndices()->GetArray(), 
    GetTempIndices()->Length());

  bool rmCreated;
  csRenderMesh*& rm = rmHolder.GetUnusedMesh (rmCreated, 
    rview->GetCurrentFrameNumber());
  if (rmCreated)
  {
    rm->buffers.AttachNew (new csRenderBufferHolder);
    rm->variablecontext.AttachNew (new csShaderVariableContext);
    rm->meshtype = CS_MESHTYPE_TRIANGLES;
    rm->material = material;
    rm->mixmode = MixMode;
  }

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
      clip_z_plane);
  csVector3 camera_origin = tr_o2c.GetT2OTranslation ();

  rm->worldspace_origin = movable->GetFullPosition ();
  rm->clip_portal = clip_portal;
  rm->clip_plane = clip_plane;
  rm->clip_z_plane = clip_z_plane;
  rm->do_mirror = camera->IsMirrored ();

  rm->variablecontext->GetVariableAdd (string_object2world)->
    SetValue (camera->GetTransform ());

  rm->indexend = (uint)GetTempIndices()->Length();

  rm->buffers->SetRenderBuffer (CS_BUFFER_INDEX, indices.buffer);
  rm->buffers->SetRenderBuffer (CS_BUFFER_POSITION, vertices.buffer);
  rm->buffers->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texels.buffer);

  delete[] layer_poly;
  delete[] layer_pts;
  delete[] layer_uvs;
  delete[] cam_pts;

  n = 1; 
  return &rm; 
}


void csHazeMeshObject::ComputeHullOutline(iHazeHull *hull, float layer_scale,
  const csVector3& campos, csReversibleTransform& tr_o2c, float fov, float shx,
  float shy, int &layer_num, int *& layer_poly, csVector3 *& layer_pts,
  csVector3** cam_pts, csVector2 *&layer_uvs)
{
  int i;
  // get hull outline in screenspace
  layer_num = 0;
  layer_poly = 0;
  //csPrintf("campos %g,%g,%g\n", campos.x, campos.y, campos.z);
  csHazeHull::ComputeOutline(hull, campos, layer_num, layer_poly);
  //csPrintf("has outline of size %d: ", layer_num);
  if(layer_num <= 0) return;
  layer_pts = new csVector3[layer_num];
  *cam_pts = new csVector3[layer_num];

  for(i=0; i<layer_num; i++)
  {
    //csPrintf(" %d", layer_poly[i]);
    csVector3 objpos;
    hull->GetVertex(objpos, layer_poly[i] );

    ProjectO2S(tr_o2c, fov, shx, shy, objpos, layer_pts[i],
      &((*cam_pts)[i]));
  }
  //csPrintf("\n");
  // get hull 0 uv values
  layer_uvs = new csVector2[layer_num];
  csVector2 center(0.5, 0.5);
  // project to screenspace
  csVector3 scr_orig;
  csVector3 cam_orig;
  ProjectO2S (tr_o2c, fov, shx, shy, origin, scr_orig, &cam_orig);
  csVector2 dir;
  for(i=0; i<layer_num; i++)
  {
    dir.x = layer_pts[i].x - scr_orig.x;
    dir.y = layer_pts[i].y - scr_orig.y;
    dir /= dir.Norm();
    layer_uvs[i] = center + dir * layer_scale;
  }
}



void csHazeMeshObject::DrawPolyAdapt(iRenderView *rview, iGraphics3D *g3d,
     iMaterialHandle *mat, int num_sides, csVector3* pts, csVector2* uvs,
     float layer_scale, float quality, int depth, int maxdepth)
{
  /// only triangles
  (void)num_sides;
  CS_ASSERT(num_sides == 3);
  // check if the angle is OK
  csVector2 dir1;
  dir1.x = pts[1].x - pts[0].x;
  dir1.y = pts[1].y - pts[0].y;
  csVector2 dir2;
  dir2.x = pts[2].x - pts[0].x;
  dir2.y = pts[2].y - pts[0].y;
  csVector2 normdir1 = dir1 / dir1.Norm();
  csVector2 normdir2 = dir2 / dir2.Norm();
  float cosangle = normdir1 * normdir2;
  //csPrintf("cosangle %g, quality %g\n", cosangle, quality);
  if(cosangle > quality || depth >= maxdepth)
  {
    // draw it
    DrawPoly(rview, g3d, mat, 3, pts, uvs);
    return;
  }
  // split up
  csVector3 oldpos = pts[2];
  csVector2 olduv = uvs[2];
  pts[2] = (pts[1] + pts[2])*0.5;
  csVector2 newdir;
  newdir.x = pts[2].x - pts[0].x;
  newdir.y = pts[2].y - pts[0].y;
  newdir /= newdir.Norm();
  uvs[2].Set(0.5, 0.5);
  uvs[2] += newdir * layer_scale;
  DrawPolyAdapt(rview, g3d, mat, 3, pts, uvs, layer_scale, quality, 
    depth+1, maxdepth);
  // other half
  csVector3 oldpos1 = pts[1];
  csVector2 olduv1 = uvs[1];
  pts[1] = pts[2];
  uvs[1] = uvs[2];
  pts[2] = oldpos;
  uvs[2] = olduv;
  DrawPolyAdapt(rview, g3d, mat, 3, pts, uvs, layer_scale, quality, 
    depth+1, maxdepth);
  pts[1] = oldpos1;
  uvs[1] = olduv1;
}

void csHazeMeshObject::ProjectO2S(csReversibleTransform& tr_o2c, float fov,
  float shiftx, float shifty, const csVector3& objpos, csVector3& scrpos, 
  csVector3* campos)
{
  *campos =
  scrpos = tr_o2c * objpos;  // to camera space
  scrpos.z = 1. / scrpos.z; // = iz
  float inv_z = fov * scrpos.z; // = iz
  scrpos.x = scrpos.x * inv_z + shiftx;
  scrpos.y = scrpos.y * inv_z + shifty;
}

void csHazeMeshObject::DrawPoly(iRenderView *rview, iGraphics3D *g3d,
  iMaterialHandle *mat, int num, const csVector3* pts, const csVector2* uvs)
{

}

void csHazeMeshObject::GetObjectBoundingBox (csBox3& retbbox)
{
  SetupObject ();
  retbbox = bbox;
}

void csHazeMeshObject::SetObjectBoundingBox (const csBox3& inbbox)
{
  bbox = inbbox;
  scfiObjectModel.ShapeChanged ();
}

void csHazeMeshObject::HardTransform (const csReversibleTransform& t)
{
  (void)t;
  //@@@ TODO
}

void csHazeMeshObject::NextFrame (csTicks /*current_time*/, const csVector3& /*pos*/)
{
  // nothing
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csHazeMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iHazeFactoryState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iHazeHullCreation)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeMeshObjectFactory::HazeFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iHazeFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeMeshObjectFactory::HazeHullCreation)
  SCF_IMPLEMENTS_INTERFACE (iHazeHullCreation)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csHazeMeshObjectFactory::csHazeMeshObjectFactory (csHazeMeshObjectType* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiHazeFactoryState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiHazeHullCreation);
  material = 0;
  MixMode = 0;
  origin.Set(0,0,0);
  directional.Set(0,0,0);
  logparent = 0;
  haze_type = pParent;
  object_reg = pParent->object_reg;
}

csHazeMeshObjectFactory::~csHazeMeshObjectFactory ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiHazeFactoryState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiHazeHullCreation);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObject> csHazeMeshObjectFactory::NewInstance ()
{
  csHazeMeshObject* cm = new csHazeMeshObject (this);
  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csHazeMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csHazeMeshObjectType)


csHazeMeshObjectType::csHazeMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csHazeMeshObjectType::~csHazeMeshObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csHazeMeshObjectType::NewFactory ()
{
  csHazeMeshObjectFactory* cm = new csHazeMeshObjectFactory (this);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

