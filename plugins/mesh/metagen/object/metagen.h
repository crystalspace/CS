/*
    Meta Surface Generator
    Copyright (C) 2001 by Michael H. Voase
	
    Based on the meta surface tesselator
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

#ifndef __METAGEN_H__
#define __METAGEN_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "imesh/object.h"
#include "imesh/metagen.h"
#include "csutil/csvector.h"
#include "csutil/cscolor.h"

#define ALL_FEATURES (CS_OBJECT_FEATURE_LIGHTING|CS_OBJECT_FEATURE_ANIMATION)

#define I_PI_4 1 / (M_PI * 4.0)

struct iSystem;
struct iGraphics3D;
struct iGraphics2D;

//--------------- The core of the tesselator -- the grid cell
struct GridCell
{
   csVector3 p[8];
   float val[8];
   GridCell() {} // NextStep 3.3 compiler barfs without this.
};

//----------------------------------- Slices
struct SliceCharge
{
  csVector2 pos;
  float charge;
};

struct MetaSlice
{
  SliceCharge* charges;
  int num_charges;
  bool is_endcap;
};

struct MetaBone
{
  float iso_level;
  int start_slice;
  MetaSlice **slices;
  int num_slices;
}; 
//--------------------------- Fields
struct PointCharge
{
  csVector3 pos;
  float charge;
};

struct MetaField
{
  float iso_level;
  int num_points;
  PointCharge *points;
};
//------------------------ Data Arrays: Done this way to make
// exporting the data easier. Can get very cumbersome with csVector.
// The vertices and triangles are optimized after tesselation,
// so the actual size will be greater than their current content.

struct VertArray
{
  csVector3 *v;
  int max_vertices; // Actual buffer size - not current count
};

struct TriangleArray
{
  csTriangle *t;
  int num_triangles; // Actuall buffer size, not current count
};

struct TexelArray
{
  csVector2 *v;
  int num_texels; // Actuall buffer size, not current count
};

// Note: MetaGen is just a factory, there is no mesh object here and
// New Instance returns NULL. This is to avoid having NextFrame visiting
// through here all the time ( which is not necessary ).

class csMetaGen : public iMeshObjectFactory
{

  float XStart, YStart, ZStart; // Rendering Cache size and resolution
  float XFin, YFin, ZFin;

  float stepx, stepy, stepz;
  float istepx, istepy, istepz;

  float *asin_table;
  int asin_table_res;

  VertArray *verts;
  int current_vertices;
  TriangleArray *trigs;
  int current_triangles;
  TexelArray *tex;
  int current_texels;
  
  DECLARE_TYPED_VECTOR(MetaBoneVector,MetaBone) bones;
  DECLARE_TYPED_VECTOR(MetaFieldVector, MetaField) fields;

  bool cache_ready;
  bool asin_table_ready;
  bool initialized;
  bool do_lighting;
  bool is_complete;

//------------- MetaGen
  int vertices_tesselated;
  EnvMappingModes env_mapping;
  float env_map_mult;
  char frame;
  float splinter_size;
//------------- MeshFactory Data
  csBox3 object_bbox;
  float current_lod;
  uint32 current_features;
    

public:
  DECLARE_IBASE;
  
  csMetaGen (iBase *parent);
  virtual ~csMetaGen ();
  virtual bool Initialize ();
  virtual bool InitializeCache ();
  
  virtual int GenerateLinearSurface( int bone_index );
	
  virtual int GenerateFieldSurface( int field_index );
  
  virtual void SetCacheLimits( csVector3 s, csVector3 f )
	{ if ( s.x < f.x && s.y < f.y && s.z < f.z  ) 
	  { XStart = s.x; YStart = s.y; ZStart = s.z;
		XFin = f.x; YFin = f.y; ZFin = f.z; }}
		
  virtual void MapTriangleMesh( csTriangle *, csVector3 * ) {};

  virtual void ClearCache() 
  { frame++; DeleteBuffers(); }
  virtual void ZeroCache();
  void DeleteBones();

  virtual void SetMaxVertices( int limit );
  virtual int GetMaxVertices() 
  { if ( verts ) return verts->max_vertices;  
	  else return 0; }
  
  virtual void SetQualityEnvironmentMapping (bool toggle);
  virtual bool GetQualityEnvironmentMapping ()
	{ return env_mapping; }

  virtual void SetEnvironmentMappingFactor (float env_mult);
  virtual float GetEnvironmentMappingFactor ()
  { return env_map_mult; }

  virtual int ReportTriangleCount ()
  { return vertices_tesselated; }

  virtual void SetArcSineTableRes( int res )
	{ if (res > 0) asin_table_res = res; }
  virtual int GetArcSineTableRes() { return asin_table_res; }
  virtual void DeleteArcSineTable();
  virtual bool InitArcSineTable();

  void FillArcSineTable();
  void DeleteBuffers();
// Data addition calls.

  virtual void CreateBone( int start, float iso_lev);
  virtual void AddSlice( bool endcap );
  virtual void AddCharge( csVector2 pos, float charge );
  virtual void CreateField( float iso_level );
  virtual void AddPoint( csVector3 pos, float charge );

// Data extraction calls. You provide the pointer, the method will
// memcpy the vertices into a new buffer and return the count.

  virtual csVector3* GetVertices();
  virtual csVector2* GetTexels();
  virtual csTriangle* GetTriangles();
  virtual int GetVertexCount()
	{ return current_vertices; }
  virtual int GetTexelCount()
	{ return current_texels; }
  virtual int GetTriangleCount()
	{ return current_triangles; }
  virtual void SetSplinterSize( float sz )
	{ splinter_size = sz; }
  virtual float GetSplinterSize()
	{ return splinter_size; }

// Where the real work gets done....
  inline int _2intY( float fy ) { return int(istepy * (fy - YStart )); }
  inline float _2coordY( int y ) { return y * stepy + YStart; }
  int Tesselate (const GridCell &grid, csVector3* verts);
  int CalcBlobSurf (MetaField *field);
  int CalcLinSurf (MetaBone *bone);
  void BlobCalc (int x, int y, int z);
  void RingCalc (int x, int z );
  void FillCell (int x, int y, int z, GridCell &c);
  void FillCellSlice(int x, int y, int z, GridCell &c);
  void GenCell(int x, int y, int z, GridCell &c);
  
  float map (float x);
  float potential (const csVector3 &p);
  float potential (float px, float py, int slice );
  void _2int( const csVector3 &pos, int &x, int &y, int &z );
  void _2int2( const csVector2 &pos, int &x, int &z );
  void _2coord( int x, int y, int z, csVector3 &r);
  int check_cell_assume_inside (const GridCell &c);
  void CreateLighting ( iLight **, int, iMovable *);
  const int GetResX();
  const int GetResY();
  const int GetResZ();
  void RemapVertices( int* mapping, int num );
  void CleanupSurface();
  
///-------------------- iMeshFactory implementation --------------

  void CreateBoundingBoxLinear(int num);
  void CreateBoundingBoxBlob(int num);
  virtual void GetObjectBoundingBox ( csBox3& bbox, int type = CS_BBOX_NORMAL );  
  virtual iMeshObject* NewInstance() { return NULL; }
  virtual void HardTransform( const csReversibleTransform &t );
  virtual bool SupportsHardTransform() const { return true; }
  virtual bool IsLighting() { return do_lighting; }
  virtual void SetLighting( bool set ) { do_lighting = set; }
  virtual uint32 GetLODFeatures () const { return current_features; }
  virtual void SetLODFeatures (uint32 mask, uint32 value)
  {
    mask &= ALL_FEATURES;
    current_features = (current_features & ~mask) | (value & mask);
  }
  virtual void SetLOD (float lod) { current_lod = lod; }
  virtual float GetLOD () const { return current_lod; }
  virtual int GetLODPolygonCount (float /*lod*/) const
  {
    return 0;	// @@@ Implement me please!
  }

///-------------------- Meta Ball state implementation
  class MetaGen : public iMetaGen
  {
	DECLARE_EMBEDDED_IBASE(csMetaGen);

	virtual int GenerateLinearSurface( int b )
	  { return scfParent->GenerateLinearSurface(b); }
	virtual int GenerateFieldSurface( int f )
	  { return scfParent->GenerateFieldSurface(f); }
	virtual void SetCacheLimits( csVector3 start, csVector3 end )
	  { scfParent->SetCacheLimits(start, end ); }
	virtual void MapTriangleMesh( csTriangle *tri, csVector3 *verts )
	  { scfParent->MapTriangleMesh( tri, verts ); }
	virtual void ClearCache() { scfParent->ClearCache(); }
	virtual void ZeroCache() { scfParent->ZeroCache(); }
	virtual bool Initialize() { return scfParent->Initialize(); }
	virtual bool InitializeCache() { return scfParent->InitializeCache(); }
	virtual void SetMaxVertices( int limit ) 
	  { scfParent->SetMaxVertices(limit); }
	virtual int GetMaxVertices() { return scfParent->GetMaxVertices(); }
	virtual void SetArcSineTableRes( int res )
	  { scfParent->SetArcSineTableRes( res ); }
	virtual int GetArcSineTableRes()
	  { return scfParent->GetArcSineTableRes(); }
	virtual void DeleteArcSineTable() { scfParent->DeleteArcSineTable(); }
	virtual void InitArcSineTable() { scfParent->InitArcSineTable(); }
	virtual void SetQualityEnvironmentMapping ( bool tog )
	  { scfParent->SetQualityEnvironmentMapping( tog ); }
	virtual bool GetQualityEnvironmentMapping()
	  { return scfParent->GetQualityEnvironmentMapping(); }
	virtual float GetEnvironmentMappingFactor()
	  { return scfParent->GetEnvironmentMappingFactor(); }
	virtual void SetEnvironmentMappingFactor(float env )
	  { scfParent->SetEnvironmentMappingFactor( env ); }
	virtual int ReportTriangleCount()
	  { return scfParent->ReportTriangleCount(); }
	virtual bool IsLighting() 
	  { return scfParent->IsLighting(); }
	virtual void SetLighting(bool set) 
	  { scfParent->SetLighting(set); }
	virtual void CreateBone( int start, float iso_lev)
	  { scfParent->CreateBone( start, iso_lev); }
	virtual void AddSlice( bool endcap )
	  { scfParent->AddSlice(endcap); }
	virtual void AddCharge( csVector2 pos, float charge )
	  { scfParent->AddCharge( pos, charge); }
	virtual void CreateField( float iso_level )
	  { scfParent->CreateField(iso_level); }
	virtual void AddPoint( csVector3 pos, float charge )
	  { scfParent->AddPoint(pos,charge); }
	virtual csVector3* GetVertices()
	  { return scfParent->GetVertices(); }
	virtual csVector2* GetTexels()
	  { return scfParent->GetTexels(); }
	virtual csTriangle* GetTriangles()
	  { return scfParent->GetTriangles(); }
	virtual int GetVertexCount()
	  { return scfParent->GetVertexCount(); }
	virtual int GetTexelCount()
	  { return scfParent->GetTexelCount(); }
	virtual int GetTriangleCount()
	  { return scfParent->GetTriangleCount(); }
	virtual void SetSplinterSize(float size)
	  { scfParent->SetSplinterSize(size); }
	virtual float GetSplinterSize()
	  { return scfParent->GetSplinterSize(); }
  } scfiMetaGen;
  friend class MetaGen;
};

/*
 *	MetaBall type. Use this plugin to create instances of  
 *	csMetaGenMeshObjectFactory.
 */

class csMetaGenType : public iMeshObjectType
{
public:

  csMetaGenType ( iBase * );
  virtual ~csMetaGenType();
  virtual bool Initialize ( iSystem *sys );
  
  DECLARE_IBASE;
  
  virtual iMeshObjectFactory* NewFactory();
  virtual uint32 GetFeatures () const
  {
    return ALL_FEATURES;
  }
};

#endif // __METAGEN_H__
