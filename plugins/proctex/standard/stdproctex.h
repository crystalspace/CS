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

#ifndef __CS_STDPROCTEX_H__
#define __CS_STDPROCTEX_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "imap/reader.h"
#include "imap/services.h"
#include "imap/writer.h"
#include "itexture/itexfact.h"

class csProcTexture;

class csBaseProctexType :
  public scfImplementation2<csBaseProctexType,
    iComponent, iTextureType>
{
protected:
  iObjectRegistry* object_reg;

public:
  csBaseProctexType (iBase *p);
  virtual ~csBaseProctexType ();

  virtual bool Initialize(iObjectRegistry *object_reg);
};  

class csBaseProctexLoader :
  public scfImplementation2<csBaseProctexLoader, iLoaderPlugin, iComponent>
{
protected:
  iObjectRegistry* object_reg;

  csPtr<iBase> PrepareProcTex (csProcTexture* pt);

public:
  csBaseProctexLoader (iBase *p);
  virtual ~csBaseProctexLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource*, iLoaderContext* ldr_context,
  	iBase* context) = 0;
};  

class csBaseProctexSaver :
  public scfImplementation2<csBaseProctexSaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  csBaseProctexSaver (iBase*);
  virtual ~csBaseProctexSaver ();

  bool Initialize (iObjectRegistry* p);

  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*) = 0;
};

#endif

