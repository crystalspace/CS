/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#ifndef __CS_TERRAIN_LOADER_H
#define __CS_TERRAIN_LOADER_H

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

struct iObjectRegistry;
struct iSyntaxService;
struct iVFS;

/**
 *
 */
class csTerrainFactoryLoader :
  public scfImplementation2<csTerrainFactoryLoader,
                            iLoaderPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor
  csTerrainFactoryLoader (iBase*);
  /// Destructor
  virtual ~csTerrainFactoryLoader ();

  /// Setup the plugin with the system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Parse the given node block and build the terrain factory
  csPtr<iBase> Parse (iDocumentNode *node,
    iStreamSource*, iLoaderContext *ldr_context,
    iBase* context);	
};

/**
 *
 */
class csTerrainObjectLoader :
  public scfImplementation2<csTerrainObjectLoader,
                            iLoaderPlugin,
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;
public:
  /// Constructor
  csTerrainObjectLoader (iBase*);

  /// Destructor
  virtual ~csTerrainObjectLoader ();

  /// Register plugin with system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Parse the given block to create a new Terrain object
  csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context,
    iBase *context);
};

#endif // __CS_CHUNKLDR_H
