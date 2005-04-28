/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_CSSAVER_H__
#define __CS_CSSAVER_H__

#include "imap/saver.h"
#include "csutil/ref.h"
#include "iengine/engine.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/plugin.h"
#include "csutil/cscolor.h"
#include "csutil/hash.h"

struct iSyntaxService;

class csSaver : public iSaver
{
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;
  csRef<iSyntaxService> synldr;
  csRef<iPluginManager> plugin_mgr;
  csHash<csStrKey, csStrKey> plugins;
  csRef<iDocumentNode> before;

public:
  SCF_DECLARE_IBASE;

  csSaver(iBase*);
  virtual ~csSaver();

  bool Initialize(iObjectRegistry*);

  static csRef<iDocumentNode> CreateNode(
    iDocumentNode *parent, const char* name);
  static csRef<iDocumentNode> CreateValueNode(
    iDocumentNode *parent, const char* name, const char* value);

  const char* GetPluginName (const char* plugin, const char* type);
  bool SavePlugins (iDocumentNode* parent);

  bool SaveCameraPositions(iDocumentNode *parent);
  bool SaveMaterials(iDocumentNode *parent);
  bool SaveRenderPriorities(iDocumentNode* node);
  bool SaveSettings(iDocumentNode* node);
  bool SaveSectors(iDocumentNode *parent);
  bool SaveSequence(iDocumentNode *parent);
  bool SaveShaders(iDocumentNode *parent);
  bool SaveTextures(iDocumentNode *parent);
  bool SaveTriggers(iDocumentNode *parent);
  bool SaveVariables (iDocumentNode* node);

  bool SaveMeshFactories(iMeshFactoryList* factList, iDocumentNode *parent,
    iMeshFactoryWrapper* parentfact = 0);
  bool SavePortals(iPortal *portal, iDocumentNode *parent);
  bool SaveSectorLights(iSector *s, iDocumentNode *parent);
  bool SaveSectorMeshes(iMeshList *meshList, iDocumentNode *parent);

  bool SaveKeys (iDocumentNode* node, iObject* object);

  virtual csRef<iString> SaveMapFile();
  virtual bool SaveMapFile(const char *filename);
  virtual bool SaveMapFile(csRef<iDocumentNode> &root);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_CSSAVER_H__
