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

#ifndef __CS_IVIDEO_EFFECTCLIENT_H__
#define __CS_IVIDEO_EFFECTCLIENT_H__

/**\file
 * Effect client interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csutil/scf.h"

struct iEffectTechnique;
struct iEffectDefinition;

SCF_VERSION (iEffectClient, 0, 0, 1);

/**
 * Effect client
 */
struct iEffectClient : public iBase
{
public:
  /// Validate a technique
  virtual bool Validate( iEffectDefinition* effect,
	iEffectTechnique* technique ) = 0;
};

/** @} */

#endif // __CS_IVIDEO_EFFECTCLIENT_H__
