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

#ifndef __CS_EFFECTTECHNIQUE_H__
#define __CS_EFFECTTECHNIQUE_H__

#include "csutil/scf.h"
#include "csutil/refarr.h"
#include "cstypes.h"
#include "../ieffects/efpass.h"

class csEffectTechnique : public iEffectTechnique
{
private:
  csRefArray<iEffectPass> passes;
  float quality;
  int valid;
  uint32 clientflags;

public:
  SCF_DECLARE_IBASE;

  csEffectTechnique ();
  virtual ~csEffectTechnique ();

  csPtr<iEffectPass> CreatePass();
  int GetPassCount();
  iEffectPass* GetPass (int pass);

  void SetValidation( int validation );
  int GetValidation();

  void SetQuality( float q );
  float GetQuality();

  void SetClientFlags( uint32 flags);
  uint32 GetClientFlags();
};

#endif // __CS_EFFECTTECHNIQUE_H__
