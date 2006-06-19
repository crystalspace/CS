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

#ifndef __CS_SNOWLDR_H__
#define __CS_SNOWLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iObjectRegistry;
struct iReporter;

/**
 * Snow factory loader.
 */
class csSnowFactoryLoader :
  public scfImplementation2<csSnowFactoryLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csSnowFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSnowFactoryLoader ();

  bool Initialize (iObjectRegistry* p);

  //------------------------ iLoaderPlugin implementation --------------
  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

/**
 * Snow factory saver.
 */
class csSnowFactorySaver :
  public scfImplementation2<csSnowFactorySaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csSnowFactorySaver (iBase*);

  /// Destructor.
  virtual ~csSnowFactorySaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Snow loader.
 */
class csSnowLoader :
  public scfImplementation2<csSnowLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iReporter> reporter;
  csStringHash xmltokens;

public:
  /// Constructor.
  csSnowLoader (iBase*);

  /// Destructor.
  virtual ~csSnowLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

/**
 * Snow saver.
 */
class csSnowSaver :
  public scfImplementation2<csSnowSaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csSnowSaver (iBase*);

  /// Destructor.
  virtual ~csSnowSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

#endif // __CS_SNOWLDR_H__
