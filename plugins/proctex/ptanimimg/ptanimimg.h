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

#ifndef __CS_PTANIMIMG_H__
#define __CS_PTANIMIMG_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "imap/reader.h"
#include "igraphic/image.h"
#include "igraphic/animimg.h"
#include "cstool/proctex.h"
#include "cstool/proctxtanim.h"

class csProcTexture;

CS_PLUGIN_NAMESPACE_BEGIN(PTAnimImg)
{

class csAnimateProctexLoader :
  public scfImplementation2<csAnimateProctexLoader,
    iLoaderPlugin, iComponent>
{
protected:
  iObjectRegistry* object_reg;

  void Report (int severity, iDocumentNode* node, const char* msg, ...);

public:
  csAnimateProctexLoader (iBase *p);
  virtual ~csAnimateProctexLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource*, iLoaderContext* ldr_context,
  	iBase* context);

  virtual bool IsThreadSafe() { return true; }
};  

}
CS_PLUGIN_NAMESPACE_END(PTAnimImg)

#endif
