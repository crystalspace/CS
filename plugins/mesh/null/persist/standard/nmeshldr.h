/*
    Based on genmesh.h, Copyright (C) 2002 by Jorrit Tyberghein
    Copyright (C) 2003 by Mat Sutcliffe

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

#ifndef __CS_NULLMESHLDR_H__
#define __CS_NULLMESHLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;
struct iSyntaxService;
struct iNullFactoryState;

/**
 * Null Mesh factory loader.
 */
class csNullFactoryLoader :
  public scfImplementation2<csNullFactoryLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor.
  csNullFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csNullFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }

  bool ParseRenderBuffer(iDocumentNode *node, iNullFactoryState* state);
};

/**
 * Null Mesh factory saver.
 */
class csNullFactorySaver :
  public scfImplementation2<csNullFactorySaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csNullFactorySaver (iBase*);

  /// Destructor.
  virtual ~csNullFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Null Mesh loader.
 */
class csNullMeshLoader :
  public scfImplementation2<csNullMeshLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor.
  csNullMeshLoader (iBase*);

  /// Destructor.
  virtual ~csNullMeshLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Null Mesh saver.
 */
class csNullMeshSaver :
  public scfImplementation2<csNullMeshSaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csNullMeshSaver (iBase*);

  /// Destructor.
  virtual ~csNullMeshSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

#endif // __CS_NULLMESHLDR_H__

