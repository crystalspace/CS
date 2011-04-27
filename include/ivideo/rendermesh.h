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
  /// Rendering priority.
  class RenderPriority
  {
    uint value;
  public:
    RenderPriority () : value (uint (~0)) {}
    explicit RenderPriority (uint value) : value (value) {}
    CS_DEPRECATED_METHOD_MSG("Please use CS::Graphics::RenderPriority to store render priorities")
    RenderPriority (long value) : value (value) {}
    CS_DEPRECATED_METHOD_MSG("Please use CS::Graphics::RenderPriority to store render priorities")
    RenderPriority (int value) : value (value) {}
    
    bool IsValid() const { return value != uint (~0); }
    operator uint () const { return value; }
  };

  /// Culling mode of a mesh.
  enum MeshCullMode
  {
    cullNormal,    /*!< Normal culling. */
    cullFlipped,   /*!< Flipped culling. */
    cullDisabled   /*!< Culling is disabled. */
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
   * Alpha test function.
   * The alpha test function specifies how the source pixel alpha value 
   * (left side of comparison) and alpha test threshold value (right side of
   * comparison) are compared. If the comparison is \c true, the pixel is
   * drawn; otherwise, it is discarded.
   */
  enum AlphaTestFunction
  {
    /**
     * Draw pixel if source alpha value is greater or equal than the threshold
     * value.
     */
    atfGreaterEqual,
    /**
     * Draw pixel if source alpha value is greater than the threshold
     * value.
     */
    atfGreater,
    /**
     * Draw pixel if source alpha value is lower or equal than the threshold
     * value.
     */
    atfLowerEqual,
    /**
     * Draw pixel if source alpha value is lower than the threshold
     * value.
     */
    atfLower
  };
  
  /**
   * Alpha test options.
   * These options take effect if:
   * - The mixmode alpha test type (bits #CS_MIXMODE_ALPHATEST_MASK)
   *   is #CS_MIXMODE_ALPHATEST_ENABLE.
   * - The alpha test type (bits #CS_MIXMODE_ALPHATEST_MASK)
   *   is #CS_MIXMODE_ALPHATEST_AUTO and the alpha type used is 
   *   csAlphaMode::alphaBinary.
   */
  struct AlphaTestOptions
  {
    /**
     * Threshold value the source pixel alpha is compared against.
     * Default is 0.5
     */
    float threshold;
    /**
     * Comparison between source pixel alpha and threshold value.
     * Default atfGreaterEqual
     */
    AlphaTestFunction func;
    
    AlphaTestOptions() : threshold (0.5f), func (atfGreaterEqual) {}
  };

  /**
   * Mesh render mode information. Contains the Z, mix and alpha modes to use
   * for rendering a mesh. 
   * \remarks Is separate from CS::Graphics::CoreRenderMesh to allow preprocessing steps 
   *  to modify the mode data. 
   */
  struct RenderMeshModes
  {
    RenderMeshModes () : z_buf_mode ((csZBufMode)~0), mixmode (CS_FX_COPY),
      alphaToCoverage (false), atcMixmode (CS_MIXMODE_BLEND (ONE, ZERO)),
      cullMode (cullNormal),
      alphaType (csAlphaMode::alphaNone), zoffset (false), doInstancing (false),
      instParams (nullptr), instParamBuffers (nullptr)
    {
    }

    RenderMeshModes (RenderMeshModes const& x) :
      z_buf_mode (x.z_buf_mode),
      mixmode (x.mixmode),
      alphaToCoverage (x.alphaToCoverage),
      atcMixmode (x.atcMixmode),
      renderPrio (x.renderPrio),
      cullMode (x.cullMode),
      alphaType (x.alphaType),
      alphaTest (x.alphaTest),
      zoffset (x.zoffset),
      buffers (x.buffers),
      doInstancing (x.doInstancing),
      instParamNum (x.instParamNum),
      instParamsTargets (x.instParamsTargets),
      instanceNum (x.instanceNum),
      instParams (x.instParams),
      instParamBuffers (x.instParamBuffers)
    {
    }

    ~RenderMeshModes () { }

    /// Z mode to use
    csZBufMode z_buf_mode;

    /// mixmode to use
    uint mixmode;
    
    /**
     * Whether to enable alpha to coverage.
     * Note that alpha to coverage requires enabled multisampling
     * If that is the case alpha to coverage is
     * enabled and the mixmode from \c atcMixmode is used.
     * Otherwise, the normal mixmode is used.
     */
    bool alphaToCoverage;
    /// Mixmode to use together with alpha to coverage
    uint atcMixmode;
    
    /// Mesh render priority
    RenderPriority renderPrio;

    /// Mesh culling mode
    MeshCullMode cullMode;

    /**
     * Alpha mode this mesh is drawn.
     * 
     * - If the mixmode mode type (bits #CS_MIXMODE_TYPE_MASK) is
     *   #CS_MIXMODE_TYPE_AUTO, the alpha type affects blending.
     * - If the mixmode alpha test type (bits #CS_MIXMODE_ALPHATEST_MASK)
     *   is #CS_MIXMODE_ALPHATEST_AUTO, the alpha type affects whether alpha
     *   test is used or not.
     */
    csAlphaMode::AlphaType alphaType;
    
    /// Alpha test setting (take effect when mixmode enables alpha test)
    AlphaTestOptions alphaTest;
    
    /// Whether Z value offsetting should be enabled.
    bool zoffset;

    /// Holder of default render buffers
    csRef<csRenderBufferHolder> buffers;

    /// Whether to enable instancing.
    bool doInstancing; 
    /// Number of instance parameters.
    size_t instParamNum; 
    /// Targets of instance parameters.
    const csVertexAttrib* instParamsTargets; 
    /// Number of instances.
    size_t instanceNum;
    /**
     * Instance parameters, as shader variables.
     * The "instance" array (elements of type csShaderVariable**) has one
     * "parameter" array for each instance. The parameter array (elements of
     * type csShaderVariable*) has one shader variable for each instance
     * parameter.
     */
    csShaderVariable** const * instParams;
    /**
     * Instance parameters, as shader variables.
     * Each element in the array is a render buffer with the values
     * for an instance parameter; there must be as many render buffers
     * as parameters.
     *
     * The instance data can be given in both the instParams and
     * instParamsBuffers array; in that case, the render buffer takes
     * precedence. Only when a buffer is null the shader variable data
     * is taken.
     */
    iRenderBuffer** instParamBuffers;
  };

  /**
  * Start and end for a range of indices to render.
  * The indices are used in the range from \a start (inclusive) to \a end
  * (exclusive): start <= n < end
  */
  struct RenderMeshIndexRange
  {
    /// Start index.
    unsigned int start;
    /// End index.
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
