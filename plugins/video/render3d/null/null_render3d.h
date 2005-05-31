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
#include "ivideo/polyrender.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "csutil/cfgacc.h"
#include "csutil/weakref.h"
#include "null_txt.h"

struct iObjectRegistry;
struct iGraphics2D;
struct iShaderManager;
struct iBugPlug;

class csNullPolygonRenderer : public iPolygonRenderer
{
public:
  csNullPolygonRenderer ()
  {
    SCF_CONSTRUCT_IBASE (0);
  }
  virtual ~csNullPolygonRenderer ()
  {
    SCF_DESTRUCT_IBASE ();
  }
  SCF_DECLARE_IBASE;

  virtual void PrepareRenderMesh (csRenderMesh&) { }
  virtual void Clear () { }
  virtual void AddPolygon (csPolygonRenderData*, 
    iUserRenderBufferIterator*) { }
};

class csNullGraphics3D : public iGraphics3D
{
public:
  SCF_DECLARE_IBASE;

  csNullGraphics3D (iBase *parent);
  virtual ~csNullGraphics3D ();

  bool Initialize (iObjectRegistry* objreg);
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csNullGraphics3D);
    bool Initialize (iObjectRegistry* objreg)
    { return scfParent->Initialize (objreg); }
  } scfiComponent;

  struct eiShaderRenderInterface : public iShaderRenderInterface
  {
    SCF_DECLARE_EMBEDDED_IBASE(csNullGraphics3D);
    void Initialize (iObjectRegistry*) { }
    void* GetPrivateObject (const char *)
    { return 0; }
  } scfiShaderRenderInterface;

  bool HandleEvent (iEvent& Event);
  struct EventHandler : public iEventHandler
  {
  private:
    csNullGraphics3D* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csNullGraphics3D* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    virtual bool HandleEvent (iEvent& ev)
    { return parent->HandleEvent (ev); }
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
  void SetPerspectiveCenter (int x, int y) { cx = x; cy = y; }
  void GetPerspectiveCenter (int& x, int& y) const { x = cx, y = cy; }
  void SetPerspectiveAspect (float aspect) { a = aspect; }
  float GetPerspectiveAspect () const { return a; }
  void SetRenderTarget (iTextureHandle* handle, bool persistent = false);
  iTextureHandle* GetRenderTarget () const;
  bool BeginDraw (int DrawFlags);
  void FinishDraw ();
  void Print (csRect const *area);
  void DrawPixmap (iTextureHandle *htex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha = 0);
  void DrawLine (const csVector3& v1, const csVector3& v2,
    float fov, int color) { }
  void SetClipper (iClipper2D* clipper, int cliptype);
  iClipper2D* GetClipper ();
  int GetClipType () const;
  void SetNearPlane (const csPlane3& pl);
  void ResetNearPlane ();
  const csPlane3& GetNearPlane () const;
  bool HasNearPlane () const;
  bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  long GetRenderState (G3D_RENDERSTATEOPTION op) const;
  bool SetOption (const char*, const char*);
  bool ActivateBuffers (csRenderBufferHolder* holder, 
    csRenderBufferName mapping[CS_VATTRIB_SPECIFIC_LAST+1])
  {
    return true;
  }
  bool ActivateBuffers (csVertexAttrib *attribs,
    iRenderBuffer **buffers, unsigned int count)
  {
    return true;
  }
  virtual void DeactivateBuffers (csVertexAttrib *attribs, unsigned int count)
  {
  }
  void SetTextureState (int* units, iTextureHandle** textures, int count);
  void DrawMesh (const csCoreRenderMesh* mymesh,
    const csRenderMeshModes& modes,
    const csArray<csShaderVariable*> &stacks);
  void SetWriteMask (bool red, bool green, bool blue, bool alpha);
  void GetWriteMask (bool& red, bool& green, bool& blue, bool& alpha) const;
  void SetZMode (csZBufMode mode) { zmode = mode; }
  csZBufMode GetZMode () { return zmode; }
  void EnableZOffset ();
  void DisableZOffset ();
  void SetShadowState (int state);
  // OR functions, unimplemented
  uint32* GetZBuffAt (int x, int y) { return 0; }
  float GetZBuffValue (int x, int y) { return 0.0; };

  void OpenPortal (size_t, const csVector2*, const csPlane3&, bool) { }
  void ClosePortal (bool) { }
  iHalo* CreateHalo (float iR, float iG, float iB, unsigned char *iAlpha,
    int iWidth, int iHeight) { return 0; }

  void RemoveFromCache (iRendererLightmap* rlm) { }
  csPtr<iPolygonRenderer> CreatePolygonRenderer ();
  void SetWorldToCamera (const csReversibleTransform& w2c) { }
  void DrawSimpleMesh (const csSimpleRenderMesh& mesh, uint flags = 0) { }
private:
  iObjectRegistry* object_reg;
  csRef<iGraphics2D> G2D;
  csRef<iShaderManager> shadermgr;
  csRef<iStringSet> strings;
  csWeakRef<iBugPlug> bugplug;
  csRef<csTextureManagerNull> txtmgr;

  csConfigAccess config;

  csStringID string_vertices;
  csStringID string_texture_coordinates;
  csStringID string_normals;
  csStringID string_colors;
  csStringID string_indices;

  csGraphics3DCaps Caps;
  int w, h;
  int cx, cy;
  float a;
  csReversibleTransform other2cam;

  int current_drawflags;

  iClipper2D* clipper;
  int cliptype;

  bool do_near_plane;
  csPlane3 near_plane;
  csZBufMode zmode;

  csPixelFormat pfmt;
  bool red_mask, green_mask, blue_mask, alpha_mask;
};

#endif // __CS_NULL_RENDER3D_H__
