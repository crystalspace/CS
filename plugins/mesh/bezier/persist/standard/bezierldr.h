/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_BEZIERLDR_H__
#define __CS_BEZIERLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

struct iEngine;
struct iPluginManager;
struct iObjectRegistry;
struct iSyntaxService;
struct iReporter;

/**
 * Private information during the loading process of a thing.
 */
class BezierLoadInfo
{
public:
  iMaterialWrapper* default_material;
  float default_texlen;

  BezierLoadInfo () : default_material (0),
    default_texlen (1)
    {}
};

/**
 * Thing loader.
 */
class csBezierLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iReporter> reporter;
  csStringHash xmltokens;

  bool ParseCurve (iCurve* curve, iLoaderContext* ldr_context,
  	iDocumentNode* node);
  bool LoadThingPart (
  	iDocumentNode* node, iLoaderContext* ldr_context,
	iObjectRegistry* object_reg, iReporter* reporter,
	iSyntaxService *synldr, BezierLoadInfo& info,
	iEngine* engine, iBezierState* thing_state,
	iBezierFactoryState* thing_fact_state,
	bool isParent);

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBezierLoader (iBase*);
  /// Destructor.
  virtual ~csBezierLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBezierLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * Thing saver.
 */
class csBezierSaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBezierSaver (iBase*);
  /// Destructor.
  virtual ~csBezierSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  bool WriteDown (iBase* obj, iDocumentNode* parent,
  	iStreamSource*);
  bool WriteObject (iBase* obj, iDocumentNode* parent);
  bool WriteFactory (iBase* obj, iDocumentNode* parent);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBezierSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

#endif // __CS_BEZIERLDR_H__
