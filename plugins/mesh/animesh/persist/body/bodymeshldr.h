/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
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
#ifndef __CS_BODYMESHLDR_H__
#define __CS_BODYMESHLDR_H__

#include "csutil/scf_implementation.h"
#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/comp.h"
#include "csutil/csstring.h"
#include "imesh/bodymesh.h"


CS_PLUGIN_NAMESPACE_BEGIN(BodyMeshLdr)
{

  class BodyMeshLoader :
    public scfImplementation2<BodyMeshLoader,
                              iLoaderPlugin,
                              iComponent>
  {
  public:
    BodyMeshLoader (iBase* parent);

    //-- iLoaderPlugin
    virtual csPtr<iBase> Parse (iDocumentNode* node,
      iStreamSource* ssource, iLoaderContext* ldr_context,
      iBase* context);

    virtual bool IsThreadSafe() { return true; }

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

  private: 
    bool ParseSkeleton (iDocumentNode* node, iLoaderContext* ldr_context);
    bool ParseBone (iDocumentNode* node, iLoaderContext* ldr_context,
		    CS::Animation::iBodySkeleton* skeleton);
    bool ParseProperties (iDocumentNode* node, CS::Animation::iBodyBone* bone);
    bool ParseColliders (iDocumentNode* node, iLoaderContext* ldr_context,
			 CS::Animation::iBodyBone* bone);
    bool ParseColliderParams (iDocumentNode* node, CS::Animation::iBodyBoneCollider* collider);
    bool ParseJoint (iDocumentNode* node, CS::Animation::iBodyBone* bone);
    bool ParseConstraint (iDocumentNode *node, bool &x, bool &y, bool &z,
			  csVector3 &min, csVector3 &max);
    bool ParseChain (iDocumentNode* node, CS::Animation::iBodySkeleton* skeleton);

    iObjectRegistry* object_reg;
    csRef<iSyntaxService> synldr;
    csRef<CS::Animation::iBodyManager> bodyManager;

    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/animesh/persist/body/bodymeshldr.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
  };


}
CS_PLUGIN_NAMESPACE_END(BodyMeshLdr)

#endif
