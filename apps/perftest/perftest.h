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


class Tester
{
protected:
  int draw;

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest) = 0;
  virtual void Draw (iGraphics3D* g3d) = 0;
  virtual char* Description () = 0;
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
  virtual char* Description ()
  {
    return "Texture mapped polygon with gouraud shading... ";
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
  virtual char* Description ()
  {
    return "Polygon with gouraud shading... ";
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
  virtual char* Description ()
  {
    return "Texture mapped polygon with alpha transparency... ";
  }
  virtual Tester* NextTester ();
};

class MultiPolygonTester : public Tester
{
private:
  G3DPolygonDPFX poly[10][10];

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual char* Description ()
  {
    return "100 Texture mapped polygons with gouraud shading... ";
  }
  virtual Tester* NextTester ();
};

class PerfTest : public SysSystemDriver
{
  typedef SysSystemDriver superclass;

private:
  bool needs_setup;
  Tester* current_tester;
  iTextureHandle* texture1;

public:
  PerfTest ();
  virtual ~PerfTest ();

  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame (time_t elapsed_time, time_t current_time);
  virtual bool HandleEvent (csEvent &Event);
  
  iTextureHandle* GetTexture1 () { return texture1; }
};

#endif // PERF_H

