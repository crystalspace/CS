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

#ifndef __EFFECTDEFINITION_H__
#define __EFFECTDEFINITION_H__

#include "csutil/scf.h"
#include "cstypes.h"

struct iEffectTechnique;

class csEffectDefinition : public iEffectDefinition
{
private:
  csBasicVector techniques;
  char* techniquename;
public:

  SCF_DECLARE_IBASE;

  csEffectDefinition()
  {
    SCF_CONSTRUCT_IBASE( NULL );
  }
  virtual ~csEffectDefinition ()
  {
  }

  iEffectTechnique* CreateTechnique();
  int GetTechniqueCount();
  iEffectTechnique* GetTechnique( int technique );

  void SetName( const char* name );
  const char* GetName();
};

#endif // __EFFECTDEFINITION_H__
