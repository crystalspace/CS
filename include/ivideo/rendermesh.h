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

#include "csgeom/box.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"

#include "iengine/material.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

struct iPortalContainer;

namespace CS
{
namespace Graphics
{
  
  typedef int RenderPriority;
  
  enum MeshCullMode
  {
    cullNormal,
    cullFlipped,
    cullDisabled
  };
  
  /**
   * Returns inverse culling mode for a given culling mode.
   * Specifically, for "normal" culling, "flipped" is returned; for "flipped"
   * culling, "normal" is returned.
   */
  static inline MeshCullMode GetFlippedCullMode (MeshCullMode cullMode)
  {
    switch (cullMode)
    {
      case cullNormal:
        return cullFlipped;
      case cullFlipped:
        return cullNormal;
      case cullDisabled: 
        return cullDisabled;
    }
    // Should not happen ...
    return cullNormal;
  }

  /**
   * Mesh render mode information. Contains the Z, mix and alpha modes to use
   * for rendering a mesh. 
   * \remarks Is separate from CS::Graphics::CoreRenderMesh to allow preprocessing steps 
   *  to modify the mode data. 
   */
  struct RenderMeshModes
  {
    RenderMeshModes () : z_buf_mode ((csZBufMode)~0), mixmode (CS_FX_COPY),
      renderPrio (-1), flipCulling (false), cullMode (cullNormal),
      alphaType (csAlphaMode::alphaNone), zoffset (false)
    {
    }

    RenderMeshModes (RenderMeshModes const& x) :
      z_buf_mode (x.z_buf_mode),
      mixmode (x.mixmode),
      renderPrio (x.renderPrio),
      flipCulling (x.flipCulling),
      cullMode (x.cullMode),
      alphaType (x.alphaType),
      zoffset (x.zoffset),
      buffers (x.buffers)
    {
    }

    ~RenderMeshModes () { }

    /// Z mode to use
    csZBufMode z_buf_mode;

    /// mixmode to use
    uint mixmode;
    
    /// Mesh render priority
    RenderPriority renderPrio;

    // Deprecated in 1.3.
    // ***NOTE*** Though deprecated, actual compiler-based deprecation is not
    // presently enabled since the inline constructors references this variable
    // and cause a Niagara of warnings to flood the build. Worse, even if
    // external projects fix their own code to avoid this variable, they still
    // get penalized by warnings because of the constructors (over which code
    // external projects have no control). One way to avoid this problem would be
    // to un-inline the constructors and then disable the deprecation warning
    // only when compiling the implementation file containing the constructors. A
    // potential difficulty with this approach, however, is that there is no
    // obvious location in the CS/libs hierarchy for such an implementation file
    // to reside.  Since it is considered very unlikely that external projects
    // will be accessing this variable, and since un-inlining the constructors is
    // perhaps unnecessarily complicated, the decision was made instead to remove
    // the compiler-based deprecation attribute, but retain the
    // documentation-based deprecation warning. If real-world experience shows
    // that removing the compiler attribute was the wrong approach, then the
    // un-inlining approach can instead be implemented.
    // CS_DEPRECATED_VAR_MSG("Use cullMode instead", bool flipCulling );
    bool flipCulling;
    
    /// Mesh culling mode
    MeshCullMode cullMode;

    /// Alpha mode this mesh is drawn.
    csAlphaMode::AlphaType alphaType;
    
    /// Whether Z value offsetting should be enabled.
    bool zoffset;

    /// Holder of default render buffers
    csRef<csRenderBufferHolder> buffers;
  };

  /**
  * Start and end for a  range of indices to render.
  * The indices are used in the range from \a start (inclusive) to \a end
  * (exclusive): start <= n < end
  */
  struct RenderMeshIndexRange
  {
    unsigned int start;
    unsigned int end;
  };

  /**
  * Data required by the renderer to draw a mesh.
  */
  struct CoreRenderMesh
  {
    /**
     * To make debugging easier we add the name of the mesh object
     * here in debug mode.
     */
    const char* db_mesh_name;

    CoreRenderMesh () : db_mesh_name ("<unknown>"), clip_portal (0), 
      clip_plane (0), clip_z_plane (0), do_mirror (false),
      multiRanges (0), rangesNum (0), indexstart (0), indexend (0)
    {
    }

    ~CoreRenderMesh () {}

    /// Clipping parameter
    int clip_portal;

    /// Clipping parameter
    int clip_plane;

    /// Clipping parameter
    int clip_z_plane;

    // @@@ FIXME: should prolly be handled by component managing rendering
    /**
     * Mirror mode - whether the mesh should be mirrored.
     * Essentially toggles between back- and front-face culling. 
     * It should be set to \p true if \a object2camera contains a negative
     * scaling. Basically, in almost any case it should be set to the camera's
     * mirror mode.
     *
     * \code
     * iCamera* camera;
     * csRenderMesh myMesh;
     *   ...
     * myMesh.object2camera = camera->GetTransform () / 
     *   movable->GetFullTransform ();
     * myMesh.do_mirror = camera->IsMirrored ();
     * \endcode
     */
    bool do_mirror;

    /// Mesh type
    csRenderMeshType meshtype;
    /**
    * Index ranges to render. If ranges are specified they have precedence
    * over \a indexstart and \a indexend.
    */
    RenderMeshIndexRange* multiRanges;
    /// Number of index ranges in \a multiRanges.
    size_t rangesNum;

    /** @{ */
    /**
     * Start and end of the range of indices to use. The indices are
     * used in the range from \a indexstart (inclusive) to \a indexend
     * (exclusive): indexstart <= n < indexend
     */
    unsigned int indexstart;
    unsigned int indexend;
    /** @} */

    /**
     * Material used for this mesh.
     * Used for e.g. sorting by material.
     */
    iMaterialWrapper* material;

    /** 
     * Transform object space -> world space.
     * \remarks 'this' space is object space, 'other' space is world space
     */
    csReversibleTransform object2world;
    
    /// Render mesh bounding box, object space
    csBox3 bbox;
  };

  /**
   * Mesh data as returned by mesh plugins. Contains both the data needed for
   * rendering as well as some additional data for preprocessing.
   */
  struct RenderMesh : public CoreRenderMesh, public RenderMeshModes
  {
    RenderMesh () : geometryInstance (0), portal (0)
    {
    }

    ~RenderMesh () {}

    /**
     * Some unique ID for the geometry used to render this mesh.
     * Used for sorting purposes, and is allowed to be 0, although
     * that means non-optimal mesh sorting at rendering.
     */
    void *geometryInstance;

    /// Pointer to a portalcontainer, if there is any
    iPortalContainer* portal;

    /// \todo Document me!
    csRef<iShaderVariableContext> variablecontext;

    /// Worldspace origin of the mesh
    csVector3 worldspace_origin;
  };

} // namespace Graphics
} // namespace CS

typedef CS::Graphics::RenderMeshModes csRenderMeshModes;
typedef CS::Graphics::CoreRenderMesh csCoreRenderMesh;
typedef CS::Graphics::RenderMesh csRenderMesh;

/** @} */

#endif // __CS_IVIDEO_RENDERMESH_H__
