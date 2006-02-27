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

#ifndef __CS_SKELLDR_H__
#define __CS_SKELLDR_H__

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
struct iGeneralFactoryState;
struct iGeneralMeshState;

class csSkeletonFactoryLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  SCF_DECLARE_IBASE;

  csSkeletonFactoryLoader (iBase*);

  virtual ~csSkeletonFactoryLoader ();

  virtual bool Initialize (iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  const char *ParseBone (iDocumentNode* node, 
    iSkeletonFactory *skel_fact, iSkeletonBoneFactory *parent_bone);

  const char *ParseScript (iDocumentNode* node, iSkeletonFactory *skel_fact);
  const char *ParseFrame (iDocumentNode* node, iSkeletonFactory *skel_fact, 
    iSkeletonScript *script);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSkeletonFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_GMESHLDR_H__

