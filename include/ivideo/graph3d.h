/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Written by Jorrit Tyberghein, Dan Ogles, and Gary Clark.

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

#ifndef __CS_IVIDEO_GRAPH3D_H__
#define __CS_IVIDEO_GRAPH3D_H__

/**\file
 * 3D graphics interface
 */

/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csgeom/vector2.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "csutil/cscolor.h"
#include "ivideo/rndbuf.h"
#include "ivideo/polyrender.h"

struct csCoreRenderMesh;
struct csRenderMeshModes;
class csMatrix3;
class csVector4;
class csVector3;
class csVector2;
class csPlane3;
class csRect;
class csReversibleTransform;
class csShaderVariable;

struct iGraphics2D;
struct iPolygonBuffer;
struct iPolygonRenderer;
struct iVertexBuffer;
struct iVertexBufferManager;
struct iTextureManager;
struct iTextureHandle;
struct iMaterialHandle;
struct iMaterial;
struct iClipper2D;
struct iHalo;
struct iRendererLightmap;
struct csRGBpixel;
struct csPixelFormat;
struct csPolyTextureMapping;
struct iRenderBuffer;
struct iRenderBufferManager;
struct iLightingManager;
struct iShader;
struct iShaderVariableContext;


/**\name iGraphics3D::BeginDraw() flags
 * @{ */
/// We're going to draw 2D graphics
#define CSDRAW_2DGRAPHICS   0x00000001
/// We're going to draw 3D graphics
#define CSDRAW_3DGRAPHICS   0x00000002
/// Clear Z-buffer ?
#define CSDRAW_CLEARZBUFFER 0x00000010
/// Clear frame buffer ?
#define CSDRAW_CLEARSCREEN  0x00000020
/** @} */

/**\name Type of clipper (for iGraphics3D::SetClipper())
 * @{ */
/**
 * There is no clipper.
 */
#define CS_CLIPPER_NONE -1
/**
 * Clipper is optional.
 */
#define CS_CLIPPER_OPTIONAL 0
/**
 * Clipper is top-level.
 */
#define CS_CLIPPER_TOPLEVEL 1
/**
 * Clipper is required.
 */
#define CS_CLIPPER_REQUIRED 2
/** @} */

/**\name Clipping requirement for DrawTriangleMesh
 * @{ */
/**
 * No clipping required.
 * (setting for clip_portal, clip_plane, or clip_z_plane).
 */
#define CS_CLIP_NOT 0
/**
 * Clipping may be needed. Depending on the type of the clipper
 * (one of the CS_CLIPPER_??? flags) the renderer has to clip or
 * not. (setting for clip_portal, clip_plane, or clip_z_plane).
 */
#define CS_CLIP_NEEDED 1
/** @} */

/**
 * Fog structure.
 */
struct csFog
{
  /// If true then fog is enabled.
  bool enabled;
  /// Density (0 is off).
  float density;
  /// Color (red).
  float red;
  /// Color (green).
  float green;
  /// Color (blue).
  float blue;
};

/// Z-buffer modes
enum csZBufMode
{
  // values below are sometimes used as bit masks, so don't change them!
  /// Don't test/write
  CS_ZBUF_NONE     = 0x00000000,
  /// write
  CS_ZBUF_FILL     = 0x00000001,
  /// test
  CS_ZBUF_TEST     = 0x00000002,
  /// write/test
  CS_ZBUF_USE      = 0x00000003,
  /// only write
  CS_ZBUF_FILLONLY = 0x00000004,
  /// test if equal
  CS_ZBUF_EQUAL    = 0x00000005,
  /// inverted test
  CS_ZBUF_INVERT   = 0x00000006,
  /// use the z mode of the mesh (NOTE: NOT VALID AS MESH ZMODE)
  CS_ZBUF_MESH     = 0x00000007,
  /**
   * Use a z mode depending on the mesh zmode.
   * The mesh zmode is used a to choose a zmode that makes sure only pixels
   * that are changed by the mesh zmode can be touched, e.g. if the mesh has a
   * zmode of "zuse", zmesh2 will resolve to "ztest". This is useful for multi-
   * pass stuff.
   * (NOTE: NOT VALID AS MESH ZMODE)
   */
  CS_ZBUF_MESH2    = 0x00000008
};

// @@@ Keep in sync with values below
// @@@ Document me better!
#define CS_VATTRIB_GENERIC_FIRST     0
#define CS_VATTRIB_GENERIC_LAST     15
#define CS_VATTRIB_SPECIFIC_FIRST  100
#define CS_VATTRIB_SPECIFIC_LAST   (CS_VATTRIB_SPECIFIC_FIRST + 15)

#define CS_VATTRIB_IS_GENERIC(va)   \
  ((va >= CS_VATTRIB_GENERIC_FIRST) && (va <=CS_VATTRIB_GENERIC_LAST))
#define CS_VATTRIB_IS_SPECIFIC(va)   \
  ((va >= CS_VATTRIB_SPECIFIC_FIRST) && (va <=CS_VATTRIB_SPECIFIC_LAST))

/**
 * For NR:
 * Vertex attributes.
 */
enum csVertexAttrib
{
  //@{
  /**
   * General vertex attribute
   */
  CS_VATTRIB_0	= 0,
  CS_VATTRIB_1	= 1,
  CS_VATTRIB_2	= 2,
  CS_VATTRIB_3	= 3,
  CS_VATTRIB_4	= 4,
  CS_VATTRIB_5	= 5,
  CS_VATTRIB_6	= 6,
  CS_VATTRIB_7	= 7,
  CS_VATTRIB_8	= 8,
  CS_VATTRIB_9	= 9,
  CS_VATTRIB_10 = 10,
  CS_VATTRIB_11 = 11,
  CS_VATTRIB_12 = 12,
  CS_VATTRIB_13 = 13,
  CS_VATTRIB_14 = 14,
  CS_VATTRIB_15 = 15,
  //@}
  /// Position vertex attribute
  CS_VATTRIB_POSITION	      = CS_VATTRIB_SPECIFIC_FIRST + 0,
  /// Vertex weight attribute
  CS_VATTRIB_WEIGHT	      = CS_VATTRIB_SPECIFIC_FIRST + 1,
  /// Normal attribute
  CS_VATTRIB_NORMAL	      = CS_VATTRIB_SPECIFIC_FIRST + 2,
  /// Primary color attribute
  CS_VATTRIB_COLOR	      = CS_VATTRIB_SPECIFIC_FIRST + 3,
  /// Primary color attribute
  CS_VATTRIB_PRIMARY_COLOR    = CS_VATTRIB_SPECIFIC_FIRST + 3,
  /// Secondary color attribute
  CS_VATTRIB_SECONDARY_COLOR  = CS_VATTRIB_SPECIFIC_FIRST + 4,
  /// Fog coordinate attribute
  CS_VATTRIB_FOGCOORD	      = CS_VATTRIB_SPECIFIC_FIRST + 5,
  /// TU 0 texture coordinates
  CS_VATTRIB_TEXCOORD	      = CS_VATTRIB_SPECIFIC_FIRST + 8,
  /// TU 0 texture coordinates
  CS_VATTRIB_TEXCOORD0	      = CS_VATTRIB_SPECIFIC_FIRST + 8,
  /// TU 1 texture coordinates
  CS_VATTRIB_TEXCOORD1	      = CS_VATTRIB_SPECIFIC_FIRST + 9,
  /// TU 2 texture coordinates
  CS_VATTRIB_TEXCOORD2	      = CS_VATTRIB_SPECIFIC_FIRST + 10,
  /// TU 3 texture coordinates
  CS_VATTRIB_TEXCOORD3	      = CS_VATTRIB_SPECIFIC_FIRST + 11,
  /// TU 4 texture coordinates
  CS_VATTRIB_TEXCOORD4	      = CS_VATTRIB_SPECIFIC_FIRST + 12,
  /// TU 5 texture coordinates
  CS_VATTRIB_TEXCOORD5	      = CS_VATTRIB_SPECIFIC_FIRST + 13,
  /// TU 6 texture coordinates
  CS_VATTRIB_TEXCOORD6	      = CS_VATTRIB_SPECIFIC_FIRST + 14,
  /// TU 7 texture coordinates
  CS_VATTRIB_TEXCOORD7	      = CS_VATTRIB_SPECIFIC_FIRST + 15
};

/// 
enum G3D_FOGMETHOD
{
  G3DFOGMETHOD_NONE = 0x00,
  G3DFOGMETHOD_ZBUFFER = 0x01,
  G3DFOGMETHOD_VERTEX = 0x02
};

/**\name Mix modes for DrawPolygonFX ()
 * The constants can be ORed together if they belong to different masks.
 * @{ */
/// SRC/DST mixing mode mask
#define CS_FX_MASK_MIXMODE 0xF0000000 
/// =SRC
#define CS_FX_COPY         0x00000000 
/// =SRC*DST
#define CS_FX_MULTIPLY     0x10000000 
/// =2*SRC*DST
#define CS_FX_MULTIPLY2    0x20000000 
/// =SRC+DST
#define CS_FX_ADD          0x30000000 
/// =(1-alpha)*SRC + alpha*DST
#define CS_FX_ALPHA        0x40000000 
/// =DST
#define CS_FX_TRANSPARENT  0x50000000 
/// =(dstalpha)*SRC + DST
#define CS_FX_DESTALPHAADD 0x60000000 
/// =(srcalpha)*SRC + DST
#define CS_FX_SRCALPHAADD  0x70000000
/// =SRC + DST*(1-srcalpha)
#define CS_FX_PREMULTALPHA 0x80000000
/**
  * Use the mix mode of the mesh zmode.
  * (NOTE: NOT VALID AS MESH ZMODE - only for shader pass mixmodes)
  */
#define CS_FX_MESH	   0xf0000000
/// color 0 is transparent
#define CS_FX_KEYCOLOR     0x08000000 
/// Flat shading
#define CS_FX_FLAT         0x04000000 
/// Tiling
#define CS_FX_TILING       0x02000000 
/// alpha = 0..FF (opaque..transparent)
#define CS_FX_MASK_ALPHA   0x000000FF 

/// Macro for setting of alpha bits into mixmode (alpha between 0 and 1).
#define CS_FX_SETALPHA(alpha) \
  (CS_FX_ALPHA | uint (alpha * CS_FX_MASK_ALPHA))
/// Macro for setting of alpha bits into mixmode (alpha between 0 and 255).
#define CS_FX_SETALPHA_INT(alpha) \
  (CS_FX_ALPHA | uint (alpha & CS_FX_MASK_ALPHA))
/** @} */

/**
 * Describes how to deal with alpha values in textures.
 */
struct csAlphaMode
{
  /// How to handle alpha
  enum AlphaType
  {
    /// Ignore alpha
    alphaNone = 1,
    /// Binary alpha (pixels with alpha >0.5 are drawn, all others not)
    alphaBinary,
    /// 'Smooth' alpha (colors are mixed based on a pixel's alpha value)
    alphaSmooth
  };
  /// Whether 'automatic alpha mode' should be used.
  bool autoAlphaMode;
  union
  {
    /// Alpha mode to use when autoAlphaMode is \p false
    AlphaType alphaType;
    /// Texture to retrieve the alpha mode from when autoAlphaMode is \p true
    csStringID autoModeTexture;
  };
};

/**\name Light parameters
 * @{ */
/// Position of the light.
#define CS_LIGHTPARAM_POSITION 0
/// Diffuse color of the light.
#define CS_LIGHTPARAM_DIFFUSE 1
/// Specular color of the light.
#define CS_LIGHTPARAM_SPECULAR 2
/// Attenuation of the light.
#define CS_LIGHTPARAM_ATTENUATION 3
/** @} */

/**\name Shadow states
 * @{ */
/// Clear stencil.
#define CS_SHADOW_VOLUME_BEGIN 1
/// Setup for pass 1.
#define CS_SHADOW_VOLUME_PASS1 2
/// Setup for pass 2.
#define CS_SHADOW_VOLUME_PASS2 3
/// Setup for carmack's reverse pass 1.
#define CS_SHADOW_VOLUME_FAIL1 4
/// Setup for carmack's reverse pass 2.
#define CS_SHADOW_VOLUME_FAIL2 5
/// Setup for shadow masking.
#define CS_SHADOW_VOLUME_USE 6
/// Restore states.
#define CS_SHADOW_VOLUME_FINISH 7
/** @} */

/// Graphics3D render state options
enum G3D_RENDERSTATEOPTION
{
  /// Set Z-buffer fill/test/use mode (parameter is a csZBufMode)
  G3DRENDERSTATE_ZBUFFERMODE,
  /// Enable/disable dithering (parameter is a bool)
  G3DRENDERSTATE_DITHERENABLE,
  /// Enable/disable bi-linear mapping (parameter is a bool)
  G3DRENDERSTATE_BILINEARMAPPINGENABLE,
  /// Enable/disable tri-linear mapping (parameter is a bool)
  G3DRENDERSTATE_TRILINEARMAPPINGENABLE,
  /// Enable/disable transparent textures (parameter is a bool)
  G3DRENDERSTATE_TRANSPARENCYENABLE,
  /// Enable/disable mip-mapping (parameter is a bool)
  G3DRENDERSTATE_MIPMAPENABLE,
  /// Enable/disable textures (parameter is a bool)
  G3DRENDERSTATE_TEXTUREMAPPINGENABLE,
  /// Enable/disable lighting (parameter is a bool)
  G3DRENDERSTATE_LIGHTINGENABLE,
  /// Enable/disable interlacing (parameter is a bool)
  G3DRENDERSTATE_INTERLACINGENABLE,
  /// Enable/disable MMX instructions usage (parameter is a bool)
  G3DRENDERSTATE_MMXENABLE,
  /// Set perspective-correction interpolation step (parameter is a int)
  G3DRENDERSTATE_INTERPOLATIONSTEP,
  /// Set maximal number of polygons per frame to draw (parameter is a int)
  G3DRENDERSTATE_MAXPOLYGONSTODRAW,
  /// Enable/disable Gouraud shading (parameter is a bool)
  G3DRENDERSTATE_GOURAUDENABLE,
  /// Enable/disable edge drawing (debugging) (parameter is a bool)
  G3DRENDERSTATE_EDGES
};

/// Information about 3d renderer capabilities.
struct csGraphics3DCaps
{
  bool CanClip;
  int minTexHeight, minTexWidth;
  int maxTexHeight, maxTexWidth;
  G3D_FOGMETHOD fog;
  bool NeedsPO2Maps;
  int MaxAspectRatio;

  // The following caps are only used by NR

  /**
   * Whether point sprites are supported. If \a true, geometry of the type
   * CS_MESHTYPE_POINT_SPRITES can be drawn.
   */
  bool SupportsPointSprites;
  /**
   * Mixmodes utilizing destination alpha are properly supported.
   */
  bool DestinationAlpha;
  /**
   * Enough stencil bits for stencil shadows are available.
   */
  bool StencilShadows;
};

//===========================================================================
// Not for new renderer! @@@NR@@@
// The stuff below is either to be ported to new renderer or else
// it will be removed.
//===========================================================================

#define CS_FOG_FRONT  0
#define CS_FOG_BACK   1
#define CS_FOG_VIEW   2

//======================================================================
// For vertex based fog the following defines are used:
#define CS_FOGTABLE_SIZE 64

// Each texel in the fog table holds the fog alpha value at a certain
// (distance*density).  The median distance parameter determines the
// (distance*density) value represented by the texel at the center of
// the fog table.  The fog calculation is:
// alpha = 1.0 - exp( -(density*distance) / CS_FOGTABLE_MEDIANDISTANCE)
#define CS_FOGTABLE_MEDIANDISTANCE 10.0f
#define CS_FOGTABLE_MAXDISTANCE (CS_FOGTABLE_MEDIANDISTANCE * 2.0f)
#define CS_FOGTABLE_DISTANCESCALE (1.0f / CS_FOGTABLE_MAXDISTANCE)

// Fog (distance*density) is mapped to a texture coordinate and then
// clamped.  This determines the clamp value.  Some drivers don't
// like clamping textures so we must do it ourself
#define CS_FOGTABLE_CLAMPVALUE 0.85f
#define CS_FOG_MAXVALUE (CS_FOGTABLE_MAXDISTANCE * CS_FOGTABLE_CLAMPVALUE)
//======================================================================

/// Extra information for vertex fogging.
class G3DFogInfo
{
public:
  /// Color.
  float r, g, b;
  /**
   * Intensity (== density * thickness).
   * The second intensity value is always 0 and is put there
   * to make it easier for 3D implementations to just use the
   * two values below as a coordinate in a texture of 64*1.
   */
  float intensity;
  float intensity2;
};

/// Information about a texture plane.
class G3DCam2TextureTransform
{
public:
  /// Transformation from camera space to texture space.
  csMatrix3* m_cam2tex;
  /// Transformation from camera space to texture space.
  csVector3* v_cam2tex;
};

/// Structure containing all info needed by DrawPolygonFX (DPFX)
struct G3DPolygonDPFX
{
  /// Current number of vertices.
  int num;
  /// Vertices that form the polygon.
  csVector2 vertices[100];
  /// 1/z for every vertex.
  float z[100];
  /// Texels per vertex.
  csVector2 texels[100];
  /// Lighting info per vertex.
  csColor colors[100];

  /// Extra optional fog information.
  G3DFogInfo fog_info[100];
  /// Use fog info?
  bool use_fog;

  /// The material handle as returned by iTextureManager.
  iMaterialHandle *mat_handle;
  /// Mixmode to use. If CS_FX_COPY then no mixmode is used.
  uint mixmode;

  //@{
  /// Use this color for drawing (if txt_handle == 0) instead of a material.
  uint8 flat_color_r;
  uint8 flat_color_g;
  uint8 flat_color_b;
  //@}

  // A dummy constructor to appease NextStep compiler which otherwise
  // complains that it is unable to create this object.  This happens when
  // a subcomponent such as csVector2 has a constructor.
  G3DPolygonDPFX() {}
};

/// Structure containing all info needed by DrawFogPolygon (DFP)
struct G3DPolygonDFP
{
  /// Current number of vertices.
  int num;
  /// Vertices that form the polygon.
  csVector2 vertices[100];

  /// The plane equation in camera space of this polygon.
  csPlane3 normal;
};

/// Structure containing all info needed by DrawPolygon (DP)
struct G3DPolygonDP : public G3DPolygonDFP
{
  /// Extra optional fog information.
  G3DFogInfo fog_info[100];
  /// Use fog info?
  bool use_fog;

  /// The material handle as returned by iTextureManager.
  iMaterialHandle* mat_handle;

  /// Transformation matrices for the texture. @@@ BAD NAME
  G3DCam2TextureTransform cam2tex;

  /// Handle to lighted textures (texture + lightmap)
  csPolyTextureMapping* texmap;
  iRendererLightmap* rlm;
  
  /// Draw fullbright?
  bool do_fullbright;

  /// Mixmode to use. If CS_FX_COPY then no mixmode is used.
  uint mixmode;

  /// Z value (in camera space) of vertex[0].
  float z_value;

  iTextureHandle* txt_handle;
};

/// Structure containing all info needed by DrawPolygonFlat (DPF)
typedef G3DPolygonDP G3DPolygonDPF;

/**
 * Structure containing all info needed by DrawTriangeMesh.
 * This function is capable of:<br>
 * <ul>
 * <li>Object2camera transformation and perspective.
 * <li>Linear interpolation between two sets of vertices.
 * <li>Clipping.
 * <li>Whatever else DrawPolygonFX can do.
 * </ul>
 * To disable the use of one of the components, set it to 0.
 */
struct G3DTriangleMesh
{
  enum
  {
    /// Maximum number of vertex pool, used for vertex weighting/morphing.
    MAX_VERTEXPOOL = 2
  };

  /// Number of vertex sets, if > 1, morphing will be applied.
  int num_vertices_pool;

  /// Number of triangles.
  int num_triangles;
  /// Pointer to array of triangles.
  csTriangle* triangles;

  /// Clip to portal? One of CS_CLIP_???.
  int clip_portal;
  /// Clip to near plane? One of CS_CLIP_???.
  int clip_plane;
  /// Clip to z plane? One of CS_CLIP_???.
  int clip_z_plane;

  /// Use precalculated vertex color?
  bool use_vertex_color;

  /// Apply fogging?
  bool do_fog;
  /// Consider triangle vertices in anti-clockwise order if true.
  bool do_mirror;
  /// If morphing is applied then morph texels too if true.
  bool do_morph_texels;
  /// If morphing is applied then morph vertex colors too if true.
  bool do_morph_colors;

  /// Types of vertices supplied.
  enum VertexMode
  {
    /// Must apply transformation and perspective.
    VM_WORLDSPACE,
    /// Must apply perspective.
    VM_VIEWSPACE
  };
  
  /// Type of vertices supplied.
  VertexMode vertex_mode;

  /// DrawPolygonFX flag.
  uint mixmode;
  float morph_factor;
  /**
   * Vertex buffers. Note that all vertex buffers used here MUST
   * have the same number of vertices.
   */
  iVertexBuffer* buffers[MAX_VERTEXPOOL];
  iMaterialHandle* mat_handle;
  /// Information for fogging the vertices.
  G3DFogInfo* vertex_fog;

  // TODO : store information required for lighting calculation
};

/**
 * Structure containing all info needed by DrawPolygonMesh.
 * In theory this function is capable of:<br>
 * <ul>
 * <li>Object2camera transformation and perspective.
 * <li>Clipping.
 * <li>Whatever else DrawPolygon can do.
 * </ul>
 * To disable the use of one of the components, set it to 0.
 */
struct G3DPolygonMesh
{
  /// Polygon buffer.
  iPolygonBuffer* polybuf;

  // Apply fogging?
  bool do_fog;

  /// Mixmode.
  uint mixmode;

  /// Clip to portal? One of CS_CLIP_???.
  int clip_portal;
  /// Clip to near plane? One of CS_CLIP_???.
  int clip_plane;
  /// Clip to z plane? One of CS_CLIP_???.
  int clip_z_plane;

  /// Consider polygon vertices in anti-clockwise order if true.
  bool do_mirror;

  /// Types of vertices supplied.
  enum VertexMode
  {
    /// Must apply transformation and perspective.
    VM_WORLDSPACE,
    /// Must apply perspective.
    VM_VIEWSPACE
  };
  
  /// Type of vertices supplied.
  VertexMode vertex_mode;

  /// Information for fogging the vertices.
  G3DFogInfo* vertex_fog;
};

/// Primitive type of a mesh
enum csRenderMeshType
{
  /// Triangles.
  CS_MESHTYPE_TRIANGLES,
  /// Quads.
  CS_MESHTYPE_QUADS,
  /**
   * Triangle strip.
   * The OpenGL spec describes it pretty well:
   * "A triangle strip is a series of triangles connected along shared edges. 
   * A triangle strip is specified by giving a series of defining vertices 
   * [...]. In this case, the first three vertices define the first triangle 
   * [...]. Each subsequent  vertex defines a new triangle using that point 
   * along with two vertices from the previous triangle."
   */
  CS_MESHTYPE_TRIANGLESTRIP,
  /**
   * Triangle fan.
   * Similar to a triangle strip, however, a triangle is always defined with
   * the first, previously added and the recently added vertex.
   */
  CS_MESHTYPE_TRIANGLEFAN,
  /**
   * Points.
   */
  CS_MESHTYPE_POINTS,
  /**
   * Point sprites. 
   * Note: only supported if the \a SupportsPointSprites member of the 
   * \a csGraphics3DCaps structure for this renderer is true.
   */
  CS_MESHTYPE_POINT_SPRITES,
  /**
   * Lines.
   */
  CS_MESHTYPE_LINES,
  /**
   * Line strip.
   * A line is defined from the prebviously and recently added vertex.
   */
  CS_MESHTYPE_LINESTRIP,
  /**
   * Render polygons. Note that you <b>*must*</b> supply geometry with the
   * help of an iPolygonRenderer. In the common case, if you want to draw a
   * polygon, you probably want to use CS_MESHTYPE_TRIANGLES or 
   * CS_MESHTYPE_TRIANGLEFAN and triangulate the poly yourself. 
   */
  CS_MESHTYPE_POLYGON
};

/**
 * Flags to influence the behaviour of DrawSimpleMesh().
 */
enum csSimpleMeshFlags
{
  /**
   * Ignore the object2camera transform in the csSimpleRenderMesh struct and
   * replace it with a transformation that effectively lets you specify the
   * vertices in screen space.
   */
  csSimpleMeshScreenspace = 0x01
};

/**
 * A simple render mesh.
 */
struct csSimpleRenderMesh
{
  /// Type of the geometry to draw.
  csRenderMeshType meshtype;

  /// Number of vertex indices
  uint indexCount;
  /// Vertex indices
  const uint* indices;

  /// Number of vertices
  uint vertexCount;
  /**
   * Vertices. Note: you can omit vertices or texcoords, however this 
   * will likely only give useable results if you provide a shader and 
   * shader var context (and transfer vertices and/or texcoords with SVs.)
   */
  const csVector3* vertices;
  /// (Optional) Texture coordinates.
  const csVector2* texcoords;
  /**
   * (Optional) Colors. 
   * Leaving this 0 has the same effect as having all vertex colors set to
   * white.
   */
  const csVector4* colors;
  /**
   * (Optional) Handle to the texture to select. 
   * Leaving this 0 has the same effect as using a white texture.
   */
  iTextureHandle* texture;

  /// (Optional) Shader to use.
  iShader* shader;
  /// (Optional) Shader variable context.
  iShaderVariableContext* dynDomain;
  /// (Optional) Alpha mode. Defaults to "autodetect".
  csAlphaMode alphaType;
  /// (Optional) Z buffer mode. Defaults to CS_ZBUF_NONE.
  csZBufMode z_buf_mode;
  /// (Optional) Mix mode. Defaults to CS_FX_COPY.
  uint mixmode;
  /**
   * (Optional) Transform to apply to the mesh.
   * \remark This transform is initialized to an identity transform.
   *  This effectively means that geometry is drawn in eye space.
   *  To draw in screen space, supply the \a csSimpleMeshScreenspace
   *  flag to DrawSimpleMesh(). For anything else supply an appropriate
   *  transformation.
   */
  csReversibleTransform object2camera;

  csSimpleRenderMesh () : colors(0), texture (0), shader (0), dynDomain (0), 
    z_buf_mode (CS_ZBUF_NONE), mixmode (CS_FX_COPY)
  {  
    alphaType.autoAlphaMode = true;
    alphaType.autoModeTexture = csInvalidStringID;
  };
};

SCF_VERSION (iGraphics3D, 5, 5, 1);

/**
 * This is the standard 3D graphics interface.
 * All 3D graphics rasterizer servers for Crystal Space should implement this
 * interface, as well as the iGraphics2D interface.  The standard
 * implementation is csGraphics3DSoftware.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>OpenGL Renderer plugin (crystalspace.graphics3d.opengl)
 *   <li>Software Renderer plugin (crystalspace.graphics3d.software)
 *   <li>Null 3D Renderer plugin (crystalspace.graphics3d.null)
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 */
struct iGraphics3D : public iBase
{
  /// Open the 3D graphics display.
  virtual bool Open () = 0;
  /// Close the 3D graphics display.
  virtual void Close () = 0;

  /**
   * Retrieve the associated canvas.
   * \remarks This will return a valid canvas only after
   *   csInitializer::OpenApplication() has been invoked (and if the canvas
   *   plugin loaded and initialized successfully); otherwise it will return
   *   null.
   */
  virtual iGraphics2D *GetDriver2D () = 0;

  /**
   * Retrieve the texture manager.
   * \remarks This will return a valid texture manager only after
   *   csInitializer::OpenApplication() has been invoked; otherwise it will
   *   return null.
   */
  virtual iTextureManager *GetTextureManager () = 0;

  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height) = 0;
  /// Get drawing buffer width.
  virtual int GetWidth () const = 0;
  /// Get drawing buffer height.
  virtual int GetHeight () const = 0;

  /**
   * Get the current driver's capabilities. Each driver implements their
   * own function.
   */
  virtual const csGraphics3DCaps *GetCaps () const = 0;

  /**
   * Set center of projection for perspective projection.
   * Center is set in screen space coordinates.
   */
  virtual void SetPerspectiveCenter (int x, int y) = 0;

  /// Get perspective center.
  virtual void GetPerspectiveCenter (int& x, int& y) const = 0;

  /**
   * Set aspect ratio for perspective projection.
   */
  virtual void SetPerspectiveAspect (float aspect) = 0;

  /// Get aspect ratio.
  virtual float GetPerspectiveAspect () const = 0;

  /**
   * Set object to camera transformation (currently only used by
   * DrawTriangleMesh and DrawPolygonMesh).
   */
  virtual void SetObjectToCamera (csReversibleTransform* o2c) = 0;

  /**
   * Get object to camera transformation.
   */
  virtual const csReversibleTransform& GetObjectToCamera () = 0;
  
  /**
   * Set the target of rendering. If this is 0 then the target will
   * be the main screen. Otherwise it is a texture. After calling
   * g3d->FinishDraw() the target will automatically be reset to 0 (main
   * screen). Note that on some implementions rendering on a texture
   * will overwrite the screen. So you should only do this BEFORE you
   * start rendering your frame.
   * <p>
   * If 'persistent' is true then the current contents of the texture
   * will be copied on screen before drawing occurs (in the first
   * call to BeginDraw). Otherwise it is assumed that you fully render
   * the texture.
   */
  virtual void SetRenderTarget (iTextureHandle* handle,
  	bool persistent = false) = 0;

  /**
   * Get the current render target (0 for screen).
   */
  virtual iTextureHandle* GetRenderTarget () const = 0;

  /// Start a new frame (see CSDRAW_XXX bit flags)
  virtual bool BeginDraw (int DrawFlags) = 0;

  /// End the frame and do a page swap.
  virtual void FinishDraw () = 0;

  /**
   * Print the image in backbuffer. The area parameter is only a hint to the
   * renderer. Changes outside the rectangle may or may not be printed as
   * well.
   */
  virtual void Print (csRect const* area) = 0;

  /**
   * Draw a pixmap using a rectangle from given texture.
   * The sx,sy(sw,sh) rectangle defines the screen rectangle within
   * which the drawing is performed (clipping rectangle is also taken
   * into account). The tx,ty(tw,th) rectangle defines a subrectangle
   * from texture which should be painted. If the subrectangle exceeds
   * the actual texture size, texture coordinates are wrapped around
   * (e.g. the texture is tiled). The Alpha parameter defines the
   * transparency of the drawing operation, 0 means opaque, 255 means
   * fully transparent.<p>
   * <b>WARNING: Tiling works only with textures that have power-of-two
   * sizes!</b> That is, both width and height should be a power-of-two,
   * although not neccessarily equal.
   */
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha = 0) = 0;

  /**
   * Draw a line in camera space. Warning! This is a 2D operation
   * and must be called while in BeginDraw(CSDRAW_2DGRAPHICS)!
   */
  virtual void DrawLine (const csVector3& v1, const csVector3& v2,
    float fov, int color) = 0;

  /**
   * Set optional clipper to use. If clipper == null
   * then there is no clipper.
   * Currently only used by DrawTriangleMesh.
   */
  virtual void SetClipper (iClipper2D* clipper, int cliptype) = 0;

  /**
   * Get clipper that was used.
   */
  virtual iClipper2D* GetClipper () = 0;

  /**
   * Return type of clipper.
   */
  virtual int GetClipType () const = 0;

  /**
   * Set near clip plane.
   * Currently only used by DrawTriangleMesh.
   */
  virtual void SetNearPlane (const csPlane3& pl) = 0;

  /**
   * Reset near clip plane (i.e. disable it).
   */
  virtual void ResetNearPlane () = 0;

  /**
   * Get near clip plane.
   */
  virtual const csPlane3& GetNearPlane () const = 0;

  /**
   * Return true if we have a near plane.
   */
  virtual bool HasNearPlane () const = 0;

  /// Set a renderstate value.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val) = 0;

  /// Get a renderstate value.
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op) const = 0;

  /**
   * Set a renderer specific option. Returns false if renderer doesn't
   * support that option.
   */
  virtual bool SetOption (const char*, const char*) = 0;

  /**
   * Create a render buffer.
   * \param size Size of the buffer in bytes.
   * \param type Type of buffer; CS_BUF_DYNAMIC, CS_BUF_STATIC or 
   *  CS_BUF_STREAM.
   * \param componentType Components Types; CS_BUFCOMP_FLOAT, CS_BUFCOMP_INT,
   *        etc
   * \param componentCount Number of components per element (e.g. 4 for RGBA)
   * \param copy if true (default) then this buffer will make a copy of the
   *        data. Hardware vertex buffers and interleaved buffers will always
   *        copy data.
   */
  virtual csPtr<iRenderBuffer> CreateRenderBuffer (size_t size, 
    csRenderBufferType type, csRenderBufferComponentType componentType, 
    int componentCount, bool copy = true) = 0;
  /**
   * Create an index buffer.
   * \param size Size of the buffer in bytes.
   * \param type Type of buffer; CS_BUF_DYNAMIC, CS_BUF_STATIC or 
   *  CS_BUF_STREAM.
   * \param componentType Components Types; usually CS_BUFCOMP_UNSIGNED_INT
   * \param rangeStart Minimum index value that is expected to be written to 
   *  the created buffer.
   * \param rangeEnd Maximum index value that is expected to be written to 
   *  the created buffer.
   * \param copy if true (default) then this buffer will make a copy of the
   *        data. Hardware vertex buffers and interleaved buffers will always
   *        copy data.
   */
  virtual csPtr<iRenderBuffer> CreateIndexRenderBuffer (size_t size, 
    csRenderBufferType type, csRenderBufferComponentType componentType,
    size_t rangeStart, size_t rangeEnd, bool copy = true) = 0;

  /**
   * Create an interleaved renderbuffer (You would use this then set stride to
   * determine offset and stride of the interleaved buffer
   * \param size size of the buffer in bytes
   * \param count number of render buffers you want
   * \param buffers should be an array of render buffer references that can hold
   * at least 'count' render buffers.
   */
  virtual void CreateInterleavedRenderBuffers (size_t size, 
    csRenderBufferType type, int count, csRef<iRenderBuffer>* buffers) = 0;

  /**
   * Activate or deactivate all given buffers depending on the value of
   * 'buffers' for that index.
   */
  virtual void SetBufferState (csVertexAttrib* attribs,
  	iRenderBuffer** buffers, int count) = 0;

  /**
   * Activate or deactivate all given textures depending on the value
   * of 'textures' for that unit (i.e. deactivate if 0).
   */
  virtual void SetTextureState (int* units, iTextureHandle** textures,
  	int count) = 0;

  /// Drawroutine. Only way to draw stuff
  virtual void DrawMesh (const csCoreRenderMesh* mymesh,
    const csRenderMeshModes& modes,
    const csArray< csArray<csShaderVariable*> > &stacks) = 0;

  /// Set the masking of color and/or alpha values to framebuffer
  virtual void SetWriteMask (bool red, bool green, bool blue, bool alpha) = 0;

  /// Get the masking of color and/or alpha values to framebuffer
  virtual void GetWriteMask (bool &red, bool &green, bool &blue,
	bool &alpha) const = 0;

  /// Set the z buffer write/test mode
  virtual void SetZMode (csZBufMode mode) = 0;

  /// Enables offsetting of Z values
  virtual void EnableZOffset () = 0;

  /// Disables offsetting of Z values
  virtual void DisableZOffset () = 0;

  /// Controls shadow drawing
  virtual void SetShadowState (int state) = 0;

  //=========================================================================
  // Below this line are all functions that are not yet implemented by
  // the new renderer or are not going to be implemented ever. In the
  // last case they will be removed as soon as we permanently switch
  // to the new renderer. @@@NR@@@
  //=========================================================================
  /// Debugging only: get a pointer to Z-buffer at some location
  virtual uint32 *GetZBuffAt (int x, int y) = 0;

  /// Get Z-buffer value at given X,Y position
  virtual float GetZBuffValue (int x, int y) = 0;

  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly) = 0;

  /**
   * Draw the projected polygon with light and texture.
   * Debugging version. This one does not actually draw anything
   * but it just prints debug information about what it would have
   * done.
   */
  virtual void DrawPolygonDebug (G3DPolygonDP& poly) = 0;

  /**
   * Draw a polygon with special effects. This is the most rich and slowest
   * variant of DrawPolygonXxx. (If you use these features)
   *
   * Warning! Do not rely on this method to handle Color keying under
   * all circumstances. Color Keying will only work reliable in Mixmodes
   * FX_Copy, FX_Add and FX_Transparent. When using FX_Multiply
   * and FX_Multiply2, it depends very much on the driver if it works or
   * not. For example the RivaTNT Detonator 0.48 driver can display
   * Multiply with color keying, while newer versions can't. They will
   * then not display anything at all. It is always safer to use a texture
   * where transparent sections are white or 50% gray if you want to achieve
   * transparent sections in Multiply, Multiply2.
   * There are also some drivers (which I would consider buggy...), that won't
   * display FX_Alpha correctly with Color Keying. I can't provide a valid
   * workaround for that except using FX_Multiplay and FX_Add, to manually
   * create the image, but that would be very expensive.
   */
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly) = 0;

  /**
   * Draw a triangle mesh using features similar to DrawPolygonFX.
   */
  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh) = 0;

  /**
   * Draw a triangle mesh using features similar to DrawPolygon.
   */
  virtual void DrawPolygonMesh (G3DPolygonMesh& mesh) = 0;

  /**
   * Initiate a volumetric fog object. This function will be called
   * before front-facing and back-facing fog polygons are added to
   * the object. The fog object will be convex but not necesarily closed.
   * The given CS_ID can be used to identify multiple fog objects when
   * multiple objects are started.
   */
  virtual void OpenFogObject (CS_ID id, csFog* fog) = 0;

  /**
   * Add a front or back-facing fog polygon in the current fog object.
   * Note that it is guaranteed that all back-facing fog polygons
   * will have been added before the first front-facing polygon.
   * fogtype can be:<br>
   * <ul>
   *  <li>CS_FOG_FRONT: a front-facing polygon
   *  <li>CS_FOG_BACK:  a back-facing polygon
   *  <li>CS_FOG_VIEW:  the view-plane
   * </ul>
   */
  virtual void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype) = 0;

  /**
   * Close a volumetric fog object. After the volumetric object is
   * closed it should be rendered on screen (whether you do it here
   * or in DrawFogPolygon is not important).
   */
  virtual void CloseFogObject (CS_ID id) = 0;

  /**
   * Enter a new portal. If 'floating' is true then this routine will restrict
   * all further drawing to the given 2D area and it will also respect
   * the current contents of the Z-buffer so that geometry will only
   * render where the Z-buffer allows it (even if zfill or znone is used).
   * Remember to close a portal later using ClosePortal().
   * Basically this represents a stacked layer of portals. Each subsequent
   * portal must be fully contained in the previous ones.
   */
  virtual void OpenPortal (size_t numVertices, const csVector2* vertices,
    const csPlane3& normal, bool floating) = 0;

  /**
   * Close a portal previously opened with OpenPortal().
   * If 'zfill_portal' then the portal area will be zfilled.
   */
  virtual void ClosePortal (bool zfill_portal) = 0;

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char *iAlpha, int iWidth, int iHeight) = 0;

  /// Dump the texture cache.
  virtual void DumpCache () = 0;

  /// Clear the texture cache.
  virtual void ClearCache () = 0;

  /**
   * Remove some polygon from the cache.
   * You have to call this function before deleting a polygon
   * (csPolygon3D destructor will do that).
   */
  virtual void RemoveFromCache (iRendererLightmap* rlm) = 0;

  /**
   * Get the vertex buffer manager. This will not increment the ref count
   * of the vertex buffer manager!
   */
  virtual iVertexBufferManager* GetVertexBufferManager () = 0;
  
  //=========================================================================
  // Here ends the zone of unimplemented methods.
  //=========================================================================

  /**
   * Check if renderer can handle a lightmap.
   * Returns true if it can, false if not.
   */
  virtual bool IsLightmapOK (int lmw, int lmh, 
    int lightCellSize) = 0;
    
  virtual csPtr<iPolygonRenderer> CreatePolygonRenderer () = 0;

  /*
    @@@ Needed for SW poly drawing ATM.
   */
  virtual void SetWorldToCamera (csReversibleTransform* w2c) = 0;

  /**
   * Draw a csSimpleRenderMesh on the screen.
   * Simple render meshes are intended for cases where setting up
   * a render mesh and juggling with render buffers would be too much
   * effort - e.g. when you want to draw a single polygon on the screen.
   * <p>
   * DrawSimpleMesh () hides the complexity of csRenderMesh, it cares
   * about setting up render buffers, activating the texture etc.
   * Note that you can still provide shaders and shader variables, but those
   * are optional.
   */
  virtual void DrawSimpleMesh (const csSimpleRenderMesh& mesh,
    uint flags = 0) = 0;

  /// Get the z buffer write/test mode
  virtual csZBufMode GetZMode () = 0;
};

/** @} */

#endif // __CS_IVIDEO_GRAPH3D_H__

