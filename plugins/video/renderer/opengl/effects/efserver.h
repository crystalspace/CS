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

#ifndef __CS_EFFECTSERVER_H__
#define __CS_EFFECTSERVER_H__

#include "cstypes.h"
#include "csutil/scf.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/objreg.h"
#include "iutil/comp.h"
#include "../ieffects/efdef.h"
#include "../ieffects/efstring.h"

/**
 * Effect server
 */
class csEffectServer : public iEffectServer, public iComponent
{
private:
  iObjectRegistry* objectreg;
  csStringSet strset;
  int seqnr;

  csRefArray <iEffectDefinition> effects;
  csEffectStrings* efstrings;
public:
  csEffectServer (iBase* parent);
  virtual ~csEffectServer ();

  bool Initialize( iObjectRegistry* reg );

  csPtr<iEffectDefinition> CreateEffect();

  bool Validate( iEffectDefinition* effect );

  iEffectTechnique* SelectAppropriateTechnique (iEffectDefinition* effect);

  iEffectDefinition* GetEffect(const char *s);

  csStringID RequestString( const char *s );
  const char* RequestString( csStringID id );
  
  csEffectStrings* GetStandardStrings()
  {
    return efstrings;
  }

  SCF_DECLARE_IBASE;
};

#endif // __CS_EFFECTSERVER_H__
