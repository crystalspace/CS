/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

#include "cssysdef.h"

#include <ctype.h>
#include "csqint.h"

#include "cstool/saverref.h"
#include "cstool/saverfile.h"
#include "cstool/unusedresourcehelper.h"
#include "cstool/vfsdirchange.h"

#include "csutil/documenthelper.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/scfstr.h"
#include "csutil/scfstringarray.h"
#include "csutil/threadmanager.h"
#include "csutil/xmltiny.h"

#include "iengine/engine.h"
#include "iengine/imposter.h"
#include "iengine/lod.h"
#include "iengine/mesh.h"
#include "iengine/meshgen.h"
#include "iengine/movable.h"
#include "iengine/portalcontainer.h"
#include "iengine/scenenode.h"
#include "iengine/sharevar.h"
#include "iengine/viscull.h"

#include "igeom/trimesh.h"
#include "igraphic/animimg.h"
#include "igraphic/imageio.h"

#include "imap/modelload.h"
#include "imap/saverfile.h"
#include "imap/saverref.h"
#include "imap/services.h"

#include "imesh/object.h"
#include "imesh/objmodel.h"
#include "imesh/nullmesh.h"

#include "isndsys/ss_manager.h"

#include "itexture/iproctex.h"

#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#include "ivaria/keyval.h"
#include "ivaria/terraform.h"

#include "ivideo/material.h"

#include "csloadercontext.h"
#include "csthreadedloader.h"
#include "loadtex.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  SCF_IMPLEMENT_FACTORY(csThreadedLoader)

  csThreadedLoader::csThreadedLoader(iBase *p)
  : scfImplementationType (this, p), loaderFlags (CS_LOADER_NONE), listSync(false)
  {
  }

  csThreadedLoader::~csThreadedLoader()
  {
    csRef<iEventQueue> queue (csQueryRegistry<iEventQueue> (object_reg));
    if (queue)
    {
      CS::RemoveWeakListener (queue, eventHandler);
    }
  }

  bool csThreadedLoader::Initialize(iObjectRegistry *objectreg)
  {
    object_reg = objectreg;
    loaded_plugins.SetObjectRegistry (object_reg);

    csRef<iPluginManager> plugin_mgr = csQueryRegistry<iPluginManager>(object_reg);
    loaded_plugins.plugin_mgr = plugin_mgr;

    threadman = csQueryRegistry<iThreadManager>(object_reg);
    if(!threadman.IsValid())
    {
      ReportError("crystalspace.level.threadedloader", "Failed to find the thread manager!");
      return false;
    }

    Engine = csQueryRegistry<iEngine>(object_reg);
    if(!Engine.IsValid())
    {
      ReportError("crystalspace.level.threadedloader", "Failed to find the engine plugin!");
      return false;
    }

    vfs = csQueryRegistry<iVFS>(object_reg);
    if(!vfs.IsValid())
    {
      ReportError("crystalspace.level.threadedloader", "Failed to find VFS!");
      return false;
    }

    SyntaxService = csQueryRegistryOrLoad<iSyntaxService> (object_reg,
  	"crystalspace.syntax.loader.service.text");
    if(!SyntaxService.IsValid())
    {
      return false;
    }

    InitTokenTable(xmltokens);

    stringSet = csQueryRegistryTagInterface<iStringSet>(object_reg,
      "crystalspace.shared.stringset");
    stringSetSvName = csQueryRegistryTagInterface<iShaderVarStringSet>(object_reg,
      "crystalspace.shader.variablenameset");

    ImageLoader = csQueryRegistry<iImageIO> (object_reg);
    if(!ImageLoader)
    {
      ReportError("crystalspace.level.threadedloader", "Failed to find an image loader!");
      return false;
    }

    g3d = csQueryRegistry<iGraphics3D> (object_reg);
    if(!g3d)
    {
      ReportError("crystalspace.level.threadedloader", "Failed to find iGraphics3D!");
      return false;
    }

    // Optional
    eseqmgr = csQueryRegistryOrLoad<iEngineSequenceManager> (object_reg,
      "crystalspace.utilities.sequence.engine", false);
    SndSysManager = csQueryRegistryOrLoad<iSndSysManager> (object_reg,
      "crystalspace.sndsys.manager", false);
    SndSysLoader = csQueryRegistryOrLoad<iSndSysLoader> (object_reg,
        "crystalspace.sndsys.element.loader", false);
    SndSysRenderer = csQueryRegistry<iSndSysRenderer> (object_reg);

    csRef<iEventQueue> queue (csQueryRegistry<iEventQueue> (object_reg));
    if (queue)
    {
      CS::RegisterWeakListener (queue, this, csevSystemClose (object_reg),
        eventHandler);
    }

    return true;
  }

  bool csThreadedLoader::HandleEvent (iEvent& event)
  {
    // We only receive close events

    /* Clear loader list when system closes, discarding anything not yet added
       to the engine. Presumably, that stuff isn't needed anyway since we
       close ... */
    ClearLoaderLists ();

    return false;
  }

  THREADED_CALLABLE_IMPL4(csThreadedLoader, LoadMeshObjectFactory, const char* cwd, const char* fname,
    csRef<iStreamSource> ssource, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
      new csLoaderContext (object_reg, Engine, this, 0, 0, KEEP_USED, do_verbose));

    csRef<iFile> databuff (vfs->Open (fname, VFS_FILE_READ));

    if (!databuff || !databuff->GetSize ())
    {
      ReportError (
        "crystalspace.maploader.parse.meshfactory",
        "Could not open mesh object file '%s' on VFS!", fname);
      return false;
    }

    csRef<iDocument> doc;
    bool er = LoadStructuredDoc (fname, databuff, doc);
    if (!er)
    {
      return false;
    }

    if (doc)
    {
      csRef<iDocumentNode> meshfactnode = doc->GetRoot ()->GetNode ("meshfact");
      if (!meshfactnode)
      {
        ReportError (
          "crystalspace.maploader.parse.map",
          "File '%s' does not seem to contain a 'meshfact'!", fname);
        return false;
      }
      csRef<iMeshFactoryWrapper> t = Engine->CreateMeshFactory (
        meshfactnode->GetAttributeValue ("name"), false);
      if (LoadMeshObjectFactory (ldr_context, t, 0, meshfactnode, 0, ssource))
      {
        ldr_context->AddToCollection(t->QueryObject ());
        AddMeshFactToList(t);
        ret->SetResult(csRef<iBase>(t));

        if(sync)
        {
          Engine->SyncEngineListsWait(this);
        }

        return true;
      }
    }
    else
    {
      ReportError ("crystalspace.maploader.parse.plugin",
        "File does not appear to be a structured mesh factory (%s)!", fname);
    }
    return false;
  }

  THREADED_CALLABLE_IMPL4(csThreadedLoader, LoadMeshObject, const char* cwd, const char* fname,
    csRef<iStreamSource> ssource, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csRef<iFile> databuff (vfs->Open (fname, VFS_FILE_READ));
    csRef<iMeshWrapper> mesh;
    csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
      new csLoaderContext (object_reg, Engine, this, 0, 0, KEEP_USED, do_verbose));

    if (!databuff || !databuff->GetSize ())
    {
      ReportError (
        "crystalspace.maploader.parse.meshobject",
        "Could not open mesh object file '%s' on VFS!", fname);
      return false;
    }

    csRef<iDocument> doc;
    bool er = LoadStructuredDoc (fname, databuff, doc);
    if (!er)
    {
      return false;
    }

    if (doc)
    {
      csRef<iDocumentNode> meshobjnode = doc->GetRoot ()->GetNode ("meshobj");
      if (!meshobjnode)
      {
        ReportError (
          "crystalspace.maploader.parse.map",
          "File '%s' does not seem to contain a 'meshobj'!", fname);
        return false;
      }
      const char* name = meshobjnode->GetAttributeValue ("name");
      mesh = Engine->CreateMeshWrapper (name, false);
      csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
      csString cwd = vfs->GetCwd ();
      if(LoadMeshObjectTC (itr, false, ldr_context, mesh, 0, meshobjnode, ssource, 0, name, cwd))
      {
        ret->Copy(itr);

        if(sync)
        {
          Engine->SyncEngineListsWait(this);
        }

        return true;
      }
    }
    else
    {
      ReportError ("crystalspace.maploader.parse.plugin",
        "File does not appear to be a structured mesh object (%s)!", fname);
    }
    return false;
  }

  THREADED_CALLABLE_IMPL4(csThreadedLoader, LoadShader, const char* cwd, const char* filename,
    bool registerShader, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csRef<iShaderManager> shaderMgr = csQueryRegistry<iShaderManager> (
      object_reg);

    csRef<iFile> shaderFile = vfs->Open (filename, VFS_FILE_READ);
    if (!shaderFile)
    {
      ReportError ("crystalspace.maploader",
        "Unable to open shader file '%s'!", filename);
      return false;
    }

    csRef<iDocumentSystem> docsys = csQueryRegistry<iDocumentSystem> (
      object_reg);
    if (docsys == 0)
      docsys.AttachNew (new csTinyDocumentSystem ());

    csRef<iDocument> shaderDoc = docsys->CreateDocument ();
    const char* err = shaderDoc->Parse (shaderFile, false);
    if (err != 0)
    {
      ReportError ("crystalspace.maploader",
        "Could not parse shader file '%s': %s",
        filename, err);
      return false;
    }
    csRef<iDocumentNode> shaderNode = shaderDoc->GetRoot ()->GetNode ("shader");
    if (!shaderNode)
    {
      ReportError ("crystalspace.maploader",
        "Shader file '%s' is not a valid shader XML file!", filename);
      return false;
    }

    dirChange.ChangeTo (filename);

    const char* type = shaderNode->GetAttributeValue ("compiler");
    if (type == 0)
      type = shaderNode->GetAttributeValue ("type");
    if (type == 0)
      type = "xmlshader";
    csRef<iShaderCompiler> shcom = shaderMgr->GetCompiler (type);

    csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
      new csLoaderContext (object_reg, Engine, this, 0, 0, KEEP_USED, do_verbose));

    csRef<iShader> shader = shcom->CompileShader (ldr_context, shaderNode);
    if (shader)
    {
      shader->SetFileName (filename);
      if (registerShader)
        shaderMgr->RegisterShader (shader);
      ret->SetResult(csRef<iBase>(shader));

      if(sync)
      {
        Engine->SyncEngineListsWait(this);
      }

      return true;
    }
    return false;
  }

  THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadMapFile, const char* cwd, const char* filename,
    bool clearEngine, csRef<iCollection> collection, csRef<iStreamSource> ssource,
    csRef<iMissingLoaderData> missingdata, uint keepFlags, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csRef<iFile> buf = vfs->Open (filename, VFS_FILE_READ);

    if (!buf)
    {
      ReportError (
        "crystalspace.maploader.parse.map",
        "Could not open map file '%s' on VFS!", filename);
      return false;
    }

    csRef<iDocument> doc;
    bool er = LoadStructuredDoc (filename, buf, doc);
    if (!er)
    {
      return false;
    }

    if (doc)
    {
      csRef<iDocumentNode> world_node = doc->GetRoot ()->GetNode ("world");
      if (!world_node)
      {
        SyntaxService->ReportError (
          "crystalspace.maploader.parse.expectedworld",
          world_node, "Expected 'world' token!");
        return false;
      }

      if (Engine->GetSaveableFlag () && collection)
      {
        csRef<iSaverFile> saverFile;
        saverFile.AttachNew (new csSaverFile (filename, CS_SAVER_FILE_WORLD));
        collection->Add(saverFile->QueryObject());
      }

      bool res = LoadMapTC (ret, false, cwd, world_node, clearEngine, collection, 
        ssource, missingdata, keepFlags, do_verbose);

      if(sync && res)
      {
        Engine->SyncEngineListsWait(this);
      }

      return res;
    }
    else
    {
      ReportError ("crystalspace.maploader.parse.plugin", 
        "File does not appear to be a structured map file (%s)!", filename);
    }
    return false;
  }

  THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadMap, const char* cwd, csRef<iDocumentNode> world_node,
    bool clearEngine, csRef<iCollection> collection, csRef<iStreamSource> ssource,
    csRef<iMissingLoaderData> missingdata, uint keepFlags, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    if (clearEngine)
    {
      Engine->DeleteAll ();
      Engine->ResetWorldSpecificSettings();
    }
    csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
      new csLoaderContext (object_reg, Engine, this, collection,
      missingdata, keepFlags, do_verbose));

    bool res = LoadMap (ldr_context, world_node, ssource, missingdata, do_verbose);

    if(sync && res)
    {
      Engine->SyncEngineListsWait(this);
    }

    return res;
  }

  THREADED_CALLABLE_IMPL7(csThreadedLoader, LoadLibraryFile, const char* cwd, const char* filename,
    csRef<iCollection> collection, csRef<iStreamSource> ssource, csRef<iMissingLoaderData> missingdata,
    uint keepFlags, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csRef<iFile> buf = vfs->Open (filename, VFS_FILE_READ);

    if (!buf)
    { 
      ReportError (
        "crystalspace.maploader.parse.library",
        "Could not open library file '%s' on VFS!", filename);
      return false;
    }

    if(Engine->GetSaveableFlag () && collection)
    {
      csRef<iSaverFile> saverFile;
      saverFile.AttachNew (new csSaverFile (filename, CS_SAVER_FILE_LIBRARY));
      collection->Add(saverFile->QueryObject());
    }

    csRef<iDocument> doc;
    bool er = LoadStructuredDoc (filename, buf, doc);
    if (!er) return false;
    if (doc)
    {
      csRef<iDocumentNode> lib_node = doc->GetRoot ()->GetNode ("library");
      if (!lib_node)
      {
        SyntaxService->ReportError (
          "crystalspace.maploader.parse.expectedlib",
          lib_node, "Expected 'library' token!");
        return false;
      }

      bool res = LoadLibraryTC(ret, false, cwd, lib_node, collection, ssource, missingdata, keepFlags, do_verbose);

      if(sync && res)
      {
        Engine->SyncEngineListsWait(this);
      }

      return res;
    }
    else
    {
      ReportError ("crystalspace.maploader.parse.plugin",
        "File does not appear to be a structure map library (%s)!", filename);
    }
    return false;
  }

  THREADED_CALLABLE_IMPL7(csThreadedLoader, LoadLibrary, const char* cwd, csRef<iDocumentNode> lib_node,
    csRef<iCollection> collection, csRef<iStreamSource> ssource, csRef<iMissingLoaderData> missingdata,
    uint keepFlags, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext>
      (new csLoaderContext (object_reg, Engine, this, collection,
      missingdata, keepFlags, do_verbose));

    // Arrays for 'inlined' libraries.
    csRefArray<iDocumentNode> libs;
    csArray<csString> libIDs;

    // Pre-parse.
    ParseAvailableObjects(dynamic_cast<csLoaderContext*>((iLoaderContext*)ldr_context),
      lib_node, libs, libIDs);

    // Array of all thread jobs created from this parse.
    csRefArray<iThreadReturn> threadReturns;

    // The actual parse.
    bool success = LoadLibrary (ldr_context, lib_node, ssource, missingdata, threadReturns, libs, libIDs);

    if(sync && success)
    {
      Engine->SyncEngineListsWait(this);
    }

    // Wait for all jobs to finish.
    return threadman->Wait(threadReturns) && success;
  }

  THREADED_CALLABLE_IMPL7(csThreadedLoader, LoadFile, const char* cwd, const char* fname,
    csRef<iCollection> collection, csRef<iStreamSource> ssource, csRef<iMissingLoaderData> missingdata,
    uint keepFlags, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csRef<iDataBuffer> buf = vfs->ReadFile (fname);

    if (!buf)
    {
      ReportError("crystalspace.maploader.parse", "Could not open map file '%s' on VFS!", fname);
      return false;
    }

    bool res = Load (ret, buf, fname, collection, ssource, missingdata, keepFlags, do_verbose);

    if(sync && res)
    {
      Engine->SyncEngineListsWait(this);
    }

    return res;
  }

  THREADED_CALLABLE_IMPL7(csThreadedLoader, LoadBuffer, const char* cwd, csRef<iDataBuffer> buffer,
    csRef<iCollection> collection, csRef<iStreamSource> ssource, csRef<iMissingLoaderData> missingdata,
    uint keepFlags, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    bool res = Load(ret, buffer, 0, collection, ssource, missingdata, keepFlags, do_verbose);

    if(sync && res)
    {
      Engine->SyncEngineListsWait(this);
    }

    return res;
  }

  THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadNode, const char* cwd, csRef<iDocumentNode> node,
      csRef<iCollection> collection, csRef<iSector> sector, csRef<iStreamSource> ssource,
      csRef<iMissingLoaderData> missingdata, uint keepFlags, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csRef<iLoaderContext> ldr_context;
    ldr_context.AttachNew(new csLoaderContext(object_reg, Engine, this, collection,
      missingdata, keepFlags, do_verbose));

    for(int attempt=1; attempt<3; attempt++)
    {
      // texture.
      csRef<iDocumentNode> texturenode;
      if(attempt == 1)
      {
        texturenode = node->GetNode ("texture");
      }
      if(attempt == 2)
      {
        if(csString("texture") == csString(node->GetValue()))
          texturenode = node;
      }
      if (texturenode)
      {
        csSafeCopyArray<ProxyTexture> proxyTextures;
	csString cwd = vfs->GetCwd ();
        bool res = ParseTextureTC(ret, false, ldr_context, texturenode, &proxyTextures, cwd);
        if(sync && res)
        {
          Engine->SyncEngineListsWait(this);
        }
        return res;
      }

      // Mesh Factory
      csRef<iDocumentNode> meshfactnode;
      if(attempt == 1)
      {
        meshfactnode = node->GetNode ("meshfact");
      }
      if(attempt == 2)
      {
        if(csString("meshfact") == csString(node->GetValue()))
          meshfactnode = node;
      }
      if (meshfactnode)
      {
        bool res = FindOrLoadMeshFactoryTC(ret, false, 0, ldr_context, meshfactnode, 0, 0, ssource, 0);
        if(sync && res)
        {
          Engine->SyncEngineListsWait(this);
        }
        return res;
      }

      // Meshgen
      csRef<iDocumentNode> meshgennode;
      if(attempt == 1)
      {
        if(csString("meshgen") == csString(node->GetValue()))
          meshgennode = node;
      }
      if(attempt == 2)
      {
        meshgennode = node->GetNode ("meshgen");
      }
      if (meshgennode)
      {
        return LoadMeshGen(ldr_context, meshgennode, sector);
      }

      // Mesh Object
      csRef<iDocumentNode> meshobjnode;
      if(attempt == 1)
      {
        meshobjnode = node->GetNode ("meshobj");
      }
      if(attempt == 2)
      {
        if(csString("meshobj") == csString(node->GetValue()))
          meshobjnode = node;
      }
      if (meshobjnode)
      {
        const char* name = meshobjnode->GetAttributeValue ("name");
        csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (name, false);
        ret->SetResult(scfQueryInterfaceSafe<iBase>(mesh));
	csString cwd = vfs->GetCwd ();
        bool res = LoadMeshObjectTC(ret, false, ldr_context, mesh, 0, meshobjnode, ssource, sector, name, cwd);
        if(sync && res)
        {
          Engine->SyncEngineListsWait(this);
        }
        return res;
      }

      // Trimesh
      csRef<iDocumentNode> trimeshnode;
      if(attempt == 1)
      {
        if(csString("trimesh") == csString(node->GetValue()))
          trimeshnode = node;
      }
      if(attempt == 2)
      {
        trimeshnode = node->GetNode ("trimesh");
      }
      if (trimeshnode)
      {
        const char* name = trimeshnode->GetAttributeValue ("name");
        csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (
          "crystalspace.mesh.object.null", name, 0, csVector3(0), false);

        bool res = LoadTriMeshInSector (ldr_context, mesh, trimeshnode, ssource);
        ret->SetResult(scfQueryInterfaceSafe<iBase>(mesh));
        if(sync && res)
        {
          Engine->SyncEngineListsWait(this);
        }
        return res;
      }

      // World node.
      csRef<iDocumentNode> worldnode;
      if(attempt == 1)
      {
        worldnode = node->GetNode ("world");
      }
      if(attempt == 2)
      {
        if(csString("world") == csString(node->GetValue()))
          worldnode = node;
      }
      if (worldnode)
      {
        bool res = LoadMap (ldr_context, worldnode, ssource, missingdata, do_verbose);
        if(sync && res)
        {
          Engine->SyncEngineListsWait(this);
        }
        return res;
      }

      // Library node.
      csRef<iDocumentNode> libnode;
      if(attempt == 1)
      {
        libnode = node->GetNode ("library");
      }
      if(attempt == 2)
      {
        if(csString("library") == csString(node->GetValue()))
          libnode = node;
      }
      if (libnode)
      {
        bool res = LoadLibraryTC(ret, false, cwd, libnode, collection, ssource, missingdata, keepFlags, do_verbose);
        if(sync && res)
        {
          Engine->SyncEngineListsWait(this);
        }
        return res;
      }

      // Portals.
      csRef<iDocumentNode> portalsnode;
      if(attempt == 1)
      {
        portalsnode = node->GetNode ("portals");
      }
      if(attempt == 2)
      {
        if(csString("portals") == csString(node->GetValue()))
          portalsnode = node;
      }
      if (portalsnode)
      {
        const char* portalsname = portalsnode->GetAttributeValue ("name");
        if (portalsname)
        {
          csRef<iMeshWrapper> mw = Engine->FindMeshObject (portalsname);
          if (mw)
          {
            csRef<iPortalContainer> pc = 
              scfQueryInterface<iPortalContainer>(mw->GetMeshObject());
            if (pc)
            {
              ldr_context->AddToCollection(mw->QueryObject());
              ret->SetResult(csRef<iBase>(mw));
              if(sync)
              {
                Engine->SyncEngineListsWait(this);
              }
              return true;
            }
          }
        }
        if (ParsePortals (ldr_context, portalsnode, 0, 0, ssource))
        {
          csRef<iMeshWrapper> mw = 0;
          if (ldr_context->GetCollection())
            mw = ldr_context->GetCollection()->FindMeshObject(portalsname);

          if (mw)
          {
            mw->QueryObject()->SetName(portalsname);
            ret->SetResult(csRef<iBase>(mw));
            if(sync)
            {
              Engine->SyncEngineListsWait(this);
            }
            return true;
          }
        }

        return false;
      }

      // Light.
      csRef<iDocumentNode> lightnode;
      if(attempt == 1)
      {
        lightnode = node->GetNode ("light");
      }
      if(attempt == 2)
      {
        if(csString("light") == csString(node->GetValue()))
          lightnode = node;
      }
      if (lightnode)
      {
        const char* lightname = lightnode->GetAttributeValue ("name");
        csRef<iLight> light = ParseStatlight (ldr_context, lightnode);
        if (light)
        {
          light->QueryObject()->SetName(lightname);
          ret->SetResult(csRef<iBase>(light));
          if(sync)
          {
            Engine->SyncEngineListsWait(this);
          }
          return true;
        }

        return false;
      }

      // MeshRef
      csRef<iDocumentNode> meshrefnode;
      if(attempt == 1)
      {
        meshrefnode = node->GetNode ("meshref");
      }
      if(attempt == 2)
      {
        if(csString("meshref") == csString(node->GetValue()))
          meshrefnode = node;
      }
      if (meshrefnode)
      {
        const char* meshobjname = meshrefnode->GetAttributeValue ("name");
        if (meshobjname)
        {
          csRef<iMeshWrapper> mw = Engine->FindMeshObject (meshobjname);
          if (mw)
          {
            ldr_context->AddToCollection(mw->QueryObject());
            ret->SetResult(csRef<iBase>(mw));
            if(sync)
            {
              Engine->SyncEngineListsWait(this);
            }
            return true;
          }
        }
        csRef<iMeshWrapper> mw = LoadMeshObjectFromFactory (ldr_context, meshrefnode, ssource);
        if (mw)
        {
          ldr_context->AddToCollection(mw->QueryObject());
          ret->SetResult(csRef<iBase>(mw));
          if(sync)
          {
            Engine->SyncEngineListsWait(this);
          }
          return true;
        }
      }

      // Plugins
      csRef<iDocumentNode> pluginsnode;
      if(attempt == 1)
      {
        pluginsnode = node->GetNode ("plugins");
      }
      if(attempt == 2)
      {
        if(csString("plugins") == csString(node->GetValue()))
          pluginsnode = node;
      }
      if (pluginsnode)
      {
        LoadPlugins(pluginsnode);
        if(sync)
        {
          Engine->SyncEngineListsWait(this);
        }
        return true;
      }

      csRef<iDocumentNode> sequencenode;
      if(attempt == 1)
      {
          sequencenode = node->GetNode ("sequence");
      }
      if(attempt == 2)
      {
          if(csString("sequence") == csString(node->GetValue()))
              sequencenode = node;
      }
      if (sequencenode)
      {
          iSequenceWrapper* sw = CreateSequence(sequencenode);
          LoadSequence(ldr_context, sequencenode);
          ret->SetResult(csRef<iBase>(sw));
          if(sync)
          {
              Engine->SyncEngineListsWait(this);
          }
          return true;
      }

      csRef<iDocumentNode> triggernode;
      if(attempt == 1)
      {
          triggernode = node->GetNode ("trigger");
      }
      if(attempt == 2)
      {
          if(csString("trigger") == csString(node->GetValue()))
              triggernode = node;
      }
      if (triggernode)
      {
          iSequenceTrigger* st = LoadTrigger(ldr_context, triggernode);
          ret->SetResult(csRef<iBase>(st));
          if(sync)
          {
              Engine->SyncEngineListsWait(this);
          }
          return true;
      }
    }

    ReportError("crystalspace.maploader.parse",
      "File doesn't seem to be a world, library, meshfact, meshobj, meshref, portals, light or plugins file!");
    return false;
  }

  THREADED_CALLABLE_IMPL7(csThreadedLoader, FindOrLoadMeshFactory, const char* name,
    csRef<iLoaderContext> ldr_context, csRef<iDocumentNode> meshfactnode,
    csRef<iMeshFactoryWrapper> parent, csReversibleTransform* transf,
    csRef<iStreamSource> ssource, const char* path)
  {
    csVfsDirectoryChanger dirChange(vfs);
    if(path)
    {
      dirChange.ChangeTo(path);
    }

    const char* meshfactname = (name != 0) ? name : meshfactnode->GetAttributeValue("name");

    csRef<iMeshFactoryWrapper> mfw = ldr_context->FindMeshFactory(meshfactname, false);
    if(mfw)
    {
      ldr_context->AddToCollection(mfw->QueryObject());
      ret->SetResult(scfQueryInterface<iBase>(mfw));
      return true;
    }

    if(!AddLoadingMeshFact(meshfactname))
    {
      // Fixme.
      while(!mfw)
      {
        mfw = ldr_context->FindMeshFactory(meshfactname, false);
      }

      ldr_context->AddToCollection(mfw->QueryObject());
      ret->SetResult(scfQueryInterface<iBase>(mfw));
      return true;
    }

    mfw = Engine->CreateMeshFactory(meshfactname, false);
    if(!LoadMeshObjectFactory(ldr_context, mfw, parent, meshfactnode, transf, ssource))
    {
      return false;
    }

    AddMeshFactToList(mfw);
    RemoveLoadingMeshFact(meshfactname);
    ret->SetResult(scfQueryInterface<iBase>(mfw));
    return true;
  }

  bool csThreadedLoader::LoadShaderExpressions (iLoaderContext* ldr_context,
    iDocumentNode* node)
  {
    csRef<iShaderManager> shaderMgr (
      csQueryRegistry<iShaderManager> (object_reg));

    if(!shaderMgr)
    {
      ReportNotify ("iShaderManager not found, ignoring shader expressions!");
      return true;
    }

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_SHADEREXPRESSION:
        {
          csRef<iShaderVariableAccessor> ac = SyntaxService->
            ParseShaderVarExpr (child);
          if (!ac) return false;
          const char* name = child->GetAttributeValue ("name");
          csRef<iObject> obj = scfQueryInterface<iObject> (ac);
          if (obj)
          {
            obj->SetName (name);
            ldr_context->AddToCollection(obj);
          }
          shaderMgr->RegisterShaderVariableAccessor (name, ac);
        }
        break;
      }
    }
    return true;
  }

  void csThreadedLoader::ParseAvailableObjects(csLoaderContext* ldr_context, iDocumentNode* doc,
    csRefArray<iDocumentNode>& libs, csArray<csString>& libIDs)
  {
    /// Make all library files 'inline' and do a pre-parse of each.
    csRefArray<iThreadReturn> threadReturns;
    csRef<iDocumentNodeIterator> it = doc->GetNodes("library");
    while(it->HasNext())
    {
      csString cwd = vfs->GetCwd ();
      threadReturns.Push(LoadLibraryFromNode (ldr_context, it->Next(), &libs, &libIDs, cwd));
    }

    csRef<iDocumentNodeIterator> itr = doc->GetNodes("plugins");
    while(itr->HasNext())
    {
      ldr_context->ParseAvailablePlugins(itr->Next());
    }

    itr = doc->GetNodes("textures");
    while(itr->HasNext())
    {
      ldr_context->ParseAvailableTextures(itr->Next());
    }

    itr = doc->GetNodes("shaders");
    while(itr->HasNext())
    {
      ldr_context->ParseAvailableShaders(itr->Next());
    }

    itr = doc->GetNodes("materials");
    while(itr->HasNext())
    {
      ldr_context->ParseAvailableMaterials(itr->Next());
    }

    ldr_context->ParseAvailableMeshfacts(doc);

    /// Wait for library parse to finish.
    threadman->Wait(threadReturns);
  }

  bool csThreadedLoader::LoadMap (iLoaderContext* ldr_context, iDocumentNode* world_node,
    iStreamSource* ssource, iMissingLoaderData* missingdata, bool do_verbose)
  {
    /// Points to proxy textures ready for processing.
    csSafeCopyArray<ProxyTexture> proxyTextures;

    /// Points to materials created by the current map loading.
    csWeakRefArray<iMaterialWrapper> materialArray;

    csRef<iDocumentNode> sequences;
    csRef<iDocumentNode> triggers;

    // Array of all thread jobs created from this parse.
    csRefArray<iThreadReturn> threadReturns;

    // Arrays for 'inlined' libraries.
    csRefArray<iDocumentNode> libs;
    csArray<csString> libIDs;

    // Array for deferred libraries.
    csRefArray<iDocumentNode> defLibs;

    // Parse the map to find all materials and meshfacts.
    csLoaderContext* thisContext = dynamic_cast<csLoaderContext*>(ldr_context);
    ParseAvailableObjects(thisContext, world_node, libs, libIDs);

    /// Main parse.
    csRef<iDocumentNodeIterator> it = world_node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_SHADEREXPRESSIONS:
        if (!LoadShaderExpressions (ldr_context, child))
          return false;
        break;
      case XMLTOKEN_SETTINGS:
        if (!LoadSettings (child))
          return false;
        break;
      case XMLTOKEN_ADDON:
        if (!LoadAddOn (ldr_context, child, (iEngine*)Engine, false, ssource))
          return false;
        break;
      case XMLTOKEN_META:
        if (!LoadAddOn (ldr_context, child, (iEngine*)Engine, true, ssource))
          return false;
        break;
      case XMLTOKEN_SECTOR:
      case XMLTOKEN_MESHFACT:
        {
          // Parse deferred libraries first.
          if(!LoadDeferredLibs(defLibs, ldr_context, ssource, missingdata,
            threadReturns, libs, libIDs, do_verbose))
              return false;

          if(!LoadMeshfacts (thisContext, ssource, &proxyTextures, materialArray))
            return false;

          if(id == XMLTOKEN_SECTOR)
          {
            if (!ParseSector (ldr_context, child, ssource, threadReturns) ||
              !threadman->Wait(threadReturns))
              return false;
          }
        }
        break;
      case XMLTOKEN_SEQUENCES:
        // Defer sequence parsing to later.
        sequences = child;
        break;
      case XMLTOKEN_TRIGGERS:
        // Defer trigger parsing to later.
        triggers = child;
        break;
      case XMLTOKEN_PLUGINS:
        {
          for(size_t i=0; i<thisContext->availPlugins.GetSize(); ++i)
          {
            LoadPlugin(thisContext->availPlugins[i].node);
          }
        }
        break;
      case XMLTOKEN_TEXTURES:
        {
          if(!LoadTextures(thisContext, &proxyTextures))
            return false;
          break;
        }
      case XMLTOKEN_MATERIALS:
        {
          if(!LoadMaterials(thisContext, &proxyTextures, materialArray))
            return false;
          break;
        }
      case  XMLTOKEN_VARIABLES:
        if (!ParseVariableList (ldr_context, child))
          return false;
        break;
      case XMLTOKEN_SOUNDS:
        if (!LoadSounds (child))
          return false;
        break;
      case XMLTOKEN_LIBRARY:
        {
          defLibs.Push(child);
          break;
        }
      case XMLTOKEN_START:
        {
          const char* name = child->GetAttributeValue ("name");
          csRef<iCameraPosition> campos = Engine->GetCameraPositions ()->
            CreateCameraPosition (name ? name : "Start");
          AddCamposToList(campos);
          ldr_context->AddToCollection(campos->QueryObject ());
          if (!ParseStart (child, campos))
            return false;
          break;
        }
      case XMLTOKEN_KEY:
        {
          if (!ParseKey (child, Engine->QueryObject()))
            return false;
          break;
        }
      case XMLTOKEN_SHADERS:
        if(!ParseShaderList (thisContext))
          return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    // Parse deferred libraries.
    if(!LoadDeferredLibs(defLibs, ldr_context, ssource, missingdata,
      threadReturns, libs, libIDs, do_verbose))
        return false;

    // Sequences and triggers are parsed at the end because
    // all sectors and other objects need to be present.
    if (sequences)
      if (!LoadSequences (ldr_context, sequences))
        return false;
    if (triggers)
      if (!LoadTriggers (ldr_context, triggers))
        return false;

    // Go through the list of proxy textures and load those needed.
    if(ldr_context->GetKeepFlags() == KEEP_USED)
    {
      if(!LoadProxyTextures(proxyTextures, materialArray))
        return false;
    }

    // Wait for all jobs to finish.
    return threadman->Wait(threadReturns);
  }

  bool csThreadedLoader::LoadTextures (csLoaderContext* ldr_context,
      csSafeCopyArray<ProxyTexture>* proxyTextures)
  {
    if(!ldr_context->availTextures.IsEmpty())
    {
      csRefArray<iThreadReturn> threadReturns;
      for(size_t i=0; i<ldr_context->availTextures.GetSize(); ++i)
      {
        threadReturns.Push(ParseTexture(ldr_context, ldr_context->availTextures[i].node,
          proxyTextures, ldr_context->availTextures[i].path));
      }

      if(!threadman->Wait(threadReturns))
      {
        return false;
      }

      ldr_context->availTextures.DeleteAll();
    }

    return true;
  }

  bool csThreadedLoader::LoadMaterials (csLoaderContext* ldr_context,
    csSafeCopyArray<ProxyTexture>* proxyTextures,
    csWeakRefArray<iMaterialWrapper> &materialArray)
  {
    if(!ldr_context->availMaterials.IsEmpty())
    {
      if(!LoadTextures(ldr_context, proxyTextures))
        return false;

      if(!ParseShaderList (ldr_context))
        return false;

      for(size_t i=0; i<ldr_context->availMaterials.GetSize(); ++i)
      {
        if(!ParseMaterial(ldr_context, ldr_context->availMaterials[i].node, materialArray))
          return false;
      }

      ldr_context->availMaterials.DeleteAll();
    }

    return true;
  }

  bool csThreadedLoader::LoadMeshfacts (csLoaderContext* ldr_context,
    iStreamSource* ssource, csSafeCopyArray<ProxyTexture>* proxyTextures,
    csWeakRefArray<iMaterialWrapper> &materialArray)
  {
    if(!ldr_context->availMeshfacts.IsEmpty())
    {
      if(!LoadTextures(ldr_context, proxyTextures))
        return false;

      if(!LoadMaterials(ldr_context, proxyTextures, materialArray))
        return false;

      csRefArray<iThreadReturn> threadReturns;
      for(size_t i=0; i<ldr_context->availMeshfacts.GetSize(); ++i)
      {
        csRef<iDocumentAttribute> attr_name = ldr_context->availMeshfacts[i].node->GetAttribute ("name");
        csRef<iDocumentAttribute> attr_file = ldr_context->availMeshfacts[i].node->GetAttribute ("file");
        if (attr_file && attr_file->GetValue ())
        {
          const char* name = attr_name->GetValue();
          const char* filename = attr_file->GetValue ();
          csRef<iDataBuffer> buffer = vfs->ReadFile (filename);
          csRef<iDocument> doc;
          if(!LoadStructuredDoc (filename, buffer, doc))
            return false;

          csRef<iDocumentNode> node = doc->GetRoot ()->GetNode ("meshfact");
          if(!node.IsValid())
            return false;

          threadReturns.Push(FindOrLoadMeshFactory(name, ldr_context,
            node, 0, 0, ssource, ldr_context->availMeshfacts[i].path));
        }
        else
        {
          threadReturns.Push(FindOrLoadMeshFactory(0, ldr_context, ldr_context->availMeshfacts[i].node,
            0, 0, ssource, ldr_context->availMeshfacts[i].path));
        }
      }

      if(!threadman->Wait(threadReturns))
      {
        return false;
      }

      ldr_context->availMeshfacts.DeleteAll();
    }

    return true;
  }

  bool csThreadedLoader::LoadDeferredLibs(csRefArray<iDocumentNode>& defLibs,
    iLoaderContext* ldr_context, iStreamSource* ssource, iMissingLoaderData* missingdata,
    csRefArray<iThreadReturn>& threadReturns, csRefArray<iDocumentNode>& libs,
    csArray<csString>& libIDs, bool do_verbose)
  {
    for(size_t i=0; i<defLibs.GetSize(); ++i)
    {
      const char* file = defLibs[i]->GetAttributeValue("file");
      const char* path = 0;
      if(file)
      {
        path = defLibs[i]->GetAttributeValue("path");
        if(path)
        {
          vfs->PushDir(path);
        }
      }
      else
      {
        file = defLibs[i]->GetContentsValue();
      }

      csRef<iDocumentNode> lib = defLibs[i];
      size_t idx = libIDs.Find(file);
      if(idx != csArrayItemNotFound)
      {
        lib = libs.Get(idx);
      }

      if(!LoadLibrary(ldr_context, lib, ssource, missingdata, threadReturns,
        libs, libIDs, false, true, do_verbose))
      {
        return false;
      }

      if(path)
      {
        vfs->PopDir();
      }
    }

    defLibs.DeleteAll();

    return true;
  }

  THREADED_CALLABLE_IMPL5(csThreadedLoader, LoadLibraryFromNode,
    csRef<iLoaderContext> ldr_context, csRef<iDocumentNode> lib,
    csRefArray<iDocumentNode>* libs, csArray<csString>* libIDs,
    const char* cwd)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    const char* file = lib->GetAttributeValue("file");
    const char* path = 0;
    if(file)
    {
      path = lib->GetAttributeValue("path");
      if(path)
      {
        dirChange.ChangeToFull(path);
      }
    }
    else
    {
      file = lib->GetContentsValue();
    }

    if (Engine->GetSaveableFlag ())
    {
      csRef<iLibraryReference> libraryRef;
      libraryRef.AttachNew (new csLibraryReference (file, path, true));
      ldr_context->AddToCollection (libraryRef->QueryObject ());
    }

    csRef<iFile> buf = vfs->Open(file, VFS_FILE_READ);

    if(!buf)
    {
      return true;
    }

    csRef<iDocument> doc;
    bool er = LoadStructuredDoc(file, buf, doc);

    if(er && doc)
    {
      // Do the quick pre-parse.
      csRef<iDocumentNode> lib_node = doc->GetRoot()->GetNode("library");
      ParseAvailableObjects(dynamic_cast<csLoaderContext*>((iLoaderContext*)ldr_context), lib_node, *libs, *libIDs);

      // Add info to arrays later.
      CS::Threading::MutexScopedLock lock(preParseLibsLock);
      libs->Push(lib_node);
      libIDs->Push(file);

      return true;
    }

    return false;
  }

  bool csThreadedLoader::LoadLibrary(iLoaderContext* ldr_context, iDocumentNode* node,
    iStreamSource* ssource, iMissingLoaderData* missingdata, csRefArray<iThreadReturn>& threadReturns,
    csRefArray<iDocumentNode>& libs, csArray<csString>& libIDs, bool loadProxyTex, bool mapLoad, bool do_verbose)
  {
    if (!Engine)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.noengine",
        node, "No engine present while in LoadLibrary!");
      return false;
    }

    /// Points to proxy textures ready for processing.
    csSafeCopyArray<ProxyTexture> proxyTextures;

    /// Points to materials created by the current map loading.
    csWeakRefArray<iMaterialWrapper> materialArray;

    csRef<iDocumentNode> sequences;
    csRef<iDocumentNode> triggers;

    csLoaderContext* thisContext = dynamic_cast<csLoaderContext*>(ldr_context);
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_SHADEREXPRESSIONS:
        if (!LoadShaderExpressions (ldr_context, child))
          return false;
        break;
      case XMLTOKEN_LIBRARY:
        {
          const char* file = child->GetAttributeValue("file");
          const char* path = 0;
          if(file)
          {
            path = child->GetAttributeValue("path");
            if(path)
            {
              vfs->PushDir();
              vfs->ChDir(path);
            }
          }
          else
          {
            file = child->GetContentsValue();
          }

          csRef<iDocumentNode> lib;
          size_t idx = libIDs.Find(file);
          if(idx != csArrayItemNotFound)
          {
            lib = libs.Get(idx);
          }

          if(!lib.IsValid())
          {
            lib = child;
          }

          if(!LoadLibrary(ldr_context, lib, ssource, missingdata, threadReturns, libs, libIDs, false, mapLoad, do_verbose))
            return false;
          break;
        }
      case XMLTOKEN_ADDON:
        if (!LoadAddOn (ldr_context, child, (iEngine*)Engine, false, ssource))
          return false;
        break;
      case XMLTOKEN_META:
        if (!LoadAddOn (ldr_context, child, (iEngine*)Engine, true, ssource))
          return false;
        break;
      case XMLTOKEN_SEQUENCES:
        // Defer sequence parsing to later.
        sequences = child;
        break;
      case XMLTOKEN_TRIGGERS:
        // Defer trigger parsing to later.
        triggers = child;
        break;
      case XMLTOKEN_TEXTURES:
        if(!mapLoad)
        {
          // Append textures to engine.
          if (!LoadTextures (thisContext, &proxyTextures))
            return false;
        }
        break;
      case XMLTOKEN_MATERIALS:
        if(!mapLoad)
        {
          if (!LoadMaterials (thisContext, &proxyTextures, materialArray))
            return false;
        }
        break;
      case XMLTOKEN_SHADERS:
        if(!mapLoad)
        {
          if (!ParseShaderList (thisContext))
            return false;
        }
        break;
      case  XMLTOKEN_VARIABLES:
        if (!ParseVariableList (ldr_context, child))
          return false;
        break;
      case XMLTOKEN_SOUNDS:
        if (!LoadSounds (child))
          return false;
        break;
      case XMLTOKEN_MESHREF:
        {
          threadReturns.Push(LoadMeshRef(child, 0, ldr_context, ssource));
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
          const char* name = child->GetAttributeValue ("name");
          csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (name, false);
	  csString cwd = vfs->GetCwd ();
          csRef<iThreadReturn> itr = LoadMeshObject (ldr_context, mesh, 0, child, ssource, 0, name, cwd);
          AddLoadingMeshObject(name, itr);
          threadReturns.Push(itr);
        }
        break;
      case XMLTOKEN_MESHFACT:
        if(!mapLoad)
        {
          if(!LoadMeshfacts (thisContext, ssource, &proxyTextures, materialArray))
            return false;
        }
        break;
      case XMLTOKEN_PLUGINS:
        if(!mapLoad)
        {
          for(size_t i=0; i<thisContext->availPlugins.GetSize(); ++i)
          {
            LoadPlugin(thisContext->availPlugins[i].node);
          }
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    // Sequences and triggers are parsed at the end because
    // all sectors and other objects need to be present.
    if (sequences)
      if (!LoadSequences (ldr_context, sequences))
        return false;
    if (triggers)
      if (!LoadTriggers (ldr_context, triggers))
        return false;

    if(ldr_context->GetKeepFlags() == KEEP_USED && loadProxyTex)
    {
      if(!LoadProxyTextures(proxyTextures, materialArray))
        return false;
    }

    return true;
  }

  bool csThreadedLoader::LoadMeshObjectFactory(iLoaderContext* ldr_context,
    iMeshFactoryWrapper* stemp, iMeshFactoryWrapper* parent, iDocumentNode* node,
    csReversibleTransform* transf, iStreamSource* ssource)
  {
    iLoaderPlugin* plug = 0;
    iBinaryLoaderPlugin* binplug = 0;
    iMaterialWrapper *mat = 0;
    bool staticshape = false;
    csRef<iDocumentNodeIterator> prev_it;
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (true)
    {
      if (!it->HasNext ())
      {
        // Iterator has finished. Check if we still have to continue
        // with the normal iterator first (non-defaults).
        if (!prev_it) break;
        it = prev_it;
        prev_it = 0;
        continue;
      }

      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_LOD:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying LOD!");
            return false;
          }
          csRef<iLODControl> lodctrl (scfQueryInterface<iLODControl> (
            stemp->GetMeshObjectFactory ()));
          if (!lodctrl)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "This mesh factory doesn't implement LOD control!");
            return false;
          }
          if (!LoadLodControl (lodctrl, child))
          {
            return false;
          }
        }
        break;
      case XMLTOKEN_KEY:
        {
          if (!ParseKey (child, stemp->QueryObject()))
          {
            return false;
          }
        }
        break;
      case XMLTOKEN_ADDON:
        if (!LoadAddOn (ldr_context, child, stemp, false, ssource))
        {
          return false;
        }
        break;
      case XMLTOKEN_META:
        if (!LoadAddOn (ldr_context, child, stemp, true, ssource))
        {
          return false;
        }
        break;
      case XMLTOKEN_LODLEVEL:
        {
          if (!parent)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshfactory", child,
              "Factory must be part of a hierarchy for <lodlevel>!");
            return false;
          }
          parent->AddFactoryToStaticLOD (child->GetContentsValueAsInt (),
            stemp);
        }
        break;
      case XMLTOKEN_STATICLOD:
        {
          iLODControl* lodctrl = stemp->CreateStaticLOD ();
          if (!LoadLodControl (lodctrl, child))
          {
            return false;
          }
        }
        break;
      case XMLTOKEN_STATICSHAPE:
        if (!SyntaxService->ParseBool (child, staticshape, true))
        {
          return false;
        }
        break;
      case XMLTOKEN_NULLMESH:
        {
          if (plug)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.plugin",
              child, "Don't specify the plugin if you use <nullmesh>!");
            return false;
          }
          if (stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory", child,
              "Please don't use <params> in combination with <nullmesh>!");
            return false;
          }
          csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
            object_reg, "crystalspace.mesh.object.null", false);
          if (!type)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.plugin",
              child, "Could not find the nullmesh plugin!");
            return false;
          }
          csRef<iMeshObjectFactory> fact = type->NewFactory ();
          stemp->SetMeshObjectFactory (fact);
          fact->SetMeshFactoryWrapper (stemp);
          csBox3 b;
          if (!SyntaxService->ParseBox (child, b))
          {
            return false;
          }
          csRef<iNullFactoryState> nullmesh = 
            scfQueryInterface<iNullFactoryState> (fact);
          nullmesh->SetBoundingBox (b);
        }
        break;
      case XMLTOKEN_PARAMS:
        if (!plug)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin",
            child, "Could not load plugin!");
          return false;
        }
        else
        {
          // We give here the iMeshObjectFactory as the context. If this
          // is a new factory this will be 0. Otherwise it is possible
          // to append information to the already loaded factory.
          csRef<iBase> mof = plug->Parse (child, ssource, ldr_context,
            stemp->GetMeshObjectFactory ());
          if (!mof)
          {
            // Error is reported by plug->Parse().
            return false;
          }
          else
          {
            csRef<iMeshObjectFactory> mof2 (
              scfQueryInterface<iMeshObjectFactory> (mof));
            if (!mof2)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.meshfactory",
                child,
                "Returned object does not implement iMeshObjectFactory!");;
              return false;
            }
            stemp->SetMeshObjectFactory (mof2);
            mof2->SetMeshFactoryWrapper (stemp);
          }
        }
        break;
      case XMLTOKEN_PARAMSFILE:
        if (!plug && !binplug)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin",
            child, "Could not load plugin!");
          return false;
        }
        else
        {
          csRef<iFile> buf (vfs->Open (child->GetContentsValue (),
            VFS_FILE_READ));
          if (!buf)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.loadingfile",
              child, "Error opening file '%s'!", child->GetContentsValue ());
            return false;
          }
          // We give here the iMeshObjectFactory as the context. If this
          // is a new factory this will be 0. Otherwise it is possible
          // to append information to the already loaded factory.
          csRef<iBase> mof;
          if (plug)
            mof = LoadStructuredMap (ldr_context,
            plug, buf, stemp->GetMeshObjectFactory (),
            child->GetContentsValue (), ssource);
          else
          {
            csRef<iDataBuffer> dbuf = vfs->ReadFile (
              child->GetContentsValue ());
            mof = binplug->Parse (dbuf,
              ssource, ldr_context, stemp->GetMeshObjectFactory ());
          }
          if (!mof)
          {
            // Error is reported by plug->Parse().
            return false;
          }
          else
          {
            csRef<iMeshObjectFactory> mof2 (
              scfQueryInterface<iMeshObjectFactory> (mof));
            if (!mof2)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.meshfactory",
                child,
                "Returned object does not implement iMeshObjectFactory!");
              return false;
            }
            stemp->SetMeshObjectFactory (mof2);
            mof2->SetMeshFactoryWrapper (stemp);
          }
        }
        break;

      case XMLTOKEN_TRIMESH:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'trimesh'!");
            return false;
          }
          iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
            ->GetObjectModel ();
          if (!objmodel)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory", child,
              "This factory doesn't support setting of other 'trimesh'!");
            return false;
          }
          if (!ParseTriMesh (child, objmodel))
          {
            // Error already reported.
            return false;
          }
        }
        break;

      case XMLTOKEN_CLOSED:
        if (!stemp->GetMeshObjectFactory ())
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.meshfactory",
            child, "Please use 'params' before specifying 'closed'!");
          return false;
        }
        else
        {
          iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
            ->GetObjectModel ();
          csRef<iTriangleMeshIterator> it = objmodel->GetTriangleDataIterator ();
          while (it->HasNext ())
          {
            csStringID id;
            iTriangleMesh* trimesh = it->Next (id);
            if (trimesh) trimesh->GetFlags ().Set (
              CS_TRIMESH_CLOSED | CS_TRIMESH_NOTCLOSED, CS_TRIMESH_CLOSED);
          }
        }
        break;
      case XMLTOKEN_CONVEX:
        if (!stemp->GetMeshObjectFactory ())
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.meshfactory",
            child, "Please use 'params' before specifying 'convex'!");
          return false;
        }
        else
        {
          iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
            ->GetObjectModel ();
          csRef<iTriangleMeshIterator> it = objmodel->GetTriangleDataIterator ();
          while (it->HasNext ())
          {
            csStringID id;
            iTriangleMesh* trimesh = it->Next (id);
            if (trimesh) trimesh->GetFlags ().Set (
              CS_TRIMESH_CONVEX | CS_TRIMESH_NOTCONVEX, CS_TRIMESH_CONVEX);
          }
        }
        break;
      case XMLTOKEN_MATERIAL:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'material'!");
            return false;
          }
          const char* matname = child->GetContentsValue ();
          mat = ldr_context->FindMaterial (matname);
          if (mat)
          {
            if (!stemp->GetMeshObjectFactory ()->SetMaterialWrapper (mat))
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.meshfactory",
                child, "This factory doesn't support setting materials this way!");
              return false;
            }
          }
          else
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.unknownmaterial",
              child, "Material '%s' not found!", matname);
            return false;
          }
        }
        break;

      case XMLTOKEN_PLUGIN:
        {
          if (prev_it || plug || binplug)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfact",
              child, "Please specify only one plugin!");
            return false;
          }

          iDocumentNode* defaults = 0;
          if (!loaded_plugins.FindPlugin (child->GetContentsValue (),
            plug, binplug, defaults))
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfact",
              child, "Error loading plugin '%s'!",
              child->GetContentsValue ());
            return false;
          }
          if (defaults)
          {
            // Set aside current iterator and start a new one.
            prev_it = it;
            it = defaults->GetNodes ();
          }
        }
        break;

      case XMLTOKEN_MESHFACT:
        {
          csReversibleTransform child_transf;
          csRef<iThreadReturn> ret = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
	  csString cwd = vfs->GetCwd ();
          if(!FindOrLoadMeshFactoryTC(ret, false, 0, ldr_context, child, stemp, &child_transf, ssource, cwd))
          {
            return false;
          }
        }
        break;
    case XMLTOKEN_INSTANCES:
      {
        // Get the factory to be instanced.
        csRef<iDocumentNode> instancedFact = child->GetNode("meshfact");
        if (!instancedFact)
          return false;

        csReversibleTransform child_transf;
        csRef<iThreadReturn> ret = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
	csString cwd = vfs->GetCwd ();
        if(!FindOrLoadMeshFactoryTC(ret, false, 0, ldr_context, instancedFact, stemp, &child_transf, ssource, cwd))
        {
          return false;
        }

        csRef<iMeshFactoryWrapper> instanceFactory = scfQueryInterface<iMeshFactoryWrapper>(ret->GetResultRefPtr());
        stemp->SetInstanceFactory(instanceFactory);
        instanceFactory->GetMeshObjectFactory()->SetMeshFactoryWrapper(stemp);

        // Get instances.
        csRef<iDocumentNodeIterator> instances = child->GetNodes("instance");
        while(instances->HasNext())
        {
          csRef<iDocumentNode> instance = instances->Next();

          // Get o2w rotation.
          csMatrix3 rotation;
          csRef<iDocumentNode> matrix_node = instance->GetNode ("matrix");
          if (matrix_node)
          {
            if (!SyntaxService->ParseMatrix(matrix_node, rotation))
              return false;
          }

          // Get position offset.
          csRef<iDocumentNode> vector_node = instance->GetNode ("v");
          if (!vector_node)
            return false;
          
          csVector3 v;
          if (!SyntaxService->ParseVector (vector_node, v))
            return false;

          stemp->AddInstance(v, rotation);
        }
      }
      break;
    case XMLTOKEN_BBOX:
      {
        csBox3 bbox;

        csRef<iDocumentNodeIterator> vertices = child->GetNodes("v");
        while(vertices->HasNext())
        {
          csRef<iDocumentNode> vertex = vertices->Next();

          csVector3 v;
          if (!SyntaxService->ParseVector (vertex, v))
            return false;

          bbox.AddBoundingVertex(v);
        }

        stemp->GetMeshObjectFactory()->GetObjectModel()->SetObjectBoundingBox(bbox);
      }
      break;
      case XMLTOKEN_MOVE:
        {
          if (!transf)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshfactory",
              child,
              "'move' is only useful for hierarchical transformations!");
            return false;
          }
          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
            {
              return false;
            }
            transf->SetO2T (m);
          }
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
            {
              return false;
            }
            transf->SetO2TTranslation (v);
          }
        }
        break;
      case XMLTOKEN_HARDMOVE:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'hardmove'!");
            return false;
          }
          if (!stemp->GetMeshObjectFactory ()->SupportsHardTransform ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "This factory doesn't support 'hardmove'!");
            return false;
          }
          csReversibleTransform tr;
          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
            {
              return false;
            }
            tr.SetT2O (m);
          }
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
            {
              return false;
            }
            tr.SetOrigin (v);
          }
          stemp->HardTransform (tr);
        }
        break;

      case XMLTOKEN_ZUSE:
        stemp->SetZBufMode (CS_ZBUF_USE);
        break;
      case XMLTOKEN_ZFILL:
        stemp->SetZBufMode (CS_ZBUF_FILL);
        break;
      case XMLTOKEN_ZNONE:
        stemp->SetZBufMode (CS_ZBUF_NONE);
        break;
      case XMLTOKEN_ZTEST:
        stemp->SetZBufMode (CS_ZBUF_TEST);
        break;
      case XMLTOKEN_PRIORITY:
        {
          const char* priname = child->GetContentsValue ();
          long pri = Engine->GetRenderPriority (priname);
          if (pri == 0)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Unknown render priority '%s'!", priname);
            return false;
          }
          stemp->SetRenderPriority (pri);
        }
        break;
      case XMLTOKEN_SHADERVAR:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'shadervar'!");
            return false;
          }
          csRef<iShaderVariableContext> svc = stemp->GetSVContext();
          CS_ASSERT (svc.IsValid());
          //create a new variable
          csRef<csShaderVariable> var;
          var.AttachNew (new csShaderVariable);
          if (!SyntaxService->ParseShaderVar (ldr_context, child, *var))
          {
            break;
          }
          svc->AddVariable (var);
        }
        break;
      case XMLTOKEN_NOLIGHTING:
        stemp->GetFlags ().Set (CS_ENTITY_NOLIGHTING, CS_ENTITY_NOLIGHTING);
        break;
      case XMLTOKEN_NOSHADOWS:
        stemp->GetFlags ().Set (CS_ENTITY_NOSHADOWS, CS_ENTITY_NOSHADOWS);
        break;
      case XMLTOKEN_NOSHADOWCAST:
        stemp->GetFlags ().Set (CS_ENTITY_NOSHADOWCAST, CS_ENTITY_NOSHADOWCAST);
        break;
      case XMLTOKEN_NOSHADOWRECEIVE:
        stemp->GetFlags ().Set (CS_ENTITY_NOSHADOWRECEIVE, CS_ENTITY_NOSHADOWRECEIVE);
        break;
      case XMLTOKEN_NOCLIP:
        stemp->GetFlags ().Set (CS_ENTITY_NOCLIP, CS_ENTITY_NOCLIP);
        break;
      case XMLTOKEN_NOHITBEAM:
        stemp->GetFlags ().Set (CS_ENTITY_NOHITBEAM, CS_ENTITY_NOHITBEAM);
        break;
      case XMLTOKEN_INVISIBLEMESH:
        stemp->GetFlags ().Set (CS_ENTITY_INVISIBLEMESH,
          CS_ENTITY_INVISIBLEMESH);
        break;
      case XMLTOKEN_INVISIBLE:
        stemp->GetFlags ().Set (CS_ENTITY_INVISIBLE, CS_ENTITY_INVISIBLE);
        break;
      case XMLTOKEN_DETAIL:
        stemp->GetFlags ().Set (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
        break;
      case XMLTOKEN_STATICLIT:
        stemp->GetFlags ().Set (CS_ENTITY_STATICLIT, CS_ENTITY_STATICLIT);
        break;
      case XMLTOKEN_LIMITEDSHADOWCAST:
        stemp->GetFlags ().Set (CS_ENTITY_LIMITEDSHADOWCAST,
          CS_ENTITY_LIMITEDSHADOWCAST);
        break;
      case XMLTOKEN_IMPOSTER:
        {
          csRef<iImposterFactory> imposter = scfQueryInterface<iImposterFactory> (stemp);
          if (!imposter)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              node, "This factory doesn't implement impostering!");
            {
              return false;
            }
          }
          if (!ParseImposterSettings (imposter, child))
          {
            return false;
          }
          break;
        }
      default:
        {
          SyntaxService->ReportBadToken (child);
          return false;
        }
      }
    }

    stemp->GetMeshObjectFactory ()->GetFlags ().SetBool (CS_FACTORY_STATICSHAPE,
      staticshape);

    ldr_context->AddToCollection(stemp->QueryObject());

    if(parent)
    {
      parent->GetChildren()->Add(stemp);
    }

    if(transf)
    {
      stemp->SetTransform(*transf);
    }

    return true;
  }

  THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadMeshObject, csRef<iLoaderContext> ldr_context,
    csRef<iMeshWrapper> mesh, csRef<iMeshWrapper> parent, csRef<iDocumentNode> node, 
    csRef<iStreamSource> ssource, csRef<iSector> sector, csString name, const char* cwd)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    csString priority;

    iLoaderPlugin* plug = 0;
    iBinaryLoaderPlugin* binplug = 0;
    bool staticpos = false;
    bool staticshape = false;
    bool zbufSet = false;
    bool prioSet = false;

    csRef<iDocumentNodeIterator> prev_it;
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (true)
    {
      if (!it->HasNext ())
      {
        // Iterator has finished. Check if we still have to continue
        // with the normal iterator first (non-defaults).
        if (!prev_it) break;
        it = prev_it;
        prev_it = 0;
        continue;
      }

      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      bool handled;
      if (!HandleMeshParameter (ldr_context, mesh, parent, child, id,
        handled, priority, false, staticpos, staticshape, zbufSet, prioSet,
        false, ssource))
      {
        RemoveLoadingMeshObject(name, ret);
        return false;
      }

      if (!handled) switch (id)
      {
      case XMLTOKEN_PORTAL:
        {
          iMeshWrapper* container_mesh = 0;
          if (!ParsePortal (ldr_context, child, 0, 0, container_mesh, mesh))
          {
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
        }
        break;
      case XMLTOKEN_PORTALS:
        if (!ParsePortals (ldr_context, child, 0, mesh, ssource))
        {
          RemoveLoadingMeshObject(name, ret);
          return false;
        }
        break;
      case XMLTOKEN_MESHREF:
        {
          LoadMeshRef(child, 0, ldr_context, ssource);
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
          if(child->GetAttributeValue ("name") == 0)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.plugin",
              child, "Some meshobj has no name!");
            return false;
          }
          csString childName = csString(name).AppendFmt(":%s", child->GetAttributeValue("name"));
          csRef<iMeshWrapper> sp = Engine->CreateMeshWrapper(childName, false);
	  csString cwd = vfs->GetCwd ();
          csRef<iThreadReturn> itr = LoadMeshObject (ldr_context, sp, mesh, child, ssource, 0, childName, cwd);
          AddLoadingMeshObject(childName, itr);
        }
        break;
      case XMLTOKEN_LIGHT:
        {
          iLight * light = ParseStatlight (ldr_context, child);
          if (light)
          {
            light->QuerySceneNode ()->SetParent (mesh->QuerySceneNode ());
          }
          else
          {
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
        }
        break;
      case XMLTOKEN_NULLMESH:
        {
          if (plug)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.plugin",
              child, "Don't specify the plugin if you use <nullmesh>!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          if (mesh->GetMeshObject ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.mesh", child,
              "Please don't use <params> in combination with <nullmesh>!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
            object_reg, "crystalspace.mesh.object.null", false);
          if (!type)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.plugin",
              child, "Could not find the nullmesh plugin!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          csRef<iMeshObjectFactory> fact = type->NewFactory ();
          csRef<iMeshObject> mo = fact->NewInstance ();
          mesh->SetMeshObject (mo);
          mo->SetMeshWrapper (mesh);
          csBox3 b;
          if (!SyntaxService->ParseBox (child, b))
          {
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          csRef<iNullMeshState> nullmesh = 
            scfQueryInterface<iNullMeshState> (mo);
          if (nullmesh)
            nullmesh->SetBoundingBox (b);
        }
        break;

      case XMLTOKEN_PARAMS:
        if (!plug)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin",
            child, "Could not load plugin!");
          RemoveLoadingMeshObject(name, ret);
          return false;
        }
        else
        {
          csRef<iBase> mo = plug->Parse (child, ssource, ldr_context,
            mesh);
          if (!mo || !HandleMeshObjectPluginResult (mo, child, mesh, zbufSet, 
            prioSet))
          {
            RemoveLoadingMeshObject(name, ret);
            return false;	// Error already reported.
          }
        }
        break;
      case XMLTOKEN_FILE:
        {
          const char* fname = child->GetContentsValue ();
          if (!fname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.loadingfile",
              child, "Specify a VFS filename with 'file'!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          csRef<iFile> buf = vfs->Open (fname, VFS_FILE_READ);
          if (!buf)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.loadingfile",
              child, "Error opening file '%s'!", fname);
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          csRef<iDocument> doc;
          bool er = LoadStructuredDoc (fname, buf, doc);
          if (!er)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.loadingfile",
              child, "'%s' is not an XML file!", fname);
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          csRef<iDocumentNode> paramsnode = doc->GetRoot ()->GetNode ("params");
          if (paramsnode)
          {
            if (!plug && !binplug)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.load.plugin",
                child, "Could not load plugin for mesh '%s'!",
                mesh->QueryObject ()->GetName ());
              RemoveLoadingMeshObject(name, ret);
              return false;
            }
            csRef<iBase> mo;
            if (plug)
              mo = plug->Parse (paramsnode, ssource, ldr_context, mesh);
            else
            {
              csRef<iDataBuffer> dbuf = vfs->ReadFile (fname);
              mo = binplug->Parse (dbuf,
                ssource, ldr_context, mesh);
            }
            if (!mo || !HandleMeshObjectPluginResult (mo, child, mesh,
              zbufSet, prioSet))
            {
              RemoveLoadingMeshObject(name, ret);
              return false;	// Error already reported.
            }
            break;
          }
          csRef<iDocumentNode> meshobjnode = doc->GetRoot ()->GetNode (
            "meshobj");
          if (meshobjnode)
          {
            csString childName = csString(name).AppendFmt(":%s", child->GetAttributeValue("name"));
	    csString cwd = vfs->GetCwd ();
            csRef<iThreadReturn> itr = LoadMeshObject (ldr_context, mesh, parent, meshobjnode, ssource,
              0, childName, cwd);
            AddLoadingMeshObject(childName, itr);
            break;
          }
          csRef<iDocumentNode> meshfactnode = doc->GetRoot ()->GetNode (
            "meshfact");
          if (meshfactnode)
          {
            const char* meshfactname = meshfactnode->GetAttributeValue ("name");
            // @@@ Handle regions correctly here???
            csRef<iMeshFactoryWrapper> t = Engine->GetMeshFactories ()
              ->FindByName (meshfactname);
            if (!t)
            {
              t = Engine->CreateMeshFactory (meshfactname, false);
              if (!t || !LoadMeshObjectFactory (ldr_context, t, 0,
                meshfactnode, 0, ssource))
              {
                // Error is already reported.
                RemoveLoadingMeshObject(name, ret);
                return false;
              }
              else
              {
                AddMeshFactToList(t);
                ldr_context->AddToCollection(t->QueryObject ());
              }
            }
            break;
          }
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin", child,
            "File '%s' doesn't contain <params>, <meshobj>, nor <meshfact>!",
            fname);
          RemoveLoadingMeshObject(name, ret);
          return false;
        }
        break;
      case XMLTOKEN_PARAMSFILE:
        if (!plug && !binplug)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin",
            child, "Could not load plugin for mesh '%s'!",
            mesh->QueryObject ()->GetName ());
          RemoveLoadingMeshObject(name, ret);
          return false;
        }
        else
        {
          const char* fname = child->GetContentsValue ();
          if (!fname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.loadingfile",
              child, "Specify a VFS filename with 'paramsfile'!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          csRef<iFile> buf (vfs->Open (fname, VFS_FILE_READ));
          if (!buf)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.loadingfile",
              child, "Error opening file '%s'!", fname);
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          csRef<iBase> mo;
          if (plug)
            mo = LoadStructuredMap (ldr_context, plug, buf, mesh, fname,
            ssource);
          else
          {
            csRef<iDataBuffer> dbuf = vfs->ReadFile (fname);
            mo = binplug->Parse (dbuf,
              ssource, ldr_context, mesh);
          }
          if (!mo || !HandleMeshObjectPluginResult (mo, child, mesh,
            zbufSet, prioSet))
            return false;	// Error already reported.
        }
        break;

      case XMLTOKEN_TRIMESH:
        {
          if (!mesh->GetMeshObject ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.mesh",
              child, "Please use 'params' before specifying 'trimesh'!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
          if (!objmodel)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.mesh", child,
              "This mesh doesn't support setting of other 'trimesh'!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          if (!ParseTriMesh (child, objmodel))
          {
            // Error already reported.
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
        }
        break;

      case XMLTOKEN_PLUGIN:
        {
          if (prev_it || plug || binplug)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.plugin",
              child, "Please specify only one plugin!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }

          const char* plugname = child->GetContentsValue ();
          if (!plugname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.plugin",
              child, "Specify a plugin name with 'plugin'!");
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          iDocumentNode* defaults = 0;
          if (!loaded_plugins.FindPlugin (plugname, plug, binplug, defaults))
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshobj",
              child, "Error loading plugin '%s'!", plugname);
            RemoveLoadingMeshObject(name, ret);
            return false;
          }
          if (defaults)
          {
            // Set aside current iterator and start a new one.
            prev_it = it;
            it = defaults->GetNodes ();
          }
        }
        break;
      default:
        {
          SyntaxService->ReportBadToken (child);
          RemoveLoadingMeshObject(name, ret);
          return false;
        }
      }
    }

    if (!priority.IsEmpty ())
      mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
    mesh->GetMeshObject ()->GetFlags ().SetBool (CS_MESH_STATICPOS, staticpos);
    mesh->GetMeshObject ()->GetFlags ().SetBool (CS_MESH_STATICSHAPE, staticshape);

    AddMeshToList(mesh);
    ldr_context->AddToCollection(mesh->QueryObject());
    if(parent)
    {
      mesh->QuerySceneNode ()->SetParent (parent->QuerySceneNode ());
    }

    if(sector && mesh->GetMovable ())
    {
      AddObjectToSector(mesh->GetMovable (), sector);
    }

    ret->SetResult(csRef<iBase>(mesh));

    RemoveLoadingMeshObject(name, ret);

    return true;
  }

  THREADED_CALLABLE_IMPL2(csThreadedLoader, AddObjectToSector, csRef<iMovable> movable,
      csRef<iSector> sector)
  {
      movable->SetSector(sector);
      movable->UpdateMove();
      return true;
  }

  THREADED_CALLABLE_IMPL4(csThreadedLoader, LoadMeshRef, csRef<iDocumentNode> node,
    csRef<iSector> sector, csRef<iLoaderContext> ldr_context, csRef<iStreamSource> ssource)
  {
    const char* meshname = node->GetAttributeValue ("name");
    if (!meshname)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.load.meshobject",
        node, "'meshref' requires a name in sector '%s'!",
        sector && sector->QueryObject()->GetName() ? sector->QueryObject()->GetName() : "<noname>");
      return false;
    }
    csRef<iMeshWrapper> mesh = LoadMeshObjectFromFactory (ldr_context,
      node, ssource);
    if (!mesh)
    {
      // Error is already reported.
      return false;
    }
    mesh->QueryObject ()->SetName (meshname);
    if(sector)
    {
      mesh->GetMovable ()->SetSector (sector);
      mesh->GetMovable ()->UpdateMove ();
    }
    Engine->AddMeshAndChildren (mesh);
    return true;
  }

  bool csThreadedLoader::LoadLodControl(iLODControl* lodctrl, iDocumentNode* node)
  {
    lodctrl->SetLOD (0, 1);
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_DISTANCE:
        {
          csRef<iDocumentAttribute> at = child->GetAttribute ("varm");
          if (at)
          {
            // We use variables.
            iSharedVariable *varm = Engine->GetVariableList()->FindByName (
              child->GetAttributeValue ("varm"));
            iSharedVariable *vara = Engine->GetVariableList()->FindByName (
              child->GetAttributeValue ("vara"));
            lodctrl->SetLOD (varm, vara);
            break;
          }

          at = child->GetAttribute ("m");
          if (at)
          {
      float lodm = child->GetAttributeValueAsFloat ("m");
      float loda = child->GetAttributeValueAsFloat ("a");
      lodctrl->SetLOD (lodm, loda);
    }
    else
    {
      float d0 = child->GetAttributeValueAsFloat ("d0");
      float d1 = child->GetAttributeValueAsFloat ("d1");
      float lodm = 1.0 / (d1-d0);
      float loda = -lodm * d0;
      lodctrl->SetLOD (lodm, loda);
    }
  }
  break;
      case XMLTOKEN_FADE:
        {
          csRef<iDocumentAttribute> at = child->GetAttribute ("varf");
          if (at)
          {
            // We use variables.
            iSharedVariable *varf = Engine->GetVariableList()->FindByName (
              child->GetAttributeValue ("varf"));
            lodctrl->SetLODFade (varf);
            break;
          }

          at = child->GetAttribute ("f");
          if (at)
          {
            float lodf = child->GetAttributeValueAsFloat ("f");
            lodctrl->SetLODFade (lodf);
          }
          else
          {
            float d = child->GetAttributeValueAsFloat ("d");
            float lodm, loda;
            lodctrl->GetLOD (lodm, loda);
            float d0 = loda/-lodm;
            float d1 = 1.0/lodm + d0;
            float lodf = (d1-d0)/(2*d);
            lodctrl->SetLODFade (lodf);
          }
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
    }
  }

    return true;
  }

  bool csThreadedLoader::LoadAddOn (iLoaderContext* ldr_context,
    iDocumentNode* node, iBase* context, bool is_meta, iStreamSource* ssource)
  {
    iLoaderPlugin* plug = 0;
    iBinaryLoaderPlugin* binplug = 0;

    const char* plugin_name = node->GetAttributeValue ("plugin");
    if (plugin_name != 0)
    {
      // Short-hand notation: <addon plugin="bla"> ... </addon>
      iDocumentNode* defaults = 0;
      if (!loaded_plugins.FindPlugin (plugin_name, plug, binplug, defaults))
      {
        if (!is_meta)
          ReportWarning (
          "crystalspace.maploader.parse.addon",
          node, "Couldn't find or load addon plugin '%s'!",
          plugin_name);
        return true;
      }
      if (!plug)
      {
        if (!is_meta)
          ReportWarning (
          "crystalspace.maploader.load.plugin",
          node, "Could not find or load addon plugin!");
        return true;
      }
      if (defaults != 0)
      {
        ReportWarning (
          "crystalspace.maploader.load.plugin",
          node, "'defaults' section is ignored for addons!");
      }

      csRef<iBase> rc;
      if(plug->IsThreadSafe())
      {
        csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
	csString cwd = vfs->GetCwd ();
        if(!ParseAddOnTC(itr, false, plug, node, ssource, ldr_context, context, cwd))
        {
          return false;
        }
        rc = itr->GetResultRefPtr();
      }
      else
      {
	csString cwd = vfs->GetCwd ();
        csRef<iThreadReturn> itr = ParseAddOnWait(plug, node, ssource, ldr_context, context, cwd);
        if(!itr->WasSuccessful())
        {
          return false;
        }
        rc = itr->GetResultRefPtr();
      }

      if (Engine->GetSaveableFlag ())
      {
        csRef<iAddonReference> addon;
        addon.AttachNew (new csAddonReference (plugin_name, 0, rc));
        object_reg->Register (addon);
        ldr_context->AddToCollection(addon->QueryObject ());
      }

      return true;
    }
    else
    {
      // Long notation: <addon> <plugin>bla</plugin> <params>...</params> </addon>
      csRef<iDocumentNodeIterator> it = node->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        if (child->GetType () != CS_NODE_ELEMENT) continue;
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);
        switch (id)
        {
        case XMLTOKEN_PARAMS:
          if (!plug)
          {
            if (!is_meta)
              ReportWarning (
              "crystalspace.maploader.load.plugin",
              child, "Could not find or load plugin!");
            return true;
          }
          else
          {
            csRef<iBase> rc;
            if(plug->IsThreadSafe())
            {
              csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
	      csString cwd = vfs->GetCwd ();
              if(!ParseAddOnTC(itr, false, plug, child, ssource, ldr_context, context, cwd))
              {
                return false;
              }
              rc = itr->GetResultRefPtr();
            }
            else
            {
	      csString cwd = vfs->GetCwd ();
              csRef<iThreadReturn> itr = ParseAddOnWait(plug, child, ssource, ldr_context, context, cwd);
              if(!itr->WasSuccessful())
              {
                return false;
              }
              rc = itr->GetResultRefPtr();
            }

            if (Engine->GetSaveableFlag ())
            {
              csRef<iAddonReference> addon;
              addon.AttachNew (new csAddonReference (plugin_name, 0, rc));
              object_reg->Register (addon);
              ldr_context->AddToCollection(addon->QueryObject());
            }
          }
          break;

        case XMLTOKEN_PARAMSFILE:
          if (!plug && !binplug)
          {
            if (!is_meta)
              ReportWarning (
              "crystalspace.maploader.load.plugin",
              child, "Could not find or load plugin!");
            return true;
          }
          else
          {
            const char* fname = child->GetContentsValue ();
            if (!fname)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.loadingfile",
                child, "Specify a VFS filename with 'paramsfile'!");
              return false;
            }
            csRef<iFile> buf (vfs->Open (fname, VFS_FILE_READ));
            if (!buf)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.loadingfile",
                child, "Error opening file '%s'!", fname);
              return false;
            }

            csRef<iBase> ret;
            if(plug)
            {
              ret = LoadStructuredMap (ldr_context, plug, buf, 0, fname, ssource);
              if(!ret.IsValid())
              {
                return false;
              }
            }
            else
            {
              csRef<iDataBuffer> dbuf = vfs->ReadFile (fname);
              if(binplug->IsThreadSafe())
              {
                csRef<iThreadReturn> itr;
                if(!ParseAddOnBinaryTC(itr, false, binplug, dbuf, ssource, ldr_context, 0))
                {
                  return false;
                }
                ret = itr->GetResultRefPtr();
              }
              else
              {
                csRef<iThreadReturn> itr = ParseAddOnBinaryWait(binplug, dbuf, ssource, ldr_context, 0);
                if(!itr->WasSuccessful())
                {
                  return false;
                }
                ret = itr->GetResultRefPtr();
              }
            }

            if (Engine->GetSaveableFlag ())
            {
              csRef<iAddonReference> addon;
              addon.AttachNew (new csAddonReference (plugin_name,
                fname, ret));
              object_reg->Register (addon);
              ldr_context->AddToCollection(addon->QueryObject());
            }
          }
          break;

        case XMLTOKEN_PLUGIN:
          {
            iDocumentNode* defaults = 0;
            plugin_name = child->GetContentsValue ();
            if (!loaded_plugins.FindPlugin (plugin_name,
              plug, binplug, defaults))
            {
              if (!is_meta)
                ReportWarning (
                "crystalspace.maploader.parse.addon",
                child, "Could not find or load plugin '%s'!",
                child->GetContentsValue ());
              return true;
            }
            if (defaults != 0)
            {
              ReportWarning (
                "crystalspace.maploader.parse.addon",
                child, "'defaults' section is ignored for addons!");
            }
          }
          break;
        default:
          SyntaxService->ReportBadToken (child);
          return false;
        }
      }
    }
    return true;
  }

  bool csThreadedLoader::ParseKey (iDocumentNode* node, iObject* obj)
  {
    csRef<iKeyValuePair> kvp = SyntaxService->ParseKey (node);
    if (!kvp.IsValid())
      return false;

    bool editoronly = node->GetAttributeValueAsBool ("editoronly");
    if (!editoronly || !Engine || Engine->GetSaveableFlag())
      obj->ObjAdd (kvp->QueryObject ());

    return true;
  }

  csPtr<iBase> csThreadedLoader::LoadStructuredMap (iLoaderContext* ldr_context,
    iLoaderPlugin* plug, iFile* buf,
    iBase* context, const char* fname, iStreamSource* ssource)
  {
    csRef<iDocument> doc;
    csString filename (fname);
    csVfsDirectoryChanger dirChanger (vfs);
    size_t slashPos = filename.FindLast ('/');
    if (slashPos != (size_t)-1)
    {
      dirChanger.ChangeTo (filename);
      filename.DeleteAt (0, slashPos + 1);
    }
    bool er = LoadStructuredDoc (filename, buf, doc);
    csRef<iBase> ret;
    if (er)
    {
      if (doc)
      {
        // First find the <params> node in the loaded file.
        csRef<iDocumentNode> paramsnode = doc->GetRoot ()->GetNode ("params");
        if (!paramsnode)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin",
            doc->GetRoot (), "Could not find <params> in '%s'!", fname);
        }
        else
        {
          if(plug->IsThreadSafe())
          {
            csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
	    csString cwd = vfs->GetCwd ();
            if(!ParseAddOnTC(itr, false, plug, paramsnode, ssource, ldr_context, context, cwd))
            {
              return false;
            }
            ret = itr->GetResultRefPtr();
          }
          else
          {
	    csString cwd = vfs->GetCwd ();
            csRef<iThreadReturn> itr = ParseAddOnWait(plug, paramsnode, ssource, ldr_context, context, cwd);
            if(!itr->WasSuccessful())
            {
              return false;
            }
            ret = itr->GetResultRefPtr();
          }
        }
      }
      else
      {
        ReportError ("crystalspace.maploader.load.plugin",
          "File does not appear to be a structured map file (%s)!", fname);
      }
    }
    return csPtr<iBase> (ret);
  }

  bool csThreadedLoader::LoadStructuredDoc (const char* file, iFile* buf,
    csRef<iDocument>& doc)
  {
    csRef<iDocumentSystem> docsys (
      csQueryRegistry<iDocumentSystem> (object_reg));
    if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
    doc = docsys->CreateDocument ();
    const char* error = doc->Parse (buf, true);
    if (error != 0)
    {
      ReportError (
        "crystalspace.maploader.parse.plugin",
        "Document system error for file '%s': %s!", file, error);
      doc = 0;
      return false;
    }
    return true;
  }

  bool csThreadedLoader::LoadStructuredDoc (const char* file, iDataBuffer* buf,
    csRef<iDocument>& doc)
  {
    csRef<iDocumentSystem> docsys (
      csQueryRegistry<iDocumentSystem> (object_reg));
    if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
    doc = docsys->CreateDocument ();
    const char* error = doc->Parse (buf, true);
    if (error != 0)
    {
      ReportError (
        "crystalspace.maploader.parse.plugin",
        file
        ? "Document system error for file '%s': %s!"
        : "Document system error for buffer%s: %s!",
        file ? file : "", error);
      doc = 0;
      return false;
    }
    return true;
  }

  bool csThreadedLoader::LoadProxyTextures(csSafeCopyArray<ProxyTexture> &proxyTextures,
    csWeakRefArray<iMaterialWrapper> &materialArray)
  {
    // Remove all unused textures and materials.
    csWeakRefArray<iTextureWrapper> texArray;

    for(uint i=0; i<proxyTextures.GetSize(); i++)
    {
      ProxyTexture& proxTex = proxyTextures.Get(i);
      if(!proxTex.textureWrapper)
      {
        proxyTextures.DeleteIndex(i);
        i--;
        continue;
      }

      texArray.Push(proxTex.textureWrapper);
    }

    CS::Utility::UnusedResourceHelper::UnloadUnusedMaterials(Engine,
      materialArray);
    CS::Utility::UnusedResourceHelper::UnloadUnusedTextures(Engine, texArray);
    materialArray.Empty();

    // Load the remaining textures.
    iTextureManager *tm = g3d->GetTextureManager();
    size_t i = proxyTextures.GetSize();
    while (i-- > 0)
    {
      ProxyTexture& proxTex = proxyTextures.Get(i);

      if(!proxTex.textureWrapper)
      {
        continue;
      }

      csRef<iImage> img = proxTex.img->GetProxiedImage();

      csRef<iAnimatedImage> anim = scfQueryInterface<iAnimatedImage>(img);
      if (anim && anim->IsAnimated())
      {
        iLoaderPlugin* plugin = NULL;
        iBinaryLoaderPlugin* Binplug = NULL;
        iDocumentNode* defaults = NULL;

        loaded_plugins.FindPlugin(PLUGIN_TEXTURELOADER_ANIMIMG, plugin, Binplug, defaults);
        if(plugin)
        {
          TextureLoaderContext context(proxTex.textureWrapper->QueryObject()->GetName());
          context.SetClass(proxTex.textureWrapper->GetTextureClass());
          context.SetFlags(proxTex.textureWrapper->GetFlags());
          context.SetImage(img);

          csRef<iBase> b = plugin->Parse(0, 0, 0, static_cast<iBase*>(&context));
          if (b)
          {
            csWeakRef<iTextureWrapper> newTex = scfQueryInterface<iTextureWrapper>(b);
            newTex->QueryObject()->SetName(proxTex.textureWrapper->QueryObject()->GetName());
            newTex->SetTextureClass(context.GetClass());

            proxTex.textureWrapper->SetTextureHandle(newTex->GetTextureHandle());
            proxTex.textureWrapper->SetUseCallback(newTex->GetUseCallback());

            csRef<iProcTexture> ipt = scfQueryInterface<iProcTexture> (proxTex.textureWrapper);
            if(ipt)
              ipt->SetAlwaysAnimate (proxTex.always_animate);
          }
        }

      }
      else
      {
        proxTex.textureWrapper->SetImageFile (img);
        proxTex.textureWrapper->Register (tm);
      }

      if(proxTex.keyColour.do_transp)
      {
        proxTex.textureWrapper->SetKeyColor(csQint(proxTex.keyColour.colours.red * 255.99),
          csQint(proxTex.keyColour.colours.green * 255.99),
          csQint(proxTex.keyColour.colours.blue * 255.99));
      }

      if(proxTex.alphaType != csAlphaMode::alphaNone)
      {
        proxTex.textureWrapper->GetTextureHandle()->SetAlphaType(proxTex.alphaType);
      }
    }
    proxyTextures.Empty ();

    return true;
  }

  bool csThreadedLoader::HandleMeshObjectPluginResult (iBase* mo, iDocumentNode* child,
    iMeshWrapper* mesh, bool keepZbuf, bool keepPrio)
  {
    csRef<iMeshObject> mo2 = scfQueryInterface<iMeshObject> (mo);
    if (!mo2)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.mesh",
        child, "Returned object does not implement iMeshObject!");
      return false;
    }
    mesh->SetMeshObject (mo2);
    mo2->SetMeshWrapper (mesh);
    if (mo2->GetFactory () && mo2->GetFactory ()->GetMeshFactoryWrapper ())
    {
      iMeshFactoryWrapper* mfw = mo2->GetFactory ()->GetMeshFactoryWrapper ();
      if (mfw)
      {
        mesh->SetFactory (mfw);
        if (!keepZbuf) mesh->SetZBufMode (mfw->GetZBufMode ());
        if (!keepPrio) mesh->SetRenderPriority (mfw->GetRenderPriority ());
        mesh->GetFlags ().Set (mfw->GetFlags ().Get (),
          mfw->GetFlags ().Get ());
      }
    }
    return true;
  }

  // Return true if the matrix does not scale.
  static bool TestOrthoMatrix (csMatrix3& m)
  {
    // Test if the matrix does not scale. Scaling meshes is illegal
    // in CS (must be done through hardmove).
    csVector3 v = m * csVector3 (1, 1, 1);
    float norm = v.Norm ();
    float desired_norm = 1.7320508f;
    return ABS (norm-desired_norm) < 0.01f;
  }

  template<typename T>
  static void ClearList (CS::Threading::ReadWriteMutex& mutex, T& list)
  {
    CS::Threading::ScopedWriteLock lock (mutex);
    list.DeleteAll ();
  }

  void csThreadedLoader::ClearLoaderLists ()
  {
    ClearList (sectorsLock, loaderSectors);
    ClearList (meshfactsLock, loaderMeshFactories);
    ClearList (meshesLock, loaderMeshes);
    ClearList (camposLock, loaderCameraPositions);
    ClearList (texturesLock, loaderTextures);
    ClearList (materialsLock, loaderMaterials);
    ClearList (sharedvarLock, loaderSharedVariables);
  }

  void csThreadedLoader::CollectAllChildren (iMeshWrapper* meshWrapper,
    csRefArray<iMeshWrapper>& meshesArray)
  {  
    size_t lastMeshVisited = 0;
    meshesArray.Push (meshWrapper);

    while (lastMeshVisited < meshesArray.GetSize ())
    {
      // Get the children of the current mesh (ie 'mesh').
      const csRef<iSceneNodeArray> ml = 
        meshesArray[lastMeshVisited++]->QuerySceneNode ()->GetChildrenArray ();
      size_t i;
      for (i = 0; i < ml->GetSize(); i++)
      {
        iMeshWrapper* m = ml->Get(i)->QueryMesh ();
        if (m)
          meshesArray.Push (m);
      }
    }

    return;
  }

  void csThreadedLoader::ClosedFlags (iMeshWrapper* mesh)
  {
    iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
    csRef<iTriangleMeshIterator> it = objmodel->GetTriangleDataIterator ();
    while (it->HasNext ())
    {
      csStringID id;
      iTriangleMesh* trimesh = it->Next (id);
      if (trimesh) trimesh->GetFlags ().Set (
        CS_TRIMESH_CLOSED | CS_TRIMESH_NOTCLOSED, CS_TRIMESH_CLOSED);
    }
  }

  void csThreadedLoader::ConvexFlags (iMeshWrapper* mesh)
  {
    iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
    csRef<iTriangleMeshIterator> it = objmodel->GetTriangleDataIterator ();
    while (it->HasNext ())
    {
      csStringID id;
      iTriangleMesh* trimesh = it->Next (id);
      if (trimesh) trimesh->GetFlags ().Set (
        CS_TRIMESH_CONVEX | CS_TRIMESH_NOTCONVEX, CS_TRIMESH_CONVEX);
    }
  }

  bool csThreadedLoader::HandleMeshParameter (iLoaderContext* ldr_context,
    iMeshWrapper* mesh, iMeshWrapper* parent, iDocumentNode* child,
    csStringID id, bool& handled, csString& priority,
    bool do_portal_container, bool& staticpos, bool& staticshape,
    bool& zmodeChanged, bool& prioChanged,
    bool recursive, iStreamSource* ssource)
  {
#undef TEST_MISSING_MESH
#define TEST_MISSING_MESH \
  if (!mesh) \
    { \
    SyntaxService->ReportError ( \
    "crystalspace.maploader.load.meshobject", \
    child, do_portal_container ? "Specify at least one portal first!" : \
    "First specify the parent factory with 'factory'!"); \
    return false; \
  }

    handled = true;
    switch (id)
    {
    case XMLTOKEN_STATICPOS:
      if (!SyntaxService->ParseBool (child, staticpos, true))
        return false;
      break;
    case XMLTOKEN_STATICSHAPE:
      if (!SyntaxService->ParseBool (child, staticshape, true))
        return false;
      break;
    case XMLTOKEN_MINRENDERDIST:
      {
        TEST_MISSING_MESH
          csRef<iDocumentAttribute> attr;
        if (attr = child->GetAttribute ("value"))
        {
          float dist = attr->GetValueAsFloat ();
          mesh->SetMinimumRenderDistance (dist);
        }
        else if (attr = child->GetAttribute ("var"))
        {
          csString varname = attr->GetValue ();
          iSharedVariable *var = Engine->GetVariableList()->FindByName (
            varname);
          if (!var)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshobject",
              child, "Variable '%s' doesn't exist!", varname.GetData ());
            return false;
          }
          mesh->SetMinimumRenderDistanceVar (var);
        }
        else
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.meshobject",
            child, "'value' or 'var' should be specified!");
          return false;
        }
      }
      break;
    case XMLTOKEN_MAXRENDERDIST:
      {
        TEST_MISSING_MESH
          csRef<iDocumentAttribute> attr;
        if (attr = child->GetAttribute ("value"))
        {
          float dist = attr->GetValueAsFloat ();
          mesh->SetMaximumRenderDistance (dist);
        }
        else if (attr = child->GetAttribute ("var"))
        {
          csString varname = attr->GetValue ();
          iSharedVariable *var = Engine->GetVariableList()->FindByName (
            varname);
          if (!var)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshobject",
              child, "Variable '%s' doesn't exist!", varname.GetData ());
            return false;
          }
          mesh->SetMaximumRenderDistanceVar (var);
        }
        else
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.meshobject",
            child, "'value' or 'var' should be specified!");
          return false;
        }
      }
      break;
    case XMLTOKEN_STATICLOD:
      {
        TEST_MISSING_MESH
          iLODControl* lodctrl = mesh->CreateStaticLOD ();
        if (!LoadLodControl (lodctrl, child))
          return false;
      }
      break;
    case XMLTOKEN_LODLEVEL:
      {
        TEST_MISSING_MESH
          if (!parent)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshobject", child,
              "Mesh must be part of a hierarchical mesh for <lodlevel>!");
            return false;
          }
          if (!parent->GetStaticLOD ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshobject", child,
              "Parent mesh must use <staticlod>!");
            return false;
          }
          parent->AddMeshToStaticLOD (child->GetContentsValueAsInt (), mesh);
      }
      break;
    case XMLTOKEN_LOD:
      {
        TEST_MISSING_MESH
          if (!mesh->GetMeshObject ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshobject",
              child, "Mesh object is missing!");
            return false;
          }
          csRef<iLODControl> lodctrl (scfQueryInterface<iLODControl> (
            mesh->GetMeshObject ()));
          if (!lodctrl)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshobject",
              child, "This mesh doesn't implement LOD control!");
            return false;
          }
          if (!LoadLodControl (lodctrl, child))
            return false;
      }
      break;
    case XMLTOKEN_PRIORITY:
      {
        priority = child->GetContentsValue ();
        long pri = Engine->GetRenderPriority (priority);
        if (pri == 0)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.meshobject",
            child, "Unknown render priority '%s'!", (const char*)priority);
          return false;
        }
        if (recursive)
          mesh->SetRenderPriorityRecursive (pri);
        else
          mesh->SetRenderPriority (pri);
        prioChanged = true;
      }
      break;
    case XMLTOKEN_ADDON:
      TEST_MISSING_MESH
        if (!LoadAddOn (ldr_context, child, mesh, false, ssource))
          return false;
      break;
    case XMLTOKEN_META:
      TEST_MISSING_MESH
        if (!LoadAddOn (ldr_context, child, mesh, true, ssource))
          return false;
      break;
    case XMLTOKEN_NOLIGHTING:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_NOLIGHTING, CS_ENTITY_NOLIGHTING);
        else
          mesh->GetFlags ().Set (CS_ENTITY_NOLIGHTING, CS_ENTITY_NOLIGHTING);
      break;
    case XMLTOKEN_NOSHADOWS:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_NOSHADOWS, CS_ENTITY_NOSHADOWS);
        else
          mesh->GetFlags ().Set (CS_ENTITY_NOSHADOWS, CS_ENTITY_NOSHADOWS);
      break;
    case XMLTOKEN_NOSHADOWCAST:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_NOSHADOWCAST, CS_ENTITY_NOSHADOWCAST);
        else
          mesh->GetFlags ().Set (CS_ENTITY_NOSHADOWCAST, CS_ENTITY_NOSHADOWCAST);
      break;
    case XMLTOKEN_NOSHADOWRECEIVE:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_NOSHADOWRECEIVE, CS_ENTITY_NOSHADOWRECEIVE);
        else
          mesh->GetFlags ().Set (CS_ENTITY_NOSHADOWRECEIVE, CS_ENTITY_NOSHADOWRECEIVE);
      break;
    case XMLTOKEN_NODECAL:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_NODECAL, CS_ENTITY_NODECAL);
        else
          mesh->GetFlags ().Set (CS_ENTITY_NODECAL, CS_ENTITY_NODECAL);
      break;
    case XMLTOKEN_NOCLIP:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_NOCLIP, CS_ENTITY_NOCLIP);
        else
          mesh->GetFlags ().Set (CS_ENTITY_NOCLIP, CS_ENTITY_NOCLIP);
      break;
    case XMLTOKEN_NOHITBEAM:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_NOHITBEAM, CS_ENTITY_NOHITBEAM);
        else
          mesh->GetFlags ().Set (CS_ENTITY_NOHITBEAM, CS_ENTITY_NOHITBEAM);
      break;
    case XMLTOKEN_INVISIBLEMESH:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_INVISIBLEMESH,
          CS_ENTITY_INVISIBLEMESH);
        else
          mesh->GetFlags ().Set (CS_ENTITY_INVISIBLEMESH,
          CS_ENTITY_INVISIBLEMESH);
      break;
    case XMLTOKEN_INVISIBLE:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_INVISIBLE, CS_ENTITY_INVISIBLE);
        else
          mesh->GetFlags ().Set (CS_ENTITY_INVISIBLE, CS_ENTITY_INVISIBLE);
      break;
    case XMLTOKEN_DETAIL:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
        else
          mesh->GetFlags ().Set (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
      break;
    case XMLTOKEN_STATICLIT:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_STATICLIT, CS_ENTITY_STATICLIT);
        else
          mesh->GetFlags ().Set (CS_ENTITY_STATICLIT, CS_ENTITY_STATICLIT);
      break;
    case XMLTOKEN_LIMITEDSHADOWCAST:
      TEST_MISSING_MESH
        if (recursive)
          mesh->SetFlagsRecursive (CS_ENTITY_LIMITEDSHADOWCAST,
          CS_ENTITY_LIMITEDSHADOWCAST);
        else
          mesh->GetFlags ().Set (CS_ENTITY_LIMITEDSHADOWCAST,
          CS_ENTITY_LIMITEDSHADOWCAST);
      break;
    case XMLTOKEN_ZFILL:
      TEST_MISSING_MESH
        if (priority.IsEmpty ()) priority = "wall";
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (
          Engine->GetRenderPriority (priority));
        mesh->SetZBufModeRecursive (CS_ZBUF_FILL);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->SetZBufMode (CS_ZBUF_FILL);
      }
      zmodeChanged = true;
      break;
    case XMLTOKEN_ZUSE:
      TEST_MISSING_MESH
        if (priority.IsEmpty ()) priority = "object";
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetZBufModeRecursive (CS_ZBUF_USE);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->SetZBufMode (CS_ZBUF_USE);
      }
      zmodeChanged = true;
      break;
    case XMLTOKEN_ZNONE:
      TEST_MISSING_MESH
        if (priority.IsEmpty ()) priority = "sky";
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetZBufModeRecursive (CS_ZBUF_NONE);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->SetZBufMode (CS_ZBUF_NONE);
      }
      zmodeChanged = true;
      break;
    case XMLTOKEN_ZTEST:
      TEST_MISSING_MESH
        if (priority.IsEmpty ()) priority = "alpha";
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetZBufModeRecursive (CS_ZBUF_TEST);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->SetZBufMode (CS_ZBUF_TEST);
      }
      zmodeChanged = true;
      break;
    case XMLTOKEN_CAMERA:
      TEST_MISSING_MESH
        if (priority.IsEmpty ()) priority = "sky";
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetFlagsRecursive (CS_ENTITY_CAMERA, CS_ENTITY_CAMERA);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->GetFlags ().Set (CS_ENTITY_CAMERA, CS_ENTITY_CAMERA);
      }
      break;
    case XMLTOKEN_BADOCCLUDER:
      TEST_MISSING_MESH
        else
      {
        //Apply the flag CS_CULLER_HINT_BADOCCLUDER to all the meshes in 
        //the meshes' hierarchy, starting from the 'mesh' mesh object.
        csRefArray<iMeshWrapper> meshesArray;
        CollectAllChildren (mesh, meshesArray);
        size_t i, count = meshesArray.GetSize ();
        for (i = 0; i < count; i++)
        {
          csRef<iVisibilityObject> visobj = 
            scfQueryInterface<iVisibilityObject> (meshesArray[i]);
          if (visobj)
            visobj->GetCullerFlags ().Set (CS_CULLER_HINT_BADOCCLUDER);
        }
        }
        break;
    case XMLTOKEN_GOODOCCLUDER:
      TEST_MISSING_MESH
        else
      {
        //Apply the flag CS_CULLER_HINT_GOODOCCLUDER to all the meshes in 
        //the meshes' hierarchy, starting from the 'mesh' mesh object.
        csRefArray<iMeshWrapper> meshesArray;
        CollectAllChildren (mesh, meshesArray);
        size_t i, count = meshesArray.GetSize ();
        for (i = 0; i < count; i++)
        {
          csRef<iVisibilityObject> visobj = 
            scfQueryInterface<iVisibilityObject> (meshesArray[i]);
          if (visobj)
            visobj->GetCullerFlags ().Set (CS_CULLER_HINT_GOODOCCLUDER);
        }
        }
        break;
    case XMLTOKEN_CLOSED:
      TEST_MISSING_MESH
        else
      {
        if (recursive)//Test if recursion on children has been specified.
        {
          csRefArray<iMeshWrapper> meshesArray;
          CollectAllChildren (mesh, meshesArray);
          size_t i, count = meshesArray.GetSize ();
          for (i = 0; i < count; i++)
          {
            ClosedFlags (meshesArray[i]);
          }
        }//if
        else
          ClosedFlags (mesh);
        }
        break;
    case XMLTOKEN_CONVEX:
      TEST_MISSING_MESH
        else
      {
        //Test if recursion on children has been specified.
        if (recursive)
        {
          csRefArray<iMeshWrapper> meshesArray;
          CollectAllChildren (mesh, meshesArray);
          size_t i, count = meshesArray.GetSize ();
          for (i = 0; i < count; i++)
          {
            ConvexFlags (meshesArray[i]);
          }
        }
        else
          ConvexFlags (mesh);
        }
        break;
    case XMLTOKEN_KEY:
      TEST_MISSING_MESH
        else
      {
        if (!ParseKey (child, mesh->QueryObject()))
          return false;
        }
        break;
    case XMLTOKEN_HARDMOVE:
      TEST_MISSING_MESH
        if (!mesh->GetMeshObject())
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.meshobject",
            child, "Please specify the params of the meshobject first!");
          return false;
        }
        else if (!mesh->GetMeshObject ()->SupportsHardTransform ())
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.meshobject",
            child, "This mesh object doesn't support 'hardmove'!");
          return false;
        }
        else
        {
          csReversibleTransform tr;
          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
              return false;
            tr.SetT2O (m);
          }
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
              return false;
            tr.SetOrigin (v);
          }
          mesh->HardTransform (tr);
        }
        break;
    case XMLTOKEN_MOVE:
      TEST_MISSING_MESH
        else
      {
        mesh->GetMovable ()->SetTransform (csMatrix3 ());     // Identity
        mesh->GetMovable ()->SetPosition (csVector3 (0));
        csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
        if (matrix_node)
        {
          csMatrix3 m;
          if (!SyntaxService->ParseMatrix (matrix_node, m))
            return false;
          if (!TestOrthoMatrix (m))
          {
            ReportWarning (
              "crystalspace.maploader.load.mesh",
              child, "Scaling of mesh objects is not allowed in CS!");
          }
          mesh->GetMovable ()->SetTransform (m);
        }
        csRef<iDocumentNode> vector_node = child->GetNode ("v");
        if (vector_node)
        {
          csVector3 v;
          if (!SyntaxService->ParseVector (vector_node, v))
            return false;
          mesh->GetMovable ()->SetPosition (v);
        }
        mesh->GetMovable ()->UpdateMove ();
        }
        break;
    case XMLTOKEN_BOX:
      TEST_MISSING_MESH
        else
      {
        csBox3 b;
        if (!SyntaxService->ParseBox (child, b))
          return false;
        mesh->GetMeshObject ()->GetObjectModel ()->SetObjectBoundingBox (b);
        }
        break;
    case XMLTOKEN_SHADERVAR:
      TEST_MISSING_MESH
      else
      {
        csRef<iShaderVariableContext> svc = 
          scfQueryInterface<iShaderVariableContext> (mesh);
        CS_ASSERT (svc.IsValid());
        //create a new variable
        const char* varname = child->GetAttributeValue ("name");
        csRef<csShaderVariable> var;
        var.AttachNew (new csShaderVariable (stringSetSvName->Request (varname)));
        if (!SyntaxService->ParseShaderVar (ldr_context, child, *var))
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.meshobject", child,
            "Error loading shader variable '%s' in mesh '%s'.", 
            varname, mesh->QueryObject()->GetName());
          break;
        }
        svc->AddVariable (var);
      }
      break;
    default:
      handled = false;
      return true;
    }
    return true;
#undef TEST_MISSING_MESH
  }

  csRef<iMeshWrapper> csThreadedLoader::LoadMeshObjectFromFactory (
    iLoaderContext* ldr_context, iDocumentNode* node,
    iStreamSource* ssource)
  {
    if (!Engine) return 0;

    csString priority;

    csRef<iMeshWrapper> mesh;
    bool staticpos = false;
    bool staticshape = false;
    bool zbufSet = false;
    bool prioSet = false;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      bool handled;
      if (!HandleMeshParameter (ldr_context, mesh, 0, child, id,
        handled, priority, false, staticpos, staticshape, zbufSet,
        prioSet, true, ssource))
        return 0;
      if (!handled) switch (id)
      {
      case XMLTOKEN_FACTORY:
        if (mesh)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.meshobject",
            child, "There is already a factory for this mesh!");
          return 0;
        }
        else
        {
          iMeshFactoryWrapper* t = ldr_context->FindMeshFactory (
            child->GetContentsValue ());
          if (!t)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshobject",
              child, "Can't find factory '%s'!", child->GetContentsValue ());
            return 0;
          }
          mesh = t->CreateMeshWrapper ();
          if (mesh)
          {
            ldr_context->AddToCollection(mesh->QueryObject ());
            // Now also add the child mesh objects to the region.
            const csRef<iSceneNodeArray> children = 
              mesh->QuerySceneNode ()->GetChildrenArray ();
            AddChildrenToCollection (ldr_context, children);
          }
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return 0;
      }
    }

    if (!mesh)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.load.meshobject",
        node, "There is no 'factory' for this mesh!");
      return 0;
    }
    if (priority.IsEmpty ()) priority = "object";
    mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));

    //I had to put these ugly curly brackets. It's due to the uglier label
    //below! 'children' and 'set' need an initialization indeed. Luca
    {
      csRefArray<iMeshWrapper> meshesArray;
      CollectAllChildren (mesh, meshesArray);
      size_t i, count = meshesArray.GetSize ();
      for (i = 0; i < count; i++)
      {
        iMeshWrapper* mesh = meshesArray[i];
        mesh->GetMeshObject ()->GetFlags ().SetBool (
          CS_MESH_STATICPOS, staticpos);
        mesh->GetMeshObject ()->GetFlags ().SetBool (
          CS_MESH_STATICSHAPE, staticshape);
      }
    }

    return mesh;
  }

  void csThreadedLoader::AddChildrenToCollection (iLoaderContext* ldr_context,
    const iSceneNodeArray* children)
  {
    size_t i;
    for (i = 0 ; i < children->GetSize(); i++)
    {
      iSceneNode* sn = children->Get(i);
      iObject* obj = 0;
      if (sn->QueryMesh ()) obj = sn->QueryMesh ()->QueryObject ();
      else if (sn->QueryLight ()) obj = sn->QueryLight ()->QueryObject ();
      //else if (sn->QueryCamera ()) obj = sn->QueryCamera ()->QueryObject ();
      if (obj)
        ldr_context->AddToCollection(obj);
      const csRef<iSceneNodeArray> nodeChildren = sn->GetChildrenArray ();
      AddChildrenToCollection (ldr_context, nodeChildren);
    }
  }

  bool csThreadedLoader::LoadSounds (iDocumentNode* node)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_SOUND:
        {
          const char* name = child->GetAttributeValue ("name");
          const char* filename = name;
          csRef<iDocumentNode> filenode = child->GetNode ("file");
          if (filenode)
          {
            filename = filenode->GetContentsValue ();
          }
          else
          {
            const char* fname = child->GetAttributeValue ("file");
            if (fname) filename = fname;
          }
          int mode3d = -1;
          csRef<iDocumentAttribute> at = child->GetAttribute ("mode3d");
          if (at)
          {
            ReportWarning (
              "crystalspace.maploader.parse.sound",
              child, "The 'mode3d' attribute is deprecated! Specify 2d/3d mode when playing sound.");
            const char* v = at->GetValue ();
            if (!strcasecmp ("disable", v))
              mode3d = CS_SND3D_DISABLE;
            else if (!strcasecmp ("relative", v))
              mode3d = CS_SND3D_RELATIVE;
            else if (!strcasecmp ("absolute", v))
              mode3d = CS_SND3D_ABSOLUTE;
            else
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.sound", child,
                "Use either 'disable', 'relative', or 'absolute' for mode3d attribute!");
              return false;
            }
          }

          // New sound system.
          if (!SndSysLoader || !SndSysManager)
          {
            //SyntaxService->ReportError (
            //"crystalspace.maploader.parse.sound", child,
            //"New sound loader not loaded!");
            return true;
          }
          csRef<iSndSysWrapper> snd = SndSysManager->FindSoundByName (name);
          if (!snd)
          {
            csRef<iThreadReturn> ret = csPtr<iThreadReturn>(new csThreadReturn(threadman));
	    csString cwd = vfs->GetCwd ();
            LoadSoundWrapperTC (ret, false, cwd, name, filename, false);
            snd = scfQueryInterface<iSndSysWrapper>(ret->GetResultRefPtr());
          }
          if (snd)
          {
            csRef<iDocumentNodeIterator> it2 (child->GetNodes ());
            while (it2->HasNext ())
            {
              csRef<iDocumentNode> child2 = it2->Next ();
              if (child2->GetType () != CS_NODE_ELEMENT) continue;
              switch (xmltokens.Request (child2->GetValue ()))
              {
              case XMLTOKEN_KEY:
                {
                  if (!ParseKey (child2, snd->QueryObject()))
                    return false;
                }
                break;
              }
            }
          }
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  void csThreadedLoader::LoadPlugin (iDocumentNode* node)
  {
    const char* plugin_name = node->GetAttributeValue ("name");
    loaded_plugins.NewPlugin (plugin_name, node);
    if (Engine->GetSaveableFlag ())
    {
      const char* plugin_id = loaded_plugins.FindPluginClassID (
        plugin_name);
      if (plugin_id)
      {
        csRef<iPluginReference> pluginref;
        pluginref.AttachNew (new csPluginReference (plugin_name,
          plugin_id));
        object_reg->Register (pluginref,
          csString ("_plugref_") + plugin_id);
      }
    }
  }

  bool csThreadedLoader::LoadPlugins (iDocumentNode* node)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_PLUGIN:
        {
          LoadPlugin(child);
        }
        break;

      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool csThreadedLoader::LoadSettings (iDocumentNode* node)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_FASTMESH:
        ReportWarning (
          "crystalspace.maploader.parse.xml",
          "<fastmesh> is no longer needed.");
        break;
      case XMLTOKEN_CLEARZBUF:
        {
          bool yesno;
          if (!SyntaxService->ParseBool (child, yesno, true))
            return false;
          Engine->SetClearZBuf (yesno);
        }
        break;
      case XMLTOKEN_CLEARSCREEN:
        {
          bool yesno;
          if (!SyntaxService->ParseBool (child, yesno, true))
            return false;
          Engine->SetClearScreen (yesno);
        }
        break;
      case XMLTOKEN_LIGHTMAPCELLSIZE:
        ReportWarning("crystalspace.maploader.parse.settings",
          child, "The 'lightmapcellsize' attribute is deprecated!");
        break;
      case XMLTOKEN_MAXLIGHTMAPSIZE:
        ReportWarning("crystalspace.maploader.parse.settings",
          child, "The 'maxlightmapsize' attribute is deprecated!");
        break;
      case XMLTOKEN_AMBIENT:
        {
          csColor c;
          if (!SyntaxService->ParseColor (child, c))
            return false;
          Engine->SetAmbientLight (c);
        }
        break;
      case XMLTOKEN_RENDERLOOP:
        {
          bool set;
          iRenderLoop* loop = ParseRenderLoop (child, set);
          if (!loop)
          {
            return false;
          }
          if (set)
          {
            Engine->SetCurrentDefaultRenderloop (loop);
          }
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool csThreadedLoader::LoadTriMeshInSector (iLoaderContext* ldr_context,
    iMeshWrapper* mesh, iDocumentNode* node, iStreamSource* ssource)
  {
    iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
    csRef<iTriangleMesh> trimesh;
    csArray<csStringID> ids;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_ADDON:
        if (!LoadAddOn (ldr_context, child, mesh, false, ssource))
          return false;
        break;
      case XMLTOKEN_META:
        if (!LoadAddOn (ldr_context, child, mesh, true, ssource))
          return false;
        break;
      case XMLTOKEN_MOVE:
        {
          mesh->GetMovable ()->SetTransform (csMatrix3 ());     // Identity
          mesh->GetMovable ()->SetPosition (csVector3 (0));
          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
              return false;
            if (!TestOrthoMatrix (m))
            {
              ReportWarning (
                "crystalspace.maploader.load.mesh",
                child, "Scaling of mesh objects is not allowed in CS!");
            }
            mesh->GetMovable ()->SetTransform (m);
          }
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
              return false;
            mesh->GetMovable ()->SetPosition (v);
          }
          mesh->GetMovable ()->UpdateMove ();
          break;
        }
      case XMLTOKEN_BOX:
        if (!ParseTriMeshChildBox (child, trimesh))
          return false;
        break;
      case XMLTOKEN_MESH:
        if (!ParseTriMeshChildMesh (child, trimesh))
          return false;
        break;
      case XMLTOKEN_ID:
        ids.Push (stringSet->Request (child->GetContentsValue ()));
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    if (ids.GetSize () == 0)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.trimesh",
        node, "No id's for this triangle mesh!");
      return false;
    }
    if (!trimesh)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.sector.trimesh",
        node, "Please specify either <mesh/> or <box/>!");
      return false;
    }

    size_t i;
    for (i = 0 ; i < ids.GetSize () ; i++)
      objmodel->SetTriangleData (ids[i], trimesh);

    csRef<iNullMeshState> nullmesh = scfQueryInterface<iNullMeshState> (
      mesh->GetMeshObject ());
    CS_ASSERT (nullmesh != 0);
    csBox3 bbox;
    csVector3* vt = trimesh->GetVertices ();
    bbox.StartBoundingBox (vt[0]);
    for (i = 1 ; i < trimesh->GetVertexCount () ; i++)
    {
      bbox.AddBoundingVertexSmart (vt[i]);
    }
    nullmesh->SetBoundingBox (bbox);

    return true;
  }

  bool csThreadedLoader::LoadMeshGen (iLoaderContext* ldr_context,
    iDocumentNode* node, iSector* sector)
  {
    const char* name = node->GetAttributeValue ("name");
    iMeshGenerator* meshgen = sector->CreateMeshGenerator (name);
    ldr_context->AddToCollection(meshgen->QueryObject ());

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_GEOMETRY:
        if (!LoadMeshGenGeometry (ldr_context, child, meshgen))
          return false;
        break;
      case XMLTOKEN_MESHOBJ:
        {
          const char* meshname = child->GetContentsValue ();
          iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
          if (!mesh)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find mesh object '%s' for mesh generator!",
              meshname);
            return false;
          }
          meshgen->AddMesh (mesh);
        }
        break;
      case XMLTOKEN_DENSITYSCALE:
        {
          float mindist = child->GetAttributeValueAsFloat ("mindist");
          float maxdist = child->GetAttributeValueAsFloat ("maxdist");
          float maxfactor = child->GetAttributeValueAsFloat ("maxfactor");
          meshgen->SetDensityScale (mindist, maxdist, maxfactor);
        }
        break;
      case XMLTOKEN_ALPHASCALE:
        {
          float mindist = child->GetAttributeValueAsFloat ("mindist");
          float maxdist = child->GetAttributeValueAsFloat ("maxdist");
          meshgen->SetAlphaScale (mindist, maxdist);
        }
        break;
      case XMLTOKEN_NUMBLOCKS:
        meshgen->SetBlockCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_CELLDIM:
        meshgen->SetCellCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_SAMPLEBOX:
        {
          csBox3 b;
          if (!SyntaxService->ParseBox (child, b))
            return false;
          meshgen->SetSampleBox (b);
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }
    return true;
  }

  bool csThreadedLoader::LoadMeshGenGeometry (iLoaderContext* ldr_context,
    iDocumentNode* node,
    iMeshGenerator* meshgen)
  {
    iMeshGeneratorGeometry* geom = meshgen->CreateGeometry ();

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_FACTORY:
        {
          const char* factname = child->GetAttributeValue ("name");
          float maxdist = child->GetAttributeValueAsFloat ("maxdist");
          iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
          if (!fact)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find mesh factory '%s' for mesh generator!",
              factname);
            return false;
          }
          geom->AddFactory (fact, maxdist);
        }
        break;
      case XMLTOKEN_POSITIONMAP:
        {
          const char* map_name = child->GetAttributeValue ("mapname");
          csRef<iTerraFormer> map = csQueryRegistryTagInterface<iTerraFormer> 
            (object_reg, map_name);
          if (!map)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find map position map terraformer '%s'!", map_name);
            return false;
          }

          csVector2 min, max;
          csRef<iDocumentNode> region_node = child->GetNode ("region");

          SyntaxService->ParseVector (region_node->GetNode ("min"), min);
          SyntaxService->ParseVector (region_node->GetNode ("max"), max);

          float value = child->GetAttributeValueAsFloat ("value");

          uint resx = child->GetAttributeValueAsInt ("resx");
          uint resy = child->GetAttributeValueAsInt ("resy");

          csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
            object_reg, "crystalspace.shared.stringset");

          geom->AddPositionsFromMap (map, csBox2 (min.x, min.y, max.x, max.y), 
            resx, resy, value, strings->Request ("heights"));
        }
        break;
      case XMLTOKEN_DENSITYMAP:
        {
          const char* map_name = child->GetContentsValue ();
          csRef<iTerraFormer> map_tf = csQueryRegistryTagInterface<iTerraFormer> 
            (object_reg, map_name);
          if (!map_tf)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find map density map terraformer '%s'!", map_name);
            return false;
          }
          float factor = child->GetAttributeValueAsFloat ("factor");
          csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
            object_reg, "crystalspace.shared.stringset");
          geom->SetDensityMap (map_tf, factor, strings->Request ("densitymap"));
        }
        break;
      case XMLTOKEN_MATERIALFACTOR:
        {
          const char* matname = child->GetAttributeValue ("material");
          if (!matname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "'material' attribute is missing!");
            return false;
          }
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
          if (!mat)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find material '%s'!", matname);
            return false;
          }
          float factor = child->GetAttributeValueAsFloat ("factor");
          geom->AddDensityMaterialFactor (mat, factor);
        }
        break;
      case XMLTOKEN_DEFAULTMATERIALFACTOR:
        {
          float factor = child->GetContentsValueAsFloat ();
          geom->SetDefaultDensityMaterialFactor (factor);
        }
        break;
      case XMLTOKEN_RADIUS:
        geom->SetRadius (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_DENSITY:
        geom->SetDensity (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_WINDBIAS:
        geom->SetWindBias(child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_WINDDIRECTION:
        geom->SetWindDirection(child->GetAttributeValueAsFloat("x"),
          child->GetAttributeValueAsFloat("y"));
        break;
      case XMLTOKEN_WINDSPEED:
        geom->SetWindSpeed(child->GetContentsValueAsFloat ());
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }
    return true;
  }

  static bool TestXML (const char* b)
  {
    while (*b && isspace (*b)) b++;
    if (*b != '<') return false;
    b++;
    if (*b == '?')
    {
      return (b[1] == 'x' && b[2] == 'm' && b[3] == 'l' && isspace (b[4]));
    }
    else if (*b == '!')
    {
      return (b[1] == '-' && b[2] == '-');
    }
    else
    {
      b++;
      if (!isalpha (*b) && *b != '_')
        return false;
      b++;
      while (isalnum (*b)) b++;
      if (!isspace (*b) && *b != '>')
        return false;
      return true;
    }
  }

  bool csThreadedLoader::Load (iThreadReturn* ret, iDataBuffer* buffer, const char* fname, iCollection* collection,
    iStreamSource* ssource, iMissingLoaderData* missingdata, uint keepFlags, bool do_verbose)
  {
    if (TestXML (buffer->GetData ()))
    {
      csRef<iDocument> doc;
      bool er = LoadStructuredDoc (fname, buffer, doc);
      if (!er)
      {
        return false;
      }

      if (doc)
      {
        csRef<iDocumentNode> node = doc->GetRoot ();
	      csString cwd = vfs->GetCwd ();
        return LoadNodeTC(ret, false, cwd, node, collection, 0, ssource, missingdata, keepFlags, do_verbose);
      }
      else
      {
        ReportError ("crystalspace.maploader.parse",
          fname
          ? "File does not appear to be correct XML file (%s)!"
          : "Buffer does not appear to be correct XML!", fname);
        return false;
      }
    }
    else
    {
      csRef<iPluginManager> plugin_mgr = csQueryRegistry<iPluginManager> (
        object_reg);
      csRef<iStringArray> model_loader_ids = iSCF::SCF->QueryClassList (
        "crystalspace.mesh.loader.factory.");
      for (size_t i = 0; i < model_loader_ids->GetSize(); i++)
      {
        const char* plugin = model_loader_ids->Get (i);
        csRef<iModelLoader> l = csQueryPluginClass<iModelLoader> (plugin_mgr,
          plugin);
        if (!l)
          l = csLoadPlugin<iModelLoader> (plugin_mgr, plugin);
        if (l && l->IsRecognized (buffer))
        {
          iMeshFactoryWrapper* ff = l->Load (fname ? fname : "__model__", buffer);
          if(ff)
          {
            ret->SetResult(scfQueryInterfaceSafe<iBase>(ff));
            return true;
          }
        }
      }
      ReportError ("crystalspace.maploader.parse",
        fname
        ? "Model file not recognized (%s)!"
        : "Model buffer not recognized!", fname);
    }

    return false;
  }

  //-------------------------------------------------------------------------//

  void csThreadedLoader::ReportError (const char* id, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR, id, description, arg);
    va_end (arg);
  }

  void csThreadedLoader::ReportNotify (const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportNotifyV ("crystalspace.maploader", description, arg);
    va_end (arg);
  }

  void csThreadedLoader::ReportNotifyV (const char* id, const char* description, va_list arg)
  {
    csReportV (object_reg, CS_REPORTER_SEVERITY_NOTIFY, id, description, arg);
  }

  void csThreadedLoader::ReportNotify2 (const char* id, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportNotifyV (id, description, arg);
    va_end (arg);
  }

  void csThreadedLoader::ReportWarning (const char* id, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_WARNING, id, description, arg);
    va_end (arg);
  }

  void csThreadedLoader::ReportWarning (const char* id, iDocumentNode* node, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csString buf;
    buf.FormatV (description, arg);
    va_end (arg);
    SyntaxService->Report (id, CS_REPORTER_SEVERITY_WARNING, node, "%s", 
      buf.GetData());
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
