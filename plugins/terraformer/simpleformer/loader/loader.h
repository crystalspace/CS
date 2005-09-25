/*
    Copyright (C) 2004 Anders Stenberg, Daniel Duhprey

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

#ifndef __LOADER_H__
#define __LOADER_H__

#include "imap/reader.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

struct iObjectRegistry;
struct iLoader;
struct iSyntaxService;
struct iPluginManager;
struct iEngine;

namespace cspluginSimpleFormerLoader
{

template<typename Tgetter>
class RawHeightmapReader;

class csSimpleFormerLoader : public iLoaderPlugin
{
public:
  SCF_DECLARE_IBASE;

  csSimpleFormerLoader (iBase*);
  virtual ~csSimpleFormerLoader ();

  bool Initialize (iObjectRegistry*);

  virtual csPtr<iBase> Parse (iDocumentNode *node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSimpleFormerLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;

  csRef<iSyntaxService> synldr;
  csRef<iDataBuffer> GetDataBuffer (iDocumentNode* child);
private:
  iObjectRegistry* objreg;
  csRef<iPluginManager> pluginmgr;
  csRef<iEngine> engine;
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/terraformer/simpleformer/loader/loader.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE 

  bool LoadHeightmap32 (iDocumentNode* child, iSimpleFormerState* state);
  bool LoadHeightmapRaw16LE (iDocumentNode* child, iSimpleFormerState* state);
  bool LoadHeightmapRaw16BE (iDocumentNode* child, iSimpleFormerState* state);
  bool LoadHeightmapRaw32LE (iDocumentNode* child, iSimpleFormerState* state);
  bool LoadHeightmapRaw32BE (iDocumentNode* child, iSimpleFormerState* state);
  bool LoadHeightmapRawFloatLE (iDocumentNode* child, iSimpleFormerState* state);
  bool LoadHeightmapRawFloatBE (iDocumentNode* child, iSimpleFormerState* state);
};

} // namespace cspluginSimpleFormerLoader

#endif /* __LOADER_H__ */
