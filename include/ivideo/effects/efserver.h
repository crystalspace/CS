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

#ifndef __CS_IVIDEO_EFFECTSERVER_H__
#define __CS_IVIDEO_EFFECTSERVER_H__

/**\file
 * Effect server interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csutil/scf.h"
#include "csutil/ref.h"
#include "csutil/strset.h"
#include "cstypes.h"

struct iEffectDefinition;
struct iEffectTechnique;
class csEffectStrings;

SCF_VERSION (iEffectServer, 0, 0, 1);

/**
 * Effect server
 */
struct iEffectServer : public iBase
{
public:
  /// Create a new effect.
  virtual csPtr<iEffectDefinition> CreateEffect() = 0;

  /// Validate an effect.
  virtual bool Validate (iEffectDefinition* effect ) = 0;

  /**
   * Select the best technique in an effect, based on validity and quality
   * settings.
   */
  virtual iEffectTechnique* SelectAppropriateTechnique (
  	iEffectDefinition* effect ) = 0;

  /// Get a effect based on it's name
  virtual iEffectDefinition* GetEffect (const char *s) = 0;

  /// Request an ID for a string.
  virtual csStringID RequestString (const char *s) = 0;
  /// Request string for an ID.
  virtual const char* RequestString (csStringID id) = 0;
  /// Get our csEffectStrings
  virtual csEffectStrings* GetStandardStrings() = 0;
};

/** @} */

#endif // __CS_IVIDEO_EFFECTSERVER_H__
