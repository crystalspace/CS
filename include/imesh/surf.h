/*
    Copyright (C) 2001 by Jorrit Tyberghein
  
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

#ifndef __IMESH_SURF_H__
#define __IMESH_SURF_H__

#include "csutil/scf.h"

class csVector3;
class csColor;

struct iMaterialWrapper;

SCF_VERSION (iSurfaceState, 0, 0, 1);

/**
 * This interface describes the API for the surface mesh object.
 * Using this you can set up the surface. This surface will always
 * be oriented along the x,y plane.
 */
struct iSurfaceState : public iBase
{
  /// Set number of subdivisions to use in both directions.
  virtual void SetResolution (int x, int y) = 0;
  /// Get the x resolution.
  virtual int GetXResolution () const = 0;
  /// Get the y resolution.
  virtual int GetYResolution () const = 0;

  /// Set the top left corner to use for the surface.
  virtual void SetTopLeftCorner (const csVector3& tl) = 0;
  /// Get the top left corner.
  virtual csVector3 GetTopLeftCorner () const = 0;

  /// Set the x and y scale to use for the surface.
  virtual void SetScale (float x, float y) = 0;
  /// Get the x scale.
  virtual float GetXScale () const = 0;
  /// Get the y scale.
  virtual float GetYScale () const = 0;

  /// Set material.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;

  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;

  /// Set lighting.
  virtual void SetLighting (bool l) = 0;
  /// Is lighting enabled.
  virtual bool IsLighting () const = 0;

  /// Set the global color to use. Will be added to the lighting values.
  virtual void SetColor (const csColor& col) = 0;
  /// Get the color.
  virtual csColor GetColor () const = 0;
};

#endif

