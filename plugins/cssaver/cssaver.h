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
#include "csutil/cscolor.h"

class csSaver : public iSaver
{
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;

public:
  SCF_DECLARE_IBASE;

  csSaver(iBase*);
  virtual ~csSaver();

  bool Initialize(iObjectRegistry*);

  static csRef<iDocumentNode> CreateNode(
    csRef<iDocumentNode>& parent, const char* name);
  static csRef<iDocumentNode> CreateValueNode(
    csRef<iDocumentNode>& parent, const char* name, const char* value);
  static csRef<iDocumentNode> CreateValueNodeAsFloat(
    csRef<iDocumentNode>& parent, const char* name, float value);
  static csRef<iDocumentNode> CreateValueNodeAsColor(
    csRef<iDocumentNode>& parent, const char* name, const csColor &color);

  bool SaveTextures(csRef<iDocumentNode>& parent);
  bool SaveMaterials(csRef<iDocumentNode>& parent);

  csRef<iString> SaveMapFile();
  bool SaveMapFile(const char *filename);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_CSSAVER_H__
