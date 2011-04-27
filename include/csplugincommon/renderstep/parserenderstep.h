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

#ifndef __CS_PARSERENDERSTEP_H__
#define __CS_PARSERENDERSTEP_H__

/**\file
 * Base class for render step loaders.
 */

#include "csextern.h"
#include "csutil/strhash.h"
#include "csutil/weakref.h"
#include "csutil/leakguard.h"
#include "iengine/rendersteps/icontainer.h"
#include "iengine/rendersteps/irenderstep.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

/**\addtogroup plugincommon
 * @{ */
/**
 * Parser for render steps and render step lists.
 */
class CS_CRYSTALSPACE_EXPORT csRenderStepParser
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csWeakRef<iPluginManager> plugmgr;

  csStringHash tokens;
  enum
  {
    XMLTOKEN_STEP
  };

public:
  CS_LEAKGUARD_DECLARE (csRenderStepParser);

  bool Initialize(iObjectRegistry *object_reg);

  csPtr<iRenderStep> Parse (iObjectRegistry* object_reg,
    iDocumentNode* node);
  bool ParseRenderSteps (iRenderStepContainer* container, 
    iDocumentNode* node);
};
/** @} */

#endif
