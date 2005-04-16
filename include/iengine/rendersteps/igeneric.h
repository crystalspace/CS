/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_IENGINE_RENDERSTEPS_IGENERIC_H__
#define __CS_IENGINE_RENDERSTEPS_IGENERIC_H__

#include "iutil/strset.h"
#include "ivideo/graph3d.h"

/**\file
 * Generic render step.
 */

/**\addtogroup engine3d_rloop
 * @{ */

struct iShader;

SCF_VERSION (iGenericRenderStep, 0, 0, 3);

/**
 * A generic render step.
 */
struct iGenericRenderStep : public iBase
{
  /// Set shader type.
  virtual void SetShaderType (const char* type) = 0;
  /// Get shader type.
  virtual const char* GetShaderType () = 0;

  /// Set Z offset flag.
  virtual void SetZOffset (bool zOffset) = 0;
  /// Get Z offset flag.
  virtual bool GetZOffset () const = 0;

  /// Enable/disable portal traversal for this renderstep.
  virtual void SetPortalTraversal (bool p) = 0;
  /// Get portal traversal flag.
  virtual bool GetPortalTraversal () const = 0;

  /// Set Z buffer mode.
  virtual void SetZBufMode (csZBufMode zmode) = 0;
  /// Get Z buffer mode.
  virtual csZBufMode GetZBufMode () const = 0;

  /**
   * Set the default shader that is used if a material doesn't provide one
   * for the shader type of this step.
   */
  virtual void SetDefaultShader (iShader* shader) = 0;
  /// Get the default shader.
  virtual iShader* GetDefaultShader () const = 0;

  /* @@@ Those below are not nice */
  /**
   * Add a shader type that, when present on a material, prevents the
   * "use default shader" logic to not kick in.
   */
  virtual void AddDisableDefaultTriggerType (const char* type) = 0;
  /// Remove a shader type that prevents default shader usage.
  virtual void RemoveDisableDefaultTriggerType (const char* type) = 0;
};

/** @} */

#endif
