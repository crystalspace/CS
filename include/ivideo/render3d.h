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

#ifndef __IVIDEO_RENDER3D_H__
#define __IVIDEO_RENDER3D_H__

/**
 * New 3D Interface. Work in progress!
 */
struct iRender3D : public iBase
{
  /// Open 3d renderer.
  virtual bool Open() = 0;

  /// Close renderer and release all resources used
  virtual void Close() = 0;

  /**
   * Get a pointer to our 2d canvas driver. NOTE: It's not increfed,
   * and therefore it shouldn't be decref-ed by caller.
   */
  virtual iGraphic2d* Get2DDriver() = 0;

  /// Get a pointer to our texture manager
  virtual iTextureManager* GetTextureManager() = 0;

  /**
   * Get a pointer to the VB-manager
   * Always use the manager referenced here to get VBs
   */
  virtual iRenderBufferManager* GetBufferManager() = 0;

  /// Get a pointer to lighting manager
  virtual iLightingManager* GetLightingManager() = 0;

  /// Dimensions of window
  virtual void SetDimension(int width, int height) = 0;
  virtual void GetDimension(int &width, int &height) = 0;

  /// Capabilities of the driver
  virtual csRender3dCaps* GetCaps() = 0;

  /// Field of view
  virtual void SetFOV(float fov) = 0;
  virtual float GetFOV() = 0;

  /// Set world to view transform
  virtual void SetWVMatrix(csReversibleTransform* wvmatrix) = 0;
  virtual csReversibleTransform* GetWVMatrix() = 0;

  /// Begin drawing in the renderer
  virtual bool BeginDraw(int drawflags) = 0;

  /// Indicate that drawing is finished
  virtual void FinishDraw() = 0;

  /// Do backbuffer printing
  virtual void Print(csRect* area) = 0;

  /// Drawroutine. Only way to draw stuff
  virtual void DrawMesh(csRenderMesh* mymesh) = 0;

  /// Get a stringhash to be used by our streamsources etc.
  virtual csStringHash GetStringContainer() = 0;
};

#endif // __IVIDEO_RENDER3D_H__
