/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __SHAGNETRON_H__
#define __SHAGNETRON_H__

#include "crystalspace.h"

class Shagnetron
{
private:
  iObjectRegistry* object_reg;
  bool doVerbose;
  
  csRef<iDocumentSystem> docsys;
  csRef<iVFS> vfs;
  csRef<iShaderManager> shaderMgr;
  
  bool FileBlacklisted (const char* file);
  bool PrecacheShaderFile (const char* file, bool doQuick);
public:
  Shagnetron (iObjectRegistry* object_reg);

  bool Initialize ();
  bool Run ();

  void PrintHelp();
};

#endif // __SHAGNETRON_H__

