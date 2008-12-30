/*
    Copyright (C) 2001 by Jorrit Tyberghein
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

#ifndef __CS_HAZELDR_H__
#define __CS_HAZELDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iObjectRegistry;
struct iReporter;
struct iDocumentNode;
struct iHazeFactoryState;
struct iHazeHull;

/**
 * Haze factory loader.
 */
class csHazeFactoryLoader :
  public scfImplementation2<csHazeFactoryLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iReporter> reporter;
  csStringHash xmltokens;

public:
  /// Constructor.
  csHazeFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csHazeFactoryLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Haze factory saver.
 */
class csHazeFactorySaver :
  public scfImplementation2<csHazeFactorySaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csHazeFactorySaver (iBase*);

  /// Destructor.
  virtual ~csHazeFactorySaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Haze loader.
 */
class csHazeLoader :
  public scfImplementation2<csHazeLoader, iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iReporter> reporter;
  csStringHash xmltokens;

public:
  /// Constructor.
  csHazeLoader (iBase*);

  /// Destructor.
  virtual ~csHazeLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Haze saver.
 */
class csHazeSaver :
  public scfImplementation2<csHazeSaver, iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csHazeSaver (iBase*);

  /// Destructor.
  virtual ~csHazeSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

#endif // __CS_HAZELDR_H__
