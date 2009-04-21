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

#ifndef __MAYABASE_H__
#define __MAYABASE_H__

#include "csutil/csstring.h"
#include <csutil/array.h>

#include "cstypes.h"
#include <stdio.h>

typedef float scalar_t;       // Scalar value,

struct Animation;

class MayaModel
{
private:
  bool     bError;
  csString sError;

protected:
  void clearError();
  bool setError(const char* errorstring, FILE* closethis = 0);
  static bool CheckMagic(const char* mdlfile, const char* magic);

public:
  MayaModel();
  virtual ~MayaModel();
  virtual void dumpstats(FILE*) = 0;
  const char* getErrorString() const { return sError; }
  bool getError() const { return bError; }

  virtual bool ReadMAFile(const char* mdlfile) = 0;
  virtual bool WriteSPR(const char *spritename, csArray<Animation*>& anims) = 0;
};

#endif // __MAYABASE_H__
