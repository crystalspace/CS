/*
  Copyright (C) 2011 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_ENGINE_PERSIST_H__
#define __CS_ENGINE_PERSIST_H__

#include "csutil/scf_implementation.h"
#include "igraphic/imageio.h"
#include "imap/resource.h"
#include "imap/services.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  class Persist : 
    public scfImplementation3<Persist, 
                              iResourceLoader,
                              iResourceSaver,
                              iComponent>
  {
  public:
    /// Constructor.
    Persist (iBase*);

    /// Destructor.
    virtual ~Persist ();

    virtual bool Initialize (iObjectRegistry *object_reg);

    //// iResourceLoader ////
    virtual csPtr<iResource> Load (iDocumentNode* node);
    virtual csPtr<iResource> Load (CS::Resource::TypeID typeID, iDataBuffer* node);

    //// iResourceSaver ////
    virtual bool Save (iResource* resource, iDocumentNode* node);
    virtual csPtr<iDataBuffer> Save (iResource* resource);

  protected:
    csPtr<iResource> LoadImage (iDocumentNode* node);
    csPtr<iResource> LoadImage (iDataBuffer* buf);
    bool SaveImage (iResource* resource, iDocumentNode* node);
    csPtr<iDataBuffer> SaveImage (iResource* resource);

    csPtr<iResource> LoadLibrary (iDocumentNode* node);

  private:
    iObjectRegistry* objectReg;
    csRef<iSyntaxService> syntaxService;
    csRef<iStringSet> strings;

    // VFS
    csRef<iVFS> vfs;

    // Image loader.
    csRef<iImageIO> imageLoader;

    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/engine/persist/persist.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
  };
}
CS_PLUGIN_NAMESPACE_END (Engine)

#endif // __CS_ENGINE_PERSIST_H__
