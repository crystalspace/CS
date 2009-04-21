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

#ifndef __CS_BASESTEPLOADER_H__
#define __CS_BASESTEPLOADER_H__

/**\file
 * Base class for render step loaders.
 */

#include "csextern.h"
#include "csutil/leakguard.h"
#include "csutil/ref.h"
#include "csutil/scf_implementation.h"
#include "imap/reader.h"
#include "imap/services.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"

/**\addtogroup plugincommon
 * @{ */
/**
 * Base class for render step loaders.
 */
class CS_CRYSTALSPACE_EXPORT csBaseRenderStepLoader : 
  public scfImplementation2<csBaseRenderStepLoader,
                            iLoaderPlugin, 
			    iComponent>
{
protected:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:

  CS_LEAKGUARD_DECLARE (csBaseRenderStepLoader);

  csBaseRenderStepLoader (iBase *p);
  virtual ~csBaseRenderStepLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource* ssource, iLoaderContext* ldr_context,
  	iBase* context) = 0;
};  

/** @} */

#endif

