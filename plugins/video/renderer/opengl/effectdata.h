/*
    Copyright (C) 2002 by Mårten Svanfeldt
    Written by Mårten Svanfeldt

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

#ifndef __CS_EFFECTDATA_H__
#define __CS_EFFECTDATA_H__

#include "csutil/scf.h"
#include "csutil/parray.h"

///OpenGL specific effectdata stored per layer
SCF_VERSION(csOpenGlEffectLayerData, 0, 0, 1);

#define ED_SOURCE_NONE 0
#define ED_SOURCE_FOG 1
#define ED_SOURCE_MESH 2
#define ED_SOURCE_LIGHTMAP 3
#define ED_SOURCE_USERARRAY(x) (100+(x))


class csOpenGlEffectLayerData : public iBase
{
public:

  //constant colorsource
  //ed_source macros
  int ccsource;

  //texture coordinate source
  //ed_source macro
  int vcord_source;

  //texture to use
  int inputtex;

  //colorsource
  int colorsource[4];
  //color modifier
  int colormod[4];
  //color operation
  int colorp;

  //alphasource
  int alphasource[4];
  //alpha modifier
  int alphamod[4];
  //alpha operation
  int alphap;

  // rgb scale
  float scale_rgb[3];
  // alpha scale
  float scale_alpha;

  SCF_DECLARE_IBASE;

  csOpenGlEffectLayerData()
  {
    SCF_CONSTRUCT_IBASE(0);

    ccsource = ED_SOURCE_NONE;
    vcord_source = ED_SOURCE_NONE;
    inputtex = 0;

    colorsource[0] = GL_PREVIOUS_ARB; colorsource[1] = GL_TEXTURE; colorsource[2] = -1; colorsource[3] = -1;
    colormod[0] = GL_SRC_COLOR; colormod[1] = GL_SRC_COLOR; colormod[2] = GL_SRC_COLOR; colormod[3] = GL_SRC_COLOR;
    colorp = GL_MODULATE;

    alphasource[0] = GL_PREVIOUS_ARB; alphasource[1] = GL_TEXTURE; alphasource[2] = -1; alphasource[3] = -1;
    alphamod[0] = GL_SRC_ALPHA; alphamod[1] = GL_SRC_ALPHA; alphamod[2] = GL_SRC_ALPHA; alphamod[3] = GL_SRC_ALPHA;
    alphap = GL_MODULATE;

    scale_rgb[0] = scale_rgb[1] = scale_rgb[2] = 1.0f;
    scale_alpha = 1.0f;
  }

  virtual ~csOpenGlEffectLayerData()
  {
    SCF_DESTRUCT_IBASE();
  }
};

SCF_IMPLEMENT_IBASE(csOpenGlEffectLayerData)
  SCF_IMPLEMENTS_INTERFACE(csOpenGlEffectLayerData)
SCF_IMPLEMENT_IBASE_END

///OpenGL specific effectdata stored per pass
SCF_VERSION(csOpenGlEffectPassData, 0, 0, 1);

#ifndef CS_EFVARIABLETYPE_UNDEFINED
#define CS_EFVARIABLETYPE_UNDEFINED 0
#define CS_EFVARIABLETYPE_FLOAT 1
#define CS_EFVARIABLETYPE_VECTOR4 2
#endif

struct csOpenGlVPConstant
{
public:
  int constantNumber;
  int variableID;
  char efvariableType;
  csOpenGlVPConstant(int num, int id,char type)
    {constantNumber = num; variableID = id; efvariableType = type;}
};

class csOpenGlEffectPassData : public iBase
{
public:

  //datavariables
  //blending-vars
  bool doblending;
  int sblend; //sourceblend
  int dblend;

  //shademode
  GLenum shade_state;

  //vertex color source
  //one of ED_VC_SOURCE*
  int vcsource; 

  //id of initiated vertex program
  GLuint vertex_program;

  //vertex program constants
  csPDelArray<csOpenGlVPConstant> vertex_constants;

  SCF_DECLARE_IBASE;

  csOpenGlEffectPassData()
  {
    SCF_CONSTRUCT_IBASE(0);

    doblending = false;
    sblend = GL_ONE;
    dblend = GL_ZERO;

    shade_state = GL_SMOOTH;

    vcsource = ED_SOURCE_MESH;

    vertex_program = 0;
  }

  virtual ~csOpenGlEffectPassData()
  {
    SCF_DESTRUCT_IBASE();
  }
};

SCF_IMPLEMENT_IBASE(csOpenGlEffectPassData)
  SCF_IMPLEMENTS_INTERFACE(csOpenGlEffectPassData)
SCF_IMPLEMENT_IBASE_END

#endif // __CS_EFFECTDATA_H__
