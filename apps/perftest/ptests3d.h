/*
    Copyright (C) 2000 by Jorrit Tyberghein
    With additions by Samuel Humphreys

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

#ifndef __PTESTS3D_H__
#define __PTESTS3D_H__

#include <stdarg.h>
#include "csgeom/box.h"
#include "ivideo/graph3d.h"
#include "apps/perftest/perftest.h"

struct iVertexBuffer;

// Number of horizontal and vertical polygons for every multi-polygon test.
#define NUM_MULTIPOLTEST 24


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
  int num_mesh_vertices;
  csBox3 bbox;
  csVector3* mesh_vertices;
  csVector2* mesh_texels;
  csRef<iVertexBuffer> vbuf;

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

class PixmapTester : public Tester
{
  int inc_w, inc_h, tex_w, tex_h;
  iTextureHandle *texture;
public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst,
	     "100 Pixmaps with single texture (%d, %d) scaled to (%d, %d)... ",
	     tex_w, tex_h, inc_w-5, inc_h-5);
  }
  virtual Tester* NextTester ();
};


class MultiTexturePixmapTester : public Tester
{
  struct textures
  {
    iTextureHandle *texture;
    int tex_w, tex_h;
  };

  textures tex[4];
  int inc_w, inc_h;

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    strcpy (dst, "100 Pixmaps with 4 alternating textures... ");
  }
  virtual Tester* NextTester ();
};
#endif // __PTESTS3D_H__
