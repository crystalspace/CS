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

struct iObjectRegistry;
struct iGraphics2D;
struct iTextureManager;
struct iRenderBufferManager;
struct iLightingManager;
struct iEffectDefinition;
struct iEffectTechnique;
class csRender3dCaps;

class csGLRender3D : public iRender3D
{
private:
  iObjectRegistry* object_reg;
  csRef<iGraphics2D> g2d;
  csRef<iTextureManager> texturemgr;
  csRef<iRenderBufferManager> buffermgr;
  csRef<iLightingManager> lightmgr;
  csRef<iEffectServer> effectserver;

  float fov;
  int viewwidth, viewheight;

  csRender3dCaps rendercaps;

  csStringHash strings;
  
public:
  SCF_DECLARE_IBASE;

  csGLRender3D (iBase *parent);
  ~csGLRender3D ();

  ////////////////////////////////////////////////////////////////////
  //                            iRender3d
  ////////////////////////////////////////////////////////////////////

  /// Open 3d renderer.
  bool Open();

  /// Close renderer and release all resources used
  void Close();

  /**
   * Get a pointer to our 2d canvas driver. NOTE: It's not increfed,
   * and therefore it shouldn't be decref-ed by caller.
   */
  iGraphics2d* Get2DDriver() 
    { return g2d; }

  /// Get a pointer to our texture manager
  iTextureManager* GetTextureManager() 
    { return texturemgr; }

  /**
   * Get a pointer to the VB-manager
   * Always use the manager referenced here to get VBs
   */
  iRenderBufferManager* GetBufferManager() 
    { return buffermgr; }

  /// Get a pointer to lighting manager
  iLightingManager* GetLightingManager() 
    { return lightmgr; }

  /// Dimensions of window
  void SetDimension(int width, int height);
  void GetDimension(int &width, int &height) 
    { width = viewwidth; height = viewheight; }

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
  csStringHash *GetStringContainer() 
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

  bool Initialize (iObjectRegistry reg);

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
