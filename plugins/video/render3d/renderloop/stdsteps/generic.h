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

#ifndef __CS_GENERIC_H__
#define __CS_GENERIC_H__

#include "csutil/scf.h"
#include "iengine/renderloop.h"
#include "ivideo/rendersteps/igeneric.h"

#include "../common/basesteptype.h"
#include "../common/basesteploader.h"

SCF_DECLARE_FACTORY(csGenericRSType);
SCF_DECLARE_FACTORY(csGenericRSLoader);

class csGenericRenderStep : public iRenderStep, iGenericRenderStep
{
public:
  SCF_DECLARE_IBASE;

  csGenericRenderStep (iObjectRegistry* object_reg);

  virtual void Perform (csRenderView* rview, iSector* sector);
};

class csGenericRenderStepFactory : public iRenderStepFactory
{
  csRef<iObjectRegistry> object_reg;
public:
  SCF_DECLARE_IBASE;

  csGenericRenderStepFactory (iObjectRegistry* object_reg);

  virtual csPtr<iRenderStep> Create ();
};

class csGenericRSType : public csBaseRenderStepType
{
public:
  csGenericRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csGenericRSLoader : public csBaseRenderStepLoader
{
public:
  csGenericRSLoader (iBase* p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iLoaderContext* ldr_context, 	
    iBase* context);
};

#endif
