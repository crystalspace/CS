/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_OGL3DCOM_H__
#define __CS_OGL3DCOM_H__

// csGraphics3DOGLCommon OpenGL rasterizer class.

// Concieved by Jorrit Tyberghein and Gary Clark
// Expanded by Dan Ogles
// Further expanded by Gary Haussmann
// Further expanded by Heiko Jakob

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "csutil/scf.h"
#include "csutil/cfgacc.h"
#include "csutil/parray.h"
#include "plugins/video/renderer/common/dtmesh.h"
#include "plugins/video/renderer/common/dpmesh.h"
#include "plugins/video/renderer/common/polybuf.h"
#include "csgeom/transfrm.h"
#include "csgeom/poly3d.h"
#include "csgeom/poly2d.h"
#include "ivideo/graph3d.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "plugins/video/canvas/openglcommon/iogl.h"
#include "plugins/video/canvas/openglcommon/glstates.h"

#include "ieffects/efdef.h"
#include "ieffects/efserver.h"
#include "ieffects/efclient.h"

struct iGraphics2D;
class OpenGLTextureCache;
class csTextureHandleOpenGL;
class csTextureManagerOpenGL;
class csTriangleArrayVertexBufferManager;
struct iClipper2D;
struct iObjectRegistry;
struct iPluginManager;
struct iTextureManager;

/// OpenGL capabilities
struct csOpenGLCaps
{
  // Can we use stencil buffer?
  bool use_stencil;
  // Can we use OpenGL clipping planes? And if yes, for how many planes?
  int nr_hardware_planes;
  // Do we have broken driver that requires clipping to screen boundaries?
  bool need_screen_clipping;
};

/**
 * Queue to optimize polygon drawing.
 * Polygons with the same material and other modes will be added to
 * this queue so that they can be rendered in one step.
 */
class csPolyQueue
{
public:
  iMaterialHandle* mat_handle;
  uint mixmode;
  csZBufMode z_buf_mode;
  float flat_color_r;
  float flat_color_g;
  float flat_color_b;
  bool use_fog;

  // Vertices.
  int num_vertices;
  int max_vertices;
  GLfloat* glverts; // 4*max_vertices
  GLfloat* gltxt; // 2*max_vertices
  GLfloat* glcol; // 4*max_vertices
  GLfloat* layer_gltxt; // 2*max_vertices
  GLfloat* fog_color; // 3*max_vertices
  GLfloat* fog_txt; // 2*max_vertices

  // Triangles.
  int num_triangles;
  int max_triangles;
  int* tris;    // 3*max_triangles

  /// Add some vertices. Return index of the added vertices.
  int AddVertices (int num);
  /// Add a triangle.
  void AddTriangle (int i1, int i2, int i3);
  /// Reset the queue to empty.
  void Reset ()
  {
    num_triangles = 0;
    num_vertices = 0;
  }

  GLfloat* GetGLVerts (int idx) { return &glverts[idx<<2]; }
  GLfloat* GetGLTxt (int idx) { return &gltxt[idx<<1]; }
  GLfloat* GetGLCol (int idx) { return &glcol[idx<<2]; }
  GLfloat* GetLayerGLTxt (int idx) { return &layer_gltxt[idx<<1]; }
  GLfloat* GetFogColor (int idx) { return &fog_color[idx*3]; }
  GLfloat* GetFogTxt (int idx) { return &fog_txt[idx<<1]; }

  csPolyQueue () :
    mat_handle (0), mixmode (~0), z_buf_mode (CS_ZBUF_NONE),
    flat_color_r (0), flat_color_g (0), flat_color_b (0),
    use_fog (false), num_vertices (0), max_vertices (0),
    glverts (0), gltxt (0), glcol (0), layer_gltxt (0),
    fog_color (0), fog_txt (0),
    num_triangles (0), max_triangles (0),
    tris (0) { }

  ~csPolyQueue ()
  {
    delete[] glverts;
    delete[] gltxt;
    delete[] glcol;
    delete[] layer_gltxt;
    delete[] fog_color;
    delete[] fog_txt;
    delete[] tris;
  }
};

#define OPENGL_KEYCOLOR_MIN_ALPHA 0.5f // alphafunc is disabled below this

#define OPENGL_CLIP_AUTO    'a' // Used for auto-detection.
#define OPENGL_CLIP_NONE    'n'
#define OPENGL_CLIP_ZBUF    'z'
#define OPENGL_CLIP_STENCIL   's'
#define OPENGL_CLIP_PLANES    'p'
#define OPENGL_CLIP_SOFTWARE    '0'
#define OPENGL_CLIP_LAZY_NONE   'N'
#define OPENGL_CLIP_LAZY_ZBUF   'Z'
#define OPENGL_CLIP_LAZY_STENCIL  'S'
#define OPENGL_CLIP_LAZY_PLANES   'P'

#define EFFECTFLAG_RUINSZCLIPPING 1
#define EFFECTFLAG_RUINSSCLIPPING 2

/**
 * This is an additional blend mode that is used to get the best detected
 * SRC*DST mode. Ideally this is the same as CS_FX_MULTIPLY2 but this is
 * not always the case).
 */
#define CS_FX_SRCDST 0x00010000
/**
 * This is an additional blend mode that is used for halo drawing in
 * some cases.
 */
#define CS_FX_HALOOVF 0x00020000

#define CS_FX_EXTRA_MODES 0x000f0000

/**
 * Client states CS keeps for OpenGL.
 */
#define CS_CLIENTSTATE_VERTEX_ARRAY 1
#define CS_CLIENTSTATE_TEXTURE_COORD_ARRAY 2
#define CS_CLIENTSTATE_COLOR_ARRAY 4
#define CS_CLIENTSTATE_ALL 0x7
#define CS_CLIENTSTATE_VTC 0x7
#define CS_CLIENTSTATE_VC 0x5
#define CS_CLIENTSTATE_VT 0x3

/// Crystal Space OpenGL driver.
class csGraphics3DOGLCommon : public iGraphics3D
{
  friend class OpenGLLightmapCache;

public:
  static csGraphics3DOGLCommon* ogl_g3d;

  static csGLStateCache* statecache;

  /**
   * Set proper GL flags based on ZBufMode.
   * This is no longer the only legal way to set the z-buffer flags
   */
  static void SetGLZBufferFlags (csZBufMode flags);

  /**
   * Set proper GL flags based on ZBufMode.
   * This version is used to set the zbuffer flags for the second
   * pass of polygon drawing. The following second pass zbuf flags
   * are used depending on first pass flags:
   * <ul>
   * <li>ZNONE -> ZNONE
   * <li>ZFILL -> ZNONE or ZEQUAL
   * <li>ZTEST -> ZTEST
   * <li>ZUSE  -> ZEQUAL
   * <li>ZEQUAL-> ZEQUAL
   * </ul>
   * The result for ZFILL depends on the multiPol flag. If
   * multiPol == true this means that multiple polygons will
   * be rendered in the second pass (i.e. in a five polygon model
   * we will first render five polygon for first pass, then five for
   * second pass). In that case we need ZEQUAL mode.
   */
  static void SetGLZBufferFlagsPass2 (csZBufMode flags, bool multiPol);

  /**
   * Setup and remember blend mode for subsequent polygon drawing.
   */
  static float SetupBlend (uint mode, float m_alpha, bool txt_alpha);

  /**
   * Setup client states.
   * Use a combination of CS_CLIENTSTATE_...
   */
  static void SetClientStates (uint client_states);
  
  /**
   * Set mirror mode for the rasterizer. Then polygons have
   * to be rendered as seen from the other side.
   */
  void SetMirrorMode (bool mirror);

  /**
   * Set glOrtho() settings depending on current render
   * target and inverted mode.
   */
  void SetGlOrtho (bool inverted);

  /**
   * Calls glGetError(). Returns true if no error occured,
   * otherwise false. Also reports the error if enabled by user.
   */
  bool CheckGLError (const char* call);

private:
  csRef<iEffectServer> effectserver;

  // [lightmap no/yes ][fog 0/1][mixmode]
  csRef<iEffectDefinition> StockEffects[2][2][9];
  csRef<iEffectDefinition> SeparateLightmapStockEffect;
  csRef<iEffectDefinition> SeparateFogStockEffect;

  /// Inits stock effects
  void InitStockEffects();

  csStringID GLBlendToString( GLenum blend );

  /// Get a technique corresponding to a given mesh
  iEffectTechnique* GetStockTechnique( G3DTriangleMesh& mesh );

  /**
   * Return true if two z-buf modes are compatible.
   * Two z-buf modes can be compatible even if they are different
   * because we are only interested in second pass rendering here.
   */
  static bool CompatibleZBufModes (csZBufMode m1, csZBufMode m2);

  // Some common shortcut functions that may or may not apply, depending
  // on the underlying hardware
  // guess the proper blend mode to use
  void Guess_BlendMode (GLenum *src, GLenum *dst);

  /// Make sure the frustum is correct.
  void CalculateFrustum ();
  /// Make sure the stencil is correct for the current clipper.
  void SetupStencil ();
  /// Make sure the OpenGL clipping planes are correct for the current clipper.
  void SetupClipPlanes (bool add_near_clip, bool add_z_clip);

  /// Setup the portals if needed.
  void SetupClipPortals ();

  /// Do a performance test to find out the best configuration for OpenGL.
  void PerfTest ();

  /**
   * Setup planes for clipping. Used for ClipTriangleMesh().
   */
  void SetupClippingPlanes (
    csPlane3* frustum_planes, int& num_planes,
    csVector3& frust_origin,
    bool transform,
    bool mirror,
    bool plane_clipping,  // Clip to the near plane
    bool z_plane_clipping,  // Clip to the z plane
    bool frustum_clipping); // Clip to the frustum planes

  /**
   * Classify all vertices from a mesh for clipping (ClipTriangleMesh).
   * Returns false if totally invisible.
   */
  bool ClassifyForClipTriangleMesh (
    int num_vertices, csVector3* vertices,
    const csBox3& bbox,
    const csVector3& frust_origin, csPlane3* planes, int num_planes);

  /**
   * Given information from a mesh, clip the mesh to the frustum.
   * The clipped mesh will be put in the clipped_??? arrays.
   */
  void ClipTriangleMesh (
    int num_triangles,
    int num_vertices,
    csTriangle* triangles,
    csVector3* vertices,
    int& num_clipped_triangles,
    const csVector3& frust_origin,
    csPlane3* planes, int num_planes);

  /**
   * Given information from a mesh, clip the mesh to the frustum.
   * The clipped mesh will be put in the clipped_??? arrays.
   */
  void ClipTriangleMeshExact (
    int num_triangles,
    int num_vertices,
    csTriangle* triangles,
    csVector3* vertices,
    csVector2* texels,
    csColor* vertex_colors,
    float** userarrays,
    int* userarraycomponents,
    G3DFogInfo* vertex_fog,
    int& num_clipped_triangles,
    int& num_clipped_vertices,
    const csVector3& frust_origin,
    csPlane3* planes, int num_planes);


  /**
   * Draw the outlines of all triangles. This function accepts
   * the same arguments as glDrawElements() (but GL_TRIANGLES
   * is assumed).
   */
  static void DebugDrawElements (iGraphics2D* g2d, int num_tri3, int* tris,
    GLfloat* verts, int color, bool coords3d, bool transformed);

protected:
  friend class csOpenGLHalo;
  friend class csTextureManagerOpenGL;
  friend class OpenGLTextureCache;
  friend class csTextureHandleOpenGL;
  friend class csOpenGLProcBackBuffer;

  /**
   * OpenGL capabilities.
   */
  csOpenGLCaps GLCaps;

  /// Prefered clipping modes to use for optional portals.
  char clip_optional[3];
  /// Prefered clipping modes to use for required portals.
  char clip_required[3];
  /// Prefered clipping modes to use for outer portal.
  char clip_outer[3];

  /// Current render target.
  csRef<iTextureHandle> render_target;
  /// If true then the current render target has been put on screen.
  bool rt_onscreen;
  /// If true then we have set the old clip rect.
  bool rt_cliprectset;
  /// Old clip rect to restore after rendering on a proc texture.
  int rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy;

  /**
   * handle of a local 1D alpha-blend texture; this texture holds an
   * exponential alpha curve from 0 to 1.  Using this texture, you
   * can convert from a linear texture coordinate of 0-1 to an
   * exponential alpha-blend curve.  Such a mapping is useful for
   * drawing exponential fog polygons.  A value of 0 means the handle
   * is unallocated
   */
  GLuint m_fogtexturehandle;

  /// If true then we show edges around all polygons (for debugging).
  bool debug_edges;

  /// Z Buffer mode to use while rendering next polygon.
  csZBufMode z_buf_mode;

  /// Render capabilities
  csGraphics3DCaps Caps;

  /**
   * An artificial limitation on fps. On some hardware this may help
   * improve smoothness of the OpenGL renderer if frame rates get too high.
   */
  int fps_limit;

  /// Width of display.
  int width;
  /// Height of display.
  int height;
  /// Aspect center x.
  int asp_center_x;
  /// Aspect center y.
  int asp_center_y;

  /// Current transformation from world to camera.
  csReversibleTransform o2c;

  /// Current 2D clipper.
  iClipper2D* clipper;
  /// Clipper type.
  int cliptype;
  /// 3D Frustum calculated from clipper.
  csPoly3D frustum;

  // Structure used for maintaining a stack of clipper portals.
  struct csClipPortal
  {
    csVector2* poly;
    int num_poly;
    csPlane3 normal;
    csClipPortal () : poly (0) { }
    ~csClipPortal () { delete[] poly; }
  };
  csPDelArray<csClipPortal> clipportal_stack;
  bool clipportal_dirty;
  int clipportal_floating;

  /**
   * If true the frustum below is valid. If false
   * we need to calculate it from the clipper and aspect.
   */
  bool frustum_valid;
  /**
   * If true then we already have initialized our toplevel portal.
   * This flag is set to false after BeginDraw() and will be set to true
   * by the first call to SetClipper with type CS_CLIPPER_TOPLEVEL.
   */
  bool toplevel_init;
  /**
   * If true then the stencil is initialized for this clipper.
   */
  bool stencil_init;
  /**
   * If true then the OpenGL clipping planes are initialized for this clipper.
   * Also compare the 'add_near_clip' and 'add_z_clip' values to see if the
   * planes are really valid.
   */
  bool planes_init;
  bool planes_add_near_clip;
  bool planes_add_z_clip;

  /// The current near plane.
  csPlane3 near_plane;
  /// If the near plane is used.
  bool do_near_plane;

  /// Polygon queue.
  csPolyQueue queue;

  /// Current aspect ratio for perspective correction.
  float aspect;
  /// Current inverse aspect ratio for perspective correction.
  float inv_aspect;
  /// Use an inverted orthographic projection matrix?
  bool inverted;
  /**
   * render-states
   * these override any other variable settings.
   */
  struct
  {
    /// dither colors?
    bool dither; 
    /// texel/mipmap interpolate?
    bool trilinearmap; 
    /// gouraud shading on polygons?
    bool gouraud; 
    /// enable transparency?
    bool alphablend; 
    /// enable mipmapping?
    int  mipmap;    
    /// Option variable: do texture lighting? (lightmaps)
    bool lighting; 
    /// Option variable: render textures?
    bool textured; 
  } m_renderstate;

  // load-time configuration options
  struct
  {
    /**
     * Current settings of the user configurable blend parameters for
     * lightmaps.  Certain settings work better on certain cards
     */
    GLenum m_lightmap_src_blend;
    GLenum m_lightmap_dst_blend;

    /**
     * Option variable: do multitexturing?  This value is zero if multitexturing
     * is not available.  If multitexturing is available, this value holds
     * the number of textures that can be mixed together in one pass.
     */
    int do_multitexture_level;
  } m_config_options;

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

  /// previous texmatrix and -vector and polyplane
  csPlane3 prevPolyPlane;
  csMatrix3 prevTexMatrix;
  csVector3 prevTexVector;

  // helpers for cam->tex conversion
  float M, N, O;
  float J1, J2, J3, K1, K2, K3;

  /**
   * If true, CheckGLError() will report errors
   */
  bool report_gl_errors;

  virtual void InitGLExtensions ();
  csGLExtensionManager* ext;
public:
  SCF_DECLARE_IBASE;

  /// The texture cache.
  OpenGLTextureCache* texture_cache;
  /// The texture manager
  csTextureManagerOpenGL* txtmgr;
  /// The vertex buffer manager.
  csTriangleArrayVertexBufferManager* vbufmgr;
  //csPolArrayVertexBufferManager* vbufmgr;

  /**
   * If true, be verbose.
   */
  bool verbose;

  /**
   * Low-level 2D graphics layer.
   * csGraphics3DOGLCommon is in charge of creating and managing this.
   */
  csRef<iGraphics2D> G2D;

  /// The configuration file
  csConfigAccess config;

  /// The System interface.
  iObjectRegistry* object_reg;

  /// Constructor.
  csGraphics3DOGLCommon (iBase*);
  /// Destructor.
  virtual ~csGraphics3DOGLCommon ();

  void Report (int severity, const char* msg, ...);

  /// Initialization for iComponent.  Sets System pointer.
  virtual bool Initialize (iObjectRegistry*);
  /**
   * Open or close our interface.
   */
  bool HandleEvent (iEvent& ev);

  /// Common initialization for subclasses.
  bool NewInitialize ();
  /// Initialize from the state data of another driver of this type
  void SharedInitialize (csGraphics3DOGLCommon *d);
  /// Common canvas opening method.
  bool NewOpen ();
  /// Open from the state data of another driver of this type
  void SharedOpen (csGraphics3DOGLCommon *d);
  /// Helper function
  void CommonOpen ();
  /// Common canvas close.
  virtual void Close ();

  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height);

  /// Start a new frame (see CSDRAW_XXX bit flags)
  virtual bool BeginDraw (int DrawFlags);

  /// End the frame and do a page swap.
  virtual void FinishDraw ();

  /// Print the image in backbuffer
  virtual void Print (csRect const* area);

  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly);
  /**
   * Draw the projected polygon with light and texture.
   * Debugging version. This one does not actually draw anything
   * but it just prints debug information about what it would have
   * done.
   */
  virtual void DrawPolygonDebug (G3DPolygonDP& poly);

  /// Draw a line in camera space.
  virtual void DrawLine (const csVector3& v1, const csVector3& v2,
    float fov, int color);

  /// Draw a polygon with special effects.
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Give a material to csGraphics3DOGLCommon to cache it.
  void CacheTexture (iMaterialHandle *mat_handle);

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);

  /// Get a renderstate value.
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op) const;

  virtual bool SetOption (const char*, const char*);

  /**
   * Get the current driver's capabilities. Each driver implements their
   * own function.
   */
  virtual const csGraphics3DCaps *GetCaps () const
  { return &Caps; }

  /// Get address of Z-buffer at specific point
  virtual uint32 *GetZBuffAt (int, int)
  { return 0; }

  /// Dump the texture cache.
  virtual void DumpCache ();

  /// Clear the texture cache.
  virtual void ClearCache ();

  /// Remove some polygon from the cache.
  virtual void RemoveFromCache (iRendererLightmap* rlm);

  /// Get drawing buffer width
  virtual int GetWidth () const { return width; }
  /// Get drawing buffer height
  virtual int GetHeight () const { return height; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y)
  {
    asp_center_x = x;
    asp_center_y = y;
    frustum_valid = false;
  }
  /// Get center of projection.
  virtual void GetPerspectiveCenter (int& x, int& y) const
  {
    x = asp_center_x;
    y = asp_center_y;
  }
  /// Set perspective aspect.
  virtual void SetPerspectiveAspect (float aspect)
  {
    this->aspect = aspect;
    inv_aspect = 1./aspect;
    frustum_valid = false;
  }
  /// Get perspective aspect.
  virtual float GetPerspectiveAspect () const
  {
    return aspect;
  }
  /// Set world to camera transformation.
  virtual void SetObjectToCamera (csReversibleTransform* o2c)
  {
    this->o2c = *o2c;
  }
  /// Get world to camera transformation.
  virtual const csReversibleTransform& GetObjectToCamera ()
  {
    return o2c;
  }
  /// Set optional clipper.
  virtual void SetClipper (iClipper2D* clipper, int cliptype);
  /// Get clipper.
  virtual iClipper2D* GetClipper () { return clipper; }
  /// Get clipper type.
  virtual int GetClipType () const { return cliptype; }

  /// Set near clip plane.
  virtual void SetNearPlane (const csPlane3& pl)
  {
    do_near_plane = true;
    near_plane = pl;
  }

  /// Reset near clip plane (i.e. disable it).
  virtual void ResetNearPlane () { do_near_plane = false; }

  /// Get near clip plane.
  virtual const csPlane3& GetNearPlane () const { return near_plane; }

  /// Return true if we have near plane.
  virtual bool HasNearPlane () const { return do_near_plane; }

  /// Setup OpenGL transforms for DrawTriangleMesh().
  void SetupDTMTransforms (int vertex_mode);
  /// Restore OpenGL transforms after DrawTriangleMesh().
  void RestoreDTMTransforms ();
  /// Setup clipping for DrawTriangleMesh().
  void SetupDTMClipping (G3DTriangleMesh& mesh);
  /// Restore clipping.
  void RestoreDTMClipping ();
  /// Setup effects for EffectDrawTriangleMesh().
  void SetupDTMEffect (G3DTriangleMesh& mesh);

  /// Draw a triangle mesh.
  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh);
  /**
   * If setup == true then this function will call SetupDTM...()
   * and RestoreDTM...(). Returns false if the mesh was clipped away
   * completely.
   */
  bool OldDrawTriangleMesh (G3DTriangleMesh& mesh, bool setup = true);
  bool EffectDrawTriangleMesh (G3DTriangleMesh& mesh, bool setup = true,
    GLuint lightmap = 0, csVector2* lightmapcoords = 0);
  void FogDrawTriangleMesh (G3DTriangleMesh& mesh, bool setup = true);

  /**
   * Debug version that draws outlines only. Is automatically
   * called by DrawTriangleMesh if needed.
   */
  void DrawTriangleMeshEdges (G3DTriangleMesh& mesh);

  /// Draw a polygon mesh.
  virtual void DrawPolygonMesh (G3DPolygonMesh& mesh);
  //virtual void OldDrawPolygonMesh (G3DPolygonMesh& mesh);

  /// Get the iGraphics2D driver.
  virtual iGraphics2D *GetDriver2D ()
  { return G2D; }

  /// Get the iTextureManager.
  virtual iTextureManager *GetTextureManager ();

  /// Get the vertex buffer manager.
  virtual iVertexBufferManager* GetVertexBufferManager ()
  { return (iVertexBufferManager*)vbufmgr; }

  /**
   * Initiate a volumetric fog object. This function will be called
   * before front-facing and back-facing fog polygons are added to
   * the object. The fog object will be convex but not necesarily
   * closed.
   * The given CS_ID can be used to identify multiple fog objects when
   * multiple objects are started.
   * On the OpenGL driver this function is not used.  Instead the
   * fog is drawn as an additional texture on each polygon that
   * is effected by the fog.
   */
  virtual void OpenFogObject (CS_ID id, csFog* fog);

  /**
   * Add a front or back-facing fog polygon in the current fog object.
   * Note that it is guaranteed that all back-facing fog polygons
   * will have been added before the first front-facing polygon.
   * fogtype can be:
   * <ul>
   *  <li>CS_FOG_FRONT: a front-facing polygon
   *  <li>CS_FOG_BACK:  a back-facing polygon
   *  <li>CS_FOG_VIEW:  the view-plane
   * </ul>
   * On the OpenGL driver this function is not used.  Instead the
   * fog is drawn as an additional texture on each polygon that
   * is effected by the fog.
   */
  virtual void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype);

  /**
   * Close a volumetric fog object. After the volumetric object is
   * closed it should be rendered on screen (whether you do it here
   * or in DrawFrontFog/DrawBackFog is not important).
   * On the OpenGL driver this function is not used.  Instead the
   * fog is drawn as an additional texture on each polygon that
   * is effected by the fog.
   */
  virtual void CloseFogObject (CS_ID id);

  /// Open a clipped portal.
  virtual void OpenPortal (size_t numVertices, const csVector2* vertices,
    const csPlane3& normal, bool floating);
  /// Close a portal previously opened with OpenPortal().
  virtual void ClosePortal (bool use_zfill_portal);

  /// Get Z-buffer value at given X,Y position
  virtual float GetZBuffValue (int x, int y);

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char* iAlpha, int iWidth, int iHeight);

  /// Draw a 2D sprite
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha);

  //========================================================================
  // All stuff below is only to be compatible with the NR api.
  //========================================================================
  virtual csPtr<iRenderBuffer> CreateRenderBuffer (int, 
    csRenderBufferType, csRenderBufferComponentType, 
    int, bool) { return 0; }
  virtual void CreateInterleavedRenderBuffers (int size, 
    csRenderBufferType type, int count, csArray<iRenderBuffer*> &buffers) { }
  virtual void SetBufferState (csVertexAttrib*,
  	iRenderBuffer**, int count) { }
  virtual void SetTextureState (int*, iTextureHandle**,
  	int) { }
  virtual void DrawMesh (const csCoreRenderMesh*, const csRenderMeshModes&,
        const csArray< csArray<csShaderVariable*> >&) {}
  virtual void SetWriteMask (bool, bool, bool, bool) { }
  virtual void GetWriteMask (bool &, bool &, bool &,
	bool &) const { }
  virtual void SetZMode (csZBufMode) { }
  virtual void EnableZOffset () { }
  virtual void DisableZOffset () { }
  virtual void SetShadowState (int) { }
  virtual csPtr<iPolygonRenderer> CreatePolygonRenderer () { return 0; }
  virtual void SetWorldToCamera (csReversibleTransform* wvmatrix) {}
  virtual void DrawSimpleMesh (const csSimpleRenderMesh& mesh, uint flags = 0) 
  {}
  virtual csZBufMode GetZMode () { return CS_ZBUF_NONE; }
  //========================================================================

#if 0
  /**
   * Draw a fully-featured polygon assuming one has an OpenGL renderer
   * that supports ARB_multitexture
   */
  void DrawPolygonMultiTexture (G3DPolygonDP &poly);
#endif

  /**
   * Flush the DrawPolygon queue if needed.
   */
  void FlushDrawPolygon ();

  // EXPERIMENTAL@@@
  void DrawPolygonStartMaterial (iMaterialHandle* mat_handle, uint mixmode);
  void DrawPolygonMaterialOnly (G3DPolygonDP &poly);
  void DrawPolygonLightmapOnly (G3DPolygonDP &poly);

  /**
   * Draw a fully-featured polygon assuming one has an OpenGL renderer
   * that only has a single texture unit.
   */
  void DrawPolygonSingleTexture (G3DPolygonDP &poly);

  /// Check if lightmap is not too large
  virtual bool IsLightmapOK (int lmw, int lmh, 
    int lightCellSize);

  virtual void SetRenderTarget (iTextureHandle* handle, bool persistent);
  virtual iTextureHandle* GetRenderTarget () const
  {
    return render_target;
  }

  /**
   * Draw a polygon but only do z-fill. Do not actually render
   * anything.
   */
  void DrawPolygonZFill (csVector2* vertices, int num_vertices,
  	const csPlane3& normal);

  /// Validate a technique
  bool Validate( iEffectDefinition* effect, iEffectTechnique* technique );

  /// Execute a debug command.
  virtual bool DebugCommand (const char* cmd);

  struct eiEffectClient : public iEffectClient
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGraphics3DOGLCommon);
    virtual bool Validate( iEffectDefinition* effect, iEffectTechnique* technique )
    { return scfParent->Validate (effect, technique); }
  } scfiEffectClient;
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGraphics3DOGLCommon);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
  struct EventHandler : public iEventHandler
  {
  private:
    csGraphics3DOGLCommon* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csGraphics3DOGLCommon* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& ev) { return parent->HandleEvent (ev); }
  } * scfiEventHandler;

  /// iDebugHelper implementation
  struct eiDebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGraphics3DOGLCommon);
    virtual int GetSupportedTests () const
    { return 0; }
    virtual csPtr<iString> UnitTest ()
    { return 0; }
    virtual csPtr<iString> StateTest ()
    { return 0; }
    virtual csTicks Benchmark (int num_iterations)
    { return 0; }
    virtual csPtr<iString> Dump ()
    { return 0; }
    virtual void Dump (iGraphics3D* g3d)
    { }
    virtual bool DebugCommand (const char* cmd)
    { return scfParent->DebugCommand (cmd); }
  } scfiDebugHelper;
};

#endif // __CS_OGL3DCOM_H__
