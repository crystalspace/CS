/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards
  
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

#ifndef __IMESH_SPIRAL_H__
#define __IMESH_SPIRAL_H__

#include "csutil/scf.h"

class csColor;

SCF_VERSION (iSpiralState, 0, 0, 1);

/**
 * This interface describes the API for the spiral mesh object.
 */
struct iSpiralState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetNumberParticles (int num) = 0;
  /// Get the number of particles.
  virtual int GetNumberParticles () const = 0;
  /// Set the source for the particles.
  virtual void SetSource (const csVector3& source) = 0;
  /// Get the source for the particles.
  virtual const csVector3& GetSource () const = 0;
};

#endif

