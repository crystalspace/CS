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

#ifndef __CS_GL_RENDER3D_H__
#define __CS_GL_RENDER3D_H__

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "csgeom/csrect.h"
#include "csgeom/poly3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"

#include "csutil/cfgacc.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/parray.h"
#include "csutil/scf.h"
#include "csutil/scfstrset.h"
#include "csutil/weakref.h"
#include "csutil/weakrefarr.h"
#include "iutil/strset.h"
#include "csgfx/shadervarcontext.h"

#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "ivideo/graph2d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/graph3d.h"
#include "ivideo/halo.h"
#include "ivideo/shader/shader.h"
#include "ivaria/bugplug.h"

#include "csplugincommon/opengl/glstates.h"
#include "gl_renderbuffer.h"
class csGLTextureHandle;
class csGLTextureManager;
class csGLPolygonRenderer;
class csGLExtensionManager;
class csGLVBOBufferManager;
class csGLRenderBuffer;
struct iClipper2D;
struct iObjectRegistry;
struct iTextureManager;
struct iRenderBufferManager;
struct iLightingManager;

struct iEvent;

class csGLGraphics3D;
class csOpenGLHalo : public iHalo
{
  /// The halo color
  float R, G, B;
  /// The width and height
  int Width, Height;
  /// Width and height factor
  float Wfact, Hfact;
  /// Blending method
  uint dstblend;
  /// Our OpenGL texture handle
  GLuint halohandle;
  /// The OpenGL 3D driver
  csGLGraphics3D* G3D;

public:
  SCF_DECLARE_IBASE;

  csOpenGLHalo (float iR, float iG, float iB, unsigned char *iAlpha,
    int iWidth, int iHeight, csGLGraphics3D* iG3D);

  virtual ~csOpenGLHalo ();

  void DeleteTexture();

  virtual int GetWidth () { return Width; }
  virtual int GetHeight () { return Height; }

  virtual void SetColor (float &iR, float &iG, float &iB)
  { R = iR; G = iG; B = iB; }

  virtual void GetColor (float &oR, float &oG, float &oB)
  { oR = R; oG = G; oB = B; }

  virtual void Draw (float x, float y, float w, float h, float iIntensity,
    csVector2 *iVertices, int iVertCount);
};

class csGLGraphics3D : public iGraphics3D
{
private:
  //friend declarations
  friend class csGLSuperLightmap;
  friend class csGLRendererLightmap;
  friend class csGLTextureHandle;
  friend class csGLTextureManager;
  friend class eiShaderRenderInterface;
  friend class csGLPolygonRenderer;

  iObjectRegistry* object_reg;
  csRef<iGraphics2D> G2D;
  csRef<iShaderManager> shadermgr;
  csRef<csGLVBOBufferManager> vboManager;
  bool isOpen;

  csWeakRef<iBugPlug> bugplug;

  csRef<csGLTextureManager> txtmgr;
  csWeakRefArray<csOpenGLHalo> halos;

  int current_drawflags;
  int current_shadow_state;
  csZBufMode current_zmode;
  bool zmesh;
  bool forceWireframe;

  int asp_center_x, asp_center_y;
  float aspect, inv_aspect;
  bool needProjectionUpdate;
  float fov;
  int viewwidth, viewheight;
  csPoly3D frustum;
  bool frustum_valid;

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

  csReversibleTransform object2camera;

  bool verbose;
  csGraphics3DCaps rendercaps;

  csRef<iStringSet> strings;

  csStringID string_vertices;
  csStringID string_texture_coordinates;
  csStringID string_normals;
  csStringID string_colors;
  csStringID string_indices;
  csStringID string_point_radius;
  csStringID string_point_scale;
  csStringID string_texture_diffuse;

  csConfigAccess config;

  float textureLodBias;

  /**
   * Clipping related stuff.
   */

  // If number of triangles>this value we use stencil instead of plane clipping.
  int stencil_threshold;
  bool broken_stencil;		// Stencil clipping is broken and avoided.
  bool do_near_plane;
  csPlane3 near_plane;
  bool stencil_clipping_available;
  GLuint stencil_clip_mask;
  GLuint stencil_clip_value;
  GLuint stencil_shadow_mask;
  //GLuint stencil_shadow_value;
  bool stencilClearWithZ; /* When clearing the stencil buffer is needed, 
			     also clear the Z buffer. Possibly faster on 
			     some HW. */

  bool shadow_stencil_enabled;
  bool clipping_stencil_enabled;
  void EnableStencilShadow ();
  void DisableStencilShadow ();
  void EnableStencilClipping ();
  void DisableStencilClipping ();
  // Depending on stencil clipping, stencil shadows, or floating portals
  // we need to enable or disable stencil.
  void SetCorrectStencilState ();

  int stencilclipnum;
  bool stencil_initialized;	// Stencil clipper is initialized from 'clipper'
  bool clip_planes_enabled;	// glClipPlane is enabled.
  csWeakRef<iClipper2D> clipper;// Current clipper from engine.
  int cliptype;			// One of CS_CLIPPER_...
  int cache_clip_portal;	// Cache values for SetupClipper().
  int cache_clip_plane;
  int cache_clip_z_plane;
  bool hasOld2dClip;
  csRect old2dClip;

  // For debugging: inhibit all drawing of meshes till next frame.
  bool debug_inhibit_draw;

  /// Current render target.
  csRef<iTextureHandle> render_target;
  /// If true then the current render target has been put on screen.
  bool rt_onscreen;
  /// If true then we have set the old clip rect.
  bool rt_cliprectset;
  /// Old clip rect to restore after rendering on a proc texture.
  int rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy;

  /// Should we use special buffertype (VBO) or just systemmeory
  bool use_hw_render_buffers;
  size_t vbo_thresshold;
  csGLDRAWRANGEELEMENTS glDrawRangeElements;
  static GLvoid csAPIENTRY myDrawRangeElements (GLenum mode, GLuint start, 
    GLuint end, GLsizei count, GLenum type, const GLvoid* indices);

  // for simple mesh drawing
  uint scrapIndicesSize;
  csRef<iRenderBuffer> scrapIndices;
  uint scrapVerticesSize;
  csRef<iRenderBuffer> scrapVertices;
  csRef<iRenderBuffer> scrapTexcoords;
  csRef<iRenderBuffer> scrapColors;
  csShaderVariableContext scrapContext;
  csRef<csRenderBufferHolder> scrapBufferHolder;
  csRenderBufferName scrapMapping [CS_VATTRIB_SPECIFIC_LAST+1]; 

  ////////////////////////////////////////////////////////////////////
  //                         Private helpers
  ////////////////////////////////////////////////////////////////////
	
  void Report (int severity, const char* msg, ...);

  int GetMaxTextureSize () const { return rendercaps.maxTexWidth; }

  // Enables offsetting of Z values
  void EnableZOffset ()
  // @@@ Jorrit: to avoid flickering I had to increase the
  // values below and multiply them with 3.
  //{ glPolygonOffset (-0.05f, -2.0f); 
  { glPolygonOffset (-0.15f, -6.0f); 
  statecache->Enable_GL_POLYGON_OFFSET_FILL (); }

  // Disables offsetting of Z values
  void DisableZOffset ()
  { statecache->Disable_GL_POLYGON_OFFSET_FILL (); }

  // Debug function to visualize the stencil with the given mask.
  void DebugVisualizeStencil (uint32 mask);

  void SetZModeInternal (csZBufMode mode);

  void SetMirrorMode (bool mirror);

  void CalculateFrustum ();

  void SetupStencil ();

  int SetupClipPlanes (bool add_clipper, 
    bool add_near_clip, 
    bool add_z_clip);

  void SetupClipper (int clip_portal, int clip_plane, int clip_z_plane,
  	int tri_count);

  void ApplyObjectToCamera ();
  void SetupProjection ();

  csZBufMode GetZModePass2 (csZBufMode mode);

  csRef<iRenderBuffer> spec_renderBuffers[CS_VATTRIB_SPECIFIC_LAST-CS_VATTRIB_SPECIFIC_FIRST+1];
  void AssignSpecBuffer (uint attr, iRenderBuffer* buffer)
  {
    if (spec_renderBuffers[attr]) 
      RenderRelease (spec_renderBuffers[attr]);
      //spec_renderBuffers[attr]->RenderRelease ();
    spec_renderBuffers[attr] = buffer;
  }

  csRef<iRenderBuffer> gen_renderBuffers[CS_VATTRIB_GENERIC_LAST-CS_VATTRIB_GENERIC_FIRST+1];
  void AssignGenericBuffer (uint attr, iRenderBuffer* buffer)
  {
    if (gen_renderBuffers[attr]) 
      RenderRelease (gen_renderBuffers[attr]);
      //gen_renderBuffers[attr]->RenderRelease ();
    gen_renderBuffers[attr] = buffer;
  }

  void* RenderLock (iRenderBuffer* buffer, csGLRenderBufferLockType type, 
    GLenum& compGLType);
  void RenderRelease (iRenderBuffer* buffer);

/*  iRenderBuffer* vertattrib[16]; // @@@ Hardcoded max number of attributes
  bool vertattribenabled[16]; // @@@ Hardcoded max number of attributes
  bool vertattribenabled100[16]; // @@@ Hardcoded max number of attributes (for conventional)
 */ //iTextureHandle* texunit[16]; // @@@ Hardcoded max number of units
  bool texunitenabled[16]; // @@@ Hardcoded max number of units
  GLuint texunittarget[16]; // @@@ Hardcoded max number of units

  // Draw a 2D polygon (screen space coordinates) with correct Z information
  // given the plane. This function will not set up any texture mapping,
  // shading, or color.
  void Draw2DPolygon (csVector2* poly, int num_poly, const csPlane3& normal);

public:
  static csGLStateCache* statecache;
  static csGLExtensionManager* ext;

  SCF_DECLARE_IBASE;

  csGLGraphics3D (iBase *parent);
  virtual ~csGLGraphics3D ();

  iStringSet* GetStrings () { return strings; }

  ////////////////////////////////////////////////////////////////////
  //                            iGraphics3D
  ////////////////////////////////////////////////////////////////////

  /// Open 3d renderer.
  bool Open ();

  /// Close renderer and release all resources used
  void Close ();

  /// Get a pointer to our 2d canvas driver. NOTE: It's not increfed,
  /// and therefore it shouldn't be decref-ed by caller.
  iGraphics2D* GetDriver2D () 
    { return G2D; }

  /// Get a pointer to our texture manager
  iTextureManager* GetTextureManager () 
    { return (iTextureManager*)((csGLTextureManager*)txtmgr); }

  void SetMixMode (uint mode);
  void SetAlphaType (csAlphaMode::AlphaType alphaType);
  void SetGlOrtho (bool inverted);
  float GetAspect () const { return aspect; }

  /// Activate a vertex buffer
  bool ActivateBuffers (csRenderBufferHolder* holder, 
    csRenderBufferName mapping[CS_VATTRIB_SPECIFIC_LAST+1]);
  bool ActivateBuffers (csVertexAttrib *attribs,
    iRenderBuffer **buffers, unsigned int count);
  void DeactivateBuffers (csVertexAttrib *attribs, unsigned int count);

  /// Activate a texture
  bool ActivateTexture (iTextureHandle *txthandle, int unit = 0);
  /// Activate a texture (Should probably handled some better way)
  void DeactivateTexture (int unit = 0);
  virtual void SetTextureState (int* units, iTextureHandle** textures,
  	int count);

  /// Set dimensions of window
  void SetDimensions (int width, int height)
  { viewwidth = width; viewheight = height; }
  
  /// Get width of window
  int GetWidth () const
  { return viewwidth; }
  
  /// Get height of window
  int GetHeight () const
  { return viewheight; }

  /// Capabilities of the driver
  const csGraphics3DCaps* GetCaps() const
  { return &rendercaps; }

  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y)
  {
    asp_center_x = x;
    asp_center_y = y;
    frustum_valid = false;
    needProjectionUpdate = true;
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
    csGLGraphics3D::aspect = aspect;
    inv_aspect = 1.0f / aspect;
    frustum_valid = false;
    needProjectionUpdate = true;
  }

  /// Get perspective aspect.
  virtual float GetPerspectiveAspect () const
  {
    return aspect;
  }

  /// Set the z buffer write/test mode
  virtual void SetZMode (csZBufMode mode)
  {
    current_zmode = mode;
    SetZModeInternal (mode);
  }
  
  virtual csZBufMode GetZMode ()
  { return current_zmode; }
  
  /// Set object to view transform
  void SetObjectToCameraInternal (const csReversibleTransform& wvmatrix);
  virtual void SetObjectToCamera (csReversibleTransform*)
  {
    CS_ASSERT (false);	// Don't use with NR!
  }
  virtual const csReversibleTransform& GetObjectToCamera ();
  virtual void SetWorldToCamera (csReversibleTransform* wvmatrix) {}

  /// Set the current render target (0 for screen).
  virtual void SetRenderTarget (iTextureHandle* handle,
	  bool persistent = false)
  {
    render_target = handle;
    rt_onscreen = !persistent;
    rt_cliprectset = false;

    int hasRenderTarget = (handle != 0) ? 1 : 0;
    G2D->PerformExtension ("userendertarget", hasRenderTarget);
  }

  /// Get the current render target (0 for screen).
  virtual iTextureHandle* GetRenderTarget () const
  {
    return render_target;
  }

  /// Begin drawing in the renderer
  bool BeginDraw (int drawflags);

  /// Indicate that drawing is finished
  void FinishDraw ();

  /**
   * Prepare as render target. This basically makes sure
   * mipmapping will be ok for this texture. This function does
   * nothing if it has already been called before.
   */
  void PrepareAsRenderTarget (csGLTextureHandle* tex_mm);

  /// Do backbuffer printing
  void Print (csRect const* area);

  /// Drawroutine. Only way to draw stuff
  void DrawMesh (const csCoreRenderMesh* mymesh,
    const csRenderMeshModes& modes,
    const csArray< csArray<csShaderVariable*> > &stacks);

  /// Draw a 2D sprite
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha);

  /// Set the masking of color and/or alpha values to framebuffer
  virtual void SetWriteMask (bool red, bool green, bool blue, bool alpha)
  { 
    statecache->SetColorMask (red, green, blue, alpha);
  }

  virtual void GetWriteMask (bool &red, bool &green, bool &blue,
  	bool &alpha) const
  {
    GLboolean r, g, b, a;
    statecache->GetColorMask (r, g, b, a);
    red = r; green = g; blue = b; alpha = a;
  }

  /// Controls shadow drawing
  virtual void SetShadowState (int state);

  /// Enter a new clipped portal. Basically this routine will restrict
  virtual void OpenPortal (size_t numVertices, const csVector2* vertices,
    const csPlane3& normal, bool floating);

  /// Close a portal previously opened with OpenPortal().
  virtual void ClosePortal (bool use_zfill_portal);

  void SetupClipPortals ();

  /// Draw a line
  virtual void DrawLine(const csVector3 & v1,
    const csVector3 & v2, float fov, int color);

  /**
   * Set optional clipper to use. If clipper == null
   * then there is no clipper.
   * Currently only used by DrawTriangleMesh.
   */
  void SetClipper (iClipper2D* clipper, int cliptype);

  /// Get clipper that was used.
  iClipper2D* GetClipper ()
  { return clipper; }

  /// Return type of clipper.
  int GetClipType () const
  { return cliptype; }

  /// Set near clip plane.
  virtual void SetNearPlane (const csPlane3& pl)
  {
    do_near_plane = true;
    near_plane = pl;
  }

  /// Reset near clip plane (i.e. disable it).
  virtual void ResetNearPlane () 
  { do_near_plane = false; }

  /// Get near clip plane.
  virtual const csPlane3& GetNearPlane () const
  { return near_plane; }

  /// Return true if we have near plane.
  virtual bool HasNearPlane () const
  { return do_near_plane; }

  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op) const;
  virtual bool SetOption (const char*, const char*);

  virtual csPtr<iPolygonRenderer> CreatePolygonRenderer ();
  virtual void DrawSimpleMesh (const csSimpleRenderMesh& mesh, 
    uint flags = 0);

  virtual iHalo* CreateHalo (float, float, float,
    unsigned char *, int, int);
  void RemoveHalo (csOpenGLHalo* halo);
  virtual float GetZBuffValue (int, int);

  virtual void RemoveFromCache (iRendererLightmap*) { }  
  virtual bool IsLightmapOK (int, int, int) { return true; }
  //=========================================================================


  ////////////////////////////////////////////////////////////////////
  //                         iShaderRenderInterface
  ////////////////////////////////////////////////////////////////////

  class eiShaderRenderInterface : public iShaderRenderInterface
  {
  private:
    iObjectRegistry* object_reg;
  public:
    SCF_DECLARE_EMBEDDED_IBASE(csGLGraphics3D);
    eiShaderRenderInterface();
    virtual ~eiShaderRenderInterface();

    virtual void Initialize( iObjectRegistry *reg);


    virtual void* GetPrivateObject(const char* name);
  } scfiShaderRenderInterface;
  friend class eiShaderRenderInterface;

  ////////////////////////////////////////////////////////////////////
  //                          iComponent
  ////////////////////////////////////////////////////////////////////

  bool Initialize (iObjectRegistry* reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGLGraphics3D);
    virtual bool Initialize (iObjectRegistry* reg)
      { return scfParent->Initialize (reg); }
  } scfiComponent;
	
  ////////////////////////////////////////////////////////////////////
  //                         iEventHandler
  ////////////////////////////////////////////////////////////////////
  
  bool HandleEvent (iEvent& Event);

  struct EventHandler : public iEventHandler
  {
  private:
    csGLGraphics3D* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csGLGraphics3D* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& ev) 
      { return parent->HandleEvent (ev); }
  };
  csRef<EventHandler> scfiEventHandler;

  ////////////////////////////////////////////////////////////////////
  //                          iDebugHelper
  ////////////////////////////////////////////////////////////////////
  /// Execute a debug command.
  virtual bool DebugCommand (const char* cmd);
  void DumpZBuffer (const char* path);

  struct eiDebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGLGraphics3D);
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

#endif // __CS_GL_RENDER3D_H__

