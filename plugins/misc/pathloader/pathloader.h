/*
    Crystal Space
    Copyright (C) 2008 by Jorrit Tyberghein
  
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
    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __CS_ADDON_PATHLOADER__
#define __CS_ADDON_PATHLOADER__

#include "iutil/comp.h"
#include "imap/reader.h"
#include "csutil/strhash.h"
#include "csutil/weakref.h"

struct iObjectRegistry;
struct iDocumentNode;
struct iLoaderContext;
struct iSyntaxService;
struct iPath;

/**
 * This is an add-on to allow loading cs paths.
 */

class csPathLoader :
  public scfImplementation2<csPathLoader, 
                            iLoaderPlugin, 
                            iComponent>
{
private:
  iObjectRegistry* object_reg;
  csWeakRef<iSyntaxService> synldr;
  csStringHash xmltokens;

  bool ParseNode(iDocumentNode *node,csPath *path);
public:
  csPathLoader (iBase* parent);
  virtual ~csPathLoader ();

  /**
   * Initialize this plugin.
   */
  bool Initialize (iObjectRegistry* object_reg);
  /**
   * Parses a document/script and assigns behaviour and/or property class to
   * an entity. If the context is not a mesh, a standalone entity will be
   * created. For meshes an additional pcmesh property class will be assigned.
   */
  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource*, iLoaderContext* ldr_context,
  	iBase* context);

  virtual bool IsThreadSafe() { return true; }

  /*
   * Functions not used atm, but might be useful at some point...
   */
  virtual csPtr<iPath> Load (iDocumentNode* node);
  virtual csPtr<iPath> Load (const char* path, const char* file);
};

#endif // __CS_ADDON_PATHLOADER__

