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

#ifndef __CS_IVIDEO_IEFFECTTECHNIQUE_H__
#define __CS_IVIDEO_IEFFECTTECHNIQUE_H__

/**\file
 * Effect technique interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csutil/scf.h"
#include "cstypes.h"
#include "efpass.h"

/**\name Technique Validation Status
 * @{ */
/// Technique is validated.
#define CS_TECHNIQUE_PASSED          0
/// Validation failed.
#define CS_TECHNIQUE_FAILED          1
/// Not yet tested for validity
#define CS_TECHNIQUE_NOTVALIDATED    2
/** @} */

SCF_VERSION (iEffectTechnique, 0, 0, 1);

/**
 * An effect technique
 */
struct iEffectTechnique : public iBase
{
  /// Create a new pass.
  virtual csPtr<iEffectPass> CreatePass() = 0;
  /// Return number of passes.
  virtual int GetPassCount() = 0;
  /// Return a specific pass.
  virtual iEffectPass* GetPass (int pass) = 0;

  /**
   * Set validation status.
   * \sa #CS_TECHNIQUE_FAILED
   */
  virtual void SetValidation (int validation) = 0;
  /**
   * Retrieve validation status.
   * \sa #CS_TECHNIQUE_FAILED
   */
  virtual int GetValidation() = 0;

  /// Set this technique's quality.
  virtual void SetQuality (float q) = 0;
  /// Retrieve this technique's quality.
  virtual float GetQuality() = 0;

  /// Set client flags.
  virtual void SetClientFlags (uint32 flags) = 0;
  /// Retrieve client flags.
  virtual uint32 GetClientFlags() = 0;

  // Some way of setting user data/flags and automatically invalidating
  // techniques with some flag defined should be possible.
  // For example, an effect might have a "pixel shader"-flag, and then 
  // the app can disable pixel shading by saying that all techniques
  // with the "pixel shader" flag are invalid (even though it might be
  // supported by the renderer).
  // --Anders Stenberg
};

/** @} */

#endif // __CS_IVIDEO_EFFECTTECHNIQUE_H__
