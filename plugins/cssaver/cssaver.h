#ifndef __CS_CSSAVER_H__
#define __CS_CSSAVER_H__

#include "imap/saver.h"
#include "csutil/ref.h"
#include "iutil/comp.h"
#include "csutil/cscolor.h"

struct iEngine;
struct iDocumentNode;

class csSaver : public iSaver {
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;

public:
  SCF_DECLARE_IBASE;

  csSaver(iBase *base);
  virtual ~csSaver();

  bool Initialize(iObjectRegistry *p);

  static csRef<iDocumentNode> CreateNode(csRef<iDocumentNode>& parent, const char* name);
  static csRef<iDocumentNode> CreateValueNode(csRef<iDocumentNode>& parent, const char* name, const char* value);
  static csRef<iDocumentNode> CreateValueNodeAsFloat(csRef<iDocumentNode>& parent, const char* name, float value);
  static csRef<iDocumentNode> CreateValueNodeAsColor(csRef<iDocumentNode>& parent, const char* name, const csColor &color);

  bool SaveTextures(csRef<iDocumentNode>& parent);
  bool SaveMaterials(csRef<iDocumentNode>& parent);

  iString* SaveMapFile();
  bool SaveMapFile(const char *filename);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif

