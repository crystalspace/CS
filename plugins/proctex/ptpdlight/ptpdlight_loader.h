/*
    Copyright (C) 2003-2006 by Jorrit Tyberghein
	      (C) 2003-2007 by Frank Richter

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

#ifndef __CS_PTPDLIGHT_LOADER_H__
#define __CS_PTPDLIGHT_LOADER_H__

#include "iutil/comp.h"
/*#include "iutil/plugin.h"*/
#include "imap/reader.h"
/*#include "imesh/lighting.h"
#include "igraphic/image.h"

#include "csgeom/csrect.h"
#include "csgfx/imageautoconvert.h"*/
#include "csutil/csstring.h"
/*#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"*/
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"
/*#include "csutil/weakref.h"
#include "cstool/proctex.h"

class csProcTexture;*/

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

class ProctexPDLightLoader :
  public scfImplementation2<ProctexPDLightLoader,
                            iLoaderPlugin, 
                            iComponent>
{
protected:
  iObjectRegistry* object_reg;

  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "plugins/proctex/ptpdlight/ptpdlight_loader.tok"
#include "cstool/tokenlist.h"

  void Report (int severity, iDocumentNode* node, const char* msg, ...);
  bool HexToLightID (char* lightID, const char* lightIDHex);
public:
  ProctexPDLightLoader (iBase *p);
  virtual ~ProctexPDLightLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource*, iLoaderContext* ldr_context,
  	iBase* context);
};  

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)

#endif // __CS_PTPDLIGHT_LOADER_H__
