/*
    Copyright (C) 2008 by Pavel Krajcevski

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

#ifndef __CS_WATERMESHLDR_H__
#define __CS_WATERMESHLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;
struct iSyntaxService;
struct iWaterFactoryState;

namespace CS
{
namespace Plugins
{
namespace WaterMeshLoader
{


/**
 * Water Mesh factory loader.
 */
class csWaterFactoryLoader : 
  public scfImplementation2<csWaterFactoryLoader, 
                            iLoaderPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/watermesh/persist/standard/watermesh_factory.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

public:
  /// Constructor.
  csWaterFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csWaterFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Water Mesh factory saver.
 */
class csWaterFactorySaver : 
  public scfImplementation2<csWaterFactorySaver,
                            iSaverPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csWaterFactorySaver (iBase*);

  /// Destructor.
  virtual ~csWaterFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Water Mesh loader.
 */
class csWaterMeshLoader : 
  public scfImplementation2<csWaterMeshLoader, 
                            iLoaderPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/watermesh/persist/standard/watermesh_meshobject.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

public:
  /// Constructor.
  csWaterMeshLoader (iBase*);

  /// Destructor.
  virtual ~csWaterMeshLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Water Mesh saver.
 */
class csWaterMeshSaver :
  public scfImplementation2<csWaterMeshSaver,
                            iSaverPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csWaterMeshSaver (iBase*);

  /// Destructor.
  virtual ~csWaterMeshSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

} // namespace WaterMeshLoader
} // namespace Plugins
} // namespace CS

#endif // __CS_WATERMESHLDR_H__

