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

#ifndef __CS_IVIDEO_EFFECTDEFINITION_H__
#define __CS_IVIDEO_EFFECTDEFINITION_H__

/**\file
 * Effect definition interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csutil/scf.h"
#include "cstypes.h"
#include "efvector4.h"

struct iEffectTechnique;

SCF_VERSION (iEffectDefinition, 0, 0, 1);

/**
 * Effect definition
 */
struct iEffectDefinition : public iBase
{
  /// Create a new technique
  virtual csPtr<iEffectTechnique> CreateTechnique() = 0;
  /// Get number of techniques
  virtual int GetTechniqueCount() = 0;
  /// Retrieve a technique
  virtual iEffectTechnique* GetTechnique (int technique) = 0;

  /// Set this Effect's name
  virtual void SetName( const char* name ) = 0;
  /// Retrieve name of effect
  virtual const char* GetName() = 0;
  
  /// Get variable value as float
  virtual float GetVariableFloat( int variableID ) = 0;
  /// Get variable value as csEffectVector4
  virtual csEffectVector4 GetVariableVector4( int variableID ) = 0;
  /// Get varaibletype
  virtual char GetVariableType( int variableID ) = 0;

  /// Set variable value as float
  virtual void SetVariableFloat( int variableID, float value ) = 0;
  /// Set variable value as vector4
  virtual void SetVariableVector4( int variableID, csEffectVector4 value ) = 0; 

  /// Get/create variable
  virtual int GetVariableID(uint32 string, bool create = true) = 0;
  /// Get all variable stringnames (used when creatingthem)
  //virtual csBasicVector GetAllVariableNames() = 0; 
};

/** @} */

#endif // __CS_IVIDEO_EFFECTDEFINITION_H__
