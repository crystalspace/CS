/*
  Copyright (C) 2005 by Marten Svanfeldt
            (C) 2006 by Frank Richter

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

#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include "converter.h"

namespace genmeshify
{
  class App;

  class Processor
  {
    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "apps/tools/genmeshify/processor.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE 

    App* app;
    csString filename;
    Converter* converter;

    csRef<iRegion> region;
    csRef<iLoaderContext> loaderContext;

    csHash<csString, csString> plugins;
    const char* GetPluginClassID (const char* name) const
    {
      const csString* p = plugins.GetElementPointer (name);
      if (p != 0) return *p;
      return name;
    }

    /// Clone a node and children.
    void CloneNode (iDocumentNode* from, iDocumentNode* to);
    /// Clone attributes.
    void CloneAttributes (iDocumentNode* from, iDocumentNode* to);
    bool ProcessWorld (iDocumentNode* from, iDocumentNode* to);
    bool ProcessPlugins (iDocumentNode* from, iDocumentNode* to);
    bool ProcessSector (iDocumentNode* from, iDocumentNode* to);
    bool ProcessMeshfactOrObj (iDocumentNode* from, iDocumentNode* to,
      iDocumentNode* factoryInsertBefore, bool factory);
    bool ConvertMeshfactOrObj (const char* name, iDocumentNode* from, 
      iDocumentNode* to, iDocumentNode* factoryInsertBefore, bool factory);

    bool PreloadTexturesMaterials (iDocumentNode* from);
  public:
    Processor (App* app, const char* filename);
    ~Processor ();

    bool Process ();
  };
}

#endif // __PROCESSOR_H__
