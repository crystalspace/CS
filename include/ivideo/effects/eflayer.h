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

#ifndef __IEFFECTLAYER_H__
#define __IEFFECTLAYER_H__

#include "csutil/scf.h"
#include "cstypes.h"

SCF_VERSION (iEffectLayer, 0, 0, 1);

/**
 * A layer in an effect pass.
 */
struct iEffectLayer : public iBase
{
  virtual void SetStateFloat( csStringID state, float value ) = 0;
  virtual void SetStateString( csStringID state, csStringID value ) = 0;
  virtual void SetStateOpaque( csStringID state, void *value ) = 0;
  
  virtual float GetStateFloat( csStringID state ) = 0;
  virtual csStringID GetStateString( csStringID state ) = 0;
  virtual void *GetStateOpaque( csStringID state ) = 0;

  virtual csStringID GetFirstState() = 0;
  virtual csStringID GetNextState() = 0;
};

#endif // __IEFFECTLAYER_H__
