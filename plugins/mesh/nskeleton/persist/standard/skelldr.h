/*
    Copyright (C) 2006 by Hristo Hristov

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

#ifndef __CS_NSKELLDR_H__
#define __CS_NSKELLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"
#include "imesh/nskeleton.h"
#include "imesh/skelanim.h"

struct iReporter;
struct iObjectRegistry;
struct iSyntaxService;

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton)
{

class Loader :
  public scfImplementation2<Loader, iLoaderPlugin, iComponent>
{
public:
  Loader (iBase*);
  virtual ~Loader ();

  virtual bool Initialize (iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
private:
  virtual bool ParseBone (iDocumentNode* node,
    ::Skeleton::iSkeletonFactory* skelfact,
    ::Skeleton::iSkeletonFactory::iBoneFactory* parent);

  virtual bool ParseAnimation (iDocumentNode* node,
    ::Skeleton::iSkeletonFactory* skelfact);

  virtual bool ParseChannel (iDocumentNode* node,
    csRef< ::Skeleton::Animation::iChannel> animchan, float &chanlength);

  virtual bool ParsePositionRotation (iDocumentNode* node, csVector3 &pos,
    csQuaternion &rot);

  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;
};

}
CS_PLUGIN_NAMESPACE_END(Skeleton)

#endif // __CS_NSKELLDR_H__
