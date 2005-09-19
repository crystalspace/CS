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

#include "csutil/scf.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "imap/reader.h"
#include "imap/services.h"
#include "imap/writer.h"
#include "itexture/itexfact.h"

class csProcTexture;

class csBaseProctexType : public iComponent, public iTextureType
{
protected:
  iObjectRegistry* object_reg;
public:
  SCF_DECLARE_IBASE;

  csBaseProctexType (iBase *p);
  virtual ~csBaseProctexType ();

  virtual bool Initialize(iObjectRegistry *object_reg);
};  

class csBaseProctexLoader : public iLoaderPlugin
{
protected:
  iObjectRegistry* object_reg;

  csPtr<iBase> PrepareProcTex (csProcTexture* pt);
public:
  SCF_DECLARE_IBASE;

  csBaseProctexLoader (iBase *p);
  virtual ~csBaseProctexLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource*, iLoaderContext* ldr_context,
  	iBase* context) = 0;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBaseProctexLoader);

    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};  

class csBaseProctexSaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  SCF_DECLARE_IBASE;

  csBaseProctexSaver (iBase*);
  virtual ~csBaseProctexSaver ();

  bool Initialize (iObjectRegistry* p);

  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*) = 0;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBaseProctexSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

#endif

