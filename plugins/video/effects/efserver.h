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

/**
 * Effect server
 */
class csEffectServer : public iEffectServer
{
private:
  iObjectRegistry* objectreg;
  csStringSet strset;

public:

  SCF_DECLARE_IBASE;

  csEffectServer( iBase* parent)
  {
    SCF_CONSTRUCT_IBASE( parent );
    SCF_CONSTRUCT_EMBEDDED_IBASE( scfiComponent );
  }
  virtual ~csEffectServer () { }

  bool Initialize( iObjectRegistry* reg );

  iEffectDefinition* CreateEffect();

  bool Validate( iEffectDefinition* effect );

  iEffectTechnique* SelectAppropriateTechnique( iEffectDefinition* effect );

  csStringID RequestString( const char *s );
  const char* RequestString( csStringID id );

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
