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
 
#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csgeom/vector2.h"
#include "csgeom/tri.h"
#include "csutil/cscolor.h"
#include "ivideo/rndbuf.h"

class csMatrix3;
class csVector3;
class csVector2;
class csPlane3;
class csRect;
class csReversibleTransform;
class csRenderMesh;

struct iGraphics2D;
struct iPolygonTexture;
struct iPolygonBuffer;
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
struct csPolyLightMapMapping;
struct iRenderBuffer;
struct iRenderBufferManager;
struct iLightingManager;


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
/**
 * Clipping is not needed for the current clipper but it might
 * be needed for the toplevel clipper. (setting for clip_portal,
 * will never be used for clip_plane or clip_z_plane).
 */
#define CS_CLIP_TOPLEVEL 2
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
  CS_ZBUF_INVERT   = 0x00000006
};

// @@@ Keep in sync with values below
// @@@ Document me better!
#define CS_VATTRIB_GENERIC_FIRST     0
#define CS_VATTRIB_GENERIC_LAST     15
#define CS_VATTRIB_SPECIFIC_FIRST  100
#define CS_VATTRIB_SPECIFIC_LAST   111

#define CS_VATTRIB_IS_GENERIC(va)   \
  ((va >= CS_VATTRIB_GENERIC_FIRST) && (va <=CS_VATTRIB_GENERIC_LAST))
#define CS_VATTRIB_IS_SPECIFIC(va)   \
  ((va >= CS_VATTRIB_SPECIFIC_FIRST) && (va <=CS_VATTRIB_SPECIFIC_LAST))

/**
 * For NR:
 * Vertex attributes.
 * @@@ Document me better!
 */
enum csVertexAttrib
{
  CS_VATTRIB_0 = 0,
  CS_VATTRIB_1 = 1,
  CS_VATTRIB_2 = 2,
  CS_VATTRIB_3 = 3,
  CS_VATTRIB_4 = 4,
  CS_VATTRIB_5 = 5,
  CS_VATTRIB_6 = 6,
  CS_VATTRIB_7 = 7,
  CS_VATTRIB_8 = 8,
  CS_VATTRIB_9 = 9,
  CS_VATTRIB_10 = 10,
  CS_VATTRIB_11 = 11,
  CS_VATTRIB_12 = 12,
  CS_VATTRIB_13 = 13,
  CS_VATTRIB_14 = 14,
  CS_VATTRIB_15 = 15,
  CS_VATTRIB_POSITION = CS_VATTRIB_SPECIFIC_FIRST + 0,
  CS_VATTRIB_WEIGHT = CS_VATTRIB_SPECIFIC_FIRST + 1,
  CS_VATTRIB_NORMAL = CS_VATTRIB_SPECIFIC_FIRST + 2,
  CS_VATTRIB_COLOR = CS_VATTRIB_SPECIFIC_FIRST + 3,
  CS_VATTRIB_PRIMARY_COLOR = CS_VATTRIB_SPECIFIC_FIRST + 3,
  CS_VATTRIB_SECONDARY_COLOR = CS_VATTRIB_SPECIFIC_FIRST + 4,
  CS_VATTRIB_FOGCOORD = CS_VATTRIB_SPECIFIC_FIRST + 5,
  CS_VATTRIB_TEXCOORD = CS_VATTRIB_SPECIFIC_FIRST + 8,
  CS_VATTRIB_TEXCOORD0 = CS_VATTRIB_SPECIFIC_FIRST + 8,
  CS_VATTRIB_TEXCOORD1 = CS_VATTRIB_SPECIFIC_FIRST + 9,
  CS_VATTRIB_TEXCOORD2 = CS_VATTRIB_SPECIFIC_FIRST + 10,
  CS_VATTRIB_TEXCOORD3 = CS_VATTRIB_SPECIFIC_FIRST + 11
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
/// color 0 is transparent
#define CS_FX_KEYCOLOR     0x08000000 
/// Gouraud shading
#define CS_FX_GOURAUD      0x04000000 
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

  /// Use this color for drawing (if txt_handle == 0) instead of a material.
  uint8 flat_color_r;
  uint8 flat_color_g;
  uint8 flat_color_b;

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
  //iPolygonTexture* poly_texture;
  csPolyTextureMapping* texmap;
  csPolyLightMapMapping* lmap;
  iRendererLightmap* rlm;
  
  /// Draw fullbright?
  bool do_fullbright;

  /// Mixmode to use. If CS_FX_COPY then no mixmode is used.
  uint mixmode;

  /// Z value (in camera space) of vertex[0].
  float z_value;
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

SCF_VERSION (iGraphics3D, 5, 2, 0);

/**
 * This is the standard 3D graphics interface.
 * All 3D graphics rasterizer servers for Crystal Space should implement this
 * interface, as well as the iGraphics2D interface.  The standard
 * implementation is csGraphics3DSoftware.
 */
struct iGraphics3D : public iBase
{
  /// Open the 3D graphics display.
  virtual bool Open () = 0;
  /// Close the 3D graphics display.
  virtual void Close () = 0;

  /**
   * Get a pointer to our 2d canvas driver. NOTE: It's not increfed,
   * and therefore it shouldn't be decref-ed by caller.
   */
  virtual iGraphics2D *GetDriver2D () = 0;

  /// Get the texture manager: do NOT increment the refcount of texture manager
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
  virtual void Print (csRect *area) = 0;

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
   * Create a renderbuffer.
   * \param size Size of the buffer in bytes.
   * \param type Type of buffer; CS_BUF_DYNAMIC or CS_BUF_STATIC
   * \param componentType Components Types; CS_BUFCOMP_BYTE, CS_BUFCOMP_INT, etc
   * \param componentCount Number of components per element (e.g. 4 for RGBA)
   * \param index True if this buffer will contain indices. (Triangle buffer)
   */
  virtual csPtr<iRenderBuffer> CreateRenderBuffer (int size, 
    csRenderBufferType type, csRenderBufferComponentType componentType, 
    int componentCount, bool index) = 0;

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
  virtual void DrawMesh (csRenderMesh* mymesh) = 0;

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
   * Enter a new clipped portal. Basically this routine will restrict
   * all further drawing to the given 2D area and it will also respect
   * the current contents of the Z-buffer so that geometry will only
   * render where the Z-buffer allows it (even if zfill or znone is used).
   * Remember to close a portal later using ClosePortal().
   * Basically this represents a stacked layer of portals. Each subsequent
   * portal must be fully contained in the previous ones.
   */
  virtual void OpenPortal (G3DPolygonDFP* poly) = 0;

  /**
   * Close a portal previously opened with OpenPortal().
   */
  virtual void ClosePortal () = 0;

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

  /**
   * Check if renderer can handle a lightmap.
   * Returns true if it can, false if not.
   */
  virtual bool IsLightmapOK (int lmw, int lmh, 
    int lightCellSize) = 0;
};

/** @} */

#endif // __CS_IVIDEO_GRAPH3D_H__

