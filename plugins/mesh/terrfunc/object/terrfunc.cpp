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
#include "csutil/dirtyaccessarray.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "ivideo/txtmgr.h"
#include "igraphic/image.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "imap/ldrctxt.h"
#include "csgfx/rgbpixel.h"
#include "terrfunc.h"
#include "terrvis.h"
#include "terrdiv.h"
#include "csqint.h"
#include "csqsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csTerrFuncObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTerrFuncState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrFuncObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrFuncObject::TerrFuncState)
  SCF_IMPLEMENTS_INTERFACE (iTerrFuncState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrFuncObject::eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

//------------------------------------------------------------------------

csTerrBlock::csTerrBlock ()
{
  int i;
  for (i = 0 ; i < LOD_LEVELS ; i++)
  {
    memset (&mesh[i], 0, sizeof (G3DTriangleMesh));
    normals[i] = 0;
    dirlight_numbers[i] = -1;
    mesh_vertices[i] = 0;
    mesh_texels[i] = 0;
    mesh_colors[i] = 0;
  }
  material = 0;
  node = 0;
  quaddiv = 0;
  quaddiv_visible = false;
  qd_portal = false;
  qd_plane = false;
  qd_z_plane = false;
}

csTerrBlock::~csTerrBlock ()
{
  int i;
  for (i = 0 ; i < LOD_LEVELS ; i++)
  {
    delete[] mesh_vertices[i];
    delete[] mesh_colors[i];
    delete[] mesh_texels[i];
    delete[] mesh[i].vertex_fog;
    delete[] mesh[i].triangles;
    delete[] normals[i];
  }
  delete quaddiv; quaddiv = 0;
}

/// static texture computing helpder
struct qd_texuv_data {
  /// who is calling
  csTerrBlock *block;
  csTerrFuncObject *terr;
  /// texture correction
  float correct_du, correct_su, correct_dv, correct_sv;
};
static void qd_texuv_func(void *data, csVector2& uv, float x, float y)
{
  struct qd_texuv_data *dat = (struct qd_texuv_data *)data;
  float minx = dat->block->bbox.MinX();
  float miny = dat->block->bbox.MinZ();
  float boxxsize = dat->block->bbox.MaxX() - minx;
  float boxysize = dat->block->bbox.MaxZ() - miny;

  // obtain texture coords
  uv.Set(x - minx, y - miny);
  uv.x /= boxxsize; uv.y /= boxysize;
  uv.x = uv.x * dat->correct_du + dat->correct_su;
  uv.y = uv.y * dat->correct_dv + dat->correct_sv;
}

void csTerrBlock::PrepareQuadDiv(iTerrainHeightFunction *height_func,
  csTerrFuncObject *terr)
{
  struct qd_texuv_data dat;
  dat.correct_du = terr->correct_du;
  dat.correct_su = terr->correct_su;
  dat.correct_dv = terr->correct_dv;
  dat.correct_sv = terr->correct_sv;

  dat.terr = terr;
  dat.block = this;

  quaddiv->ComputeDmax(height_func, qd_texuv_func, (void*)&dat,
    terr->quad_normal, bbox.MinX(), bbox.MinZ(), bbox.MaxX(), bbox.MaxZ());
}

/// static light computing helper
struct qd_light_data {
  /// who is calling
  csTerrBlock *block;
  csTerrFuncObject *terr;
};
static void qd_light_func(void* data, csColor& col, float /*x*/, float /*y*/,
  const csVector3& normal)
{
  struct qd_light_data *dat = (struct qd_light_data *)data;
  /// need to precompute a light-map texture
  col = dat->terr->base_color;
  if(dat->terr->do_dirlight && dat->terr->quad_normal)
  {
    float l;
    l = dat->terr->dirlight * normal;
    //dat->terr->quad_normal->GetNormal(x,y);
    if(l>0) col += dat->terr->dirlight_color * l;
  }
}

void csTerrBlock::PrepareFrame(const csVector3& campos, int framenum,
  csTerrFuncObject *terr)
{
  CS_ASSERT(quaddiv);
  /// compute LOD
  static qd_light_data dat;
  dat.block = this;
  dat.terr = terr;
  quaddiv->ComputeLOD(framenum, campos, qd_light_func, (void*)&dat,
    bbox.MinX(), bbox.MinZ(), bbox.MaxX(), bbox.MaxZ());
  //printf("campos %g,%g,%g\n", campos.x, campos.y, campos.z);
  //printf("estimated triangles: %d\n", quaddiv->EstimateTris(framenum));
}

/// data to pass for drawing
struct terrdata {
  /// traingle mesh to draw for block
  G3DTriangleMesh mesh;
  /// keep track of tris, verts, tex, color
  csDirtyAccessArray<csTriangle> triangles;
  csDirtyAccessArray<csVector3> vertices;
  csDirtyAccessArray<csVector2> texels;
  csDirtyAccessArray<csColor> colors;
  /// the block
  csTerrBlock *block;
  /// texture correction
  float correct_du, correct_su, correct_dv, correct_sv;
  /// the terrain
  csTerrFuncObject *terr;
};

/// get triangles
static void TerrTri(void *userdata, const csVector3& t1, 
  const csVector3& t2, const csVector3& t3, 
  const csVector2& uv1, const csVector2& uv2, const csVector2& uv3,
  const csColor& col1, const csColor& col2, const csColor& col3
  )
{
  struct terrdata *dat = (struct terrdata*)userdata;
  /// get indices for triangle
  int trinum = dat->vertices.Length();
  //csVector3 t1 = zt1;
  //csVector3 t2 = zt2;
  //csVector3 t3 = zt3;
  //t1.y = dat->terr->quad_height->GetHeight(t1.x, t1.z);
  //t2.y = dat->terr->quad_height->GetHeight(t2.x, t2.z);
  //t3.y = dat->terr->quad_height->GetHeight(t3.x, t3.z);
  //printf("triheights %g %g %g\n", dat->terr->quad_height->GetHeight(t1.x, t1.z),
    //dat->terr->quad_height->GetHeight(t2.x, t2.z),
    //dat->terr->quad_height->GetHeight(t3.x, t3.z));
  dat->vertices.Push(t1);
  dat->vertices.Push(t2);
  dat->vertices.Push(t3);
  csTriangle tri;
  tri.a = trinum;
  tri.b = trinum+1;
  tri.c = trinum+2;
  dat->triangles.Push( tri );

  dat->colors.Push(col1);
  dat->colors.Push(col2);
  dat->colors.Push(col3);
  /*********
  /// hack for light!
  /// need to precompute a light-map texture
  csColor res1 = dat->terr->base_color;
  csColor res2 = dat->terr->base_color;
  csColor res3 = dat->terr->base_color;
  if(dat->terr->do_dirlight && dat->terr->quad_normal)
  {
    float l;
    l = dat->terr->dirlight * dat->terr->quad_normal->GetNormal(t1.x,t1.z);
    if(l>0) res1 += dat->terr->dirlight_color * l;
    l = dat->terr->dirlight * dat->terr->quad_normal->GetNormal(t2.x,t2.z);
    if(l>0) res2 += dat->terr->dirlight_color * l;
    l = dat->terr->dirlight * dat->terr->quad_normal->GetNormal(t3.x,t3.z);
    if(l>0) res3 += dat->terr->dirlight_color * l;
  }
  dat->colors.Push(res1);
  dat->colors.Push(res2);
  dat->colors.Push(res3);
  *****/

  dat->texels.Push( uv1 );
  dat->texels.Push( uv2 );
  dat->texels.Push( uv3 );
  /****
  float minx = dat->block->bbox.MinX();
  float miny = dat->block->bbox.MinZ();
  float boxxsize = dat->block->bbox.MaxX() - minx;
  float boxysize = dat->block->bbox.MaxZ() - miny;

  // obtain texture coords
  csVector2 uv(t1.x - minx, t1.z - miny);
  uv.x /= boxxsize; uv.y /= boxysize;
  uv.x = uv.x * dat->correct_du + dat->correct_su;
  uv.y = uv.y * dat->correct_dv + dat->correct_sv;
  dat->texels.Push( uv );
  uv.Set(t2.x - minx, t2.z - miny);
  uv.x /= boxxsize; uv.y /= boxysize;
  uv.x = uv.x * dat->correct_du + dat->correct_su;
  uv.y = uv.y * dat->correct_dv + dat->correct_sv;
  dat->texels.Push( uv );
  uv.Set(t3.x - minx, t3.z - miny);
  uv.x /= boxxsize; uv.y /= boxysize;
  uv.x = uv.x * dat->correct_du + dat->correct_su;
  uv.y = uv.y * dat->correct_dv + dat->correct_sv;
  dat->texels.Push( uv );
  ***/
}

void csTerrBlock::Draw(iRenderView *rview, bool clip_portal, bool clip_plane,
  bool clip_z_plane, float correct_du, float correct_su, float correct_dv, 
  float correct_sv, csTerrFuncObject *terr, int framenum)
{
  /// collect triangles and draw them
  iGraphics3D *pG3D = rview->GetGraphics3D();
  iCamera* pCamera = rview->GetCamera();
  iVertexBufferManager* vbufmgr = pG3D->GetVertexBufferManager();
  csReversibleTransform& camtrans = pCamera->GetTransform ();

  struct terrdata m; // my data
  m.mesh.vertex_fog = 0;
  m.mesh.morph_factor = 0;
  m.mesh.num_vertices_pool = 1;
  m.mesh.use_vertex_color = true;
  m.mesh.do_morph_texels = false;
  m.mesh.do_morph_colors = false;
  m.mesh.do_fog = false;
  m.mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  m.mesh.mixmode = CS_FX_COPY;
  /// can use vbuf[0]
  m.mesh.buffers[0] = vbuf[0];
  m.mesh.buffers[1] = 0;

  m.mesh.num_triangles = 0;
  m.mesh.triangles = 0;
  m.mesh.do_mirror = pCamera->IsMirrored();
  m.mesh.clip_portal = clip_portal;
  m.mesh.clip_plane = clip_plane;
  m.mesh.clip_z_plane = clip_z_plane;
  m.mesh.mat_handle = material->GetMaterialHandle();

  m.correct_du = correct_du;
  m.correct_su = correct_su;
  m.correct_dv = correct_dv;
  m.correct_sv = correct_sv;

  m.terr = terr;
  m.block = this;

  CS_ASSERT(quaddiv);
  quaddiv->Triangulate(TerrTri, (void*)&m, framenum,
    bbox.MinX(), bbox.MinZ(), bbox.MaxX(), bbox.MaxZ());

  if( (m.vertices.Length() <= 0) || (m.triangles.Length() <= 0))
    return;

  m.mesh.num_triangles = m.triangles.Length();
  m.mesh.triangles = m.triangles.GetArray();
  //printf("terrfuncdiv block %x: %d triangles\n", (int)this,
    //m.mesh.num_triangles);

  CS_ASSERT(m.mesh.buffers[0]);
  CS_ASSERT(!m.mesh.buffers[0]->IsLocked ());
  vbufmgr->LockBuffer(m.mesh.buffers[0],
    m.vertices.GetArray(), m.texels.GetArray(), m.colors.GetArray(), 
    m.vertices.Length(), 0, bbox);
  rview->CalculateFogMesh(camtrans, m.mesh);
  pG3D->DrawTriangleMesh(m.mesh);
  vbufmgr->UnlockBuffer (m.mesh.buffers[0]);
}

//---- divisor height function --------------------------
struct QuadDivHeightFunc : public iTerrainHeightFunction
{
  /// height func to wrap
  iTerrainHeightFunction *hf;
  /// scale and offset in world
  float scx, scy, offx, offy;
  /// scale and offset of heights
  float sch, offh;

  SCF_DECLARE_IBASE;
  QuadDivHeightFunc () 
  { 
    SCF_CONSTRUCT_IBASE (0); 
    hf=0; scx=scy=sch=1.0; offx=offy=offh=0.0f;
  }
  virtual ~QuadDivHeightFunc () 
  {
    SCF_DESTRUCT_IBASE (); 
  }
  /// get height using world space in world space answers
  virtual float GetHeight (float x, float y)
  {
    float localx = x * scx + offx;
    float localy = y * scy + offy;
    localx -= floor(localx);
    localy -= floor(localy);
    float res = hf->GetHeight(localx, localy);
    res = res*sch + offh;
    //printf("QDHF %g, %g -> (%g,%g) to %g\n", x, y, localx, localy, res);
    return res;
  }
};

SCF_IMPLEMENT_IBASE (QuadDivHeightFunc)
  SCF_IMPLEMENTS_INTERFACE (iTerrainHeightFunction)
SCF_IMPLEMENT_IBASE_END

//---- divisor normal function --------------------------
struct QuadDivNormalFunc : public iTerrainNormalFunction
{
  /// Normal func to wrap
  iTerrainNormalFunction *nf;
  /// Height func to wrap (and approx a normal)
  iTerrainHeightFunction *hf;
  /// scale and offset in world
  float scx, scy, offx, offy;
  /// inverse tox
  float inv_totx, inv_toty;

  SCF_DECLARE_IBASE;
  QuadDivNormalFunc () 
  { 
    SCF_CONSTRUCT_IBASE (0); 
    nf=0; hf=0;
    }
  virtual ~QuadDivNormalFunc () 
  {
    SCF_DESTRUCT_IBASE (); 
  }
  /// get Normal using world space in world space answers
  virtual csVector3 GetNormal (float x, float y)
  {
    float dx = x * scx + offx;
    float dy = y * scy + offy;
    if(nf)
    {
      return nf->GetNormal(dx, dy);
    }

    CS_ASSERT(hf);
    csVector3 n(0,0,0);
    csVector3 v[8];
    v[0].Set(-0.1f, hf->GetHeight (dx-inv_totx, dy-inv_toty), -0.1f);
    v[1].Set( 0.0f, hf->GetHeight (dx, dy-inv_toty), -0.1f);
    v[2].Set( 0.1f, hf->GetHeight (dx+inv_totx, dy-inv_toty), -0.1f);
    v[3].Set( 0.1f, hf->GetHeight (dx+inv_totx, dy), 0);
    v[4].Set( 0.1f, hf->GetHeight (dx+inv_totx, dy+inv_toty),  0.1f);
    v[5].Set( 0.0f, hf->GetHeight (dx, dy+inv_toty),  0.1f);
    v[6].Set(-0.1f, hf->GetHeight (dx-inv_totx, dy+inv_toty),  0.1f);
    v[7].Set(-0.1f, hf->GetHeight (dx-inv_totx, dy), 0);
    int j1, j;
    j1 = 7;
    for(j = 0; j < 8; j++)
    {
      n += (v[j1] % v[j]).Unit ();
      j1 = j;
    }
    n.Normalize();
    return n;
  }
};

SCF_IMPLEMENT_IBASE (QuadDivNormalFunc)
  SCF_IMPLEMENTS_INTERFACE (iTerrainNormalFunction)
SCF_IMPLEMENT_IBASE_END

//------------------------------------------------------------------------

struct DefaultFunction : public iTerrainHeightFunction
{
  SCF_DECLARE_IBASE;
  DefaultFunction () 
  { 
    SCF_CONSTRUCT_IBASE (0); 
  }
  virtual ~DefaultFunction () 
  { 
    SCF_DESTRUCT_IBASE (); 
  }
  virtual float GetHeight (float x, float y)
  {
    return 8. * (sin (x*40.)+cos (y*40.));
  }
};

SCF_IMPLEMENT_IBASE (DefaultFunction)
  SCF_IMPLEMENTS_INTERFACE (iTerrainHeightFunction)
SCF_IMPLEMENT_IBASE_END

struct HeightMapData : public iTerrainHeightFunction
{
  iImage* im;
  int iw, ih;	// Image width and height.
  float w, h;	// Image width and height.
  csRGBpixel* p;
  float hscale, hshift;
  bool flipx, flipy;
  SCF_DECLARE_IBASE;

  HeightMapData () : im (0) 
  { 
    SCF_CONSTRUCT_IBASE (0); 
  }
  virtual ~HeightMapData ()
  {
    if (im) im->DecRef ();
    SCF_DESTRUCT_IBASE (); 
  }
  virtual float GetHeight (float dx, float dy);
};

SCF_IMPLEMENT_IBASE (HeightMapData)
  SCF_IMPLEMENTS_INTERFACE (iTerrainHeightFunction)
SCF_IMPLEMENT_IBASE_END

float HeightMapData::GetHeight (float x, float y)
{
  if(flipx) x = 1.0f-x;
  if(flipy) y = 1.0f-y;
  float dw = fmod (x*(w-1), 1.0f);
  float dh = fmod (y*(h-1), 1.0f);
  int ix = int (x*(w-1));
  int iy = int (y*(h-1));
  int idx = iy * iw + ix;
  if (idx >= iw*ih)
    return hshift;
  float col00, col01, col10, col11;
  col00 = float (p[idx].red + p[idx].green + p[idx].blue)/3.;
  if (ix < iw-1)
    col10 = float (p[idx+1].red + p[idx+1].green + p[idx+1].blue)/3.;
  else
    col10 = col00;
  if (iy < ih-1)
    col01 = float (p[idx+iw].red + p[idx+iw].green + p[idx+iw].blue)/3.;
  else
    col01 = col00;
  if (ix < iw-1 && iy < ih-1)
    col11 = float (p[idx+iw+1].red + p[idx+iw+1].green + p[idx+iw+1].blue)/3.;
  else
    col11 = col00;
  float col0010 = col00 * (1-dw) + col10 * dw;
  float col0111 = col01 * (1-dw) + col11 * dw;
  float col = col0010 * (1-dh) + col0111 * dh;
  return col * hscale + hshift;
}

void csTerrFuncObject::SetHeightMap (iImage* im, float hscale, float hshift,
  bool flipx, bool flipy)
{
  HeightMapData* data = new HeightMapData ();
  data->im = im;
  data->iw = im->GetWidth ();
  data->ih = im->GetHeight ();
  data->w = float (data->iw);
  data->h = float (data->ih);
  data->p = (csRGBpixel*)im->GetImageData ();
  data->hscale = hscale;
  data->hshift = hshift;
  data->flipx = flipx;
  data->flipy = flipy;
  data->im->IncRef ();
  SetHeightFunction (data);
  data->DecRef ();
}

csTerrFuncObject::csTerrFuncObject (iObjectRegistry* object_reg,
	iMeshObjectFactory *pFactory)
{
  SCF_CONSTRUCT_IBASE (0)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTerrFuncState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
  csTerrFuncObject::object_reg = object_reg;
  csTerrFuncObject::pFactory = pFactory;
  logparent = 0;
  initialized = false;
  blockxy = 4;
  gridx = 8; gridy = 8;

  grid_stepx = grid_stepy = 1/8;
  inv_block_stepx = 1.;
  inv_block_stepy = 1.;
  inv_grid_stepx = inv_grid_stepy = 8.;

  topleft.Set (0, 0, 0);
  scale.Set (1, 1, 1);
  blocks = 0;
  do_dirlight = false;
  do_vis_test = true;
  dirlight_number = 0;
  dirlight.Set (0, 1, 0);
  base_color.red = 0;
  base_color.green = 0;
  base_color.blue = 0;
  height_func = csPtr<iTerrainHeightFunction> (new DefaultFunction ());
  lod_sqdist[0] = 100*100;
  lod_sqdist[1] = 400*400;
  lod_sqdist[2] = 800*800;
  max_cost[0] = 0.03f;
  max_cost[1] = 0.08f;
  max_cost[2] = 0.2f;
  CorrectSeams (0, 0);
  quad_depth = 6;
  quadtree = 0;
  current_lod = 1;
  current_features = 0;
  vbufmgr = 0;
  quaddiv_enabled = false;
  quad_height = 0;
  quad_normal = 0;
  qd_framenum = 1;
}

csTerrFuncObject::~csTerrFuncObject ()
{
  if (vbufmgr) vbufmgr->RemoveClient (&scfiVertexBufferManagerClient);
  delete[] blocks;
  delete quad_height;
  
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiTerrFuncState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
  SCF_DESTRUCT_IBASE ()
}

void csTerrFuncObject::CorrectSeams (int tw, int th)
{
  correct_tw = tw;
  correct_th = th;
  if (tw)
  {
    correct_du = 1. - 2. / float (tw);
    correct_su = 1. / float (tw);
  }
  else
  {
    correct_du = 1;
    correct_su = 0;
  }
  if (th)
  {
    correct_dv = 1. - 2. / float (th);
    correct_sv = 1. / float (th);
  }
  else
  {
    correct_dv = 1;
    correct_sv = 0;
  }
}

void csTerrFuncObject::LoadMaterialGroup (iLoaderContext* ldr_context,
	const char *pName, int iStart, int iEnd)
{
  if (!blocks || block_dim_invalid)
  {
    blocks = new csTerrBlock [blockxy*blockxy];
    block_dim_invalid = false;
  }
  int i, bx, by, newi;
  char pMatName[256];

  for (i = iStart ; i <= iEnd ; i++)
  {
    sprintf (pMatName, pName, i);
    iMaterialWrapper* mat = ldr_context->FindMaterial (pMatName);
    Index2Block(i, bx, by);
    Block2Index(by, bx, newi);
    blocks[newi].material = mat;
  }
}

//---------------------------------------------------------------------------

class TerrFuncTriangleVertices;

/*
 * A class which holds vertices and connectivity information for a triangle
 * mesh. This is a general vertices structure but it is mostly useful
 * for LOD generation since every vertex contains information which
 * helps selecting the best vertices for collapsing.
 */
class TerrFuncTriangleVertices
{
public:
  /*
   * The representation of a vertex in a triangle mesh.
   * This is basically used as a temporary structure to be able to
   * calculate the cost of collapsing this vertex more quickly.
   */
  class csTriangleVertex
  {
  public:
    // Position of this vertex in 3D space.
    csVector3 pos;
    // Terrain function coordinates for this vertex.
    float dx, dy;
    // True if a corner point.
    bool at_corner;
    // True if a horizontal edge point.
    bool at_hor_edge;
    // True if a vertical edge point.
    bool at_ver_edge;
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

    csTriangleVertex () : deleted (false), con_triangles (0),
                          num_con_triangles (0), max_con_triangles (0),
                          con_vertices (0), num_con_vertices (0), max_con_vertices (0) { }
    ~csTriangleVertex () { delete [] con_triangles; delete [] con_vertices; }
    void AddTriangle (int idx);
    void AddVertex (int idx);
    bool DelVertex (int idx);
    void ReplaceVertex (int old, int replace);

    /*
     * Calculate the minimal cost of collapsing this vertex to some other.
     * Also remember which other vertex was selected for collapsing to.
     */
    void CalculateCost(TerrFuncTriangleVertices* vertices, csTerrFuncObject* terrfunc);
  };

private:
  csTriangleVertex* vertices;
  int num_vertices;


public:
  // Build vertex table for a triangle mesh.
  TerrFuncTriangleVertices (const G3DTriangleMesh& mesh,
  	csVector3* mesh_vertices, int num_mesh_vertices,
  	csTerrFuncObject* terrfunc);
  ///
  ~TerrFuncTriangleVertices ();
  // Update vertex table for a given set of vertices
  // (with the same number as at init).
  void UpdateVertices (csVector3* verts);

  int GetVertexCount () { return num_vertices; }
  csTriangleVertex& GetVertex (int idx) { return vertices[idx]; }

  void CalculateCost (csTerrFuncObject* terrfunc);
  int GetMinimalCostVertex ();
};

void TerrFuncTriangleVertices::csTriangleVertex::AddTriangle (int idx)
{
  int i;
  for (i = 0 ; i < num_con_triangles ; i++)
    if (con_triangles[i] == idx) return;

  if (num_con_triangles >= max_con_triangles)
  {
    int* new_con_triangles = new int [max_con_triangles+4];
    if (con_triangles)
    {
      memcpy(new_con_triangles, con_triangles, sizeof (int)*max_con_triangles);
      delete [] con_triangles;
    }
    con_triangles = new_con_triangles;
    max_con_triangles += 4;
  }
  con_triangles[num_con_triangles] = idx;
  num_con_triangles++;
}

void TerrFuncTriangleVertices::csTriangleVertex::AddVertex (int idx)
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

bool TerrFuncTriangleVertices::csTriangleVertex::DelVertex (int idx)
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

void TerrFuncTriangleVertices::csTriangleVertex::ReplaceVertex (int old, int replace)
{
  if (DelVertex (old)) AddVertex (replace);
}

void TerrFuncTriangleVertices::csTriangleVertex::CalculateCost (TerrFuncTriangleVertices* vertices,
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
  if (at_corner)
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
  if (!at_hor_edge && !at_ver_edge)
    this_height = terrfunc->height_func->GetHeight (
      vertices->GetVertex (idx).dx,
      vertices->GetVertex (idx).dy) * terrfunc->scale.y + terrfunc->topleft.y;
  for (i = 0 ; i < num_con_vertices ; i++)
  {
    // Consider collapsing to vertex i.
    csTriangleVertex& vt_to = vertices->GetVertex (con_vertices[i]);

    if (at_hor_edge && !vt_to.at_hor_edge)
    {
      // This point is at horizontal edge, so we only allow edge collapse
      // to another point at the horizontal edge.
      continue;
    }
    if (at_ver_edge && !vt_to.at_ver_edge)
    {
      // This point is at vertical edge, so we only allow edge collapse
      // to another point at the vertical edge.
      continue;
    }

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

    // Also note that for vertices at the edge we only allow edge
    // collapsing to vertices at the same edge.

    // First we need to find out in which triangle we are after the
    // collapse.
    int j0, j1;
    csVector2 v[3];
    float height[3];
    v[2].Set (vt_to.pos.x, vt_to.pos.z);
    height[2] = vt_to.pos.y;
    j0 = num_con_vertices-1; vv = vertices->GetVertex (con_vertices[j0]).pos;
    v[0].Set (vv.x, vv.z); height[0] = vv.y;
    for (j1 = 0 ; j1 < num_con_vertices ; j1++)
    {
      vv = vertices->GetVertex (con_vertices[j1]).pos;
      v[1].Set (vv.x, vv.z); height[1] = vv.y;
      if (j1 != i && j0 != i)
      {
        // v[0..2] is a triangle. Check if our point is in it (only consider
        // x and z).
        int rc1 = csMath2::WhichSide2D (this_pos, v[0], v[1]);
        int rc2 = csMath2::WhichSide2D (this_pos, v[1], v[2]);
        int rc3 = csMath2::WhichSide2D (this_pos, v[2], v[0]);
        int rc = rc1+rc2+rc3;
        if (rc == 3 || rc == 2 || rc == 0 || rc == -2 || rc == -3 ||
	    (rc1*rc2*rc3 == 0))
        {
	  // Found the triangle!
          break;
        }
      }

      j0 = j1; v[0] = v[1]; height[0] = height[1];
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
    //  y     fig. 2     is fairly arbitrary, this probably needs to be
    //                   refined.
    if (v[top] == v[_vbl]) v[_vbl].x -= EPSILON;
    if (v[top] == v[_vbr]) v[_vbr].x -= EPSILON;
    if (v[_vbr] == v[_vbl]) v[_vbl].x -= EPSILON;//@@@

    // Find the original triangle top/left/bottom/right vertices
    // between which the desired point is located.
    int vtl = top, vtr = top, vbl = _vbl, vbr = _vbr;
    float x = this_pos.x;
    float y = this_pos.y;
    int ry = csQround (y);
    if (ry > csQround (v[vbl].y))
    {
      vtl = vbl;
      if (--vbl < 0) vbl = 2;
    }
    else if (ry > csQround (v[vbr].y))
    {
      vtr = vbr;
      if (++vbr > 2) vbr = 0;
    }
//printf ("a: vbl=%d v[vbl].y=%g vtl=%d v[vtl].y=%g y=%g\n",
//vbl, v[vbl].y, vtl, v[vtl].y, y);
//printf ("a: v[vbl].x=%g v[vtl].x=%g x=%g\n", v[vbl].x, v[vtl].x, x);
    // Now interpolate the height.
    float tL, tR, xL, xR, tX;
    if (csQround (v[vbl].y) != csQround (v[vtl].y))
      tL = (y - v[vtl].y) / (v[vbl].y - v[vtl].y);
    else
      tL = ((v[vbl].x - v[vtl].x) > SMALL_EPSILON) ? (x - v[vtl].x) / (v[vbl].x - v[vtl].x) : 1000000.;
//printf ("b\n");
    if (csQround (v[vbr].y) != csQround (v[vtr].y))
      tR = (y - v[vtr].y) / (v[vbr].y - v[vtr].y);
    else
      tR = ((v[vbr].x - v[vtr].x) > SMALL_EPSILON) ? (x - v[vtr].x) / (v[vbr].x - v[vtr].x) : 1000000.;

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
    // Increase the cost of collapsing an edge vertex.
    if (at_hor_edge || at_ver_edge) dheight *= 2.;

    if (dheight < min_dheight)
    {
      min_dheight = dheight;
      to_vertex = con_vertices[i];
    }
  }
  cost = min_dheight;
}

TerrFuncTriangleVertices::TerrFuncTriangleVertices (const G3DTriangleMesh& mesh,
	csVector3* mesh_vertices,
	int num_mesh_vertices,
	csTerrFuncObject* terrfunc)
{
  int i, j;

  vertices = new csTriangleVertex [num_mesh_vertices];
  num_vertices = num_mesh_vertices;

  // First build a bounding box in 2D for all vertices of this mesh.
  // This bounding box will be used to detect if a vertex is on the
  // corner of edge of the mesh.
  csBox2 box;
  box.StartBoundingBox ();
  for (i = 0 ; i < num_vertices ; i++)
  {
    csVector3 v = mesh_vertices[i] - terrfunc->topleft;
    box.AddBoundingVertex (csVector2 (v.x, v.z));
  }

  // Build connectivity information for all vertices in this mesh.
  csTriangle* triangles = mesh.triangles;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].pos = mesh_vertices[i];
    csVector3 v = vertices[i].pos - terrfunc->topleft;
    bool at_hor_edge = ABS (v.z-box.MinY ()) < .001 ||
      		       ABS (v.z-box.MaxY ()) < .001;
    bool at_ver_edge = ABS (v.x-box.MinX ()) < .001 ||
      		       ABS (v.x-box.MaxX ()) < .001;
    v.x /= terrfunc->scale.x * float (terrfunc->blockxy);
    v.z /= terrfunc->scale.z * float (terrfunc->blockxy);
    vertices[i].dx = v.x;
    vertices[i].dy = v.z;
    vertices[i].at_hor_edge = at_hor_edge;
    vertices[i].at_ver_edge = at_ver_edge;
    vertices[i].at_corner = at_hor_edge && at_ver_edge;
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

TerrFuncTriangleVertices::~TerrFuncTriangleVertices ()
{
  delete [] vertices;
}

void TerrFuncTriangleVertices::UpdateVertices (csVector3* verts)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].pos = verts[i];
}

int TerrFuncTriangleVertices::GetMinimalCostVertex ()
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

void TerrFuncTriangleVertices::CalculateCost (csTerrFuncObject* terrfunc)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].CalculateCost (this, terrfunc);
}


//---------------------------------------------------------------------------

void csTerrFuncObject::ComputeLODLevel (
	const G3DTriangleMesh& source,
	csVector3* source_vertices,
	csVector2* source_texels,
	csColor* source_colors,
	int num_source_vertices,
	G3DTriangleMesh& dest,
	csVector3*& dest_vertices,
	csVector2*& dest_texels,
	csColor*& dest_colors,
	int& num_dest_vertices,
	float maxcost, int& del_tri, int& tot_tri)
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
  TerrFuncTriangleVertices* verts = new TerrFuncTriangleVertices (source,
  	source_vertices, num_source_vertices, this);

  // First copy the triangles table. While doing the lod we will
  // modify the triangles in this table.
  csTriangle* new_triangles = new csTriangle[source.num_triangles];
  memcpy (new_triangles, source.triangles,
  	source.num_triangles * sizeof (csTriangle));

  verts->CalculateCost (this);
  while (true)
  {
    int from = verts->GetMinimalCostVertex ();
    TerrFuncTriangleVertices::csTriangleVertex& vt_from = verts->GetVertex (from);
    float cost = vt_from.cost;
    if (cost > maxcost) break;
    int to = vt_from.to_vertex;
    TerrFuncTriangleVertices::csTriangleVertex& vt_to = verts->GetVertex (to);

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

  // Now we are going to fill the destination with the vertices
  // and triangles. First we count how many vertices and triangles
  // are really left.
  num_dest_vertices = 0;
  int* translate = new int[num_source_vertices];
  for (i = 0 ; i < num_source_vertices ; i++)
  {
    if (!verts->GetVertex (i).deleted)
    {
      translate[i] = num_dest_vertices;
      num_dest_vertices++;
    }
    else
      translate[i] = -1;
  }
  dest_vertices = new csVector3[num_dest_vertices];
  dest_colors = new csColor[num_dest_vertices];
  dest_texels = new csVector2[num_dest_vertices];
  dest.vertex_fog = new G3DFogInfo[num_dest_vertices];
  num_dest_vertices = 0;
  for (i = 0 ; i < num_source_vertices ; i++)
  {
    if (translate[i] != -1)
    {
      dest.vertex_fog[num_dest_vertices] = source.vertex_fog[i];
      dest_texels[num_dest_vertices] = source_texels[i];
      dest_colors[num_dest_vertices] = source_colors[i];
      dest_vertices[num_dest_vertices++] = source_vertices[i];
    }
  }
  dest.num_triangles = 0;
  for (i = 0 ; i < source.num_triangles ; i++)
  {
    if (new_triangles[i].a != new_triangles[i].b &&
	new_triangles[i].a != new_triangles[i].c &&
	new_triangles[i].b != new_triangles[i].c)
    {
      // Triangle is not collapsed.
      dest.num_triangles++;
    }
  }
  del_tri = source.num_triangles - dest.num_triangles;
  tot_tri = source.num_triangles;
  dest.triangles = new csTriangle[dest.num_triangles];
  dest.num_triangles = 0;
  for (i = 0 ; i < source.num_triangles ; i++)
  {
    if (new_triangles[i].a != new_triangles[i].b &&
	new_triangles[i].a != new_triangles[i].c &&
	new_triangles[i].b != new_triangles[i].c)
    {
      // Triangle is not collapsed.
      dest.triangles[dest.num_triangles] = new_triangles[i];
      csTriangle& tr = dest.triangles[dest.num_triangles];
      tr.a = translate[tr.a];
      tr.b = translate[tr.b];
      tr.c = translate[tr.c];
      dest.num_triangles++;
    }
  }

  delete[] translate;
  delete[] new_triangles;
  delete verts;
}

void csTerrFuncObject::ComputeNormals (const G3DTriangleMesh& /*mesh*/,
	csVector3* mesh_vertices,
	int num_mesh_vertices,
    	csVector3** pNormals)
{
  csVector3* normals = new csVector3 [num_mesh_vertices];
  *pNormals = normals;
  float inv_totx = .5 / float (1+blockxy*gridx);
  float inv_toty = .5 / float (1+blockxy*gridy);
  csVector3 v[8];
  int i;
  for (i = 0 ; i < num_mesh_vertices ; i++)
  {
    csVector3 vv = mesh_vertices[i] - topleft;
    vv.x /= scale.x * float (blockxy);
    vv.z /= scale.z * float (blockxy);
    float dx = vv.x, dy = vv.z;
    csVector3 n;
    if (normal_func)
      n = normal_func->GetNormal (dx, dy);
    else
    {
      v[0].Set(-0.1f, GetClampedHeight (dx-inv_totx, dy-inv_toty), -0.1f);
      v[1].Set( 0.0f, GetClampedHeight (dx, dy-inv_toty), -0.1f);
      v[2].Set( 0.1f, GetClampedHeight (dx+inv_totx, dy-inv_toty), -0.1f);
      v[3].Set( 0.1f, GetClampedHeight (dx+inv_totx, dy), 0);
      v[4].Set( 0.1f, GetClampedHeight (dx+inv_totx, dy+inv_toty),  0.1f);
      v[5].Set( 0.0f, GetClampedHeight (dx, dy+inv_toty),  0.1f);
      v[6].Set(-0.1f, GetClampedHeight (dx-inv_totx, dy+inv_toty),  0.1f);
      v[7].Set(-0.1f, GetClampedHeight (dx-inv_totx, dy), 0);
      n.Set (0, 0, 0);
      int j1, j;
      j1 = 7;
      for (j = 0 ; j < 8 ; j++)
      {
	n += (v[j1] % v[j]).Unit ();
	j1 = j;
      }
      n.Normalize ();
    }
    *normals++ = n;
  }
}

void csTerrFuncObject::ComputeNormals ()
{
  int lod;
  int bx, by;
  for (lod = 0 ; lod < LOD_LEVELS ; lod++)
    for (by = 0 ; by < blockxy ; by++)
      for (bx = 0 ; bx < blockxy ; bx++)
      {
	int blidx = by*blockxy + bx;
	ComputeNormals (blocks[blidx].mesh[lod],
		blocks[blidx].mesh_vertices[lod],
		blocks[blidx].num_mesh_vertices[lod],
		&blocks[blidx].normals[lod]);
      }
}

void csTerrFuncObject::ComputeBBox (const G3DTriangleMesh& /*mesh*/,
	csVector3* mesh_vertices,
	int num_mesh_vertices, csBox3& bbox)
{
  int i;
  bbox.StartBoundingBox ();
  for (i = 0 ; i < num_mesh_vertices ; i++)
    bbox.AddBoundingVertex (mesh_vertices[i]);
}

void csTerrFuncObject::ComputeBBoxes ()
{
  global_bbox.StartBoundingBox ();
  int lod;
  int bx, by;
  csVector3 v;
  float t;
  for (by = 0 ; by < blockxy ; by++)
    for (bx = 0 ; bx < blockxy ; bx++)
    {
      int blidx = by*blockxy + bx;
      blocks[blidx].bbox.StartBoundingBox ();
      for (lod = 0 ; lod < LOD_LEVELS ; lod++)
      {
	csBox3 bb;
	ComputeBBox (blocks[blidx].mesh[lod],
		blocks[blidx].mesh_vertices[lod],
		blocks[blidx].num_mesh_vertices[lod], bb);
	blocks[blidx].bbox += bb;
      }
      global_bbox += blocks[blidx].bbox;
    }
  rad_center = global_bbox.GetCenter(); v = global_bbox.Max();
  t = (v.x - rad_center.x)*(v.x - rad_center.x) +
      (v.y - rad_center.y)*(v.y - rad_center.y) +
	  (v.z - rad_center.z)*(v.z - rad_center.z);
  t = csQsqrt (t);
  radius = csVector3 (t,t,t);
}

void csTerrFuncObject::InitMesh (G3DTriangleMesh& mesh,
	csVector3*& mesh_vertices, csVector2*& mesh_texels,
	csColor*& mesh_colors)
{
  delete[] mesh_colors;
  mesh_colors = 0;
  delete[] mesh_vertices;
  mesh_vertices = 0;
  delete[] mesh_texels;
  mesh_texels = 0;
  delete[] mesh.vertex_fog;
  mesh.vertex_fog = 0;
  mesh.morph_factor = 0;
  mesh.num_vertices_pool = 1;
  mesh.use_vertex_color = true;
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.do_fog = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  mesh.mixmode = CS_FX_COPY;
}

void csTerrFuncObject::SetupBaseMesh (G3DTriangleMesh& mesh,
	csVector3*& mesh_vertices, csVector2*& mesh_texels,
	csColor*& mesh_colors, int& num_mesh_vertices, int bx, int by)
{
  num_mesh_vertices = (gridx+1) * (gridy+1);
  mesh_vertices = new csVector3[num_mesh_vertices];
  mesh_texels = new csVector2[num_mesh_vertices];
  mesh_colors = new csColor[num_mesh_vertices];
  mesh.vertex_fog = new G3DFogInfo [num_mesh_vertices];
  csVector3 tl = topleft;
  tl.x += bx*scale.x;
  tl.y += 0;
  tl.z += by*scale.z;
  csVector2 tluv (0, 0);
#if 0
  tluv.x += float (bx) / float (blockxy);
  tluv.y += float (by) / float (blockxy);
#endif
  int gx, gy;
  float dx, dy;
  for (gy = 0 ; gy <= gridy ; gy++)
  {
    dy = float (by*gridy+gy) / float (blockxy * gridy);
    for (gx = 0 ; gx <= gridx ; gx++)
    {
      dx = float (bx*gridx+gx) / float (blockxy * gridx);
      int vtidx = gy*(gridx+1)+gx;
      csVector3 v = tl;
      v.x += gx*scale.x / float (gridx);
      v.y += height_func->GetHeight (dx, dy) * scale.y;
      v.z += gy*scale.z / float (gridy);
      mesh_vertices[vtidx] = v;
      csVector2 uv = tluv;
#if 0
      uv.x += float (gx) / float (blockxy*gridx);
      uv.y += float (gy) / float (blockxy*gridy);
#else
      uv.x += float (gx) / float (gridx);
      uv.y += float (gy) / float (gridy);
      uv.x = uv.x * correct_du + correct_su;
      uv.y = uv.y * correct_dv + correct_sv;
#endif
      mesh_texels[vtidx] = uv;
      mesh_colors[vtidx].Set (1, 1, 1);
    }
  }
  mesh.num_triangles = 2*gridx*gridy;
  mesh.triangles = new csTriangle [mesh.num_triangles];
  for (gy = 0 ; gy < gridy ; gy++)
    for (gx = 0 ; gx < gridx ; gx++)
    {
      int tridx = (gy*gridx+gx) * 2;
      int vtidx = gy*(gridx+1)+gx;	// Top left vertex of triangles
      mesh.triangles[tridx].c = vtidx;
      mesh.triangles[tridx].b = vtidx+1;
      mesh.triangles[tridx].a = vtidx+gridx+1;
      mesh.triangles[tridx+1].c = vtidx+1;
      mesh.triangles[tridx+1].b = vtidx+gridx+1+1;
      mesh.triangles[tridx+1].a = vtidx+gridx+1;
    }
}

void csTerrFuncObject::SetupVisibilityTree (csTerrainQuad* quad,
	int x1, int y1, int x2, int y2, int depth)
{
  float min_height, max_height;
  min_height = 1000000000.;
  max_height = -1000000000.;
  depth++;

  float res = float ((1 << quad_depth) -1);

  if (depth == quad_depth-block_depth+1)
  {
    // Put the pointer to this node in the corresponding block.
    // bx and by are coordinates of block.
    int bx = x1 >> block_depth;
    int by = y1 >> block_depth;
    int blidx = by*blockxy + bx;
    csTerrBlock& block = blocks[blidx];
    block.node = quad;
  }

  if (quad->IsLeaf ())
  {
    float dx1 = float (x1) / res;
    float dy1 = float (y1) / res;
    float dx2 = float (x2) / res;
    float dy2 = float (y2) / res;
    // Calculate minimum and maximum height by sampling a series
    // of points in the leaf.
    int x, y;
    float dx, dy;
    for (y = 0 ; y < 5 ; y++)
    {
      dy = dy1 + (float (y) / 4.) * (dy2-dy1);
      for (x = 0 ; x < 5 ; x++)
      {
        dx = dx1 + (float (x) / 4.) * (dx2-dx1);
	if (dx < 0) dx = 0;
	if (dx > 1) dx = 1;
	if (dy < 0) dy = 0;
	if (dy > 1) dy = 1;
        float h = height_func->GetHeight (dx, dy) * scale.y + topleft.y;
	if (h < min_height) min_height = h;
	if (h > max_height) max_height = h;
      }
    }
    quad->SetMinimumHeight (min_height);
    quad->SetMaximumHeight (max_height);
    return;
  }

  int i;
  for (i = 0 ; i < 4 ; i++)
  {
    int xx1, yy1, xx2, yy2;
	xx1 = yy1 = xx2 = yy2 = 0;
    switch (i)
    {
      case CS_QUAD_TOPLEFT:
        xx1 = x1; yy1 = y1; xx2 = x1 + (x2-x1)/2; yy2 = y1 + (y2-y1)/2;
	break;
      case CS_QUAD_TOPRIGHT:
        xx1 = x1 + (x2-x1)/2; yy1 = y1; xx2 = x2; yy2 = y1 + (y2-y1)/2;
	break;
      case CS_QUAD_BOTLEFT:
        xx1 = x1; yy1 = y1 + (y2-y1)/2; xx2 = x1 + (x2-x1)/2; yy2 = y2;
	break;
      case CS_QUAD_BOTRIGHT:
        xx1 = x1 + (x2-x1)/2; yy1 = y1 + (y2-y1)/2; xx2 = x2; yy2 = y2;
	break;
    }
    csTerrainQuad* c = quad->GetChild (i);
    SetupVisibilityTree (c, xx1, yy1, xx2, yy2, depth);
    if (c->GetMinimumHeight () < min_height)
      min_height = c->GetMinimumHeight ();
    if (c->GetMaximumHeight () > max_height)
      max_height = c->GetMaximumHeight ();
  }
  quad->SetMinimumHeight (min_height);
  quad->SetMaximumHeight (max_height);
}

void csTerrFuncObject::SetupVisibilityTree ()
{
  delete quadtree;
  quadtree = new csTerrainQuad ();
  quadtree->Build (quad_depth);
  int res = 1 << quad_depth;
  block_depth = 0;
  int b = blockxy;
  while (b > 1)
  {
    block_depth++;
    b >>= 1;
  }
  block_depth = quad_depth-block_depth;
  SetupVisibilityTree (quadtree, 0, 0, res, res, 0);
}

void csTerrFuncObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    if (!blocks || block_dim_invalid)
    {
      delete[] blocks;
      blocks = new csTerrBlock [blockxy*blockxy];
    }
    grid_stepx = scale.x / gridx;
    grid_stepy = scale.z / gridy;
    inv_block_stepx = 1 / scale.x;
    inv_block_stepy = 1 / scale.z;
    inv_grid_stepx = 1 / grid_stepx;
    inv_grid_stepy = 1 / grid_stepy;

    int bx, by;
    int blidx = 0;
    for (by = 0 ; by < blockxy ; by++)
    {
      float dy = (float (by)+.5) / float (blockxy);
      for (bx = 0 ; bx < blockxy ; bx++, blidx++)
      {
        float dx = (float (bx)+.5) / float (blockxy);
	csVector3 tl = topleft;
	tl.x += (float (bx) + .5)*scale.x;
	tl.y += height_func->GetHeight (dx, dy)*scale.y;
	tl.z += (float (by) + .5)*scale.z;
	blocks[blidx].center = tl;
      }
    }

    int lod;
    for (lod = 0 ; lod < 4 ; lod++)
    {
      printf ("Setting up LOD level %d\n", lod);
      int del_tri = 0;
      int tot_tri = 0;
      for (by = 0 ; by < blockxy ; by++)
        for (bx = 0 ; bx < blockxy ; bx++)
	{
	  int blidx = by*blockxy+bx;
	  csTerrBlock& block = blocks[blidx];
	  block.dirlight_numbers[lod] = -1;
	  SetupVertexBuffer (block.vbuf[lod], &block.mesh[lod].buffers[0]);
	  InitMesh (block.mesh[lod], block.mesh_vertices[lod],
	  	block.mesh_texels[lod], block.mesh_colors[lod]);
	  if (lod == 0)
	    SetupBaseMesh (block.mesh[lod],
	    	block.mesh_vertices[lod],
	    	block.mesh_texels[lod],
	    	block.mesh_colors[lod],
		block.num_mesh_vertices[lod], bx, by);
	  else
	  {
	    int dt, tt;
	    ComputeLODLevel (
	      block.mesh[lod-1],
	      block.mesh_vertices[lod-1],
	      block.mesh_texels[lod-1],
	      block.mesh_colors[lod-1],
	      block.num_mesh_vertices[lod-1],
	      block.mesh[lod],
	      block.mesh_vertices[lod],
	      block.mesh_texels[lod],
	      block.mesh_colors[lod],
	      block.num_mesh_vertices[lod],
	      max_cost[lod-1], dt, tt);
	    del_tri += dt;
	    tot_tri += tt;
	  }
        }
      printf ("Deleted %d triangles from %d.\n", del_tri, tot_tri);
      fflush (stdout);
    }
    ComputeNormals ();
    ComputeBBoxes ();
    if(quaddiv_enabled) InitQuadDiv();
    SetupVisibilityTree ();
  }
}

void csTerrFuncObject::SetupVertexBuffer (csRef<iVertexBuffer> &vbuf1,
					  iVertexBuffer** vbuf2)
{
 if (!vbuf1)
 {
   if (!vbufmgr)
   {
     iObjectRegistry* object_reg = ((csTerrFuncObjectFactory*)pFactory)
       ->object_reg;
     csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
     // @@@ priority should be a parameter.
     vbufmgr = g3d->GetVertexBufferManager ();
     vbufmgr->AddClient (&scfiVertexBufferManagerClient);
   }
   vbuf1 = vbufmgr->CreateBuffer (1);
   if (vbuf2) *vbuf2 = vbuf1;
 }
}


void csTerrFuncObject::RecomputeShadowMap ()
{

}

void csTerrFuncObject::RecomputeLighting (int lod, int bx, int by)
{
  if (!do_dirlight) return;
  int blidx = by*blockxy + bx;
  csTerrBlock& block = blocks[blidx];
  if (dirlight_number != block.dirlight_numbers[lod])
  {
    block.dirlight_numbers[lod] = dirlight_number;
    csColor* vtcols = block.mesh_colors[lod];
    csVector3* norms = block.normals[lod];
	int i;
    for (i = 0 ; i < block.num_mesh_vertices[lod] ; i++, vtcols++)
    {
      float l = dirlight * *(norms++);
      if (l <= 0)
        vtcols->Set (
		base_color.red,
		base_color.green,
		base_color.blue);
      else
        vtcols->Set (
		base_color.red+dirlight_color.red * l,
		base_color.green+dirlight_color.green * l,
		base_color.blue+dirlight_color.blue * l);
    }
  }
}

bool csTerrFuncObject::BBoxVisible (iRenderView* rview, const csBox3& bbox,
	int& clip_portal, int& clip_plane, int& clip_z_plane,
	csPlane3* planes, uint32 frustum_mask)
{
  return rview->ClipBBox (planes, frustum_mask, bbox,
  	clip_portal, clip_plane, clip_z_plane);
}

void csTerrFuncObject::TestVisibility (iRenderView* rview)
{
  csTerrainQuad::MarkAllInvisible ();
  quadtree->InitHorizon (horizon, CS_HORIZON_SIZE);

  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  const csVector3& origin = camtrans.GetOrigin ();
  quadtree->ComputeVisibility (origin, global_bbox, horizon, CS_HORIZON_SIZE);
}

void csTerrFuncObject::InitQuadDiv()
{
  if(!quad_height)
  {
    QuadDivHeightFunc *qdhf = new QuadDivHeightFunc();
    qdhf->hf = height_func;
    qdhf->scx = 1.0 / (scale.x * float (blockxy));
    qdhf->scy = 1.0 / (scale.z * float (blockxy));
    qdhf->offx = -topleft.x * qdhf->scx;
    qdhf->offy = -topleft.z * qdhf->scy;
    qdhf->offh = topleft.y;
    qdhf->sch = scale.y;
    quad_height = qdhf;
  }
  if(!quad_normal)
  {
    QuadDivNormalFunc *qdnf = new QuadDivNormalFunc();
    if(normal_func) 
    {
      qdnf->nf = normal_func;
      qdnf->scx = 1.0 / (scale.x * float (blockxy));
      qdnf->scy = 1.0 / (scale.z * float (blockxy));
      qdnf->offx = -topleft.x * qdnf->scx;
      qdnf->offy = -topleft.z * qdnf->scy;
      qdnf->inv_totx = 0.0;
      qdnf->inv_toty = 0.0;
    }
    else 
    {
      qdnf->hf = quad_height;
      qdnf->scx = 1.;
      qdnf->scy = 1.;
      qdnf->offx = 0.;
      qdnf->offy = 0.;
      qdnf->inv_totx = .5 / float (1+blockxy*gridx) * 
        (scale.x * float (blockxy));
      qdnf->inv_toty = .5 / float (1+blockxy*gridy) * 
        (scale.z * float (blockxy));
    }
    quad_normal = qdnf;
  }

  int initdepth = 4;
  int bx, by;
  int blidx;

  for (by = 0 ; by < blockxy ; by++)
  {
    for (bx = 0 ; bx < blockxy ; bx++)
    {
      blidx = by*blockxy + bx;
      blocks[blidx].quaddiv = new csTerrainQuadDiv(initdepth);
      blocks[blidx].PrepareQuadDiv(quad_height, this);
      blocks[blidx].quaddiv->SetVisQuad(blocks[blidx].node);
      //printf("block bx=%d by=%d block %x qd %x center %g,%g,%g\n", bx, by,
       //(int)&blocks[blidx],(int)blocks[blidx].quaddiv,blocks[blidx].center.x,
       //blocks[blidx].center.y, blocks[blidx].center.z);
    }
  }

  for (by = 0 ; by < blockxy ; by++)
  {
    for (bx = 0 ; bx < blockxy ; bx++)
    {
      blidx = by*blockxy + bx;
      if(bx>0) blocks[blidx].quaddiv->SetNeighbor(
        CS_QUAD_LEFT, blocks[by*blockxy+bx-1].quaddiv);
      if(by>0) blocks[blidx].quaddiv->SetNeighbor(
        CS_QUAD_TOP, blocks[(by-1)*blockxy+bx].quaddiv);
      if(bx<blockxy-1) blocks[blidx].quaddiv->SetNeighbor(
        CS_QUAD_RIGHT, blocks[by*blockxy+bx+1].quaddiv);
      if(by<blockxy-1) blocks[blidx].quaddiv->SetNeighbor(
        CS_QUAD_BOT, blocks[(by+1)*blockxy+bx].quaddiv);
    }
  }
}


bool csTerrFuncObject::DrawTest (iRenderView*, iMovable*, uint32)
{
  // @@@ Can we do something more sensible here?
  return true;
}

void csTerrFuncObject::QuadDivDraw (iRenderView* rview, csZBufMode zbufMode,
	csPlane3* planes, uint32 frustum_mask)
{
  qd_framenum++;
  iGraphics3D* pG3D = rview->GetGraphics3D ();
  iCamera* pCamera = rview->GetCamera ();

  csReversibleTransform& camtrans = pCamera->GetTransform ();
  const csVector3& origin = camtrans.GetOrigin ();
  pG3D->SetObjectToCamera (&camtrans);
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufMode );

  int bx, by;
  int blidx = 0;
  /// 1st pass - compute subdivision
  for (by = 0 ; by < blockxy ; by++)
  {
    for (bx = 0 ; bx < blockxy ; bx++, blidx++)
    {
      csTerrBlock& block = blocks[blidx];
      if (do_vis_test)
      {
        CS_ASSERT (block.node != 0);
        if (!block.node->IsVisible ()) continue;
      }
      int clip_portal, clip_plane, clip_z_plane;
      if (BBoxVisible (rview, block.bbox, clip_portal, clip_plane,
      	clip_z_plane, planes, frustum_mask))
      {
        block.quaddiv_visible = true;
        block.qd_portal = clip_portal;
        block.qd_plane = clip_plane;
        block.qd_z_plane = clip_z_plane;
        block.PrepareFrame (origin, qd_framenum, this);
      }
    }
  }

  /// 2nd pass - draw without seams
  blidx = 0;
  for (by = 0 ; by < blockxy ; by++)
  {
    for (bx = 0 ; bx < blockxy ; bx++, blidx++)
    {
      csTerrBlock& block = blocks[blidx];
      if(block.quaddiv_visible)
      {
        //if((bx!=0&&bx!=1) || by!=0) continue; // one block only
        SetupVertexBuffer (block.vbuf[0], 0);
        block.Draw(rview, block.qd_portal, block.qd_plane, block.qd_z_plane,
          correct_du, correct_su, correct_dv, correct_sv, this, qd_framenum);
	block.quaddiv_visible = false;
      }
    }
  }
}

bool csTerrFuncObject::Draw (iRenderView* rview, iMovable* /*movable*/,
  	csZBufMode zbufMode)
{
  SetupObject ();

  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  if (do_vis_test)
    TestVisibility (rview);

  // @@@ BUG! Ignores movable!!!
  iCamera* pCamera = rview->GetCamera ();
  csReversibleTransform& camtrans = pCamera->GetTransform ();

  uint32 frustum_mask;
  csPlane3 planes[10];
  rview->SetupClipPlanes (camtrans, planes, frustum_mask);

  if (quaddiv_enabled)
  {
    QuadDivDraw (rview, zbufMode, planes, frustum_mask);
    return true;
  }

  iGraphics3D* pG3D = rview->GetGraphics3D ();
  const csVector3& origin = camtrans.GetOrigin ();
  pG3D->SetObjectToCamera (&camtrans);
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufMode );

  int bx, by;
  int blidx = 0;
  for (by = 0 ; by < blockxy ; by++)
  {
    for (bx = 0 ; bx < blockxy ; bx++, blidx++)
    {
      csTerrBlock& block = blocks[blidx];
      if (do_vis_test)
      {
        CS_ASSERT (block.node != 0);
        if (!block.node->IsVisible ()) continue;
      }
      int clip_portal, clip_plane, clip_z_plane;
      if (BBoxVisible (rview, block.bbox, clip_portal, clip_plane,
      	clip_z_plane, planes, frustum_mask))
      {
        csVector3& bc = block.center;
        int lod = 0;
        float sqdist = csSquaredDist::PointPoint (bc, origin);
        if (sqdist > lod_sqdist[0]) lod++;
        if (sqdist > lod_sqdist[1]) lod++;
        if (sqdist > lod_sqdist[2]) lod++;
        RecomputeLighting (lod, bx, by);
        G3DTriangleMesh* m = &block.mesh[lod];
        m->mat_handle = block.material->GetMaterialHandle ();
        if (!m->mat_handle)
          m->mat_handle = block.material->GetMaterialHandle ();
        m->do_mirror = pCamera->IsMirrored ();
	m->clip_portal = clip_portal;
	m->clip_plane = clip_plane;
	m->clip_z_plane = clip_z_plane;
	SetupVertexBuffer (block.vbuf[lod], 0);
	CS_ASSERT (block.vbuf[lod]);
	CS_ASSERT (!block.vbuf[lod]->IsLocked ());
	CS_ASSERT (block.mesh_vertices[lod] != 0);
	vbufmgr->LockBuffer (block.vbuf[lod],
		block.mesh_vertices[lod],
		block.mesh_texels[lod],
		block.mesh_colors[lod],
		block.num_mesh_vertices[lod],
		0, global_bbox);
	rview->CalculateFogMesh (camtrans, *m);
        pG3D->DrawTriangleMesh (*m);
	vbufmgr->UnlockBuffer (block.vbuf[lod]);
      }
    }
  }
  return true;
}

void csTerrFuncObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
  SetupObject ();
  bbox = global_bbox;
}

void csTerrFuncObject::GetRadius (csVector3& rad, csVector3& cent)
{
  rad = radius; cent = global_bbox.GetCenter();
}

int csTerrFuncObject::CollisionDetect (csTransform* transform)
{
  // Translate us into terrain coordinate space.
  csVector3 p = transform->GetOrigin () - topleft;
  p.x /= scale.x * float (blockxy);
  p.z /= scale.z * float (blockxy);
  // If our location is outside the terrain then we cannot hit it.
  if (p.x < 0 || p.z < 0 || p.x > 1 || p.z > 1) return 0;

  // Return height of terrain at this location in Y coord.
  float h = height_func->GetHeight (p.x, p.z)*scale.y+2;
  if (h < p.y) return 0;
  p.y = h;
  // Translate us back.
  p.x *= scale.x * float (blockxy);
  p.z *= scale.z * float (blockxy);
  p = p + topleft;
  transform->SetOrigin (p);
  return 1;
}

// Notes: The terrain func mesh area consists of a number of blocks which
// are encompassed by an overall bounding box. Each block is then comprised
// of a number smaller blocks, which each contain two triangles.

bool csTerrFuncObject::HitBeamOutline (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr)
{
// Box walk. Not really fast as the name suggests. It works by stepping its way forward
// through the terrain field. This is to be complemented yet with another routine which
// will pick through the individual cells in each block.
// This variant will return on the first hit, making it a bit faster than
// the other variant.

  csSegment3 seg (start, end);
  csIntersect3::BoxSegment (global_bbox, seg, isect, 0);
  csVector3 v, *vrt;
  csTriangle *tr;
  csBox3 tbox;
  csSegment3 rev ( end, isect );
  float max_y = global_bbox.MaxY();
  float min_y = global_bbox.MinY();
  int max_index = blockxy*blockxy;
  int x, y, index, i, max = 0;

  Object2Block( isect, x, y ); // Seed the routine
  if ( x == blockxy ) x--; // Kludge to bring x and y back into bounds if hit
  if ( y == blockxy ) y--; // from the positive x or z side. not perty but quick.

  for (Block2Index(x, y, index);
      (index > -1) && (index < max_index);
      Block2Index(x, y, index))
  {
    rev.SetEnd(isect);
    tbox = blocks[index].bbox;
    if (csIntersect3::BoxSegment (tbox, seg, isect, 0) > -1)
    {
      max = blocks[index].mesh[0].num_triangles;
      vrt = blocks[index].mesh_vertices[0];
      tr = blocks[index].mesh[0].triangles;
      for (i = 0 ; i < max ; i++)
      { // Check each triangle in both orientations.
        if (csIntersect3::IntersectTriangle (vrt[tr[i].a], vrt[tr[i].b],
    	     vrt[tr[i].c], seg, isect))
        {

            if (pr) *pr = csQsqrt(csSquaredDist::PointPoint (start, isect) /
	                           csSquaredDist::PointPoint (start, end));
	    return true;
	}
      }
    }
    v = tbox.Max();
    tbox.AddBoundingVertex(v.x, max_y, v.z);
    tbox.AddBoundingVertex(v.x, min_y, v.z);

    switch (csIntersect3::BoxSegment (tbox, rev, isect, 0))
    {
      case CS_BOX_SIDE_x: x--; break;
      case CS_BOX_SIDE_X: x++; break;
      case CS_BOX_SIDE_z: y--; break;
      case CS_BOX_SIDE_Z: y++; break;
      default: return false;
    }
  }
  return false;
}

bool csTerrFuncObject::HitBeamObject (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr, int* polygon_idx)
{
  if (polygon_idx) *polygon_idx = -1;
  csSegment3 seg (start, end);
  csVector3 st = start;
// Box walk. Not really fast as the name suggests. It works by stepping its way forward
// through the terrain field. This is to be complemented yet with another routine which
// will pick through the individual cells in each block.

  csVector3 v, *vrt;
  csTriangle *tr;
  csBox3 tbox;
  csSegment3 rev ( end, st );
  float max_y = global_bbox.MaxY();
  float min_y = global_bbox.MinY();
  float dist, dist2, max_dist = csSquaredDist::PointPoint (start, end);
  int max_index = blockxy*blockxy;
  int x, y, index, i, max = 0;
  bool brk = false;

  dist = dist2 = max_dist;
  Object2Block( st, x, y ); // Seed the routine
  if ( x == blockxy ) x--; // Kludge to bring x and y back into bounds if hit
  if ( y == blockxy ) y--; // from the positive x or z side. not perty but quick.

  for (Block2Index(x, y, index);
      (index > -1) && (index < max_index);
      Block2Index(x, y, index))
  {
    rev.SetEnd(st);
    tbox = blocks[index].bbox;
    if (csIntersect3::BoxSegment (tbox, seg, st, 0) > -1)
    {
      max = blocks[index].mesh[0].num_triangles;
      vrt = blocks[index].mesh_vertices[0];
      tr = blocks[index].mesh[0].triangles;
      for (i = 0 ; i < max ; i++)
      { // Check each triangle in both orientations.
        if (csIntersect3::IntersectTriangle (vrt[tr[i].a], vrt[tr[i].b],
    	     vrt[tr[i].c], seg, st))
	{
          dist2 = csSquaredDist::PointPoint (start, st);
	  if ( dist2 < dist )
	  {
	    isect = st;
	    dist = dist2;
            if (pr) *pr = csQsqrt( dist / max_dist );
	  }
	}
      }
    }
    v = tbox.Max();
    tbox.AddBoundingVertex(v.x, max_y, v.z);
    tbox.AddBoundingVertex(v.x, min_y, v.z);

    switch (csIntersect3::BoxSegment (tbox, rev, st, 0))
    {
      case CS_BOX_SIDE_x: x--; break;
      case CS_BOX_SIDE_X: x++; break;
      case CS_BOX_SIDE_z: y--; break;
      case CS_BOX_SIDE_Z: y++; break;
      default: brk = true;
    }
    if (brk) break;
  }
  if (dist == max_dist)
      return false;
  return true;
}

void csTerrFuncObject::eiVertexBufferManagerClient::ManagerClosing ()
{
  if (scfParent->vbufmgr)
  {
    int n = scfParent->blockxy * scfParent->blockxy;
    for (int i = 0 ; i < n; i++)
    {
      csTerrBlock& block = scfParent->blocks[i];
      for (int lod = 0; lod < 4; lod++)
      {
	block.vbuf[lod] = 0;
      }
    }
    scfParent->vbufmgr = 0;
  }
}



//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTerrFuncObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csTerrFuncObjectFactory::csTerrFuncObjectFactory (iObjectRegistry* object_reg,
                                                  iMeshObjectType* terrfunc_type)
{
  SCF_CONSTRUCT_IBASE (0);
  csTerrFuncObjectFactory::object_reg = object_reg;
  logparent = 0;
}

csTerrFuncObjectFactory::~csTerrFuncObjectFactory ()
{
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObject> csTerrFuncObjectFactory::NewInstance ()
{
  csTerrFuncObject* pTerrObj = new csTerrFuncObject (object_reg, this);
  return csPtr<iMeshObject> (pTerrObj);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTerrFuncObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrFuncObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTerrFuncObjectType)


csTerrFuncObjectType::csTerrFuncObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csTerrFuncObjectType::~csTerrFuncObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csTerrFuncObjectType::NewFactory()
{
  csTerrFuncObjectFactory *pFactory = new csTerrFuncObjectFactory (object_reg, this);
  return csPtr<iMeshObjectFactory> (pFactory);
}

