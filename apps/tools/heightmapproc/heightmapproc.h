/*
    Copyright (C) 2009 by Mike Gist

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

#ifndef __HEIGHTMAPPROC_H__
#define __HEIGHTMAPPROC_H__

#include "crystalspace.h"
#include "stdarg.h"

class HeightMapProc
{
public:
  HeightMapProc(iObjectRegistry* object_reg);
  ~HeightMapProc();

  void Run();

private:
  iObjectRegistry* object_reg;
  csRef<iCommandLineParser> clp;
  csRef<iEngine> engine;
  csRef<iImage> image;
  csRef<iImageIO> imageio;
  csRef<iThreadedLoader> loader;
  csRef<iVFS> vfs;

  float* heightBuffer;
  int width;
  int height;

  void ReadHeightmap();
  void ProcessHeightmap();
  void SmoothHeightmap();
  void WriteHeightmap();
};

#endif // __HEIGHTMAPPROC_H__

