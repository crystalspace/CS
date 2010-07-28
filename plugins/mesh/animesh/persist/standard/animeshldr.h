/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_ANIMESHLDR_H__
#define __CS_ANIMESHLDR_H__

#include "csutil/csstring.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"
#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/comp.h"


CS_PLUGIN_NAMESPACE_BEGIN(Animeshldr)
{

  class AnimeshFactoryLoader :
    public scfImplementation2<AnimeshFactoryLoader,
                              iLoaderPlugin,
                              iComponent>
  {
  public:
    AnimeshFactoryLoader (iBase* parent);

    //-- iLoaderPlugin
    virtual csPtr<iBase> Parse (iDocumentNode* node,
      iStreamSource* ssource, iLoaderContext* ldr_context,
      iBase* context);

    virtual bool IsThreadSafe() { return true; }

    bool ParseMorphTarget (iDocumentNode* child,
      CS::Mesh::iAnimatedMeshFactory* amfact);

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

  private: 
    iObjectRegistry* object_reg;

    csRef<iSyntaxService> synldr;
    csRef<CS::Animation::iSkeletonManager2> skelMgr;

    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/animesh/persist/standard/animesh_factory.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
  };


  class AnimeshFactorySaver :
    public scfImplementation2<AnimeshFactorySaver,
                              iSaverPlugin,
                              iComponent>
  {
  public:
    AnimeshFactorySaver (iBase* parent);

    //-- iSaverPlugin
    virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
      iStreamSource*);

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);
  };


  class AnimeshObjectLoader :
    public scfImplementation2<AnimeshObjectLoader,
                              iLoaderPlugin,
                              iComponent>
  {
  public:
    AnimeshObjectLoader (iBase* parent);

    //-- iLoaderPlugin
    virtual csPtr<iBase> Parse (iDocumentNode* node,
      iStreamSource* ssource, iLoaderContext* ldr_context,
      iBase* context);

    virtual bool IsThreadSafe() { return true; }

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

  private: 
    iObjectRegistry* object_reg;
    csRef<iSyntaxService> synldr;
    csRef<CS::Animation::iSkeletonManager2> skelMgr;

    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/animesh/persist/standard/animesh_meshobject.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
  };


  class AnimeshObjectSaver :
    public scfImplementation2<AnimeshObjectSaver,
                              iSaverPlugin,
                              iComponent>
  {
  public:
    AnimeshObjectSaver (iBase* parent);

    //-- iSaverPlugin
    virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
      iStreamSource*);

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);
  };


}
CS_PLUGIN_NAMESPACE_END(Animeshldr)


#endif
