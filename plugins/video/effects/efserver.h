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

#ifndef __EFFECTSERVER_H__
#define __EFFECTSERVER_H__

#include "cstypes.h"
#include "csutil/scf.h"
#include "csutil/objreg.h"
#include "iutil/comp.h"
#include "csutil/csvector.h"
#include "ivideo/effects/efdef.h"
#include "ivideo/effects/efstring.h"

/**
 * Effect server
 */
class csEffectServer : public iEffectServer
{
private:
  iObjectRegistry* objectreg;
  csStringSet strset;
  int seqnr;

  csBasicVector* effects;
  csEffectStrings* efstrings;
public:

  SCF_DECLARE_IBASE;

  csEffectServer( iBase* parent)
  {
    SCF_CONSTRUCT_IBASE( parent );
    SCF_CONSTRUCT_EMBEDDED_IBASE( scfiComponent );
    seqnr = 0;

    effects = new csVector();
    efstrings = new csEffectStrings();
    efstrings->InitStrings(this);
  }
  
  virtual ~csEffectServer () 
  {
    while(effects->Length() > 0)
    {
      delete (iEffectDefinition*)(effects->Pop());
    }
    delete effects;
  }

  bool Initialize( iObjectRegistry* reg );

  iEffectDefinition* CreateEffect();

  bool Validate( iEffectDefinition* effect );

  iEffectTechnique* SelectAppropriateTechnique( iEffectDefinition* effect );

  iEffectDefinition* GetEffect(const char *s);

  csStringID RequestString( const char *s );
  const char* RequestString( csStringID id );
  
  csEffectStrings* GetStandardStrings()
  {
    return efstrings;
  }

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE( csEffectServer );
    virtual bool Initialize( iObjectRegistry* objectreg )
    {
      return scfParent->Initialize( objectreg );
    }
  } scfiComponent;
};

#endif // __EFFECTSERVER_H__
