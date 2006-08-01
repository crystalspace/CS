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

#ifndef __CONVERTER_H__
#define __CONVERTER_H__

namespace genmeshify
{
  class App;

  class Converter
  {
    App* app;
    csRef<iLoaderContext> context;

    csRef<iLoaderPlugin> thingFactLoader;
    csRef<iLoaderPlugin> thingObjLoader;
    csRef<iSaverPlugin> gmfactSaver;
    csRef<iSaverPlugin> gmSaver;

    csRandomGen rng;

    struct PolyVertex
    {
      csVector3 pos;
      csVector3 normal;
      csVector2 tc;
    };
    struct Poly
    {
      csArray<PolyVertex> vertices;
    };

    bool CopyThingToGM (iThingFactoryState* from, iGeneralFactoryState* to);
  public:
    Converter (App* app, iLoaderContext* context);
  
    bool ConvertMeshFact (iDocumentNode* from, iDocumentNode* to);
    bool ConvertMeshObj (const char* name, iDocumentNode* from, 
      iDocumentNode* to, iDocumentNode* factoryInsertBefore);
  };
}

#endif // __CONVERTER_H__
