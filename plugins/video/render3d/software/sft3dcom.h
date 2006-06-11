/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Copyright (C) 2003 by Anders Stenberg
              (C) 2005 by Marten Svanfeldt

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

#ifndef __CS_SFTR3DCOM_H__
#define __CS_SFTR3DCOM_H__

#include "csutil/scf.h"

#include "csgeom/plane3.h"
#include "csgeom/transfrm.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/shadervarcontext.h"
#include "csplugincommon/softshader/renderinterface.h"
#include "csutil/cfgacc.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/strset.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/halo.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/softshader/defaultshader.h"

#include "soft_txt.h"

struct iConfigFile;
struct iPolygonBuffer;

struct csTriangle;

#include "polyrast.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

// Forward declaration
class csSoftwareGraphics3DCommon;

#define VATTR_SPEC(x)           (CS_VATTRIB_ ## x - CS_VATTRIB_SPECIFIC_FIRST)
#define VATTR_GEN(x)							      \
  ((CS_VATTRIB_ ## x - CS_VATTRIB_GENERIC_FIRST) + CS_VATTRIB_SPECIFIC_LAST + 1)

static const size_t activeBufferCount = CS_VATTRIB_SPECIFIC_LAST - 
  CS_VATTRIB_SPECIFIC_FIRST + 1;
static const size_t activeTextureCount = 4;

struct iTriangleDrawer
{
  virtual ~iTriangleDrawer() {}
  virtual void DrawMesh (iRenderBuffer* activebuffers[], 
    size_t rangeStart, size_t rangeEnd, 
    const csCoreRenderMesh* mesh, 
    const iScanlineRenderer::RenderInfoMesh& scanRenderInfoMesh,
    const csRenderMeshType meshtype, uint8* tri, const uint8* triEnd,
    csRenderBufferComponentType compType) = 0;
};

struct iPixTypeSpecifica
{
  virtual ~iPixTypeSpecifica() {}
  virtual void DrawPixmap (csSoftwareGraphics3DCommon* G3D, 
    iTextureHandle *hTex, int sx, int sy, int sw, int sh, 
    int tx, int ty, int tw, int th, 
    uint8 Alpha) = 0;
  virtual void BlitScreenToTexture (uint8** line_table, int txt_w, int txt_h,
    int scr_w, int scr_h, uint32* bitmap) = 0;
  virtual void BlitTextureToScreen (uint8** line_table, int txt_w, int txt_h,
    int scr_w, int scr_h, uint32* bitmap) = 0;
};

/**
 * The basic software renderer class.
 */
class csSoftwareGraphics3DCommon : 
  public scfImplementation3<csSoftwareGraphics3DCommon, 
			    iGraphics3D,
			    iComponent,
			    iSoftShaderRenderInterface>
{
protected:
  //friend class csSoftHalo;
  template<typename Pix, typename SrcBlend, typename DstBlend> 
  friend class TriangleDrawer;
  friend class TriangleDrawerCommon;
  friend class csSoftwareTexture;
  template<typename Pix>
  friend class Specifica;

  /// Driver this driver is sharing info with (if any)
  csSoftwareGraphics3DCommon *partner;

  /// Current render target.
  csRef<iTextureHandle> render_target;
  /// If true then the current render target has been put on screen.
  bool rt_onscreen;
  /// If true then we have set the old clip rect.
  bool rt_cliprectset;
  /// Old clip rect to restore after rendering on a proc texture.
  int rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy;
  int oldIlaceMode;
  uint8* ilaceSaveBuf;
  size_t ilaceSaveBufSize;
  bool ilaceRestore;

  /// Z buffer.
  uint32* z_buffer;
  /// Size of Z buffer.
  long z_buf_size;
  csZBufMode zBufMode;

  /**
   * Addresses of all lines for this frame. This table is used to avoid
   * calls to GetPixelAt in the main renderer loop. It is filled every frame
   * (by BeginDraw()) because double buffering or other stuff can change
   * the addresses.
   */
  uint8** line_table;

  /// If true then really rendering with a smaller size inside a larger window.
  bool do_smaller_rendering;
  bool smallerActive;

  /// Buffer for smaller rendering.
  unsigned char* smaller_buffer;

  /**
   * Number of bytes for every pixel (expressed in shifts). Also used in
   * combination with line_table to calculate the in-screen offset.
   */
  int pixel_shift;

  /// Width of display.
  int display_width;
  /// Height of display.
  int display_height;

  /// pseudo width of display.
  int width;
  /// pseudo height of display.
  int height;
  /// Perspective center X.
  int persp_center_x;
  /// Perspective center X.
  int persp_center_y;
  /// The pixel format of display
  csPixelFormat pfmt;
  bool pixelBGR;
  /// Current transformation from world to camera.
  csReversibleTransform w2c;

  /// Current 2D clipper.
  csRef<iClipper2D> clipper;
  csRef<iClipper2D> userClipper;
  bool clipperDirty;
  csRect lastClipBox;
  /// Clipper type.
  int cliptype;
  /// Current near plane.
  csPlane3 near_plane;
  /// Is near plane used?
  bool do_near_plane;

  // for simple mesh drawing
  uint scrapIndicesSize;
  csRef<iRenderBuffer> scrapIndices;
  uint scrapVerticesSize;
  csRef<iRenderBuffer> scrapVertices;
  csRef<iRenderBuffer> scrapTexcoords;
  csRef<iRenderBuffer> scrapColors;
  csShaderVariableContext scrapContext;

  /// Current aspect ratio for perspective correction.
  float aspect;
  /// Current inverse aspect ratio for perspective correction.
  float inv_aspect;

  /// Mipmap selection coefficient (normal == 1.0)
  float mipmap_coef;

  /// Do we want mipmaps?
  int rstate_mipmap;

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

  csRef<iStringSet> strings;
  csStringID string_world2camera;
  csStringID string_indices;

  csRef<iShaderManager> shadermgr;

  iRenderBuffer* activebuffers[activeBufferCount];
  csSoftwareTextureHandle* activeSoftTex[activeTextureCount]; 
  csRef<iRenderBuffer> translatedVerts;
  csRef<csRenderBuffer> processedColors[2];
  bool processedColorsFlag[2];
  /// The perspective corrected vertices.
  csDirtyAccessArray<csVector3> persp;

  csRef<iScanlineRenderer> scanlineRenderer;
  csRef<iScanlineRenderer> defaultRenderer;
  csRef<iDefaultScanlineRenderer> defaultRendererState;
  /**
   * Helper class to change the scanline renderer to the default one if none
   * other was set.
   */
  class ScanlineRendererHelper
  {
    csSoftwareGraphics3DCommon* theG3D;
    bool unsetRenderer;
  public:
    ScanlineRendererHelper (csSoftwareGraphics3DCommon* G3D) : theG3D (G3D)
    {
      if (!theG3D->scanlineRenderer.IsValid())
      {
	theG3D->scanlineRenderer = theG3D->defaultRenderer;
	theG3D->defaultRendererState->SetFlatColor (csVector4 (1, 1, 1, 1));
	theG3D->defaultRendererState->SetShift (0, 0);
	theG3D->defaultRendererState->SetColorSum (false);
	unsetRenderer = true;
      }
      else
	unsetRenderer = false;
    }
    ~ScanlineRendererHelper()
    {
      if (unsetRenderer) theG3D->scanlineRenderer = 0;
    }
  };

  // Structure used for maintaining a stack of clipper portals.
  struct csClipPortal
  {
    csVector2* poly;
    int num_poly;
    csPlane3 normal;
    csFlags flags;
    csClipPortal () : poly (0) { }
    ~csClipPortal () { delete[] poly; }
  };
  csPDelArray<csClipPortal> clipportal_stack;
  bool clipportal_dirty;
  int clipportal_floating;

  void SetupSpecifica();
  void FlushSmallBufferToScreen();
  void SetupClipper();
public:
  /// Report
  void Report (int severity, const char* msg, ...);

  /**
   * Low-level 2D graphics layer.
   * csSoftwareGraphics3DCommon is in charge of creating and managing this.
   */
  csRef<iGraphics2D> G2D;

  /// The configuration file
  csConfigAccess config;

  /// The texture manager.
  csSoftwareTextureManager* texman;

  /// The System interface.
  iObjectRegistry* object_reg;

  /// Do interlacing? (-1 - no, 0/1 - yes)
  int do_interlaced;
  bool ilaceActive;

  /// Render capabilities
  csGraphics3DCaps Caps;

  PolygonRasterizer<SLLogic_ZFill> polyrast_ZFill;
  iTriangleDrawer* triDraw[CS_MIXMODE_FACT_COUNT*CS_MIXMODE_FACT_COUNT];
  /// Stuff that changes with the pixtype
  iPixTypeSpecifica* specifica;

  /// Setup scanline drawing routines according to current bpp and setup flags
  csSoftwareGraphics3DCommon (iBase* parent);
  /// Destructor.
  virtual ~csSoftwareGraphics3DCommon ();

  /**
   * Initialization method required by iComponent interface.
   * Sets System pointer.
   */
  virtual bool Initialize (iObjectRegistry*);

  iStringSet* GetStrings () const { return strings; }

  /**
   * Open or close our interface.
   */
  bool HandleEvent (iEvent& ev);

  /// Initialize new state from config file
  void NewInitialize ();

  /// Open a canvas.
  virtual bool Open ();

  /// Gathers all that has to be done when opening from scratch.
  bool NewOpen ();

  /**
   * Used when multiple contexts are in system, opens sharing information from
   * other driver.
   */
  bool SharedOpen ();

  /// Close.
  virtual void Close ();

  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height);

  /// Start a new frame (see CSDRAW_XXX bit flags)
  virtual bool BeginDraw (int DrawFlags);

  virtual void Print (csRect const* area);
  /// End the frame and do a page swap.
  virtual void FinishDraw ();

  /// Draw a line in camera space.
  virtual void DrawLine (const csVector3& v1, const csVector3& v2,
    float fov, int color);

  /// Get address of Z-buffer at specific point
  virtual uint32 *GetZBuffAt (int x, int y)
  { return z_buffer + x + y*width; }

  /// Dump the texture cache.
  virtual void DumpCache ();

  /// Clear the texture cache.
  virtual void ClearCache ();

  /// Remove texture from cache.
  virtual void RemoveFromCache (iRendererLightmap* rlm);

  /// Get drawing buffer width
  virtual int GetWidth ()
  { return width; }
  /// Get drawing buffer height
  virtual int GetHeight ()
  { return height; }

  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y)
  {
    persp_center_x = x;
    persp_center_y = y;
  }
  /// Get center of projection.
  virtual void GetPerspectiveCenter (int& x, int& y) const
  {
    x = persp_center_x;
    y = persp_center_y;
  }
  /// Set perspective aspect.
  virtual void SetPerspectiveAspect (float aspect)
  {
    this->aspect = aspect;
    inv_aspect = 1./aspect;
  }
  /// Get perspective aspect.
  virtual float GetPerspectiveAspect () const
  {
    return aspect;
  }
  virtual void SetWorldToCamera (const csReversibleTransform& w2c)
  {
    this->w2c = w2c;
    shadermgr->GetVariableAdd (string_world2camera)->SetValue (w2c);
  }
  virtual const csReversibleTransform& GetWorldToCamera ()
  { return w2c; }
  /// Set optional clipper.
  virtual void SetClipper (iClipper2D* clipper, int cliptype);
  /// Get clipper.
  virtual iClipper2D* GetClipper ()
  {
    return userClipper;
  }
  /// Get cliptype.
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


  /// Get the iGraphics2D driver.
  virtual iGraphics2D *GetDriver2D ()
  { return G2D; }

  /// Get the ITextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return texman; }

  /// Get the vertex buffer manager.

  virtual void SetRenderTarget (iTextureHandle* handle, bool persistent,
  	int subtexture);
  virtual iTextureHandle* GetRenderTarget () const { return render_target; }

  /// Get Z-buffer value at given X,Y position
  virtual float GetZBuffValue (int x, int y);

  /// Create a halo of the specified color and return a handle.
  /*virtual iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char *iAlpha, int iWidth, int iHeight);*/

  /**
   * Draw a sprite (possibly rescaled to given width (sw) and height (sh))
   * using given rectangle from given texture clipped with G2D's clipper.
   */
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw,
    int sh, int tx, int ty, int tw, int th, uint8 Alpha);

  static inline int ColorPreprocIndex (csVertexAttrib attrib)
  {
    if (attrib == CS_VATTRIB_PRIMARY_COLOR)
      return 0;
    else if (attrib == CS_VATTRIB_SECONDARY_COLOR)
      return 1;
    else
      return -1;
  }

  /// Activate a vertex buffer
  bool ActivateBuffer (csVertexAttrib attrib, iRenderBuffer* buffer)
  {
    if (!CS_VATTRIB_IS_SPECIFIC(attrib)) return false;
    activebuffers[attrib - CS_VATTRIB_SPECIFIC_FIRST] = buffer;
    int colindex;
    if ((colindex = ColorPreprocIndex (attrib)) != -1)
      processedColorsFlag[colindex] = false;
    return true;
  }

  /// Deactivate a vertex buffer
  void DeactivateBuffer (csVertexAttrib attrib)
  {
    if (!CS_VATTRIB_IS_SPECIFIC(attrib)) return;
    activebuffers[attrib - CS_VATTRIB_SPECIFIC_FIRST] = 0;
  }
  
  virtual bool ActivateBuffers (csRenderBufferHolder* holder, 
    csRenderBufferName mapping[CS_VATTRIB_SPECIFIC_LAST+1])
  {
    if (!holder) return false;

    iRenderBuffer* buffer = 0;
    unsigned int i = 0;
    for (i = 0; i < 16; i++)
    {
      buffer = holder->GetRenderBuffer (mapping[i]);
      ActivateBuffer ((csVertexAttrib)i, buffer);
    }
    return true;
  }


  virtual bool ActivateBuffers (csVertexAttrib *attribs,
    iRenderBuffer **buffers, unsigned int count)
  {
    unsigned int i;
    for (i = 0 ; i < count ; i++)
    {
      csVertexAttrib attrib = attribs[i];
      iRenderBuffer* buf = buffers[i];
      if (buf)
        ActivateBuffer (attrib, buf);
    }
    return true;
  }

  /**
  * Deactivate all given buffers.
  * If attribs is 0, all buffers are deactivated;
  */
  virtual void DeactivateBuffers (csVertexAttrib *attribs, unsigned int count)
  {
    unsigned int i;
    for (i = 0 ; i < count ; i++)
    {
      csVertexAttrib attrib = attribs[i];
      DeactivateBuffer (attrib);
    }
  }

  /// Activate a texture
  bool ActivateTexture (iTextureHandle *txthandle, int unit = 0)
  {
    if ((unit < 0) || ((uint)unit >= activeTextureCount)) return false;
    csSoftwareTextureHandle* softtex = (csSoftwareTextureHandle*)txthandle;
    activeSoftTex[unit] = softtex;
    return true;
  }

  virtual void SetTextureState (int* units, iTextureHandle** textures,
  	int count)
  {
    int i;
    for (i = 0 ; i < count ; i++)
    {
      int unit = units[i];
      iTextureHandle* txt = textures[i];
      if (txt)
        ActivateTexture (txt, unit);
      else
        DeactivateTexture (unit);
    }
  }

  /// Deactivate a texture
  void DeactivateTexture (int unit = 0)
  {
    if ((unit >= 0) && ((uint)unit < activeTextureCount))
    {
      activeSoftTex[unit] = 0;
    }
  }

  /// Get width of window
  int GetWidth () const
  { return display_width; }

  /// Get height of window
  int GetHeight () const
  { return display_height; }

  /// Capabilities of the driver
  const csGraphics3DCaps* GetCaps() const
  { return &Caps; }

  /// Set the z buffer write/test mode
  virtual void SetZMode (csZBufMode mode) 
  { zBufMode = mode; }
  virtual csZBufMode GetZMode ()
  { return zBufMode; }

  // Enables offsetting of Z values
  void EnableZOffset ()
  { 
  }

  // Disables offsetting of Z values
  void DisableZOffset ()
  {
  }

/*  csReversibleTransform& GetWorldToCamera ()
  {
    return w2c;
  }*/

  /// Set the masking of color and/or alpha values to framebuffer
  virtual void SetWriteMask (bool /*red*/, bool /*green*/, bool /*blue*/,
    bool /*alpha*/)
  { 
  }

  virtual void GetWriteMask (bool& /*red*/, bool& /*green*/, bool& /*blue*/,
    bool& /*alpha*/) const
  {
  }

  /// Drawroutine. Only way to draw stuff
  virtual void DrawMesh (const csCoreRenderMesh* mymesh,
    const csRenderMeshModes& modes,
    const iShaderVarStack* stacks);
  void DrawSimpleMesh (const csSimpleRenderMesh &mesh, uint flags = 0);

  bool PerformExtension (char const* /*command*/, ...) { return false; }
  bool PerformExtensionV (char const* /*command*/, va_list /*args*/)
  { return false; }

  /// Controls shadow drawing
  virtual void SetShadowState (int /*state*/)
  {
  }

  /// Get maximum number of simultaneous vertex lights supported
  virtual int GetMaxLights () const
  {
    return 0;
  }

  /// Sets a parameter for light i
  virtual void SetLightParameter (int /*i*/, int /*param*/, csVector3 /*value*/)
  {
  }

  /// Enables light i
  virtual void EnableLight (int /*i*/)
  {
  }

  /// Disables light i
  virtual void DisableLight (int /*i*/)
  {
  }

  /// Enable vertex lighting
  virtual void EnablePVL ()
  {
  }

  /// Disable vertex lighting
  virtual void DisablePVL ()
  {
  }

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION /*op*/, long /*val*/)
  {
    return 0;
  }

  /// Get a renderstate value.
  virtual long GetRenderState (G3D_RENDERSTATEOPTION /*op*/) const
  {
    return 0;
  }

  virtual bool SetOption (const char*, const char*)
  {
    return false;
  }

  virtual void OpenPortal (size_t, const csVector2*, const csPlane3&, csFlags);
  virtual void ClosePortal ();

  //=========================================================================
  // Below this line are all functions that are not yet implemented by
  // the new renderer or are not going to be implemented ever. In the
  // last case they will be removed as soon as we permanently switch
  // to the new renderer. @@@NR@@@
  //=========================================================================
  virtual iHalo *CreateHalo (float, float, float,
    unsigned char *, int, int) { return 0; }
  //=========================================================================

  struct EventHandler : public scfImplementation1<EventHandler, iEventHandler>
  {
  private:
    csSoftwareGraphics3DCommon* parent;
  public:
    EventHandler (csSoftwareGraphics3DCommon* parent) : 
	scfImplementationType (this)
    {
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
    }
    virtual bool HandleEvent (iEvent& ev)
    {
      return parent->HandleEvent (ev);
    }
    CS_EVENTHANDLER_NAMES("crystalspace.graphics3d")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  } * scfiEventHandler;

  /**\name iSoftShaderRenderInterface implementation
   * @{ */
  void SetScanlineRenderer (iScanlineRenderer* sr) { scanlineRenderer = sr; }
  /** @} */
};

}
CS_PLUGIN_NAMESPACE_END(Soft3D)

#endif // __CS_SFTR3DCOM_H__
