/*
    Copyright (C) 2000 by W.C.A. Wijngaards

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

#ifndef __IPARTCL_H__
#define __IPARTCL_H__

#include "csutil/scf.h"
class csColor;
class csVector3;
class iRenderView;
class iLight;

SCF_VERSION (iParticle, 0, 0, 2);

/**
 * A iParticle can be used in particle Systems.
 * Each particle may perform these operations in it's own manner,
 * Or even do nothing at some of the functions.
 * If some are not implemented, functionality depending on that
 * feature may not work.
 */
struct iParticle : public iBase
{
  /// Set the position of this particle in world coordinates.
  virtual void SetPosition (const csVector3& pos) = 0;
  /// Move the particle relative to position.
  virtual void MovePosition (const csVector3& move) = 0;

  /// Set the color of this particle.
  virtual void SetColor (const csColor& col) = 0;
  /// Add color to the color of the sprite.
  virtual void AddColor (const csColor& col) = 0;
  /// Scale particle by this factor. 
  virtual void ScaleBy (float factor) = 0;
  /// Set the Mixmode for the particle.
  virtual void SetMixmode (UInt mode) = 0;
  /// Rotate the particle is some particle dependent manner, in radians.
  virtual void Rotate (float angle) = 0;
  /// Draw this particle.
  virtual void Draw (iRenderView* rview) = 0;
  /// Light this particle.
  virtual void UpdateLighting (iLight** lights, int num_lights) = 0;
};

#endif // __IPARTCL_H__
