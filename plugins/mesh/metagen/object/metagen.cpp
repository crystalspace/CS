/*
    Metagen Renderer
    Copyright (C) 2001 by Michael H. Voase
    Copyright (C) 1999 by Denis Dmitriev

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

#include <stdarg.h>
#include <string.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/cscolor.h"
#include "iengine/rview.h"
#include "iengine/movable.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "igeom/clip2d.h"
#include "iutil/cfgmgr.h"
#include "qsqrt.h"
#include "metagen.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csMetaGen)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMetaGen)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaGen::MetaGen)
  SCF_IMPLEMENTS_INTERFACE (iMetaGen)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMetaGen::csMetaGen (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiMetaGen);

  XStart = YStart = ZStart = 0.0;
  XFin = YFin = ZFin = 0.0;
  stepx = stepy = stepz = 0.0;
  istepx = istepy = istepz = 0.0;
  asin_table_res = 256;
  asin_table = NULL;
  initialized = false;
  do_lighting = false;
  splinter_size = 0.0005;

  verts = NULL;
  trigs = NULL;
  tex = NULL;
  current_triangles = 0;
  current_vertices = 0;
  current_texels = 0;
  vertices_tesselated = 0;
  env_mapping = iMetaGen::TRUE_ENV_MAP;
  env_map_mult = 1.0;
  frame = 0;
  current_lod = 1;
  current_features = ALL_FEATURES;
}

csMetaGen::~csMetaGen ()
{
  DeleteArcSineTable();
  DeleteBones();
  SetMaxVertices(0);
  DeleteBuffers();
  initialized = false;
}

void csMetaGen::DeleteBones()
{
  int i, j, t, t2 = bones.Length();
  for ( i = 0; i < t2; i++ )
  {
	t = bones[i]->num_slices;
	for ( j = 0; j < t; j++ )
	{
	  delete [] bones[i]->slices[j]->charges;
	}
	delete [] bones[i]->slices;
  }
  bones.DeleteAll();
}

float csMetaGen::map (float x)
{
  return asin_table[(int)(asin_table_res*(1+x))];
}

bool csMetaGen::InitArcSineTable(void)
{
  if ( asin_table_res <= 0) return false;

  asin_table = (float *)malloc(sizeof(float) * asin_table_res * 2 + 1 );
  if (!asin_table) {
    printf("ERROR: MetaGen failed to allocate arc=sine table; "
      "out of memeory\n");
    return false;
  }
  if (asin_table_res)
	FillArcSineTable();
  return true;
}

void csMetaGen::FillArcSineTable()
{
  int i, j;
  for (i = -asin_table_res, j = 0; i <= asin_table_res; i++, j++)
  {
    float c = 1.0 * i / asin_table_res;
    switch(env_mapping)
    {
      case iMetaGen::TRUE_ENV_MAP:
        asin_table[j] = env_map_mult * (0.5f + asin(c) / PI);
        break;
      case iMetaGen::FAKE_ENV_MAP:
        asin_table[j] = 0.5f * env_map_mult * (1 + c);
        break;
    }
  }
}

void csMetaGen::DeleteArcSineTable()
{
  delete [] asin_table;
  asin_table = NULL;
}

bool csMetaGen::InitializeCache()
{
  stepx = (XFin - XStart)/GetResX();
  stepy = (YFin - YStart)/GetResY();
  stepz = (ZFin - ZStart)/GetResZ();
  istepx = 1/stepx;
  istepy = 1/stepy;
  istepz = 1/stepz;
  return true;
}

bool csMetaGen::Initialize ()
{
  if (!initialized)
  {
	initialized = true;
	if (!InitArcSineTable()) { initialized = false; return false; }
	if (!InitializeCache()) { initialized = false; return false; }
  }
  return true;
}

void csMetaGen::SetQualityEnvironmentMapping (bool toggle)
{
  env_mapping = toggle ? iMetaGen::TRUE_ENV_MAP : iMetaGen::FAKE_ENV_MAP;
  if (asin_table)
	FillArcSineTable ();
}

void csMetaGen::SetEnvironmentMappingFactor (float env_mult)
{
  env_map_mult = env_mult;
  if (asin_table)
	FillArcSineTable ();
}

#if 0
static void LitVertex( const csVector3 &n, csColor &c )
{
  if (n.z > 0)
    c.red = c.green = c.blue = 0;
  else
  {
    float l = n.z*n.z;
    c.red = c.green = c.blue = l;
  }
}
#endif

void csMetaGen::CreateLighting( iLight **, int, iMovable * )
{
  //TODO: The lighting code is incomplete...
#if 0
      csVector3 n(0, 0, 0);
	  int k;
      for(k = 0; k < num_meta_balls; k++)
      {
        csVector3 rv(mesh.vertices[0][m].x - meta_balls[k].center.x,
          mesh.vertices[0][m].y - meta_balls[k].center.y,
          mesh.vertices[0][m].z - meta_balls[k].center.z);

        float r = rv.Norm();
        float c = mp.charge / (r*r*r);

        n += rv * c;
      }
      n = n.Unit();
	  LitVertex ( n, mesh.vertex_colors[0][m]);
#endif
}

void csMetaGen::GetObjectBoundingBox(csBox3& bbox, int )
{
  bbox = object_bbox;
}

// Since the mesh comes here after
// post processing with a sorted vertex list,  I can use
// the first and last vertex for the X extremities. For
// the Y values, I use the scope of the rendering field.
// The Z vlue is a little harder- I still scan the list...

void csMetaGen::CreateBoundingBoxLinear( int bone )
{
  csVector3 start, finish;
  float zmin,zmax,y;
  zmin = zmax = int(GetResZ()/2) * stepz + ZStart;
  csVector3* vrt = verts->v;

  start.x = vrt[0].x; finish.x = vrt[current_vertices - 1].x;
  y = _2coordY(bones[bone]->start_slice); start.y = y;
  y = _2coordY(bones[bone]->start_slice + bones[bone]->num_slices);
  finish.y = y;

  int i;
  for (i = 0; i < current_vertices; i++)
  {
	if ( vrt[i].z < zmin ) zmin = vrt[i].z;
	  else
	if ( vrt[i].z > zmax ) zmax = vrt[i].z;
  }
  start.z = zmin; finish.z = zmax;
  object_bbox.StartBoundingBox(start);
  object_bbox.AddBoundingVertexSmart(finish);

}

void csMetaGen::CreateBoundingBoxBlob( int /* bone */)
{
  csVector3 start, finish; csVector3 *vrt = verts->v;
  start.x = vrt[0].x; finish.x = vrt[current_vertices - 1].x;

  object_bbox.StartBoundingBox(vrt[0]);
  object_bbox.AddBoundingVertexSmart(vrt[current_vertices - 1]);

}

// Data addition calls. ------------------------------------------------
// Start with bones and slices

void csMetaGen::CreateBone( int start, float iso_lev)
{
  MetaBone* mb = (MetaBone *)malloc(sizeof(MetaBone));
  memset( mb, 0, sizeof(MetaBone));
  mb->start_slice = start;
  mb->iso_level = iso_lev;
  bones.Push(mb);
}

void csMetaGen::AddSlice( bool endcap )
{
  MetaBone* mb = bones[bones.Length() - 1];
  int n = mb->num_slices;

  mb->slices = (!mb->slices) ? (MetaSlice **)malloc(sizeof(MetaSlice *)) :
	(MetaSlice **)realloc( mb->slices, sizeof(MetaSlice *) * (n + 1));
  mb->slices[n] = (MetaSlice *)malloc(sizeof(MetaSlice));
  mb->slices[n]->is_endcap = endcap;
  mb->slices[n]->num_charges = 0;
  mb->slices[n]->charges = NULL;

  mb->num_slices++;
}

void csMetaGen::AddCharge( csVector2 pos, float charge )
{
  MetaBone* mb = bones[bones.Length() - 1];
  MetaSlice* slc = mb->slices[mb->num_slices - 1];
  int n = slc->num_charges;

  slc->charges = (!slc->charges) ? (SliceCharge *)malloc(sizeof(SliceCharge)) :
	(SliceCharge *)realloc( slc->charges, sizeof(SliceCharge) * ( n + 1 ));

  slc->charges[n].charge = charge;
  slc->charges[n].pos = pos;

  slc->num_charges++;
}
//------------- Now do the fields and balls

void csMetaGen::CreateField( float iso_level )
{
  MetaField *fld = (MetaField *)malloc( sizeof(MetaField));
  fld->iso_level = iso_level;
  fld->points = NULL;
  fld->num_points = 0;
  fields.Push(fld);
}

void csMetaGen::AddPoint( csVector3 pos, float charge )
{
  MetaField* mf = fields[fields.Length() - 1];
  int n = mf->num_points;
  mf->points = (!mf->points)? (PointCharge *)malloc(sizeof(PointCharge)):
	(PointCharge *)realloc(mf->points, sizeof(PointCharge) * (n + 1));
  mf->points[n].pos = pos;
  mf->points[n].charge = charge;
  mf->num_points++;
}
//---------------------------------------------------------------------
// Sort stuff.

// Ripped from csVector and modified to handle changing
// mappings as well. Added a few bits as well.

static float CompareVertex( csVector3 vert1, csVector3 vert2 )
{
  float val;
  if (ABS(val = (vert1.x - vert2.x)) > SMALL_EPSILON )
	return val;
  if (ABS(val = (vert1.y - vert2.y)) > SMALL_EPSILON )
	return val;
  if (ABS(val = (vert1.z - vert2.z)) > SMALL_EPSILON )
	return val;
  return 0.0;
}

static void MapExchange( csVector3 &v1, csVector3 &v2, int &map1, int &map2)
{
  csVector3 t; int i;
  t = v2; v2 = v1; v1 = t;
  i = map2; map2 = map1; map1 = i;
}
static void QuickSort (csVector3 *verts, int *map, int Left, int Right)
{
recurse:
  int i = Left, j = Right;
  int x = (Left + Right) / 2;
  do
  {
    while ((i != x) && (CompareVertex (verts[i], verts[x]) < 0))
      i++;
    while ((j != x) && (CompareVertex (verts[j], verts[x]) > 0))
      j--;
    if (i < j)
    {
      MapExchange (verts[i], verts[j], map[i], map[j]);
      if (x == i)
        x = j;
      else if (x == j)
        x = i;
    }
    if (i <= j)
    {
      i++;
      if (j > Left)
        j--;
    }
  } while (i <= j);

  if (j - Left < Right - i)
  {
    if (Left < j)
      QuickSort (verts, map, Left, j);
    if (i < Right)
    {
      Left = i;
      goto recurse;
    }
  }
  else
  {
    if (i < Right)
      QuickSort (verts, map, i, Right);
    if (Left < j)
    {
      Right = j;
      goto recurse;
    }
  }
}

static int SqueezeList( csVector3 *v, int *map, int max )
{
  int i = 0, j = 1; map[0] = 0;

  while (1) {
	csVector3 t = v[i];
	while ((j < max) && ((t - v[j]) < SMALL_EPSILON))
	{
	   map[j] = i;
	   j++;
	}
//  printf("cur %d next %d map[i] (%d) map[j] (%d)\n",i, j, map[i], map[j]);
	if ( j == max ) break;
	  else
	  {
		i++;
		if ( i == j ) continue;
		v[i] = v[j];
	  }
  }
  return i+1;
}

static void RemoveSplinters(
  csVector3* vrt, csTriangle* trs, int num, float disc )
{
  int i;
  csVector3 v;
  for ( i=0; i < num; i++ )
  {
	if ((vrt[trs[i].a] - vrt[trs[i].b]) < disc)
	{
//	  printf("Compressing splinter %d and %d\n",j,j+1);
	  v = (vrt[trs[i].a] + vrt[trs[i].b])/2;
	  vrt[trs[i].a] = vrt[trs[i].b] = v;
	}
	if ((vrt[trs[i].b] - vrt[trs[i].c]) < disc)
	{
//	  printf("Compressing splinter %d and %d\n",j+1,j+2);
	  v = (vrt[trs[i].b] + vrt[trs[i].c])/2;
	  vrt[trs[i].b] = vrt[trs[i].c] = v;
	}
	if ((vrt[trs[i].a] - vrt[trs[i].c]) < disc)
	{
//	  printf("Compressing splinter %d and %d\n",j,j+2);
	  v = (vrt[trs[i].a] + vrt[trs[i].c])/2;
	  vrt[trs[i].a] = vrt[trs[i].c] = v;
	}
  }
}

void csMetaGen::CleanupSurface()
{

  int i,j;
  csVector3 *vrt = verts->v;
//----------------------------------------------------------------
// OK, we have a bank of verts. Next we create a translate table
// so we can sort and eliminate duplicate vertices. We will then
// eliminate two sided triangles before allocating the triangle
// array. This save some space at the expense of speed.

  int *map = (int *)malloc( sizeof(int) * current_vertices);
  int *map2 =(int *)malloc( sizeof(int) * current_vertices);

  for (i = 0; i < current_vertices; i++ )
	map[i] = i;

  QuickSort( vrt, map, 0, current_vertices - 1);

  for ( i = 0; i < current_vertices; i++ )
	map2[map[i]] = i;

  current_vertices = SqueezeList(vrt, map, current_vertices);
  int num = 0, trs = int( vertices_tesselated / 3);
  csTriangle tr;
  trigs = (TriangleArray *)malloc(sizeof( TriangleArray ));
  trigs->t = (csTriangle *)malloc(sizeof( csTriangle ) * trs );
  trigs->num_triangles = trs; current_triangles = 0;
  for (i = 0; i < trs; i++, num += 3)
  {
	// Fill our triangle buffer with the created vertices
	tr.a = map[map2[num+2]];
	tr.b = map[map2[num+1]];
	tr.c = map[map2[num]];
	// filter out triangles with two or less sides...
	if ((tr.a != tr.b) && (tr.b != tr.c) && (tr.a != tr.c))
	{
	  trigs->t[current_triangles] = tr;
	  current_triangles++;
	}
  }

// This doesnt do a true distance check. I dont really care either
// at this stage...splinters removes a box shaped region insted of
// a true sphere.
  RemoveSplinters( vrt, trigs->t, current_triangles, splinter_size );

  map2 = (int *) realloc(map2, sizeof(int) * current_vertices);
  int *map3 = (int *) malloc(sizeof(int) * current_vertices);

  for ( i = 0; i < current_vertices; i++ ) map2[i] = i;

  QuickSort( vrt, map2, 0, current_vertices - 1);

  for ( i = 0; i<current_vertices; i++ ) map3[map2[i]] = i;

  current_vertices = SqueezeList(vrt, map2, current_vertices);

  j = 0; csTriangle *tr2 = trigs->t;
  for ( i = 0; i < current_triangles; i++)
  {
	map[j++] = tr2[i].a;
	map[j++] = tr2[i].b;
	map[j++] = tr2[i].c;
  }
  trs = current_triangles; current_triangles = 0; j = 0;
  for (i = 0; i < trs; i++)
  {
	// Triple vertex reorder...
	tr.a = map2[map3[map[j++]]];
	tr.b = map2[map3[map[j++]]];
	tr.c = map2[map3[map[j++]]];
	// filter out triangles with two or less sides...
	if ((tr.a != tr.b) && (tr.b != tr.c) && (tr.a != tr.c))
	{
	  trigs->t[current_triangles] = tr;
	  current_triangles++;
	}
  }
  free(map); free(map2); free(map3);
}

//---------------------------------------------------------------------

int csMetaGen::GenerateLinearSurface( int bone_index )
{

  if (!verts) return 0;
  current_vertices = vertices_tesselated =
	CalcLinSurf(bones[bone_index]);
  printf(";Calc lin surface completed %d\n",vertices_tesselated);
  if (!vertices_tesselated) return 0;

// Note: Cleanup Surface creates the triangle array. Dont
// reallocate it or overwrite it. When the gen is complete,
// cleanup the buffer with DeleteBuffers. This prepares the
// generator for the next run. The vertex array is kept seperate
// as it is an unknown when used. You can set the vertex buffer
// to zero which effectively removes it. However, remember to
// put it back before attempting another generate.

  CleanupSurface();

  CreateBoundingBoxLinear(bone_index);

  int i;
  csVector3 *vrt = verts->v;

// Allocate texel array
  tex = (TexelArray *)malloc(sizeof(TexelArray));
  tex->v = (csVector2 *)malloc(sizeof(csVector2) * current_vertices );
  tex->num_texels = current_texels = current_vertices;

  csVector3 tx_start = object_bbox.GetCenter();
  float cent_x = tx_start.x, cent_z = tx_start.z;
  float min_y = object_bbox.MinY(), lx, lz, t;
  float iylen = object_bbox.MaxY() - min_y;
  if ( iylen < SMALL_EPSILON ) iylen = 100000; else iylen = 1/iylen;
  for (i = 0; i < current_vertices; i++)
  {
	// Texturing. Simple texturing, v is calculated as the
	// proportion of how far the vertex is down the length
	// and the u is calculated on the rotational displacement
	// around the x,y center of the bounding box.
	// The texture is mapped from the +x axis to the -x axis
	// with the back being the reverse of the front.
	// This is limited but nominally useful.

	  t = 0.0;
	  tex->v[i].y = (vrt[i].y - min_y) * iylen;
	  lx = vrt[i].x - cent_x;
	  lz = vrt[i].z - cent_z;
	  t = qisqrt(lx*lx + lz*lz);
	  tex->v[i].x = map(lx*t);
  }
  return current_vertices;
}

int csMetaGen::GenerateFieldSurface( int field_index )
{
  int i,j,m;
  current_vertices = vertices_tesselated =
	CalcBlobSurf(fields[field_index]);
  printf(";Calc Field surface completed %d\n",vertices_tesselated);
  if (!vertices_tesselated) return 0;

  CleanupSurface();

//----------------------------------------------------------------
// The next routine sets up the triangles and the texturing for
// the mesh...

// Allocate texel array
  tex = (TexelArray *)malloc(sizeof(TexelArray));
  tex->v = (csVector2 *)malloc(sizeof(csVector2) * current_vertices );
  tex->num_texels = current_texels = current_vertices;

  csVector3* vrt = verts->v;
  MetaField* mf = fields[field_index];
  PointCharge p;
  m = mf->num_points;
  for (i = 0; i < current_vertices; i++)
  {
    csVector3 n(0, 0, 0);
    for(j = 0; j < m; j++)
    {
	  p = mf->points[j];
      csVector3 rv(vrt[i].x - p.pos.x,
        vrt[i].y - p.pos.y,
        vrt[i].z - p.pos.z);

      float r = rv.Norm();
      float c = p.charge / (r*r*r);

      n += rv * c;
    }
    n = n.Unit();

	tex->v[i].x = map (n.x);
	tex->v[i].y = map (n.y);
  }
  CreateBoundingBoxBlob(field_index);
  return current_vertices;
}
void csMetaGen::HardTransform( const csReversibleTransform& )
{
  printf("Hard transform\n");
}


void csMetaGen::RemapVertices( int * /* mapping */, int /* num */ )
{
#if 0
  if (num)
	for( int i=0; i < num; i++)
	  verts[i] = mapping[verts[i]];
#endif
}

void csMetaGen::SetMaxVertices( int limit )
{
  if (limit > 0)
  {
	if (!verts)
	{
	  verts = (VertArray *) malloc(sizeof(VertArray));
	  verts->max_vertices = 0;
	}
	verts->v = (verts->max_vertices) ?
	  (csVector3 *) realloc( verts->v, sizeof( csVector3 ) * limit) :
		(csVector3 *) malloc( sizeof(csVector3) * limit);
	verts->max_vertices = limit;
  }
  else
	if (verts)
	{
	  free(verts->v);
	  free(verts);
	  verts = NULL;
	}
}

void csMetaGen::DeleteBuffers()
{
  if (trigs)
  {
	free(trigs->t);
	free(trigs);
	trigs = NULL;
	current_triangles = 0;
  }
  if ( tex )
  {
	free(tex->v);
	free(tex);
	tex = NULL;
	current_texels = 0;
  }
}
#if 0
csVector3* csMetaGen::GetVertices()
{
  if (!current_vertices) return 0;
  csVector3* v = (csVector3*)malloc(sizeof(csVector3) * current_vertices);
  memcpy( v, verts->v, sizeof(csVector3) * current_vertices);
  return v;
}

csVector2* csMetaGen::GetTexels()
{
  if (!current_texels) return 0;
  csVector2 *t = (csVector2*)malloc(sizeof(csVector2) * current_texels);
  memcpy( t, tex->v, sizeof(csVector2) * current_texels);
  return t;
}

csTriangle* csMetaGen::GetTriangles()
{
  if (!current_triangles) return 0;
  csTriangle *t = (csTriangle*)malloc(sizeof(csTriangle) * current_triangles);
  memcpy( t, trigs->t, sizeof(csTriangle) * current_triangles);
  return t;
}
#endif
//========================================= csMetGenType

SCF_IMPLEMENT_IBASE (csMetaGenType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaGenType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csMetaGenType)

SCF_EXPORT_CLASS_TABLE (metagen)
  SCF_EXPORT_CLASS (csMetaGenType, "crystalspace.mesh.factory.metagen",
    "The Crystal Space Meta Surface Generator")
SCF_EXPORT_CLASS_TABLE_END

csMetaGenType::csMetaGenType( iBase *par )
{
  SCF_CONSTRUCT_IBASE (par);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csMetaGenType::~csMetaGenType()
{
}

iMeshObjectFactory* csMetaGenType::NewFactory()
{
  csMetaGen* cm = new csMetaGen(this);
  iMeshObjectFactory* ifact = SCF_QUERY_INTERFACE(cm, iMeshObjectFactory);
  ifact->DecRef();
  return ifact;
}
