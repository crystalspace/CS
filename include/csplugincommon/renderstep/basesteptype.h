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

#ifndef __CS_BASESTEPTYPE_H__
#define __CS_BASESTEPTYPE_H__

#include "csextern.h"
#include "csutil/scf.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iengine/rendersteps/irsfact.h"

class CS_CSPLUGINCOMMON_EXPORT csBaseRenderStepType : public iComponent, 
						       public iRenderStepType
{
protected:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  csBaseRenderStepType (iBase *p);
  virtual ~csBaseRenderStepType ();

  virtual bool Initialize(iObjectRegistry *object_reg);
};  

#endif
