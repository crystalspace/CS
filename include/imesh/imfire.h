/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __IMESHFIRE_H__
#define __IMESHFIRE_H__

#include "csutil/scf.h"

class csColor;

SCF_VERSION (iFireState, 0, 0, 1);

/**
 * This interface describes the API for the fire mesh object.
 */
struct iFireState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetNumberParticles (int num) = 0;
  /// Set the size of the fire drops.
  virtual void SetDropSize (float dropwidth, float dropheight) = 0;
  /// Set origin of the fire.
  virtual void SetOrigin (const csVector3& origin) = 0;
  /// Set direction of the fire.
  virtual void SetDirection (const csVector3& dir) = 0;
  /// Enable or disable lighting.
  virtual void SetLighting (bool l) = 0;
  /// Set swirl.
  virtual void SetSwirl (float swirl) = 0;
  /// Set color scale.
  virtual void SetColorScale (float colscale) = 0;
  /// Set total time.
  virtual void SetTotalTime (float tottime) = 0;
};

#endif

