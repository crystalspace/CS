/*
    Copyright (C) 2006 by Christoph "Fossi" Mewes

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

#ifndef __CS_OPENTREELDR_H__
#define __CS_OPENTREELDR_H__

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
struct iOpenTreeFactoryState;

CS_PLUGIN_NAMESPACE_BEGIN(OpenTreeLoader)
{


/**
 * Proto Mesh factory loader.
 */
class csOpenTreeFactoryLoader : 
  public scfImplementation2<csOpenTreeFactoryLoader, 
                            iLoaderPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/opentree/opentree_factory.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

  bool ParseSpecies (iDocumentNode* node, iOpenTreeFactoryState* fact);
  bool ParseLevel (iDocumentNode* node, iOpenTreeFactoryState* fact);

public:
  /// Constructor.
  csOpenTreeFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csOpenTreeFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

/**
 * Proto Mesh loader.
 */
class csOpenTreeMeshLoader : 
  public scfImplementation2<csOpenTreeMeshLoader, 
                            iLoaderPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/opentree/persist/standard/opentree_meshobject.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

public:
  /// Constructor.
  csOpenTreeMeshLoader (iBase*);

  /// Destructor.
  virtual ~csOpenTreeMeshLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

}
CS_PLUGIN_NAMESPACE_END(OpenTreeLoader)


#endif // __CS_OPENTREELDR_H__

