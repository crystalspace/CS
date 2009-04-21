/*
  Crystal Space Maya .ma Convertor
  Copyright (C) 2002 by Keith Fulton <keith@paqrat.com>
    (loosely based on "mdl2spr" by Nathaniel Saint Martin <noote@bigfoot.com>
                     and Eric Sunshine <sunshine@sunshineco.com>)

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

#ifndef __MAYA_MDL_H__
#define __MAYA_MDL_H__

#include "mayabase.h"
#include "mayanode.h"
#include "mayafile.h"


class Maya4Model : public MayaModel
{
  typedef MayaModel superclass;

protected:

    DAGNode  tree;

    NodeAnimCurveTL *animnode;
    NodeMesh        *meshnode;
    NodeFile        *filenode;
    csVector3        translate,scale;

    bool CreateNode(MayaInputFile& file,DAGNode& tree);

public:
  static bool IsFileMayaModel(const char* mdlfile);

  Maya4Model();
  Maya4Model(const char* mdlfile);
  virtual ~Maya4Model();
  virtual void dumpstats(FILE*);
  virtual bool ReadMAFile(const char* mdlfile);
  virtual bool WriteSPR(const char *spritename, csArray<Animation*>& anims);

protected:
  void Clear();
};

#endif // __MAYA_MDL_H__
