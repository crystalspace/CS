/*
    Copyright (C) 1999-2001 by Jorrit Tyberghein
  
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
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "isys/system.h"
#include "ivideo/txtmgr.h"
#include "isys/vfs.h"
#include "terrfunc.h"
#include "qint.h"

IMPLEMENT_IBASE (csTerrFuncObject)
  IMPLEMENTS_INTERFACE (iTerrainObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTerrFuncState)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csTerrFuncObject::TerrFuncState)
  IMPLEMENTS_INTERFACE (iTerrFuncState)
IMPLEMENT_EMBEDDED_IBASE_END

static float TerrFunc (void*, float x, float y)
{
  return 8. * (sin (x*40.)+cos (y*40.));
}

csTerrFuncObject::csTerrFuncObject (iSystem* pSys,
	iTerrainObjectFactory *pFactory)
{
  CONSTRUCT_IBASE (NULL)
  CONSTRUCT_EMBEDDED_IBASE (scfiTerrFuncState);
  pSystem = pSys;
  csTerrFuncObject::pFactory = pFactory;
  initialized = false;
  blockx = 4;
  blocky = 4;
  gridx[0] = 8; gridy[0] = 8;
  gridx[1] = 6; gridy[1] = 6;
  gridx[2] = 4; gridy[2] = 4;
  gridx[3] = 3; gridy[3] = 3;
  topleft.Set (0, 0, 0);
  scale.Set (1, 1, 1);
  normals[0] = NULL;
  normals[1] = NULL;
  normals[2] = NULL;
  normals[3] = NULL;
  trimesh[0] = NULL;
  trimesh[1] = NULL;
  trimesh[2] = NULL;
  trimesh[3] = NULL;
  dirlight_numbers[0] = NULL;
  dirlight_numbers[1] = NULL;
  dirlight_numbers[2] = NULL;
  dirlight_numbers[3] = NULL;
  do_dirlight = false;
  dirlight_number = 0;
  materials = NULL;
  base_color.red = 0;
  base_color.green = 0;
  base_color.blue = 0;
  height_func = TerrFunc;
  normal_func = NULL;
  block_centers = NULL;
  lod_sqdist1 = 200*200;
  lod_sqdist2 = 400*400;
  lod_sqdist3 = 600*600;
}

csTerrFuncObject::~csTerrFuncObject ()
{
  int i;
  for (i = 0 ; i < 4 ; i++)
    delete[] trimesh[i]; // @@@ Memory leak: Cleanup vertex arrays too!
  delete[] normals[0];
  delete[] normals[1];
  delete[] normals[2];
  delete[] normals[3];
  delete[] dirlight_numbers[0];
  delete[] dirlight_numbers[1];
  delete[] dirlight_numbers[2];
  delete[] dirlight_numbers[3];
  delete[] materials;
  delete[] block_centers;
}

void csTerrFuncObject::LoadMaterialGroup (iEngine* engine, const char *pName,
	int iStart, int iEnd)
{
  if (!materials)
    materials = new iMaterialWrapper* [blockx * blocky];

  int i;
  char pMatName[256];

  for (i = iStart ; i <= iEnd ; i++)
  {
    sprintf (pMatName, pName, i);
    iMaterialWrapper* mat = engine->FindMaterial (pMatName);
    int bx = i % blockx;
    int by = i / blockx;
    int newi = bx*blockx + by;
    materials[newi] = mat;
  }
}

//---------------------------------------------------------------------------

class csTriangleVertices;

/*
 * The representation of a vertex in a triangle mesh.
 * This is basicly used as a temporary structure to be able to
 * calculate the cost of collapsing this vertex more quickly.
 */
class csTriangleVertex
{
public:
  // Position of this vertex in 3D space.
  csVector3 pos;
  // Function coordinates for this vertex.
  float dx, dy;
  // Index of this vertex.
  int idx;
  // True if already deleted.
  bool deleted;

  // Triangles that this vertex is connected to.
  int* con_triangles;
  // Number of triangles.
  int num_con_triangles;
  int max_con_triangles;

  // Other vertices that this vertex is connected to.
  int* con_vertices;
  // Number of vertices.
  int num_con_vertices;
  int max_con_vertices;

  // Precalculated minimal cost of collapsing this vertex to some other.
  float cost;
  // Vertex to collapse to with minimal cost.
  int to_vertex;

  csTriangleVertex () : deleted (false), con_triangles (NULL),
  	num_con_triangles (0), max_con_triangles (0),
  	con_vertices (NULL), num_con_vertices (0), max_con_vertices (0) { }
  ~csTriangleVertex () { delete [] con_triangles; delete [] con_vertices; }
  void AddTriangle (int idx);
  void AddVertex (int idx);
  bool DelVertex (int idx);
  void ReplaceVertex (int old, int replace);

  /*
   * Calculate the minimal cost of collapsing this vertex to some other.
   * Also remember which other vertex was selected for collapsing to.
   */
  void CalculateCost (csTriangleVertices* vertices, csTerrFuncObject* terrfunc);
};

/*
 * A class which holds vertices and connectivity information for a triangle
 * mesh. This is a general vertices structure but it is mostly useful
 * for LOD generation since every vertex contains information which
 * helps selecting the best vertices for collapsing.
 */
class csTriangleVertices
{
private:
  csTriangleVertex* vertices;
  int num_vertices;

public:
  // Build vertex table for a triangle mesh.
  csTriangleVertices (const G3DTriangleMesh& mesh,
  	csTerrFuncObject* terrfunc);
  ///
  ~csTriangleVertices ();
  // Update vertex table for a given set of vertices
  // (with the same number as at init).
  void UpdateVertices (csVector3* verts);

  int GetNumVertices () { return num_vertices; }
  csTriangleVertex& GetVertex (int idx) { return vertices[idx]; }

  void CalculateCost (csTerrFuncObject* terrfunc);
  int GetMinimalCostVertex ();
};

void csTriangleVertex::AddTriangle (int idx)
{
  int i;
  for (i = 0 ; i < num_con_triangles ; i++)
    if (con_triangles[i] == idx) return;

  if (num_con_triangles >= max_con_triangles)
  {
    int* new_con_triangles = new int [max_con_triangles+4];
    if (con_triangles)
    {
      memcpy (new_con_triangles, con_triangles, sizeof (int)*max_con_triangles);
      delete [] con_triangles;
    }
    con_triangles = new_con_triangles;
    max_con_triangles += 4;
  }
  con_triangles[num_con_triangles] = idx;
  num_con_triangles++;
}

void csTriangleVertex::AddVertex (int idx)
{
  int i;
  for (i = 0 ; i < num_con_vertices ; i++)
    if (con_vertices[i] == idx) return;

  if (num_con_vertices >= max_con_vertices)
  {
    int* new_con_vertices = new int [max_con_vertices+4];
    if (con_vertices)
    {
      memcpy (new_con_vertices, con_vertices, sizeof (int)*max_con_vertices);
      delete [] con_vertices;
    }
    con_vertices = new_con_vertices;
    max_con_vertices += 4;
  }
  con_vertices[num_con_vertices] = idx;
  num_con_vertices++;
}

bool csTriangleVertex::DelVertex (int idx)
{
  int i;
  for (i = 0 ; i < num_con_vertices ; i++)
    if (con_vertices[i] == idx)
    {
      if (i != num_con_vertices-1)
      	memmove (con_vertices+i, con_vertices+i+1,
		sizeof (int)*(num_con_vertices-i-1));
      num_con_vertices--;
      return true;
    }
  return false;
}

void csTriangleVertex::ReplaceVertex (int old, int replace)
{
  if (DelVertex (old)) AddVertex (replace);
}

void csTriangleVertex::CalculateCost (csTriangleVertices* vertices,
	csTerrFuncObject* terrfunc)
{
  int i;
  to_vertex = -1;
  float min_dheight = 1000000.;
  if (deleted)
  {
    // If the vertex is deleted we have a very high cost.
    // The cost is higher than the maximum cost you can get for
    // a non-deleted vertex. This is to make sure that we get
    // the last non-deleted vertex at the end of the LOD algorithm.
    cost = min_dheight+1;
    return;
  }
  if (false)//@@@ TEST FOR CORNER VERTEX)
  {
    // If the vertex is on the corner of this mesh block then
    // we give it a very high cost as well. We cannot allow deletion
    // of corner vertices.
    cost = min_dheight+1;
    return;
  }

  csVector3 vv = vertices->GetVertex (idx).pos;
  csVector2 this_pos (vv.x, vv.z);
  float this_height = vv.y;
  if (true)//@@@ Vertex not on edge)
    this_height = terrfunc->height_func (terrfunc->height_func_data,
  	vertices->GetVertex (idx).dx,
  	vertices->GetVertex (idx).dy);
  for (i = 0 ; i < num_con_vertices ; i++)
  {
    // Here we calculate what will happen to the height at position
    // of this vertex to see how high the cost is. Ideally we should
    // also look at normals here (@@@) but we only consider height for now.
    // Note that we compare the using the height from the original
    // function because that is closer to what we want. Using this it is
    // even possible that going to a new LOD level will increase accuracy :-)
    // Note that we still compare to the vertex position from the mesh
    // for vertices on the edge because we want to remain as close as possible
    // to the previous location instead of the value of the function (so that
    // we fit better to adjacent meshes with lower LOD).

    // First we need to find out in which triangle we are after the
    // collapse.
    int j0, j1, j2;
    csVector2 v[3];
    float height[3];
    j1 = num_con_vertices-2; vv = vertices->GetVertex (con_vertices[j1]).pos;
    v[1].Set (vv.x, vv.z); height[1] = vv.y;
    j2 = num_con_vertices-1; vv = vertices->GetVertex (con_vertices[j2]).pos;
    v[2].Set (vv.x, vv.z); height[2] = vv.y;
    for (j0 = 0 ; j0 < num_con_vertices ; j0++)
    {
      vv = vertices->GetVertex (con_vertices[j0]).pos;
      v[0].Set (vv.x, vv.z); height[0] = vv.y;
      // v[0..2] is a triangle. Check if our point is in it (only consider
      // x and z).
      if (csPoly2D::In (v, 3, this_pos))
      {
	// Found the triangle!
        break;
      }

      j2 = j1; v[2] = v[1]; height[2] = height[1];
      j1 = j0; v[1] = v[0]; height[1] = height[0];
    }

    // Now we find out what height our original point will have when
    // rendered in this triangle. This is done with interpolation.
    // Find the top vertex of the triangle.
    int top;
    if (v[0].y < v[1].y)
      if (v[0].y < v[2].y) top = 0;
      else top = 2;
    else if (v[1].y < v[2].y) top = 1;
    else top = 2;
    int _vbl, _vbr;
    if (top <= 0) _vbl = 2; else _vbl = top - 1;
    if (top >= 2) _vbr = 0; else _vbr = top + 1;

    // Rare special case is when triangle edge on, vertices satisfy
    //  *--------->x     a == b && (a.y < c.y) && (a.x > c.x)
    //  |  *a,b          and is clipped at c, where v[0]
    //  | /              can be either a, b or c. In other words when
    //  |/               the single vertex is not 'top' and clipped.
    // /|*c              
    //  |                The '-= EPSILON' for both left and right 
    //  y     fig. 2     is fairly arbitrary, this probably needs to be refined.
    if (v[top] == v[_vbl]) v[_vbl].x -= EPSILON;
    if (v[top] == v[_vbr]) v[_vbr].x -= EPSILON;

    // Find the original triangle top/left/bottom/right vertices
    // between which the desired point is located.
    int vtl = top, vtr = top, vbl = _vbl, vbr = _vbr;
    float x = this_pos.x;
    float y = this_pos.y;
    int ry = QRound (y); 
    if (ry > QRound (v[vbl].y))
    {
      vtl = vbl;
      if (--vbl < 0) vbl = 2;
    }
    else if (ry > QRound (v[vbr].y))
    {
      vtr = vbr;
      if (++vbr > 2) vbr = 0;
    }
    // Now interpolate the height.
    float tL, tR, xL, xR, tX;
    if (QRound (v[vbl].y) != QRound (v[vtl].y))
      tL = (y - v[vtl].y) / (v[vbl].y - v[vtl].y);
    else
      tL = (x - v[vtl].x) / (v[vbl].x - v[vtl].x);
    if (QRound (v[vbr].y) != QRound (v[vtr].y))
      tR = (y - v[vtr].y) / (v[vbr].y - v[vtr].y);
    else
      tR = (x - v[vtr].x) / (v[vbr].x - v[vtr].x);

    xL = v[vtl].x + tL * (v[vbl].x - v[vtl].x);
    xR = v[vtr].x + tR * (v[vbr].x - v[vtr].x);
    tX = xR - xL;
    if (tX) tX = (x - xL) / tX;

#   define INTERPOLATE(val,tl,bl,tr,br)	\
    {					\
      float vl,vr;				\
      if (tl != bl)				\
        vl = tl + (bl - tl) * tL;		\
      else					\
        vl = tl;				\
      if (tr != br)				\
        vr = tr + (br - tr) * tR;		\
      else					\
        vr = tr;				\
      val = vl + (vr - vl) * tX;		\
    }

    // Calculate interpolated height.
    float new_height;
    INTERPOLATE(new_height, height[vtl], height[vbl], height[vtr], height[vbr]);

    float dheight = ABS (new_height - this_height);
    if (dheight < min_dheight)
    {
      min_dheight = dheight;
      to_vertex = con_vertices[i];
    }
  }
  cost = min_dheight;
}

csTriangleVertices::csTriangleVertices (const G3DTriangleMesh& mesh,
	csTerrFuncObject* terrfunc)
{
  vertices = new csTriangleVertex [mesh.num_vertices];
  num_vertices = mesh.num_vertices;

  // Build connectivity information for all vertices in this mesh.
  csTriangle* triangles = mesh.triangles;
  int i, j;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].pos = mesh.vertices[0][i];
    csVector3 v = vertices[i].pos - terrfunc->topleft;
    v.x /= terrfunc->scale.x;
    v.z /= terrfunc->scale.z;
    vertices[i].dx = v.x;
    vertices[i].dy = v.z;
    vertices[i].idx = i;
    for (j = 0 ; j < mesh.num_triangles ; j++)
      if (triangles[j].a == i || triangles[j].b == i || triangles[j].c == i)
      {
        vertices[i].AddTriangle (j);
	if (triangles[j].a != i) vertices[i].AddVertex (triangles[j].a);
	if (triangles[j].b != i) vertices[i].AddVertex (triangles[j].b);
	if (triangles[j].c != i) vertices[i].AddVertex (triangles[j].c);
      }
  }
}

csTriangleVertices::~csTriangleVertices ()
{
  delete [] vertices;
}

void csTriangleVertices::UpdateVertices (csVector3* verts)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].pos = verts[i];
}

int csTriangleVertices::GetMinimalCostVertex ()
{
  int i;
  int min_idx = -1;
  float min_cost = 2.+1000000.;
  for (i = 0 ; i < num_vertices ; i++)
    if (!vertices[i].deleted && vertices[i].cost < min_cost)
    {
      min_idx = i;
      min_cost = vertices[i].cost;
    }
  return min_idx;
}

void csTriangleVertices::CalculateCost (csTerrFuncObject* terrfunc)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].CalculateCost (this, terrfunc);
}


//---------------------------------------------------------------------------

void csTerrFuncObject::ComputeLODLevel (
	const G3DTriangleMesh& source, G3DTriangleMesh& dest,
	float maxcost)
{
  // Here is a short explanation on the algorithm we are using.
  // Basically we work with edge collapsing. An edge collapse means that
  // we take some edge and move one vertex of the edge towards the other.
  // This removes that edges and also removes all triangles that touch with
  // that edge.
  //
  // The main issue with this algorithm is deciding which is the 'best' edge
  // to collapse first. The best edge collapse is one which causes the least
  // difference in visual appearance. So the shape will be preserved best.
  //
  // To calculate this we use the following algorithm:
  // When collapsing one vertex to another the vertex that will be removed
  // has some height and normal value. After collapsing the edge that vertex
  // will no longer be there so we compute what height and normal will
  // be used there (from interpolating the heights/normals from the triangle
  // that is now where the vertex was). If the difference between the original
  // height/normal and the new interpolated height/normal is large we have
  // a big cost and so an edge collapse for that vertex is not ideal.
  //
  // Because edge collapses at the borders of a mesh are a bit problematic
  // (i.e. adjacent meshes still have to connect well) and edge collapse
  // removing a vertex at the edge will get a bigger cost. It is still
  // possible that the edge collapse will happen but first other alternatives
  // are considered.
  // An edge collapse removing a vertex on the corner will get infinite cost
  // as this is not allowed.
  //
  // This function will continue doing edge collapsing until the collapse
  // with the minimal cost exceeds maxcost.

  int i;

  // Calculate connectivity information.
  csTriangleVertices* verts = new csTriangleVertices (source, this);

  // First copy the triangles table. While doing the lod we will
  // modify the triangles in this table.
  csTriangle* new_triangles = new csTriangle[source.num_triangles];
  memcpy (new_triangles, source.triangles,
  	source.num_triangles * sizeof (csTriangle));

  verts->CalculateCost (this);
  while (true)
  {
    int from = verts->GetMinimalCostVertex ();
    csTriangleVertex& vt_from = verts->GetVertex (from);
    float cost = vt_from.cost;
    if (cost > maxcost) break;
    int to = vt_from.to_vertex;
    csTriangleVertex& vt_to = verts->GetVertex (to);

    // First update all triangles to replace the 'from' vertex with the
    // 'to' vertex. This can basically collapse triangles to a flat triangle.
    // Later we will detect that and delete those degenerated triangles.
    for (i = 0 ; i < vt_from.num_con_triangles ; i++)
    {
      int id = vt_from.con_triangles[i];
      csTriangle& tr = new_triangles[id];
      if (tr.a == from) { tr.a = to; vt_to.AddTriangle (id); }
      if (tr.b == from) { tr.b = to; vt_to.AddTriangle (id); }
      if (tr.c == from) { tr.c = to; vt_to.AddTriangle (id); }
    }
    // Fix connectivity information for the vertices.
    for (i = 0 ; i < vt_from.num_con_vertices ; i++)
    {
      int id = vt_from.con_vertices[i];
      if (id != to)
      {
        verts->GetVertex (id).ReplaceVertex (from, to);
	vt_to.AddVertex (id);
      }
    }

    // Delete the 'from' vertex now.
    vt_to.DelVertex (from);
    vt_from.deleted = true;

    // Fix vertex cost information for all relevant vertices.
    vt_from.CalculateCost (verts, this);
    vt_to.CalculateCost (verts, this);
    for (i = 0 ; i < vt_to.num_con_vertices ; i++)
    {
      int id = vt_to.con_vertices[i];
      verts->GetVertex (id).CalculateCost (verts, this);
    }
  }

  delete[] new_triangles;
  delete verts;
}

void csTerrFuncObject::ComputeNormals ()
{
  int lod;
  int i1, i, x, y;
  float dx, dy;
  float inv_totx = 1. / float (1+blockx*gridx[0]);
  float inv_toty = 1. / float (1+blocky*gridy[0]);
  csVector3 n, v[8];
  for (lod = 0 ; lod < 4 ; lod++)
  {
    int totx = 1+blockx*gridx[lod];
    int toty = 1+blocky*gridy[lod];
    float ftotx = float (totx);
    float ftoty = float (toty);
    for (y = 0 ; y < toty ; y++)
    {
      dy = float (y) / ftoty;
      for (x = 0 ; x < totx ; x++)
      {
	dx = float (x) / ftotx;
	if (normal_func)
	  n = normal_func (normal_func_data, dx, dy);
	else
	{
	  v[0].Set (-.1, height_func (height_func_data, dx-inv_totx, dy-inv_toty), -.1);
	  v[1].Set (  0, height_func (height_func_data, dx, dy-inv_toty), -.1);
	  v[2].Set ( .1, height_func (height_func_data, dx+inv_totx, dy-inv_toty), -.1);
	  v[3].Set ( .1, height_func (height_func_data, dx+inv_totx, dy), 0);
	  v[4].Set ( .1, height_func (height_func_data, dx+inv_totx, dy+inv_toty),  .1);
	  v[5].Set (  0, height_func (height_func_data, dx, dy+inv_toty),  .1);
	  v[6].Set (-.1, height_func (height_func_data, dx-inv_totx, dy+inv_toty),  .1);
	  v[7].Set (-.1, height_func (height_func_data, dx-inv_totx, dy), 0);
	  n.Set (0, 0, 0);
	  i1 = 7;
	  for (i = 0 ; i < 8 ; i++)
	  {
	    n += (v[i1] % v[i]).Unit ();
	    i1 = i;
	  }
	  n.Normalize ();
	}
	normals[lod][y*totx+x] = n;
      }
    }
  }
}

void csTerrFuncObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    if (!materials) materials = new iMaterialWrapper* [blockx * blocky];

    delete[] block_centers;
    block_centers = new csVector3 [blockx * blocky];
    int bx, by;
    int blidx = 0;
    for (by = 0 ; by < blocky ; by++)
    {
      float dy = (float (by)+.5) / float (blocky);
      for (bx = 0 ; bx < blockx ; bx++, blidx++)
      {
        float dx = (float (bx)+.5) / float (blockx);
	csVector3 tl = topleft;
	tl.x += (float (bx) + .5)*scale.x;
	tl.y += height_func (height_func_data, dx, dy)*scale.y;
	tl.z += (float (by) + .5)*scale.z;
	block_centers[blidx] = tl;
      }
    }
    
    int lod;
    for (lod = 0 ; lod < 4 ; lod++)
    {
      delete[] trimesh[lod];
      delete[] normals[lod];
      delete[] dirlight_numbers[lod];
      G3DTriangleMesh* meshes = new G3DTriangleMesh [blockx * blocky];
      trimesh[lod] = meshes;
      normals[lod] = new csVector3 [(1+blockx*gridx[lod])
      	* (1+blocky*gridy[lod])];
      dirlight_numbers[lod] = new long [blockx * blocky];
    }

    ComputeNormals ();

    for (lod = 0 ; lod < 4 ; lod++)
    {
      G3DTriangleMesh* meshes = trimesh[lod];
      int gx, gy;
      for (by = 0 ; by < blocky ; by++)
        for (bx = 0 ; bx < blockx ; bx++)
	{
	  dirlight_numbers[lod][by*blockx+bx] = -1;
	  G3DTriangleMesh* m = &meshes[by*blockx + bx];
	  m->vertex_colors[0] = NULL;
    	  m->morph_factor = 0;
    	  m->num_vertices_pool = 1;
    	  m->num_materials = 1;
    	  m->use_vertex_color = true;
    	  m->do_morph_texels = false;
    	  m->do_morph_colors = false;
    	  m->do_fog = false;
    	  m->vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    	  m->fxmode = CS_FX_GOURAUD;
	  m->num_vertices = (gridx[lod]+1) * (gridy[lod]+1);
	  m->vertices[0] = new csVector3[m->num_vertices];
	  m->texels[0][0] = new csVector2[m->num_vertices];
	  m->vertex_fog = new G3DFogInfo [m->num_vertices];
	  m->vertex_colors[0] = new csColor[m->num_vertices];
	  csVector3 tl = topleft;
	  tl.x += bx*scale.x;
	  tl.y += 0;
	  tl.z += by*scale.z;
	  csVector2 tluv (0, 0);
#if 0
	  tluv.x += float (bx) / float (blockx);
	  tluv.y += float (by) / float (blocky);
#endif
	  float dx, dy;
	  for (gy = 0 ; gy <= gridy[lod] ; gy++)
	  {
	    dy = float (by*gridy[lod]+gy) / float (1. + blocky * gridy[lod]);
	    for (gx = 0 ; gx <= gridx[lod] ; gx++)
	    {
	      dx = float (bx*gridx[lod]+gx) / float (1. + blockx * gridx[lod]);
	      int vtidx = gy*(gridx[lod]+1)+gx;
	      csVector3 v = tl;
	      v.x += gx*scale.x / float (gridx[lod]);
	      v.y += height_func (height_func_data, dx, dy) * scale.y;
	      v.z += gy*scale.z / float (gridy[lod]);
	      m->vertices[0][vtidx] = v;
	      csVector2 uv = tluv;
#if 0
	      uv.x += float (gx) / float (blockx*gridx[lod]);
	      uv.y += float (gy) / float (blocky*gridy[lod]);
#else
	      uv.x += float (gx) / float (gridx[lod]);
	      uv.y += float (gy) / float (gridy[lod]);
	      //uv.x = uv.x * .999;// + .0005;
	      //uv.y = uv.y * .999;// + .0005;
#endif
	      m->texels[0][0][vtidx] = uv;
	      m->vertex_colors[0][vtidx].Set (1, 1, 1);
	    }
	  }
	  m->num_triangles = 2*gridx[lod]*gridy[lod];
	  m->triangles = new csTriangle [m->num_triangles];
	  for (gy = 0 ; gy < gridy[lod] ; gy++)
	    for (gx = 0 ; gx < gridx[lod] ; gx++)
	    {
	      int tridx = (gy*gridx[lod]+gx) * 2;
	      int vtidx = gy*(gridx[lod]+1)+gx;	// Top left vertex of triangles
	      m->triangles[tridx].c = vtidx;
	      m->triangles[tridx].b = vtidx+1;
	      m->triangles[tridx].a = vtidx+gridx[lod]+1;
	      m->triangles[tridx+1].c = vtidx+1;
	      m->triangles[tridx+1].b = vtidx+gridx[lod]+1+1;
	      m->triangles[tridx+1].a = vtidx+gridx[lod]+1;
	    }
        }
    }
  }
}

void csTerrFuncObject::RecomputeLighting (int lod, int bx, int by)
{
  if (!do_dirlight) return;
  int blidx = by*blockx + bx;
  if (dirlight_number != dirlight_numbers[lod][blidx])
  {
    dirlight_numbers[lod][blidx] = dirlight_number;
    G3DTriangleMesh* meshes = trimesh[lod];
    G3DTriangleMesh* m = &meshes[blidx];
    int gx, gy;
    csColor* vtcols = m->vertex_colors[0];
    int normidx_dim = 1+blockx*gridx[lod];
    int normidx = by*gridy[lod]*normidx_dim + bx*gridx[lod];
    csVector3* norms_y = &normals[lod][normidx];
    for (gy = 0 ; gy <= gridy[lod] ; gy++, norms_y += normidx_dim)
    {
      csVector3* norms = norms_y;
      for (gx = 0 ; gx <= gridx[lod] ; gx++)
      {
	float l = dirlight * *(norms++);
	(*(vtcols++)).Set (
		base_color.red+dirlight_color.red * l,
		base_color.green+dirlight_color.green * l,
		base_color.blue+dirlight_color.blue * l);
      }
    }
  }
}

void csTerrFuncObject::Draw (iRenderView* rview, bool use_z_buf)
{
  SetupObject ();
  iGraphics3D* pG3D = rview->GetGraphics3D ();
  iCamera* pCamera = rview->GetCamera ();

  csReversibleTransform& camtrans = pCamera->GetTransform ();
  const csVector3& origin = camtrans.GetOrigin ();
  iClipper2D* pClipper = rview->GetClipper ();
  pG3D->SetObjectToCamera (&camtrans);
  pG3D->SetClipper (pClipper->GetClipPoly (), pClipper->GetNumVertices ());
  pG3D->SetPerspectiveAspect (pCamera->GetFOV ());
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE,
      use_z_buf ? CS_ZBUF_USE : CS_ZBUF_FILL);

  int bx, by;
  int blidx = 0;
  for (by = 0 ; by < blocky ; by++)
  {
    for (bx = 0 ; bx < blockx ; bx++, blidx++)
    {
      csVector3& bc = block_centers[blidx];
      int lod = 0;
      //float sqdist = csSquaredDist::PointPoint (bc, origin);
      //if (sqdist > lod_sqdist1) lod++;
      //if (sqdist > lod_sqdist2) lod++;
      //if (sqdist > lod_sqdist3) lod++;
      RecomputeLighting (lod, bx, by);
      G3DTriangleMesh* meshes = trimesh[lod];
      G3DTriangleMesh* m = &meshes[blidx];
      m->mat_handle[0] = materials[blidx]->GetMaterialHandle ();
      if (!m->mat_handle[0])
        m->mat_handle[0] = materials[0]->GetMaterialHandle ();
      m->do_mirror = pCamera->IsMirrored ();
      m->do_clip = true;
      pG3D->DrawTriangleMesh (*m);
    }
  }
}

int csTerrFuncObject::CollisionDetect (csTransform* transform)
{
  // Translate us into terrain coordinate space.
  csVector3 p = transform->GetOrigin () - topleft;
  p.x /= scale.x * float (blockx);
  p.z /= scale.z * float (blocky);
  // If our location is outside the terrain then we cannot hit it.
  if (p.x < 0 || p.z < 0 || p.x > 1 || p.z > 1) return 0;

  // Return height of terrain at this location in Y coord.
  float h = height_func (height_func_data, p.x, p.z)*scale.y+2;
  if (h < p.y) return 0;
  p.y = h;
  // Translate us back.
  p.x *= scale.x * float (blockx);
  p.z *= scale.z * float (blocky);
  p = p + topleft;
  transform->SetOrigin (p);
  return 1;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csTerrFuncObjectFactory)
  IMPLEMENTS_INTERFACE (iTerrainObjectFactory)
IMPLEMENT_IBASE_END

csTerrFuncObjectFactory::csTerrFuncObjectFactory (iSystem* pSys)
{
  CONSTRUCT_IBASE (NULL);
  pSystem = pSys;
}

csTerrFuncObjectFactory::~csTerrFuncObjectFactory ()
{
}

iTerrainObject* csTerrFuncObjectFactory::NewInstance ()
{
  csTerrFuncObject* pTerrObj = new csTerrFuncObject (pSystem, this);
  return (iTerrainObject*)pTerrObj;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csTerrFuncObjectType)
  IMPLEMENTS_INTERFACE (iTerrainObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csTerrFuncObjectType)

EXPORT_CLASS_TABLE (terrfunc)
  EXPORT_CLASS (csTerrFuncObjectType, "crystalspace.terrain.object.terrfunc",
    "Crystal Space Function Terrain Type")
EXPORT_CLASS_TABLE_END

csTerrFuncObjectType::csTerrFuncObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csTerrFuncObjectType::~csTerrFuncObjectType ()
{
}

bool csTerrFuncObjectType::Initialize (iSystem *pSys)
{
  pSystem = pSys;
  return true;
}

iTerrainObjectFactory* csTerrFuncObjectType::NewFactory()
{
  csTerrFuncObjectFactory *pFactory = new csTerrFuncObjectFactory (pSystem);
  return (iTerrainObjectFactory*)pFactory;
}

