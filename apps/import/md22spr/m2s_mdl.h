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

#ifndef __M2S_MDL_H__
#define __M2S_MDL_H__

#include "m2s_base.h"

#define MDL_FRAME_NAME_MAX 16

// Quake model MDL structures.

struct mdl_t
{
  vec3_t scale;               // Model scale factors.
  vec3_t origin;              // Model origin.
  scalar_t radius;            // Model bounding radius.
  vec3_t offsets;             // Eye position (useless?)
  int32 numskins ;            // the number of skin textures
  int32 skinwidth;            // Width of skin texture; must be multiple of 8
  int32 skinheight;           // Height of skin texture; must be multiple of 8
  int32 numverts;             // Number of vertices
  int32 numtris;              // Number of triangles surfaces
  int32 numframes;            // Number of frames
  int32 synctype;             // 0=synchronized, 1=random
  int32 flags;                // 0 (see Alias models)
  scalar_t size;              // average size of triangles
};

struct skin_t
{
  bool group;
  int32 nbtexs;                // number of pictures in group
  unsigned char **texs;       // the textures
  float *timebtwskin;         // time values, for each texture
};

struct vertice_t
{
  int32 onseam;                // 0 or 0x20
  int32 s;                     // position, horizontally in range [0,skinwidth)
  int32 t;                     // position, vertically in range [0,skinheight)
};

struct triangle_t
{
  int32 facefront;             // boolean
  int32 vertice[3];            // Index of 3 triangle vertices
};                            // in range [0,numverts)

struct frame_t
{
  trivertx_t min;             // minimum values of X,Y,Z
  trivertx_t max;             // maximum values of X,Y,Z
  char name[MDL_FRAME_NAME_MAX + 1]; // name of frame
  trivertx_t *trivert;        // array of vertices
};

struct frameset_t
{
  bool group;                 // is a mdl group
  trivertx_t min;             // min position in all simple frames
  trivertx_t max;             // max position in all simple frames
  int32 nbframes;              // number of frames
  float *delay;               // time for each of the single frames
  frame_t *frames;            // a group of simple frames
};

class Mdl : public QModel
{
  typedef QModel superclass;
public:
  static bool IsFileMDLModel(const char* mdlfile);

  Mdl();
  Mdl(const char* mdlfile);
  virtual ~Mdl();
  virtual void dumpstats(FILE*) const;
  virtual bool ReadMDLFile(const char* mdlfile);
  virtual bool WriteSPR(const char *spritename, float scaleMdl, int delayMdl,
    float positionMdlX, float positionMdlY, float positionMdlZ,
    bool actionNamingMdl, bool resizeSkin, int maxFrames) const;

  float radiusbound;
  float scaleX, scaleY, scaleZ;
  float originX, originY, originZ;

  int skinheight;
  int skinwidth;
  int nbskins;
  skin_t *skins;

  int nbvertices;
  vertice_t *vertices;

  int nbtriangles;
  triangle_t *triangles;

  int nbframesets;
  frameset_t *framesets;

protected:
  void Clear();
};

#endif // __M2S_MDL_H__
