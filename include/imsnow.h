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

#ifndef __IMESHSNOW_H__
#define __IMESHSNOW_H__

#include "csutil/scf.h"

class csColor;

SCF_VERSION (iSnowState, 0, 0, 1);

/**
 * This interface describes the API for the snow mesh object.
 */
struct iSnowState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetNumberParticles (int num) = 0;
  /// Set the size of the drops.
  virtual void SetDropSize (float dropwidth, float dropheight) = 0;
  /// Set box.
  virtual void SetBox (const csVector3& minbox, const csVector3& maxbox) = 0;
  /// Enable or disable lighting.
  virtual void SetLighting (bool l) = 0;
  /// Set fall speed.
  virtual void SetFallSpeed (const csVector3& fspeed) = 0;
  /// Set swirl.
  virtual void SetSwirl (float swirl) = 0;
};

#endif

