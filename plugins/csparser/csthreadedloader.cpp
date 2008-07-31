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
#include "csqint.h"

#include "cstool/saverref.h"
#include "cstool/saverfile.h"
#include "cstool/unusedresourcehelper.h"
#include "cstool/vfsdirchange.h"

#include "csutil/eventnames.h"
#include "csutil/threadmanager.h"
#include "csutil/xmltiny.h"

#include "iengine/engine.h"
#include "iengine/imposter.h"
#include "iengine/lod.h"
#include "iengine/mesh.h"
#include "iengine/sharevar.h"

#include "igeom/trimesh.h"
#include "igraphic/animimg.h"
#include "igraphic/imageio.h"

#include "imap/saverfile.h"
#include "imap/saverref.h"
#include "imap/services.h"

#include "imesh/object.h"
#include "imesh/objmodel.h"
#include "imesh/nullmesh.h"

#include "itexture/iproctex.h"

#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"

#include "ivaria/keyval.h"

#include "csthreadedloader.h"
#include "csloadercontext.h"
#include "loadtex.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  SCF_IMPLEMENT_FACTORY(csThreadedLoader)

  csThreadedLoader::csThreadedLoader(iBase *p) : scfImplementationType (this, p)
  {
  }

  csThreadedLoader::~csThreadedLoader()
  {
  }

  bool csThreadedLoader::Initialize(iObjectRegistry *objectreg)
  {
    object_reg = objectreg;
    loaded_plugins.SetObjectRegistry (object_reg);

    csRef<iPluginManager> plugin_mgr = csQueryRegistry<iPluginManager>(object_reg);
    loaded_plugins.plugin_mgr = plugin_mgr;

    tm = csQueryRegistry<iThreadManager>(object_reg);
    if(!tm.IsValid())
    {
      return false;
    }

    Engine = csQueryRegistry<iEngine>(object_reg);
    if(!Engine.IsValid())
    {
      return false;
    }

    vfs = csQueryRegistry<iVFS>(object_reg);
    if(!vfs.IsValid())
    {
      return false;
    }

    csRef<iEventQueue> eventQueue = csQueryRegistry<iEventQueue>(object_reg);
    if(eventQueue)
    {
      ProcessPerFrame = csevFrame(object_reg);
      eventQueue->RegisterListener(this, ProcessPerFrame);
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
      return false;
    }

    g3d = csQueryRegistry<iGraphics3D> (object_reg);
    if(!g3d)
    {
      return false;
    }

    return true;
  }

  bool csThreadedLoader::HandleEvent(iEvent& Event)
  {
    if(Event.Name == ProcessPerFrame)
    {
      Engine->SyncEngineLists(this);
    }
    return false;
  }

  THREADED_CALLABLE_IMPL2(csThreadedLoader, LoadImage, const char* fname, int Format)
  {
    csRef<iDataBuffer> buf = vfs->ReadFile (fname, false);
    csRef<iImage> image = LoadImage (buf, fname, Format);
    if(image.IsValid())
    {
      ret->SetResult(csRef<iBase>(image));
      ret->MarkSuccessful();
    }
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL2(csThreadedLoader, LoadImage, iDataBuffer* buf,  int Format)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL4(csThreadedLoader, LoadTexture, const char* Filename,
    int Flags, csRef<iTextureManager> texman, csRef<iImage>* image)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL4(csThreadedLoader, LoadTexture, iDataBuffer* buf, int Flags,
    iTextureManager* texman, csRef<iImage>* image)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL7(csThreadedLoader, LoadTexture, const char* Name,
    iDataBuffer* buf, int Flags, iTextureManager* texman, bool reg, bool create_material,
    bool free_image)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL9(csThreadedLoader, LoadTexture, const char* Name,
    const char* FileName, int Flags, iTextureManager* texman, bool reg, bool create_material,
    bool free_image, iCollection* Collection, uint keepFlags)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL1(csThreadedLoader, LoadSoundSysData, const char* fname)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL2(csThreadedLoader, LoadSoundStream, const char* fname, int mode3d)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL2(csThreadedLoader, LoadSoundWrapper, const char* name, const char* fname)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL2(csThreadedLoader, LoadMeshObjectFactory, const char* fname,
    iStreamSource* ssource)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL2(csThreadedLoader, LoadMeshObject, const char* fname,
    iStreamSource* ssource)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL2(csThreadedLoader, LoadShader, const char* filename,
    bool registerShader)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadMapFile, const char* filename,
    bool clearEngine, iCollection* collection, bool searchCollectionOnly, bool checkDupes,
    iStreamSource* ssource, iMissingLoaderData* missingdata, uint keepFlags)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadMap, iDocumentNode* world_node,
    bool clearEngine, iCollection* collection, bool searchCollectionOnly, bool checkDupes,
    iStreamSource* ssource, iMissingLoaderData* missingdata, uint keepFlags)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL7(csThreadedLoader, LoadLibraryFile, const char* filename,
    iCollection* collection, bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource,
    iMissingLoaderData* missingdata, uint keepFlags)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL7(csThreadedLoader, LoadLibrary, iDocumentNode* lib_node,
    iCollection* collection, bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource,
    iMissingLoaderData* missingdata, uint keepFlags)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadFile, const char* fname,
    iCollection* collection, bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource,
    const char* override_name, iMissingLoaderData* missingdata, uint keepFlags)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadBuffer, iDataBuffer* buffer,
    iCollection* collection, bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource,
    const char* override_name, iMissingLoaderData* missingdata, uint keepFlags)
  {
    ret->MarkFinished();
  }

  THREADED_CALLABLE_IMPL9(csThreadedLoader, LoadNode, csRef<iDocumentNode> node,
      csRef<iCollection> collection, bool searchCollectionOnly, bool checkDupes,
      csRef<iStreamSource> ssource, const char* override_name,
      csRef<iMissingLoaderData> missingdata, uint keepFlags, bool do_verbose)
  {
    csRef<iLoaderContext> ldr_context;
    ldr_context.AttachNew(new csLoaderContext(object_reg, Engine, collection,
      searchCollectionOnly, checkDupes, missingdata, keepFlags, do_verbose));

    // Mesh Factory
    csRef<iDocumentNode> meshfactnode = node->GetNode("meshfact");
    if(meshfactnode)
    {
      csRef<iThreadReturn> meshFactRet = LoadMeshFactory(ldr_context, meshfactnode, override_name, 0, 0, ssource);
      tm->Wait(meshFactRet);
      ret->Copy(meshFactRet);
      return;
    }
/*
    // Mesh Object
    csRef<iDocumentNode> meshobjnode = node->GetNode ("meshobj");
    if(meshobjnode)
    {
      const char* meshobjname = override_name ? override_name :
        meshobjnode->GetAttributeValue ("name");

      if(ldr_context->CheckDupes())
      {
        iMeshWrapper* mw = Engine->FindMeshObject(meshobjname);
        if(mw)
        {
          ldr_context->AddToCollection(mw->QueryObject());
          loadResult->result = mw;
          loadResult->success = true;
          return;
        }
      }

      csRef<iMeshWrapper> mw = Engine->CreateMeshWrapper (meshobjname);
      if(LoadMeshObject(ldr_context, mw, 0, meshobjnode, ssource))
      {
        ldr_context->AddToCollection(mw->QueryObject());
        loadResult->result = mw;
        loadResult->success = true;
        return;
      }
      else
      {
        // Error is already reported.
        Engine->GetMeshes()->Remove(mw);
        return;
      }
    }

    // World node.
    csRef<iDocumentNode> worldnode = node->GetNode ("world");
    if(worldnode)
    {
      loadResult->success = LoadMap (ldr_context, worldnode, ssource, missingdata);
      return;
    }
*/
    // Library node.
    csRef<iDocumentNode> libnode = node->GetNode ("library");
    if (libnode)
    {
      LoadLibrary(ldr_context, libnode, ssource, missingdata, true);
      ret->MarkFinished();      
      return;
    }
/*
    // Portals.
    csRef<iDocumentNode> portalsnode = node->GetNode ("portals");
    if (portalsnode)
    {
      const char* portalsname = override_name ? override_name :
        portalsnode->GetAttributeValue ("name");
      if (ldr_context->CheckDupes () && portalsname)
      {
        iMeshWrapper* mw = Engine->FindMeshObject (portalsname);
        if (mw)
        {
          csRef<iPortalContainer> pc = 
            scfQueryInterface<iPortalContainer>(mw->GetMeshObject());
          if (pc)
          {
            ldr_context->AddToCollection(mw->QueryObject());
            loadResult->result = mw;
            loadResult->success = true;
            return;
          }
        }
      }
      if (ParsePortals (ldr_context, portalsnode, 0, 0, ssource))
      {
        iMeshWrapper* mw = 0;
        if (ldr_context->GetCollection())
          mw = ldr_context->GetCollection()->FindMeshObject(portalsname);

        if (mw)
        {
          mw->QueryObject()->SetName(portalsname);
          loadResult->result = mw;
          loadResult->success = true;
          return;
        }
      }

      loadResult->result = 0;
      loadResult->success = false;
      return;
    }

    // Light.
    csRef<iDocumentNode> lightnode = node->GetNode ("light");
    if (lightnode)
    {
      const char* lightname = override_name ? override_name :
        lightnode->GetAttributeValue ("name");
      iLight* light = ParseStatlight (ldr_context, lightnode);
      if (light)
      {
        light->QueryObject()->SetName(lightname);
        loadResult->result = light;
        loadResult->success = true;
        return;
      }

      loadResult->result = 0;
      loadResult->success = false;
      return;
    }

    // MeshRef
    csRef<iDocumentNode> meshrefnode = node->GetNode ("meshref");
    if (meshrefnode)
    {
      const char* meshobjname = override_name ? override_name :
        meshrefnode->GetAttributeValue ("name");
      if (ldr_context->CheckDupes () && meshobjname)
      {
        iMeshWrapper* mw = Engine->FindMeshObject (meshobjname);
        if (mw)
        {
          ldr_context->AddToCollection(mw->QueryObject());
          loadResult->result = mw;
          loadResult->success = true;
          return;
        }
      }
      csRef<iMeshWrapper> mesh = LoadMeshObjectFromFactory (ldr_context, meshrefnode, ssource);
      if (mesh)
      {
        ldr_context->AddToCollection(mesh->QueryObject());
        loadResult->result = mesh;
        loadResult->success = true;
        return;
      }
      else
      {
        // Error is already reported.
        loadResult->result = 0;
        loadResult->success = false;
        return;
      }
    }
*/
    ReportError("crystalspace.maploader.parse",
      "File doesn't seem to be a world, library, meshfact, meshobj, meshref, portals or light file!");
    ret->MarkFinished(); 
  }

  THREADED_CALLABLE_IMPL6(csThreadedLoader, LoadMeshFactory, iLoaderContext* ldr_context,
    iDocumentNode* meshfactnode, const char* override_name, iMeshFactoryWrapper* parent,
    csReversibleTransform* transf, iStreamSource* ssource)
  {
    const char* meshfactname = override_name ? override_name : meshfactnode->GetAttributeValue("name");

    if(ldr_context->CheckDupes())
    {
      iMeshFactoryWrapper* mfw = Engine->FindMeshFactory(meshfactname);
      if(mfw)
      {
        ldr_context->AddToCollection(mfw->QueryObject());
        ret->MarkSuccessful();
        ret->SetResult(mfw);
      }
    }

    csRef<iMeshFactoryWrapper> mfw = Engine->CreateMeshFactory(meshfactname);
    if(LoadMeshObjectFactory(ldr_context, mfw, parent, meshfactnode, transf, ssource))
    {
      ldr_context->AddToCollection(mfw->QueryObject());
      ret->MarkSuccessful();
      ret->SetResult(csRef<iBase>(mfw));
    }

    ret->MarkFinished();
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

  bool csThreadedLoader::LoadLibraryFromNode (iLoaderContext* ldr_context,
    iDocumentNode* child, iStreamSource* ssource, iMissingLoaderData* missingdata,
    bool loadProxyTex)
  {
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
    const char* name = child->GetAttributeValue ("checkdupes");
    bool dupes = ldr_context->CheckDupes ();
    if (name)
    {
      if (!strcasecmp (name, "true") || !strcasecmp (name, "yes") ||
        !strcasecmp (name, "1") || !strcasecmp (name, "on"))
        dupes = true;
      else
        dupes = false;
    }

    const char* file = child->GetAttributeValue ("file");
    if (file)
    {
      const char* path = child->GetAttributeValue ("path");
      if (path)
      {
        vfs->PushDir ();
        vfs->ChDir (path);
      }

      if (Engine->GetSaveableFlag ())
      {
        csRef<iLibraryReference> libraryRef;
        libraryRef.AttachNew (new csLibraryReference (file, path, dupes));
        ldr_context->AddToCollection(libraryRef->QueryObject ());
      }

      bool rc;

      rc = LoadMapLibraryFile (file, ldr_context->GetCollection (),
        ldr_context->CurrentCollectionOnly (), dupes, ssource,
        missingdata, ldr_context->GetKeepFlags(), loadProxyTex);

      if (path)
      {
        vfs->PopDir ();
      }
      if (!rc)
        return false;
    }
    else
    {
      if (Engine->GetSaveableFlag ())
      {
        csRef<iLibraryReference> libraryRef;
        libraryRef.AttachNew (new csLibraryReference (
          child->GetContentsValue (), 0, dupes));
        ldr_context->AddToCollection (libraryRef->QueryObject ());
      }

      return LoadMapLibraryFile (child->GetContentsValue (), ldr_context->GetCollection (),
        ldr_context->CurrentCollectionOnly (), ldr_context->CheckDupes (),
        ssource, missingdata, ldr_context->GetKeepFlags(), loadProxyTex);
    }
    return true;
  }

  bool csThreadedLoader::LoadMapLibraryFile (const char* fname, iCollection* collection,
    bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource,
    iMissingLoaderData* missingdata, uint keepFlags, bool loadProxyTex, bool do_verbose)
  {
    csRef<iFile> buf = vfs->Open (fname, VFS_FILE_READ);

    if (!buf)
    {
      ReportError (
        "crystalspace.maploader.parse.library",
        "Could not open library file '%s' on VFS!", fname);
      return false;
    }

    if(Engine->GetSaveableFlag () && collection)
    {
      csRef<iSaverFile> saverFile;
      saverFile.AttachNew (new csSaverFile (fname, CS_SAVER_FILE_LIBRARY));
      collection->Add(saverFile->QueryObject());
    }

    csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
      new csLoaderContext (object_reg, Engine, collection, searchCollectionOnly, checkDupes,
      missingdata, keepFlags, do_verbose));

    csRef<iDocument> doc;
    bool er = LoadStructuredDoc (fname, buf, doc);
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
      return LoadLibrary (ldr_context, lib_node, ssource, missingdata, loadProxyTex);
    }
    else
    {
      ReportError ("crystalspace.maploader.parse.plugin",
        "File does not appear to be a structure map library (%s)!", fname);
    }
    return false;
  }

  bool csThreadedLoader::LoadLibrary(iLoaderContext* ldr_context, iDocumentNode* node,
    iStreamSource* ssource, iMissingLoaderData* missingdata, bool loadProxyTex)
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
/*
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
          if (!LoadLibraryFromNode (ldr_context, child, ssource, missingdata))
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
        // Append textures to engine.
        if (!ParseTextureList (ldr_context, child, proxyTextures))
          return false;
        break;
      case XMLTOKEN_MATERIALS:
        if (!ParseMaterialList (ldr_context, child, materialArray))
          return false;
        break;
      case XMLTOKEN_SHADERS:
        if (!ParseShaderList (ldr_context, child))
          return false;
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
          csRef<iMeshWrapper> mesh = LoadMeshObjectFromFactory (ldr_context,
            child, ssource);
          if (!mesh)
          {
            // Error is already reported.
            return false;
          }
          mesh->QueryObject ()->SetName (child->GetAttributeValue ("name"));
          Engine->AddMeshAndChildren (mesh);
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
          csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (
            child->GetAttributeValue ("name"));
          if (!LoadMeshObject (ldr_context, mesh, 0, child, ssource))
          {
            // Error is already reported.
            return false;
          }
          else
          {
            ldr_context->AddToCollection(mesh->QueryObject());
          }
        }
        break;
      case XMLTOKEN_MESHFACT:
        {
          csLoadResult loadResult;
          LoadMeshFactory(ldr_context, &loadResult, child, 0, 0, 0, ssource);
        }
        break;
      case XMLTOKEN_PLUGINS:
        if (!LoadPlugins (child))
          return false;
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
*/
    return true;
  }

  bool csThreadedLoader::LoadMeshObjectFactory (iLoaderContext* ldr_context,
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
            return false;
        }
        break;
      case XMLTOKEN_KEY:
        {
          if (!ParseKey (child, stemp->QueryObject()))
            return false;
        }
        break;
      case XMLTOKEN_ADDON:
        if (!LoadAddOn (ldr_context, child, stemp, false, ssource))
          return false;
        break;
      case XMLTOKEN_META:
        if (!LoadAddOn (ldr_context, child, stemp, true, ssource))
          return false;
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
            return false;
        }
        break;
      case XMLTOKEN_STATICSHAPE:
        if (!SyntaxService->ParseBool (child, staticshape, true))
          return false;
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
            return false;
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
                "Returned object does not implement iMeshObjectFactory!");
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
          csRef<iThreadReturn> ret = LoadMeshFactory(ldr_context, child, 0, stemp, &child_transf, ssource);
          tm->Wait(ret);
          if(!ret->WasSuccessful())
          {
            return false;
          }
          csRef<iMeshFactoryWrapper> mfw = scfQueryInterface<iMeshFactoryWrapper>(ret->GetResultRefPtr());
          stemp->GetChildren()->Add(mfw);
          mfw->SetTransform(child_transf);
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
              return false;
            transf->SetO2T (m);
          }
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
              return false;
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
          csRef<iImposter> imposter = scfQueryInterface<iImposter> (stemp);
          if (!imposter)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              node, "This factory doesn't implement impostering!");
            return false;
          }
          if (!ParseImposterSettings (imposter, child))
            return false;
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    stemp->GetMeshObjectFactory ()->GetFlags ().SetBool (CS_FACTORY_STATICSHAPE,
      staticshape);

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
      csRef<iBase> rc = plug->Parse (node, ssource, ldr_context, context);
      if (!rc) return false;

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
            csRef<iBase> rc = plug->Parse (child, ssource, ldr_context,
              context);

            if (!rc) return false;

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
            bool rc;
            csRef<iBase> ret;
            if (plug)
            {
              ret = LoadStructuredMap (ldr_context,
                plug, buf, 0, fname, ssource);
              rc = (ret != 0);
            }
            else
            {
              csRef<iDataBuffer> dbuf = vfs->ReadFile (fname);
              ret = binplug->Parse (dbuf,
                ssource, ldr_context, 0);
              rc = (ret != 0);
            }

            if (!rc)
              return false;

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
          ret = plug->Parse (paramsnode, ssource, ldr_context, context);
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

  csPtr<iImage> csThreadedLoader::LoadImage (iDataBuffer* buf, const char* fname,
    int Format)
  {
    if (!ImageLoader)
      return 0;

    if (Format & CS_IMGFMT_INVALID)
    {
      if (Engine)
        Format = Engine->GetTextureFormat ();
      else if (g3d)
        Format = g3d->GetTextureManager()->GetTextureFormat();
      else
        Format = CS_IMGFMT_TRUECOLOR;
    }

    if (!buf || !buf->GetSize ())
    {
      ReportWarning (
        "crystalspace.maploader.parse.image",
        "Could not open image file '%s' on VFS!", fname ? fname : "<unknown>");
      return 0;
    }

    // we don't use csRef because we need to return an Increfed object later
    csRef<iImage> image (ImageLoader->Load (buf, Format));
    if (!image)
    {
      ReportWarning (
        "crystalspace.maploader.parse.image",
        "Could not load image '%s'. Unknown format!",
        fname ? fname : "<unknown>");
      return 0;
    }

    if (fname)
    {
      csRef<iDataBuffer> xname = vfs->ExpandPath (fname);
      image->SetName (**xname);
    }

    return csPtr<iImage> (image);
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
