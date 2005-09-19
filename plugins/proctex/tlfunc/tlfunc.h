/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_TLFUNC_H__
#define __CS_TLFUNC_H__

#include "cstool/basetexfact.h"
#include "csutil/strhash.h"
#include "csutil/csstring.h"
#include "csutil/leakguard.h"

#include "iutil/comp.h"

class csFuncTexLoader : public iLoaderPlugin
{
protected:
  iObjectRegistry* object_reg;
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "plugins/proctex/tlfunc/tlfunc.tok"
#include "cstool/tokenlist.h"

public:
  CS_LEAKGUARD_DECLARE (csFuncTexLoader);

  SCF_DECLARE_IBASE;

  csFuncTexLoader (iBase *p);
  virtual ~csFuncTexLoader();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFuncTexLoader);

    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_TLFUNC_H__
