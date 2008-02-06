/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_INSTMESHLDR_H__
#define __CS_INSTMESHLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iReporter;
struct iObjectRegistry;
struct iSyntaxService;

/**
 * General Mesh factory loader.
 */
class csInstFactoryLoader :
  public scfImplementation2<csInstFactoryLoader,
    iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor.
  csInstFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csInstFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

/**
 * General Mesh factory saver.
 */
class csInstFactorySaver :
  public scfImplementation2<csInstFactorySaver,
    iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csInstFactorySaver (iBase*);

  /// Destructor.
  virtual ~csInstFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * General Mesh loader.
 */
class csInstMeshLoader :
  public scfImplementation2<csInstMeshLoader,
    iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor.
  csInstMeshLoader (iBase*);

  /// Destructor.
  virtual ~csInstMeshLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
  /// Parse a render buffer node
};

/**
 * General Mesh saver.
 */
class csInstMeshSaver :
  public scfImplementation2<csInstMeshSaver,
    iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csInstMeshSaver (iBase*);

  /// Destructor.
  virtual ~csInstMeshSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

#endif // __CS_INSTMESHLDR_H__

