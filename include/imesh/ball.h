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

#ifndef __CS_IMESH_BALL_H__
#define __CS_IMESH_BALL_H__

/**\file
 * Ball mesh object
 */

#include "csutil/scf.h"

class csVector3;
class csColor;

struct iMaterialWrapper;

/**\addtogroup meshplugins
 * @{ */

SCF_VERSION (iBallState, 0, 0, 4);

/**
 * This interface describes the API for the ball mesh object.
 * Using this you can set up the ball to whatever (ball) shape you
 * want it to have and the appearance. The ball plugin implements
 * this interface in addition to iMeshObject.
 * 
 * Main creators of instances implementing this interface:
 * - Ball mesh object plugin (crystalspace.mesh.object.ball)
 * - iMeshObjectFactory::NewInstance()
 * 
 * Main ways to get pointers to this interface:
 * - scfQueryInterface() on iMeshWrapper::GetMeshObject()
 * 
 * Main users of this interface:
 * - Ball Loader plugin (crystalspace.mesh.loader.ball)
 * - Ball Loader plugin (crystalspace.mesh.loader.factory.ball)
 */
struct iBallState : public iBase
{
  /// Set x, y, and z radius of ball.
  virtual void SetRadius (float radx, float rady, float radz) = 0;
  /// Get radius x, y and z
  virtual void GetRadius (float& radx, float& rady, float& radz) const = 0;
  /// Set shift of ball.
  virtual void SetShift (float shiftx, float shifty, float shiftz) = 0;
  /// Get shift x, y and z
  virtual const csVector3& GetShift () const = 0;
  /// Set number of vertices on outer circle of ball.
  virtual void SetRimVertices (int num) = 0;
  /// Get number of vertices on outer circle of ball.
  virtual int GetRimVertices () const = 0;
  /// Set material of ball.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of ball.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;
  /// Set reversed mode (i.e. sphere visible from inside out).
  virtual void SetReversed (bool r) = 0;
  /// Get reversed mode.
  virtual bool IsReversed () const = 0;
  /// Only show top half.
  virtual void SetTopOnly (bool t) = 0;
  /// Only top half.
  virtual bool IsTopOnly () const = 0;
  /// Set lighting.
  virtual void SetLighting (bool l) = 0;
  /// Is lighting enabled.
  virtual bool IsLighting () const = 0;
  /// Set the color to use. Will be added to the lighting values.
  virtual void SetColor (const csColor& col) = 0;
  /// Get the color.
  virtual csColor GetColor () const = 0;
  /// Use cylindrical texture mapping.
  virtual void SetCylindricalMapping (bool m) = 0;
  /// Test if cylindrical texture mapping is used.
  virtual bool IsCylindricalMapping () const = 0;

  /**
   * Set the colours of the dome to a gradient, vertically.
   * the horizon_height is 0, the zenith_height is 1,
   * the gradient is then interpolated to get the colour.
   * The gradient is specified using a float**, where entry
   * gradient[nr] is an array of 4 elements {height, r, g, b}.
   * The entries must be in sorted order, low to high. End with a 0
   * e.g. (0.0, 100.0, { {0.0, 1,0,1}, {1.0, 0,0,0}, 0} for a
   * gradient from purple to black.
   */
  virtual void ApplyVertGradient(float horizon_height, float zenith_height,
    float** gradient) = 0;

  /**
   * Create a lightspot on the colours of the dome.
   * The position indicates the direction of center of the lightspot
   * wrt. the center of the ball mesh.
   * The size gives the size of the spot, 1.0 for the sun.
   * The gradient is used to get the colours for the lightspot.
   * pass 0 for a sunlike gradient.
   */
  virtual void ApplyLightSpot(const csVector3& position, float size,
    float **gradient) = 0;

  /**
   * Animate the ball as a skydome for earth.
   * Give a time - from 0.0 to 1.0.
   * 0.0 is sunrise, daytime after, 0.5 is sunset, night following.
   * If you pass 0 for the gradients, a default will be used.
   * Note that both dayvert=nightvert and topsun=sunset, and only
   * the colors can be different in them. Thus those pairs must be
   * of the same length, and have the same interpolation values.
   * This condition can be satisfied for any pair of gradients, by
   * inserting points in one into the other with the interpolated
   * color of the other. Thus the gradient pairs are identical, save
   * for the r,g,b values.
   */
  virtual void PaintSky(float time, float **dayvert, float **nightvert,
    float **topsun, float **sunset) = 0;

};

/** @} */

#endif // __CS_IMESH_BALL_H__

