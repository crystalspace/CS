/*
    Copyright (C) 2000 by Jorrit Tyberghein
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

#ifndef __CS_FIRELDR_H__
#define __CS_FIRELDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iEngine;
struct iPluginManager;
struct iObjectRegistry;
struct iReporter;

/**
 * Fire factory loader.
 */
class csFireFactoryLoader :
  public scfImplementation2<csFireFactoryLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csFireFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csFireFactoryLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

/**
 * Fire factory saver.
 */
class csFireFactorySaver :
  public scfImplementation2<csFireFactorySaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csFireFactorySaver (iBase*);

  /// Destructor.
  virtual ~csFireFactorySaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Fire loader.
 */
class csFireLoader :
  public scfImplementation2<csFireLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iReporter> reporter;
  csStringHash xmltokens;

public:
  /// Constructor.
  csFireLoader (iBase*);

  /// Destructor.
  virtual ~csFireLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

/**
 * Fire saver.
 */
class csFireSaver :
  public scfImplementation2<csFireSaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csFireSaver (iBase*);

  /// Destructor.
  virtual ~csFireSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

#endif // __CS_FIRELDR_H__

