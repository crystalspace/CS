/*
  Copyright (C) 2011 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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
#ifndef __CS_ANIMESH_PERSIST_SPLITMORPH_H__
#define __CS_ANIMESH_PERSIST_SPLITMORPH_H__

#include "cssysdef.h"
#include "csutil/csstring.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"
#include "iutil/comp.h"
#include "imap/reader.h"

CS_PLUGIN_NAMESPACE_BEGIN (SplitMorph)
{

  class SplitMorphLoader :
  public scfImplementation2<SplitMorphLoader,
    iLoaderPlugin,
    iComponent>
  {
  public:
    SplitMorphLoader (iBase* parent);

    //-- iLoaderPlugin
    virtual csPtr<iBase> Parse (iDocumentNode* node,
      iStreamSource* ssource, iLoaderContext* ldr_context,
      iBase* context);

    // TODO: How to know if the loader that will be used is or not safe?
    virtual bool IsThreadSafe() { return true; }

    //-- iComponent
    virtual bool Initialize (iObjectRegistry* registry);

  private:
    iObjectRegistry* registry;
    csRef<iSyntaxService> synldr;

    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/animesh/persist/splitmorph/splitmorph.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
  };

}
CS_PLUGIN_NAMESPACE_END (SplitMorph)


#endif // __CS_ANIMESH_PERSIST_SPLITMORPH_H__
