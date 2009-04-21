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
    Converter* converter;

    csRef<iCollection> collection;
    csRef<iLoaderContext> loaderContext;

    csRef<iDocumentNode> texturesNode;

    csHash<csString, csString> plugins;
    const char* GetPluginClassID (const char* name) const
    {
      const csString* p = plugins.GetElementPointer (name);
      if (p != 0) return *p;
      return name;
    }

    static csRef<iFile> OpenPath (App* app, const char* path, 
                                  csString& fileNameToOpen);

    /// Clone a node and children.
    void CloneNode (iDocumentNode* from, iDocumentNode* to);
    /// Clone attributes.
    void CloneAttributes (iDocumentNode* from, iDocumentNode* to);
    bool ProcessWorld (iDocumentNode* from, iDocumentNode* to, 
      bool shallow, bool nested);
    bool ProcessPlugins (iDocumentNode* from, iDocumentNode* to);
    bool ProcessSector (iDocumentNode* from, iDocumentNode* to);
    bool ProcessMeshfactOrObj (iSector* sector, iDocumentNode* from, 
      iDocumentNode* to, iDocumentNode* sectorNode, bool factory);
    bool ConvertMeshfactOrObj (iSector* sector, const char* meshName, 
      iDocumentNode* from, iDocumentNode* to, iDocumentNode* sectorNode,
      bool factory);

    /**
     * Load stuff that the loaded mesh factories may depend on: 
     * textures, materials, settings and libs (if shallow == true).
     */
    bool PreloadDependencies (iDocumentNode* from, bool shallow);
    bool PreloadSectors (iDocumentNode* from);
  public:
    Processor (App* app);
    ~Processor ();

    static bool Preload (App* app, const csStringArray& paths);
    bool Process (const char* filename, bool shallow, bool nested = false);
  };
}

#endif // __PROCESSOR_H__
