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
#include "csgfx/shadervarcontext.h"

class csVector3;
class csVector2;
class csColor;
class csReversibleTransform;
struct iTextureHandle;
struct iMaterialWrapper;
struct iMeshFactory;
struct iPortalContainer;

/**
 * Data required by the renderer to draw a mesh.
 */
struct csRenderMesh
{
  /**
   * To make debugging easier we add the name of the mesh object
   * here in debug mode.
   */
  const char* db_mesh_name;

  csRenderMesh () 
  {
    z_buf_mode = CS_ZBUF_NONE;
    mixmode = CS_FX_COPY;
    clip_portal = 0;
    clip_plane = 0;
    clip_z_plane = 0;
    do_mirror = false;
    indexstart = indexend = 0;
    portal = 0;
    geometryInstance = 0;
    inUse = false;
    
    db_mesh_name = "<unknown>";
  }

  ~csRenderMesh () {}

  /// In use by the rendering system
  bool inUse;

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

  /**
   * Mirror mode - whether the mesh should be mirrored.
   * Essentially toggles between back- and front-face culling. 
   * It should be set to \p true if \a object2camera contains a negative
   * scaling. Basically, in almost any case it should be set to the camera's
   * mirror mode.
   *
   * \example
   * iCamera* camera;
   * csRenderMesh myMesh;
   *   ...
   * myMesh.object2camera = camera->GetTransform () / 
   *   movable->GetFullTransform ();
   * myMesh.do_mirror = camera->IsMirrored ();
   * \endexample
   */
  bool do_mirror;

  /// Mesh type
  csRenderMeshType meshtype;

  /// Start of the range of indices to use
  unsigned int indexstart;

  /// End of the range of indices to use
  unsigned int indexend;

  /// Material used for this mesh
  //iMaterialHandle* mathandle;
  iMaterialWrapper* material;

  /// Transform to use for this mesh (object->camera)
  csReversibleTransform object2camera;
  /// Camera coordinate of the local 0,0,0 point of the object.
  csVector3 camera_origin;

  /// @@@ Document me.
  csRef<iShaderVariableContext> variablecontext;

  /// Alpha mode this mesh is drawn.
  csAlphaMode::AlphaType alphaType;

  /**
   * Some unique ID for the geometry used to render this mesh
   * Used for sorting purposes, and is allowed to be 0, although
   * that means non-optimal mesh sorting at rendering
   */
  void *geometryInstance;

  /// Pointer to a portalcontainer, if there is any
  iPortalContainer* portal;
};

/** @} */

#endif // __CS_IVIDEO_RENDERMESH_H__
