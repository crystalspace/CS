/*
    Metaballs Demo
    Copyright (C) 1999 by Denis Dmitriev
    Pluggified by Samuel Humphreys

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

#ifndef __META_H__
#define __META_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "ivaria/imetabal.h"

class csMaterialHandle;
struct G3DPolygonDPFX;
struct iSystem;
struct iGraphics3D;
struct iGraphics2D;
struct iMaterialHandle;

struct GridCell
{
   csVector3 p[8];
   float val[8];
   GridCell() {} // NextStep 3.3 compiler barfs without this.
};

struct Triangle
{
   csVector3 p[3];
   Triangle() {} // NextStep 3.3 compiler barfs without this.
};

struct MetaBall
{
  csVector3 center;
};

class csMetaBalls : public iMetaBalls
{
  int num_meta_balls;
  int max_triangles;
  int triangles_tesselated;
  EnvMappingModes env_mapping;
  float env_map_mult;
  float alpha;
  MetaParameters mp;

  iGraphics3D *G3D;
  iSystem *Sys;
  iMaterialHandle *th;
  G3DPolygonDPFX* poly;

  MetaBall *meta_balls;
  Triangle *triangles_array;

  char frame;
  int z_crit;

public:
  DECLARE_IBASE;
  csMetaBalls (iBase *iParent);
  virtual ~csMetaBalls ();
  virtual bool Initialize (iSystem *sys);

  virtual void SetContext (iGraphics3D *g3d);
  virtual void SetMaterial (iMaterialHandle *tex)
  { th = tex; }

  virtual void SetNumberMetaBalls (int number);
  virtual int GetNumberMetaBalls ()
  { return num_meta_balls; }

  virtual void SetQualityEnvironmentMapping (bool toggle);
  virtual bool GetQualityEnvironmentMapping ()
  { return env_mapping; }

  virtual void SetEnvironmentMappingFactor (float env_mult);
  virtual float GetEnvironmentMappingFactor ()
  { return env_map_mult; }

  virtual MetaParameters *GetParameters ()
  { return &mp; }

  virtual int ReportNumberTriangles ()
  { return triangles_tesselated; }

  virtual bool Draw ();

  // Where the real work gets done....
  void DrawSomething(void);
  int Tesselate (const GridCell &grid,Triangle *triangles);
  void CalculateMetaBalls (void);
  void CalculateBlob (int x, int y, int z);
  void FillCell (int x, int y, int z, GridCell &c);
  float map (float x);
  float potential (const csVector3 &p);
  int check_cell_assume_inside (const GridCell &c);
  void InitTables (void);
};

#endif // __META_H__
