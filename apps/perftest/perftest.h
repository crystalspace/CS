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

#ifndef __PERFTEST_H__
#define __PERFTEST_H__

class PerfTest;
struct iMaterialHandle;
struct iImageIO;
struct iObjectRegistry;
struct iVirtualClock;
struct iGraphics3D;
struct iGraphics2D;
struct iVFS;
struct iEvent;

class Tester
{
protected:
  int draw;

public:
  virtual ~Tester() {}
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest) = 0;
  virtual void Draw (iGraphics3D* g3d) = 0;
  virtual void Description (char* dst) = 0;
  virtual Tester* NextTester () = 0;
  int GetCount () { return draw; }
};


class PerfTest
{
public:
  iObjectRegistry* object_reg;

private:
  bool draw_3d;
  bool draw_2d;
  bool needs_setup;
  Tester* current_tester;
  /// true if user requests to skip the rest of the test (spacebar)
  bool test_skip;
  csRef<iMaterialHandle> materials[4];
  // Load a material.
  csPtr<iMaterialHandle> LoadMaterial (char* file);
  csRef<iImageIO> ImageLoader;
  csRef<iGraphics3D> myG3D;
  csRef<iVFS> myVFS;
  csRef<iVirtualClock> vc;

public:
  PerfTest ();
  virtual ~PerfTest ();

  void Report (int severity, const char* msg, ...);

  bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  void SetupFrame ();
  void FinishFrame ();
  bool HandleEvent (iEvent &Event);

  iMaterialHandle* GetMaterial (int idx)
  {
    return materials[idx];
  }
};

#endif // __PERFTEST_H__

