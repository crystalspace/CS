/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef PERF_H
#define PERF_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "igraph3d.h"

class PerfTest;

// Number of horizontal and vertical polygons for every multi-polygon test.
#define NUM_MULTIPOLTEST 24

class Tester
{
protected:
  int draw;

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest) = 0;
  virtual void Draw (iGraphics3D* g3d) = 0;
  virtual void Description (char* dst) = 0;
  virtual Tester* NextTester () = 0;
  int GetCount () { return draw; }
};

class SinglePolygonTester : public Tester
{
private:
  G3DPolygonDPFX poly;

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    strcpy (dst, "Texture mapped polygon with gouraud shading... ");
  }
  virtual Tester* NextTester ();
};

class SinglePolygonTesterFlat : public Tester
{
private:
  G3DPolygonDPFX poly;

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    strcpy (dst, "Polygon with gouraud shading... ");
  }
  virtual Tester* NextTester ();
};

class SinglePolygonTesterAlpha : public Tester
{
private:
  G3DPolygonDPFX poly;

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    strcpy (dst, "Texture mapped polygon with alpha transparency... ");
  }
  virtual Tester* NextTester ();
};

class MultiPolygonTester : public Tester
{
private:
  G3DPolygonDPFX poly[NUM_MULTIPOLTEST][NUM_MULTIPOLTEST];

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst, "%d Texture mapped polygons with gouraud shading... ",
    	NUM_MULTIPOLTEST*NUM_MULTIPOLTEST);;
  }
  virtual Tester* NextTester ();
};

class MultiPolygon2Tester : public Tester
{
private:
  G3DPolygonDPFX poly[NUM_MULTIPOLTEST][NUM_MULTIPOLTEST];

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst, "%d Texture mapped polygons with gouraud shading (vs 2)... ",
    	NUM_MULTIPOLTEST*NUM_MULTIPOLTEST);
  }
  virtual Tester* NextTester ();
};

class MultiTexture1Tester : public Tester
{
private:
  G3DPolygonDPFX poly[NUM_MULTIPOLTEST][NUM_MULTIPOLTEST];

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst, "%d polygons with four alternating textures... ",
    	NUM_MULTIPOLTEST*NUM_MULTIPOLTEST);
  }
  virtual Tester* NextTester ();
};

class MultiTexture2Tester : public Tester
{
private:
  G3DPolygonDPFX poly[NUM_MULTIPOLTEST][NUM_MULTIPOLTEST];

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst, "%d polygons with four grouped textures... ",
    	NUM_MULTIPOLTEST*NUM_MULTIPOLTEST);
  }
  virtual Tester* NextTester ();
};

class MeshTester : public Tester
{
private:
  G3DTriangleMesh mesh;

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst, "%d polygons with DrawTriangleMesh... ",
    	NUM_MULTIPOLTEST*NUM_MULTIPOLTEST);
  }
  virtual Tester* NextTester ();
};

class PerfTest : public SysSystemDriver
{
  typedef SysSystemDriver superclass;

private:
  bool needs_setup;
  Tester* current_tester;
  iTextureHandle* texture[4];
  // Load a texture.
  iTextureHandle* LoadTexture (char* file);

public:
  PerfTest ();
  virtual ~PerfTest ();

  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame (time_t elapsed_time, time_t current_time);
  virtual bool HandleEvent (csEvent &Event);
  
  iTextureHandle* GetTexture (int idx)
  {
    return texture[idx];
  }
};

#endif // PERF_H

