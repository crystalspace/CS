/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CSBENCH_H__
#define __CSBENCH_H__

class CsBench;
struct iMaterialHandle;
struct iImageIO;
struct iObjectRegistry;
struct iVirtualClock;
struct iGraphics3D;
struct iGraphics2D;
struct iVFS;
struct iEvent;
struct iEngine;
struct iLoader;
struct iView;
struct iSector;

class CsBench
{
public:
  iObjectRegistry* object_reg;

private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iImageIO> imageio;
  csRef<iGraphics3D> g3d;
  csRef<iVFS> vfs;
  csRef<iVirtualClock> vc;
  csRef<iCommandLineParser> cmdline;
  csRef<iView> view;
  iSector* room1;
  iSector* room2;

  bool CreateGeometry ();
  void BenchMark (const char* name, const char* description);

public:
  CsBench ();
  virtual ~CsBench ();

  void Report (const char* msg, ...);
  bool ReportError (const char* msg, ...);

  bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  void PerformTests ();
};

#endif // __CSBENCH_H__

