/*
    Copyright (C) 2002 by Mårten Svanfeldt
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

#ifndef __GL_RENDER3D_H__
#define __GL_RENDER3D_H__


#include "csgeom/csrect.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"

#include "csutil/cfgacc.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/strset.h"

#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "ivideo/graph2d.h"
#include "ivideo/render3d.h"
#include "ivideo/effects/efclient.h"

#include "glextmanager.h"

class csGLTextureCache;
class csGLTextureHandle;
class csGLTextureManager;

struct iObjectRegistry;
struct iTextureManager;
struct iRenderBufferManager;
struct iLightingManager;

struct iEffectServer;
struct iEffectDefinition;
struct iEffectTechnique;
struct iEvent;


class csGLRender3D : public iRender3D
{
private:
friend csGLTextureHandle;
friend csGLTextureCache;
friend csGLTextureManager;

  csRef<iObjectRegistry> object_reg;
  csRef<iGraphics2D> G2D;
  csRef<iRenderBufferManager> buffermgr;
  csRef<iLightingManager> lightmgr;
  csRef<iEffectServer> effectserver;

  csGLExtensionManager ext;
  csGLTextureCache *txtcache;
  csGLTextureManager *txtmgr;
  
  int current_drawflags;

  float fov;
  int viewwidth, viewheight;

  csRender3dCaps rendercaps;

  csStringSet* strings;

  csConfigAccess config;

  ////////////////////////////////////////////////////////////////////
  //                         Private helpers
  ////////////////////////////////////////////////////////////////////
	
  void Report (int severity, const char* msg, ...);

public:
  SCF_DECLARE_IBASE;

  csGLRender3D (iBase *parent);
  virtual ~csGLRender3D ();

  int GetMaxTextureSize ();

  ////////////////////////////////////////////////////////////////////
  //                            iRender3d
  ////////////////////////////////////////////////////////////////////

  /// Open 3d renderer.
  bool Open();

  /// Close renderer and release all resources used
  void Close();

  /// Get a pointer to our 2d canvas driver. NOTE: It's not increfed,
  /// and therefore it shouldn't be decref-ed by caller.
  iGraphics2D* GetDriver2D() 
    { return G2D; }

  /// Get a pointer to our texture manager
  iTextureManager* GetTextureManager() 
    { return (iTextureManager*)txtmgr; }

  /**
   * Get a pointer to the VB-manager
   * Always use the manager referenced here to get VBs
   */
  iRenderBufferManager* GetBufferManager() 
    { return buffermgr; }

  /// Get a pointer to lighting manager
  iLightingManager* GetLightingManager() 
    { return lightmgr; }

  /// Set dimensions of window
  void SetDimensions (int width, int height)
    { viewwidth = width; viewheight = height; }
  /// Get width of window
  int GetWidth () 
    { return viewwidth; }
  /// Get height of window
  int GetHeight () 
    { return viewheight; }

  /// Capabilities of the driver
  csRender3dCaps* GetCaps() 
    { return &rendercaps; }

  /// Field of view
  void SetFOV(float fov);
  float GetFOV() 
    { return fov; }

  /// Set world to view transform
  void SetWVMatrix(csReversibleTransform* wvmatrix);
  csReversibleTransform* GetWVMatrix();

  /// Begin drawing in the renderer
  bool BeginDraw(int drawflags);

  /// Indicate that drawing is finished
  void FinishDraw();

  /// Do backbuffer printing
  void Print(csRect* area);

  /// Drawroutine. Only way to draw stuff
  void DrawMesh(csRenderMesh* mymesh);

  /// Get a stringhash to be used by our streamsources etc.
  csStringSet *GetStringContainer() 
    { return strings; }

  ////////////////////////////////////////////////////////////////////
  //                         iEffectClient
  ////////////////////////////////////////////////////////////////////

  bool Validate (iEffectDefinition* effect, iEffectTechnique* technique);

  struct eiEffectClient : public iEffectClient
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGLRender3D);
    virtual bool Validate (
      iEffectDefinition* effect, iEffectTechnique* technique)
      { return scfParent->Validate (effect, technique); }
  } scfiEffectClient;

  ////////////////////////////////////////////////////////////////////
  //                          iComponent
  ////////////////////////////////////////////////////////////////////

  bool Initialize (iObjectRegistry* reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGLRender3D);
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
    csGLRender3D* parent;
  public:
    EventHandler (csGLRender3D* parent)
    {
      SCF_CONSTRUCT_IBASE (NULL);
      EventHandler::parent = parent;
    }
    
    SCF_DECLARE_IBASE;
    virtual bool HandleEvent (iEvent& ev) 
      { return parent->HandleEvent (ev); }
  } * scfiEventHandler;

};

#endif // __GL_RENDER3D_H__

