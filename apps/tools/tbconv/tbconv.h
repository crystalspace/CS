/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __TBTOOL_H__
#define __TBTOOL_H__

#include <stdio.h>
#include <stdarg.h>

struct iObjectRegistry;
struct iCommandLineParser;
struct iVFS;
struct iImageIO;
struct iFile;

class TerrBigTool
{
public:
  TerrBigTool (iObjectRegistry* object_reg);
  ~TerrBigTool ();

  bool Init ();
  bool Convert ();

  void ReportError (const char *description, ...);
private:
  iObjectRegistry *object_reg;
  csRef<iCommandLineParser> cmdline;
  csRef<iVFS> vfs;
  csRef<iImageIO> imageio;
  
  csRef<iFile> input;
  FILE *output;

  csVector3 scale;
};

#endif // __TBTOOL_H__
