/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef __CS_NULL_RENDER3D_H__
#define __CS_NULL_RENDER3D_H__

#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"

#include "csutil/cfgacc.h"
#include "csutil/flags.h"
#include "csutil/weakref.h"
#include "csutil/scf_implementation.h"
#include "csgeom/matrix4.h"
#include "csgeom/plane3.h"

#include "null_txt.h"

struct iObjectRegistry;
struct iGraphics2D;
struct iShaderManager;
struct iBugPlug;

// To silence EnableZOffset/DisableZOffset
#include "csutil/deprecated_warn_off.h"

class csNullGraphics3D : public scfImplementation2<csNullGraphics3D, 
						   iGraphics3D,
						   iComponent>
{
public:
  csNullGraphics3D (iBase *parent);
  virtual ~csNullGraphics3D ();

  bool Initialize (iObjectRegistry* objreg);

  bool HandleEvent (iEvent& Event);
  struct EventHandler : public scfImplementation1<EventHandler,
						  iEventHandler>
  {
  private:
    csNullGraphics3D* parent;
  public:
    EventHandler (csNullGraphics3D* parent) : scfImplementationType (this)
    {
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
    }
    virtual bool HandleEvent (iEvent& ev)
    { return parent->HandleEvent (ev); }
    CS_EVENTHANDLER_NAMES("crystalspace.graphics3d")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  };
  csRef<EventHandler> scfiEventHandler;

  bool Open ();
  void Close ();
  iGraphics2D *GetDriver2D () { return G2D; }
  iTextureManager *GetTextureManager () { return txtmgr; };
  void SetDimensions (int width, int height) { w = width, h = height; }
  int GetWidth () const { return w; }
  int GetHeight () const { return h; }
  const csGraphics3DCaps *GetCaps () const { return &Caps; }
  void SetPerspectiveCenter (int x, int y)
  { cx = x; cy = y; explicitProjection = false; }
  void GetPerspectiveCenter (int& x, int& y) const { x = cx, y = cy; }
  void SetPerspectiveAspect (float aspect)
  { a = aspect; explicitProjection = false; }
  float GetPerspectiveAspect () const{ return a; }
  const CS::Math::Matrix4& GetProjectionMatrix()
  {
    if (!explicitProjection && needMatrixUpdate) ComputeProjectionMatrix();
    return projectionMatrix;
  }
  void SetProjectionMatrix (const CS::Math::Matrix4& m)
  {
    projectionMatrix = m;
    explicitProjection = true;
  }
  
  bool SetRenderTarget (iTextureHandle* handle,	bool persistent = false,
    int subtexture = 0,	csRenderTargetAttachment attachment = rtaColor0);
  bool ValidateRenderTargets() { return true; }
  bool CanSetRenderTarget (const char* format,
    csRenderTargetAttachment attachment = rtaColor0);
  iTextureHandle* GetRenderTarget (csRenderTargetAttachment attachment = rtaColor0,
    int* subtexture = 0) const;
  void UnsetRenderTargets();
  void CopyFromRenderTargets (size_t num, 
    csRenderTargetAttachment* attachments,
    iTextureHandle** textures,
    int* subtextures = 0) {}
  
  bool BeginDraw (int DrawFlags);
  void FinishDraw ();
  int GetCurrentDrawFlags() const
  { return current_drawflags; }
  void Print (csRect const *area);
  void DrawPixmap (iTextureHandle *htex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha = 0);
  void DrawLine (const csVector3& /*v1*/, const csVector3& /*v2*/,
    float /*fov*/, int /*color*/) { }
  void SetClipper (iClipper2D* clipper, int cliptype);
  iClipper2D* GetClipper ();
  int GetClipType () const;
  void SetNearPlane (const csPlane3& pl);
  void ResetNearPlane ();
  const csPlane3& GetNearPlane () const;
  bool HasNearPlane () const;
  bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  long GetRenderState (G3D_RENDERSTATEOPTION op) const;
  void SetEdgeDrawing (bool flag) {}
  bool GetEdgeDrawing () { return false; }
  bool SetOption (const char*, const char*);
  bool ActivateBuffers (csRenderBufferHolder* /*holder*/, 
    csRenderBufferName mapping[CS_VATTRIB_SPECIFIC_LAST+1])
  {
    (void)mapping;
    return true;
  }
  bool ActivateBuffers (csVertexAttrib* /*attribs*/,
    iRenderBuffer** /*buffers*/, unsigned int /*count*/)
  {
    return true;
  }
  virtual void DeactivateBuffers (csVertexAttrib* /*attribs*/,
    unsigned int /*count*/)
  {
  }
  void SetTextureState (int* units, iTextureHandle** textures, int count);
  void SetTextureComparisonModes (int*, CS::Graphics::TextureComparisonMode*,
    int) {}
  void DrawMesh (const CS::Graphics::CoreRenderMesh* mymesh,
    const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack);
  void SetWriteMask (bool red, bool green, bool blue, bool alpha);
  void GetWriteMask (bool& red, bool& green, bool& blue, bool& alpha) const;
  void SetZMode (csZBufMode mode) { zmode = mode; }
  csZBufMode GetZMode () { return zmode; }
  void EnableZOffset ();
  void DisableZOffset ();
  void SetShadowState (int state);
  // OR functions, unimplemented
  uint32* GetZBuffAt (int /*x*/, int /*y*/) { return 0; }
  float GetZBuffValue (int /*x*/, int /*y*/) { return 0.0; };

  void OpenPortal (size_t, const csVector2*, const csPlane3&, csFlags) { }
  void ClosePortal () { }
  iHalo* CreateHalo (float /*iR*/, float /*iG*/, float /*iB*/,
    unsigned char* /*iAlpha*/, int /*iWidth*/, int /*iHeight*/) { return 0; }

  void SetWorldToCamera (const csReversibleTransform& w2c) { this->w2c = w2c; }
  const csReversibleTransform& GetWorldToCamera () { return w2c; }
  void DrawSimpleMesh (const csSimpleRenderMesh& /*mesh*/, uint /*flags*/ = 0) { }
  void DrawSimpleMeshes (const csSimpleRenderMesh* /*meshes*/,
    size_t /*numMeshes*/, uint /*flags*/ = 0) { }

  bool PerformExtension (char const* /*command*/, ...) { return false; }
  bool PerformExtensionV (char const* /*command*/, va_list /*args*/)
  { return false; }

  /**
   * Initialise a set of occlusion queries.
   */
  void OQInitQueries (unsigned int *queries, int num_queries) {}

  void OQDelQueries (unsigned int *queries, int num_queries) {}

  /**
   * Returns whether an occlusion query has finished.
   */
  bool OQueryFinished (unsigned int occlusion_query) { return true; }

  /**
   * Check via occlusion query whether a mesh is visible.
   */
  bool OQIsVisible (unsigned int occlusion_query, unsigned int sampleLimit) { return true; }
  void OQBeginQuery (unsigned int occlusion_query) {}
  void OQEndQuery () {}

  void DrawMeshBasic(const CS::Graphics::CoreRenderMesh* mymesh,
    const CS::Graphics::RenderMeshModes& modes) {}

  void SetTessellation (bool flag) {}
  bool GetTessellation () { return false; }


private:
  iObjectRegistry* object_reg;
  csRef<iGraphics2D> G2D;
  csRef<iShaderManager> shadermgr;
  csRef<iShaderVarStringSet> strings;
  csWeakRef<iBugPlug> bugplug;
  csRef<csTextureManagerNull> txtmgr;

  csConfigAccess config;

  CS::ShaderVarStringID string_vertices;
  CS::ShaderVarStringID string_texture_coordinates;
  CS::ShaderVarStringID string_normals;
  CS::ShaderVarStringID string_colors;
  CS::ShaderVarStringID string_indices;

  csGraphics3DCaps Caps;
  int w, h;
  int cx, cy;
  float a;
  csReversibleTransform w2c;
  CS::Math::Matrix4 projectionMatrix;
  bool explicitProjection, needMatrixUpdate;

  int current_drawflags;

  iClipper2D* clipper;
  int cliptype;

  bool do_near_plane;
  csPlane3 near_plane;
  csZBufMode zmode;

  bool red_mask, green_mask, blue_mask, alpha_mask;
  
  enum { numTargets = 2 };
  csRef<iTextureHandle> render_targets[numTargets];
  int rt_subtex[numTargets];
  
  void ComputeProjectionMatrix();
};

// To silence EnableZOffset/DisableZOffset
#include "csutil/deprecated_warn_on.h"

#endif // __CS_NULL_RENDER3D_H__
