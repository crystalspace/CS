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

#ifndef __CS_IVIDEO_EFFECTPASS_H__
#define __CS_IVIDEO_EFFECTPASS_H__

/**\file
 * Effect pass interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csutil/scf.h"
#include "cstypes.h"
#include "csutil/ref.h"
#include "efvector4.h"
#include "eflayer.h"

SCF_VERSION (iEffectPass, 0, 0, 1);

/**
 * An effect pass.
 */
struct iEffectPass : public iBase
{
  /// Set a state float.
  virtual void SetStateFloat (csStringID state, float value) = 0;
  /// Set a state string.
  virtual void SetStateString (csStringID state, csStringID value) = 0;
  /// Set a state opaque data.
  virtual void SetStateOpaque (csStringID state, void *value) = 0;
  /// Set a state vector4.
  virtual void SetStateVector4 (csStringID state, csEffectVector4 value) = 0;

  
  /// Get a state float.
  virtual float GetStateFloat (csStringID state) = 0;
  /// Get a state string.
  virtual csStringID GetStateString (csStringID state) = 0;
  /// Get a state opaque data.
  virtual void *GetStateOpaque (csStringID state) = 0;
  /// Get a state vector4.
  virtual csEffectVector4 GetStateVector4 (csStringID state) = 0;


  /// Create a new layer.
  virtual csPtr<iEffectLayer> CreateLayer() = 0;
  /// Get number of layers.
  virtual int GetLayerCount() = 0;
  /// Get a specific layer.
  virtual iEffectLayer* GetLayer (int layer) = 0;

  /// Get the id of the first state.
  virtual csStringID GetFirstState() = 0;
  /// Get the id of the next state.
  virtual csStringID GetNextState() = 0;

  /// Get renderer specific data
  virtual iBase* GetRendererData() = 0;
  /// Set renderer specific data
  virtual void SetRendererData(iBase* data) = 0;
};

/** @} */

#endif // __CS_IVIDEO_EFFECTPASS_H__
