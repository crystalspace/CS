/*
    Copyright (C) 2002 by Anders Stenberg
    Written by Anders Stenberg

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

#ifndef __IEFFECTTECHNIQUE_H__
#define __IEFFECTTECHNIQUE_H__

#include "csutil/scf.h"
#include "cstypes.h"

struct iEffectPass;

#define CS_TECHNIQUE_PASSED          0
#define CS_TECHNIQUE_FAILED          1
#define CS_TECHNIQUE_NOTVALIDATED    2

SCF_VERSION (iEffectTechnique, 0, 0, 1);

/**
 * An effect technique
 */
struct iEffectTechnique : public iBase
{
  virtual iEffectPass* CreatePass() = 0;
  virtual int GetPassCount() = 0;
  virtual iEffectPass* GetPass( int pass ) = 0;

  virtual void SetValidation( int validation ) = 0;
  virtual int GetValidation() = 0;

  virtual void SetQuality( float q ) = 0;
  virtual float GetQuality() = 0;

  virtual void SetClientFlags( uint32 flags) = 0;
  virtual uint32 GetClientFlags() = 0;

  // Some way of setting user data/flags and automatically invalidating
  // techniques with some flag defined should be possible.
  // For example, an effect might have a "pixel shader"-flag, and then 
  // the app can disable pixel shading by saying that all techniques
  // with the "pixel shader" flag are invalid (even though it might be
  // supported by the renderer).
  // --Anders Stenberg
};

#endif // __IEFFECTTECHNIQUE_H__
