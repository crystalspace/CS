/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __FUR_MESH_LDR_H__
#define __FUR_MESH_LDR_H__

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

CS_PLUGIN_NAMESPACE_BEGIN(FurMeshLoader)
{
/**
 * Fur Mesh Factory loader.
 */
class FurMeshFactoryLoader : 
  public scfImplementation2<FurMeshFactoryLoader, 
                            iLoaderPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/furmesh/persist/standard/furmeshfactoryhldr.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

public:
  /// Constructor.
  FurMeshFactoryLoader (iBase*);

  /// Destructor.
  virtual ~FurMeshFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Fur Mesh Factor saver.
 */
class FurMeshFactorySaver :
  public scfImplementation2<FurMeshFactorySaver,
                            iSaverPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  FurMeshFactorySaver (iBase*);

  /// Destructor.
  virtual ~FurMeshFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Fur Mesh loader.
 */
class FurMeshLoader : 
  public scfImplementation2<FurMeshLoader, 
                            iLoaderPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iEngine> engine;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/furmesh/persist/standard/furmeshldr.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

public:
  /// Constructor.
  FurMeshLoader (iBase*);

  /// Destructor.
  virtual ~FurMeshLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Fur Mesh saver.
 */
class FurMeshSaver :
  public scfImplementation2<FurMeshSaver,
                            iSaverPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iEngine> engine;

public:
  /// Constructor.
  FurMeshSaver (iBase*);

  /// Destructor.
  virtual ~FurMeshSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

}
CS_PLUGIN_NAMESPACE_END(FurMeshLoader)

#endif // __FUR_MESH_LDR_H__

