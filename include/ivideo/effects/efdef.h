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

#ifndef __IEFFECTDEFINITION_H__
#define __IEFFECTDEFINITION_H__

#include "csutil/scf.h"
#include "cstypes.h"

struct iEffectTechnique;

SCF_VERSION (iEffectDefinition, 0, 0, 1);

/**
 * Effect definition
 */
struct iEffectDefinition : public iBase
{
  virtual iEffectTechnique* CreateTechnique() = 0;
  virtual int GetTechniqueCount() = 0;
  virtual iEffectTechnique* GetTechnique( int technique ) = 0;

  virtual void SetName( const char* name ) = 0;
  virtual const char* GetName() = 0;
};

#endif // __IEFFECTDEFINITION_H__
