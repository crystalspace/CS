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

#include "crystalspace.h"

#include "processor.h"
#include "genmeshify.h"

namespace genmeshify
{
  Processor::Processor (App* app, const char* filename) : app (app),
    filename (filename), converter (0)
  {
    InitTokenTable (xmltokens);

    static int n = 0;
    region = app->engine->CreateRegion (
      csString().Format ("__genmeshify_region_%d__", n++));
    loaderContext = app->engine->CreateLoaderContext (region, false);
    converter = new Converter (app, loaderContext, region);
  }

  Processor::~Processor ()
  {
    delete converter;
    app->engine->RemoveObject (region);
    region->DeleteAll();
  }

  bool Processor::Process ()
  {
    //Change path
    csStringArray paths;
    paths.Push ("/lev/");
    if (!app->vfs->ChDirAuto (filename, &paths, 0, "world"))
    {
      app->Report ("Error setting directory '%s'!", filename.GetData());
      return false;
    }

    // Load it
    csRef<iFile> buf = app->vfs->Open ("world", VFS_FILE_READ);
    if (!buf) 
    {
      app->Report ("Error opening file 'world' for reading!");
      return false;
    }
    csRef<iDataBuffer> data = buf->GetAllData ();
    if (!app->vfs->WriteFile ("world~", data->GetData(), data->GetSize()))
    {
      app->Report ("Error writing backup file 'world~'!");
      return false;
    }

    csRef<iDocument> doc = app->docSystem->CreateDocument ();
    const char* error = doc->Parse (buf, true);
    if (error != 0)
    {
      app->Report ("Document system error: %s!", error);
      return false;
    }
    data = 0; buf = 0;
    csRef<iDocumentNode> root = doc->GetRoot();
    csRef<iDocumentNode> contents;

    csRef<iDocument> newDoc = app->docSystem->CreateDocument ();
    csRef<iDocumentNode> newRoot = newDoc->CreateRoot();

    if ((contents = root->GetNode ("world"))
      || (contents = root->GetNode ("library")))
    {
      newRoot->SetValue (root->GetValue ());
      csRef<iDocumentNodeIterator> it = root->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        csRef<iDocumentNode> child_clone = newRoot->CreateNodeBefore (
  	  child->GetType (), 0);
        if (child->Equals (contents))
        {
          // Handle world/library
          if (!ProcessWorld (contents, child_clone)) return false;
        }
        else
        {
          CloneNode (child, child_clone);
        }
      }
    }
#if 0
    else if ((contents = root->GetNode ("meshfact")))
    {
      // Handle meshfact
    }
    else if ((contents = root->GetNode ("meshobj")))
    {
      // Handle meshfact
    }
#endif
    else 
    {
      app->Report ("Unrecognized type of data");
      return false;
    }

    buf = app->vfs->Open ("world", VFS_FILE_WRITE);
    if (!buf) 
    {
      app->Report ("Error opening file 'world' for writing!");
      return false;
    }
    error = newDoc->Write (buf);
    if (error != 0)
    {
      app->Report ("Document system error: %s!", error);
      return false;
    }

    return true;
  }

  void Processor::CloneNode (iDocumentNode* from, iDocumentNode* to)
  {
    to->SetValue (from->GetValue ());
    csRef<iDocumentNodeIterator> it = from->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	  child->GetType (), 0);
      CloneNode (child, child_clone);
    }
    CloneAttributes (from, to);
  }

  void Processor::CloneAttributes (iDocumentNode* from, iDocumentNode* to)
  {
    csRef<iDocumentAttributeIterator> atit = from->GetAttributes ();
    while (atit->HasNext ())
    {
      csRef<iDocumentAttribute> attr = atit->Next ();
      to->SetAttribute (attr->GetName (), attr->GetValue ());
    }
  }

  bool Processor::ProcessWorld (iDocumentNode* from, iDocumentNode* to)
  {
    PreloadTexturesMaterialsLibs (from);
    PreloadSectors (from);

    to->SetValue (from->GetValue ());
    csRef<iDocumentNodeIterator> it = from->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType() == CS_NODE_ELEMENT)
      {
        csStringID token = xmltokens.Request(child->GetValue());
        switch (token)
        {
          case XMLTOKEN_PLUGINS:
            {
              csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	          child->GetType (), 0);
              if (!ProcessPlugins (child, child_clone)) return false;
              continue;
            }
          case XMLTOKEN_MESHFACT:
            {
              csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	          child->GetType (), 0);
              if (!ProcessMeshfactOrObj (0, child, child_clone, 0, true)) return false;
              continue;
            }
          case XMLTOKEN_SECTOR:
            {
              if (!texturesNode.IsValid())
              {
                texturesNode = to->CreateNodeBefore (CS_NODE_ELEMENT, 0);
                texturesNode->SetValue ("textures");
              }
              csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	          child->GetType (), 0);
              if (!ProcessSector (child, child_clone)) return false;
              continue;
            }
          case XMLTOKEN_TEXTURES:
            {
              csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	          child->GetType (), 0);
              CloneNode (child, child_clone);
              texturesNode = child_clone;
              continue;
            }
        }
      }
      csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	  child->GetType (), 0);
      CloneNode (child, child_clone);
    }
    CloneAttributes (from, to);
    return true;
  }

  bool Processor::ProcessPlugins (iDocumentNode* from, iDocumentNode* to)
  {
    csRef<iDocumentNodeIterator> pluginIt = from->GetNodes ("plugin");
    while (pluginIt->HasNext ())
    {
      csRef<iDocumentNode> pluginNode = pluginIt->Next ();
      csString shortcut (pluginNode->GetAttributeValue ("name"));
      if (shortcut.IsEmpty()) continue;

      csRef<iDocumentNode> idNode = pluginNode->GetNode ("id");
      csString id;
      if (idNode.IsValid())
        id = idNode->GetContentsValue();
      else
        id = pluginNode->GetContentsValue();

      plugins.PutUnique (shortcut, id);
    }

    CloneNode (from, to);
    return true;
  }

  bool Processor::ProcessSector (iDocumentNode* from, iDocumentNode* to)
  {
    to->SetValue (from->GetValue ());

    const char* name = from->GetAttributeValue ("name");
    iSector* sector = app->engine->FindSector (name);
    if (!sector)
    {
      sector = app->engine->CreateSector (0); 
      region->QueryObject()->ObjAdd (sector->QueryObject());
    }

    csRef<iDocumentNodeIterator> it = from->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType() == CS_NODE_ELEMENT)
      {
        csStringID token = xmltokens.Request(child->GetValue());
        switch (token)
        {
          case XMLTOKEN_MESHOBJ:
            {
              csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	          child->GetType (), 0);
              if (!ProcessMeshfactOrObj (sector, child, child_clone, to, 
                false)) return false;
              continue;
            }
        }
      }
      csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	  child->GetType (), 0);
      CloneNode (child, child_clone);
    }
    CloneAttributes (from, to);
    return true;
  }

  bool Processor::ProcessMeshfactOrObj (iSector* sector, 
                                        iDocumentNode* from, 
                                        iDocumentNode* to, 
                                        iDocumentNode* sectorNode,
                                        bool factory)
  {
    do
    {
      // Test if it's a Thing
      csRef<iDocumentNode> pluginNode = from->GetNode ("plugin");
      if (!pluginNode.IsValid()) break;
      csString plugin (GetPluginClassID (pluginNode->GetContentsValue ()));
      if (plugin.IsEmpty()) break;
      if (factory && (plugin != "crystalspace.mesh.loader.factory.thing")) break;
      if (!factory && (plugin != "crystalspace.mesh.loader.thing")) break;

      // Yep, thing.
      to->SetValue (from->GetValue ());
      CloneAttributes (from, to);

      csRef<iDocumentNodeIterator> it = from->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        if (child->GetType() == CS_NODE_ELEMENT)
        {
          if (strcmp (child->GetValue (), "plugin") == 0)
            ; // Ignore this node
          else if (strcmp (child->GetValue (), "hardmove") == 0)
            ; // <hardmove> must be filtered; the converter will handle it
          else if (strcmp (child->GetValue (), "params") == 0)
          {
            csString meshName (from->GetAttributeValue ("name"));
            if (meshName.IsEmpty())
            {
              static int meshNum = 0;
              meshName.Format ("unnamedmesh%d", meshNum++);
            }
            if (!ConvertMeshfactOrObj (sector, meshName,
              from, to, sectorNode, factory)) 
              return false;
          }
          else
          {
            csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	        child->GetType (), 0);
            CloneNode (child, child_clone);
          }
        }
      }
      return true;
    }
    while (0);
    CloneNode (from, to);
    return true;
  }

  bool Processor::ConvertMeshfactOrObj (iSector* sector, 
                                        const char* meshName, 
                                        iDocumentNode* from, 
                                        iDocumentNode* to,
                                        iDocumentNode* sectorNode,
                                        bool factory)
  {
    if (factory)
      return converter->ConvertMeshFact (meshName, from, to);
    else
      return converter->ConvertMeshObj (sector, meshName, from, to, 
        sectorNode, texturesNode);
  }

  bool Processor::PreloadTexturesMaterialsLibs (iDocumentNode* from)
  {
    csRef<iDocument> newDoc = app->docSystem->CreateDocument ();
    csRef<iDocumentNode> newRoot = newDoc->CreateRoot();

    csRef<iDocumentNode> to = newRoot->CreateNodeBefore (from->GetType(), 0);
    to->SetValue (from->GetValue());
    CloneAttributes (from, to);

    csRef<iDocumentNodeIterator> it = from->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType() == CS_NODE_ELEMENT)
      {
        csStringID token = xmltokens.Request(child->GetValue());
        switch (token)
        {
          case XMLTOKEN_TEXTURES:
          case XMLTOKEN_MATERIALS:
          case XMLTOKEN_PLUGINS:
          case XMLTOKEN_LIBRARY:
            {
              csRef<iDocumentNode> child_clone = 
                to->CreateNodeBefore (child->GetType(), 0);
              CloneNode (child, child_clone);
            }
            break;
        }
      }
    }

    return app->loader->LoadMap (to, false, region, false);
  }

  bool Processor::PreloadSectors (iDocumentNode* from)
  {
    // Needed so portals are written out properly
    csRef<iDocumentNodeIterator> sectorIt = from->GetNodes ("sector");
    while (sectorIt->HasNext ())
    {
      csRef<iDocumentNode> child = sectorIt->Next();
      const char* name = child->GetAttributeValue ("name");
      if (name)
      {
        iSector* sector = app->engine->CreateSector (name); 
        region->QueryObject()->ObjAdd (sector->QueryObject());
      }
    }
    return true;
  }
}
