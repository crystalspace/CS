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

#ifndef __M2S_MD2_H__
#define __M2S_MD2_H__

#include "m2s_base.h"

// Quake2 model MD2 structures.

#define MD2_FRAME_NAME_MAX 16
#define MD2_SKIN_NAME_MAX 64

struct md2_t
{
  int32 skinwidth;
  int32 skinheight;
  int32 framesize;              // byte size of each frame

  int32 numskins;               // number of skins
  int32 numxyz;                 // number of xyz points
  int32 numverts;               // number of vertices
  int32 numtris;                // number of triangles
  int32 numglcmds;              // dwords in strip/fan command list ?
  int32 numframes;              // number of frames

  int32 ofsskins;              // each skin is a MD2_SKIN_NAME_MAX string
  int32 ofsverts;              // offset from start for stverts
  int32 ofstris;               // offset for dtriangles
  int32 ofsframes;             // offset for first frame
  int32 ofsglcmds;
  int32 ofsend;                // end of file
};

struct skin2_t
{
  char name[MD2_SKIN_NAME_MAX + 1]; // skin name
};

struct vertice2_t
{
  int16 s;                     // position, horizontally; range [0,skinwidth)
  int16 t;                     // position, vertically; range [0,skinheight)
};

struct triangle2_t
{
  int16 xyz[3];                // Index of 3 triangle xyz; range [0,numxyz)
  int16 vertice[3];            // Index of 3 triangle vertices
};                             // in range [0,numverts)

struct frame2_t
{
  vec3_t scale;               // scale of values of X,Y,Z
  vec3_t translate;           // translation of values of X,Y,Z
  char name[MD2_FRAME_NAME_MAX + 1]; // name of frame
  trivertx_t *trivert;        // array of vertices
};

class Md2 : public QModel
{
protected:
  typedef QModel superclass;

  class md2vertexset
  {
  private:
    int numindices;
    short *xyzcoordindices;
    short *texcoordindices;
  public:
    md2vertexset() : numindices(0), xyzcoordindices(0), texcoordindices(0) {}
    ~md2vertexset();
    int vertexindexcount() const { return numindices; }
    short get_csvertexindex(short xyzindex, short texindex);
    void get_md2vertexmap(
      short csindex, short &xyzindex, short &texindex) const;
  };

  int version;
  int skinheight;
  int skinwidth;
  int nbskins;
  skin2_t *skins;
  int nbvertices;
  vertice2_t *vertices;
  md2vertexset modelvertices;
  int nbtriangles;
  short *triangles;
  int nbxyz;
  int nbframes;
  frame2_t *frames;

  void Clear();

public:
  static bool IsFileMD2Model(const char* mdlfile);

  Md2();
  Md2(const char* mdlfile);
  virtual ~Md2();
  virtual void dumpstats(FILE*) const;
  virtual bool ReadMDLFile(const char* mdlfile);
  virtual bool WriteSPR(const char *spritename, float scaleMdl, int delayMdl,
    float positionMdlX, float positionMdlY, float positionMdlZ,
    bool actionNamingMdl, bool resizeSkin, int maxFrames) const;
    // MD2 ignores `resizeSkin'
};

#endif // __M2S_MD2_H__
