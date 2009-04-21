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

#ifndef __CS_SKELETON2LDR_H__
#define __CS_SKELETON2LDR_H__

#include "csutil/scf_implementation.h"
#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/comp.h"
#include "csutil/csstring.h"
#include "imesh/skeleton2.h"


CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2Ldr)
{

  class SkeletonLoader :
    public scfImplementation2<SkeletonLoader,
                              iLoaderPlugin,
                              iComponent>
  {
  public:
    SkeletonLoader (iBase* parent);

    //-- iLoaderPlugin
    virtual csPtr<iBase> Parse (iDocumentNode* node,
      iStreamSource* ssource, iLoaderContext* ldr_context,
      iBase* context);

    virtual bool IsThreadSafe() { return true; }

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

  private: 
    bool ParseSkeleton (iDocumentNode* node);
    bool ParseBone (iDocumentNode* node, iSkeletonFactory2* factory, BoneID parent);

    bool ParseAnimPacket (iDocumentNode* node);    
    iSkeletonAnimation2* ParseAnimation (iDocumentNode* node, 
      iSkeletonAnimPacketFactory2* packet);

    csPtr<iSkeletonAnimNodeFactory2> ParseAnimTreeNode (iDocumentNode* node,
      iSkeletonAnimPacketFactory2* packet);

    csPtr<iSkeletonAnimNodeFactory2> ParseAnimationNode (iDocumentNode* node,
      iSkeletonAnimPacketFactory2* packet);
    csPtr<iSkeletonAnimNodeFactory2> ParseBlendNode (iDocumentNode* node,
      iSkeletonAnimPacketFactory2* packet);
    csPtr<iSkeletonAnimNodeFactory2> ParsePriorityNode (iDocumentNode* node,
      iSkeletonAnimPacketFactory2* packet);
    csPtr<iSkeletonAnimNodeFactory2> ParseRandomNode (iDocumentNode* node,
      iSkeletonAnimPacketFactory2* packet);
    csPtr<iSkeletonAnimNodeFactory2> ParseFSMNode (iDocumentNode* node,
      iSkeletonAnimPacketFactory2* packet);

    iObjectRegistry* object_reg;
    csRef<iSyntaxService> synldr;
    csRef<iSkeletonManager2> skelManager;

    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/animesh/persist/skeleton2/skeleton2ldr.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
  };



}
CS_PLUGIN_NAMESPACE_END(Skeleton2Ldr)


#endif
