/*
    Copyright (C) 2001-2002 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CS_EMITLDR_H__
#define __CS_EMITLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iEngine;
struct iPluginManager;
struct iObjectRegistry;
struct iReporter;
struct iDocumentNode;
struct iEmitGen3D;
struct iEmitFactoryState;

/**
 * Emit factory loader.
 */
class csEmitFactoryLoader :
  public scfImplementation2<csEmitFactoryLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csEmitFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csEmitFactoryLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Emit factory saver.
 */
class csEmitFactorySaver :
  public scfImplementation2<csEmitFactorySaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csEmitFactorySaver (iBase*);

  /// Destructor.
  virtual ~csEmitFactorySaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Emit loader.
 */
class csEmitLoader :
  public scfImplementation2<csEmitLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iReporter> reporter;
  csStringHash xmltokens;

  csRef<iEmitGen3D> ParseEmit (iDocumentNode* node,
	      iEmitFactoryState *fstate, float* weight);

public:
  /// Constructor.
  csEmitLoader (iBase*);

  /// Destructor.
  virtual ~csEmitLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Emit saver.
 */
class csEmitSaver :
  public scfImplementation2<csEmitSaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csEmitSaver (iBase*);

  /// Destructor.
  virtual ~csEmitSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown(iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
  virtual bool WriteEmit(iEmitGen3D* emit, iDocumentNode* parent);
};

#endif // __CS_EMITLDR_H__
