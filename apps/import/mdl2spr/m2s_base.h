/*
  Crystal Space Quake MDL/MD2 convertor
  Copyright (C) 1998 by Nathaniel Saint Martin <noote@bigfoot.com>
  Significant overhaul by Eric Sunshine <sunshine@sunshineco.com> in Feb 2000

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

#ifndef __M2S_BASE_H__
#define __M2S_BASE_H__

#include "cstypes.h"
#include <stdio.h>

typedef float scalar_t;       // Scalar value,

struct vec3_t                 // Vector or Position
{
  scalar_t x;                 // horizontal
  scalar_t y;                 // horizontal
  scalar_t z;                 // vertical
};

struct magic_t
{
  char id[4];                 // "IDPO" (MDL) or "IDP2" (MD2)
  int32 version;               // MDL=6, MD2=8
};

struct trivertx_t
{
  unsigned char packedposition[3];   // X,Y,Z coordinate, packed on 0-255
  unsigned char lightnormalindex;    // index of the vertex normal
};

class QModel
{
private:
  bool bError;
  char* sError;

protected:
  void clearError();
  bool setError(const char* errorstring, FILE* closethis = 0);
  static bool CheckMagic(const char* mdlfile, const char* magic);

public:
  QModel();
  virtual ~QModel();
  virtual void dumpstats(FILE*) const = 0;
  const char* getErrorString() const { return sError; }
  bool getError() const { return bError; }

  virtual bool ReadMDLFile(const char* mdlfile) = 0;
  virtual bool WriteSPR(const char *spritename, float scaleMdl, int delayMdl,
    float positionMdlX, float positionMdlY, float positionMdlZ,
    bool actionNamingMdl, bool resizeSkin, int maxFrames) const = 0;
};

#endif // __M2S_BASE_H__
