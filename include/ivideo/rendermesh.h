/*
Copyright (C) 2002 by Marten Svanfeldt
Anders Stenberg

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

#ifndef __CS_IVIDEO_RENDERMESH_H__
#define __CS_IVIDEO_RENDERMESH_H__

/** \file 
* Rendermesh interface
*/

/**
* \addtogroup gfx3d
* @{ */

#include "csutil/strset.h"
#include "csutil/ref.h"
#include "ivideo/graph3d.h"
#include "csgeom/transfrm.h"
#include "ivideo/shader/shader.h"

class csVector3;
class csVector2;
class csColor;
class csReversibleTransform;
struct iTextureHandle;
struct iMaterialWrapper;
struct iRenderBufferSource;

/// Type of mesh
enum csRenderMeshType
{
  CS_MESHTYPE_TRIANGLES,
  CS_MESHTYPE_QUADS,
  CS_MESHTYPE_TRIANGLESTRIP,
  CS_MESHTYPE_TRIANGLEFAN,
  CS_MESHTYPE_POINTS,
  CS_MESHTYPE_LINES,
  CS_MESHTYPE_LINESTRIP,
  CS_MESHTYPE_POLYGON,
};

/// Document me! @@@
class csRenderMesh
{
public:
  csRenderMesh () 
  {
    z_buf_mode = CS_ZBUF_NONE;
    mixmode = CS_FX_COPY;
    clip_portal = 0;
    clip_plane = 0;
    clip_z_plane = 0;
    do_mirror = false;
    indexstart = indexend = 0;
    dynDomain = 0;
  }

  virtual ~csRenderMesh () {}

  /*/// Special attributes. Please don't change, it's used as flags
  typedef enum
  {
  SPECIAL_NONE = 0,
  SPECIAL_BILLBOARD = 1,
  SPECIAL_ZFILL = 2
  } specialattributes;*/

  /// Z mode to use
  csZBufMode z_buf_mode;

  /// mixmode to use
  uint mixmode;

  /// Clipping parameter
  int clip_portal;

  /// Clipping parameter
  int clip_plane;

  /// Clipping parameter
  int clip_z_plane;

  /// Mirror mode
  bool do_mirror;

  /// Mesh type
  csRenderMeshType meshtype;

  /// Start of the range of indices to use
  unsigned int indexstart;

  /// End of the range of indices to use
  unsigned int indexend;

  /// Source to get buffers from
  //iRenderBufferSource* buffersource;

  /// Material used for this mesh
  //iMaterialHandle* mathandle;
  iMaterialWrapper* material;

  /// Transform to use for this mesh (object->camera)
  csReversibleTransform object2camera;

  csRef<iShaderVariableContext> dynDomain;
  
  csAlphaMode::AlphaType alphaType;
};

/** @} */

#endif // __CS_IVIDEO_RENDERMESH_H__
