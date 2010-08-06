/*
    Copyright (C) 2010 by Jorrit Tyberghein, Eduardo Poyart

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

#include <iostream>
#include <vector>
using namespace std;

#include <crystalspace.h>
#include "LodGen.h"
#include "lod.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

Lod::Lod ()
{
  SetApplicationName ("CrystalSpace.Lod");
}

Lod::~Lod ()
{
}

bool Lod::OnInitialize (int argc, char* argv[])
{
  //PointTriangleDistanceUnitTests();
  
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN("crystalspace.documentsystem.multiplexer", iDocumentSystem),
    CS_REQUEST_PLUGIN_TAG("crystalspace.documentsystem.tinyxml", iDocumentSystem, "iDocumentSystem.1"),
    CS_REQUEST_VFS,
    CS_REQUEST_NULL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_VFS,
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  
  // Report success
  return true;
}

void Lod::OnExit ()
{
}

bool Lod::Application ()
{
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  SetupModules ();

  return true;
}

void Lod::CreateLODs(const char* filename_in, const char* filename_out)
{
  loading = tloader->LoadFileWait("", filename_in);
  
  if (!loading->WasSuccessful())
  {
    csPrintf("Loading not successful - file: %s\n", filename_in);
    loading.Invalidate();
    return;
  }
    
  if (!loading->GetResultRefPtr().IsValid())
  {
    // Library file. Find the first factory in our region.
    iMeshFactoryList* factories = engine->GetMeshFactories ();
    if (factories->GetCount() == 0)
    {
      csPrintf("No factories in file.\n");
      return;
    }
    imeshfactw = factories->Get (0);
  }
  else
  {
    imeshfactw = scfQueryInterface<iMeshFactoryWrapper> (loading->GetResultRefPtr());
    if(!imeshfactw)
    {
      csRef<iMeshWrapper> spritewrapper = scfQueryInterface<iMeshWrapper> (loading->GetResultRefPtr());
      if (spritewrapper)
        imeshfactw = spritewrapper->GetFactory();
    }
  }
  
  if (!imeshfactw)
  {
    csPrintf("Could not find loaded mesh.\n");
    return;
  }
    
  csRef<iMeshObjectFactory> fact = imeshfactw->GetMeshObjectFactory();
  assert(fact);
  
  csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(fact);
  assert(fstate);
  
  for (unsigned int submesh_index = 0; submesh_index < fstate->GetSubMeshCount(); submesh_index++)
  {
    LodGen lodgen;
    
    csVertexListWalker<float, csVector3> fstate_vertices(fstate->GetRenderBuffer(CS_BUFFER_POSITION));
    for (unsigned int i = 0; i < fstate_vertices.GetSize(); i++)
    {
      lodgen.AddVertex(*fstate_vertices);
      ++fstate_vertices;
    }
    
    csRef<iGeneralMeshSubMesh> submesh = fstate->GetSubMesh(submesh_index);
    assert(submesh);
    csRef<iRenderBuffer> index_buffer = submesh->GetIndices();
    assert(index_buffer);
    CS::TriangleIndicesStream<size_t> fstate_triangles(index_buffer, CS_MESHTYPE_TRIANGLES);
    
    while(fstate_triangles.HasNext())
    {
      const CS::TriangleT<size_t> ttri(fstate_triangles.Next());
      csTriangle tri;
      for (int i = 0; i < 3; i++)
        tri[i] = ttri[i];
      lodgen.AddTriangle(tri);
    }
    
    lodgen.GenerateLODs();
  
    assert(lodgen.GetSlidingWindowCount() >= 2);
    
    iRenderBuffer* rbindices = submesh->GetIndices();
    assert(rbindices);
    csRef<iRenderBuffer> rbindices_new = csRenderBuffer::CreateIndexRenderBuffer(
      lodgen.GetTriangleCount() * 3, rbindices->GetBufferType(), rbindices->GetComponentType(), 0, fstate_vertices.GetSize()-1);
    
    // TODO: deal with buffer not being unsigned int
    // see renderbuffer.cpp:659
    unsigned int* data = new unsigned int[lodgen.GetTriangleCount() * 3];
    unsigned int* pdata = data;
    for (int i = 0; i < lodgen.GetTriangleCount(); i++)
    {
      //csPrintf("%d %d %d\n", lodgen.GetTriangle(i)[0], lodgen.GetTriangle(i)[1], lodgen.GetTriangle(i)[2]);
      *pdata++ = lodgen.GetTriangle(i)[0];
      *pdata++ = lodgen.GetTriangle(i)[1];
      *pdata++ = lodgen.GetTriangle(i)[2];
    }
    rbindices_new->CopyInto(data, lodgen.GetTriangleCount() * 3);
    submesh->SetIndices(rbindices_new);
    delete[] data;
    
    submesh->ClearSlidingWindows();
    for (int i = 0; i < lodgen.GetSlidingWindowCount(); i++)
    {
      submesh->AddSlidingWindow(lodgen.GetSlidingWindow(i).start_index*3, lodgen.GetSlidingWindow(i).end_index*3);
    }
  }
  
  //fstate->Invalidate();
  
  Save(filename_out);

  loading.Invalidate();
}

void Lod::Save(const char* filename)
{
  csRef<iVFS> vfs;
  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs)
  {
    csPrintf("Error: No iVFS!\n");
    return;
  }
  
  csRef<iDocumentSystem> xml(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();
  
  iMeshObjectFactory* meshfact = imeshfactw->GetMeshObjectFactory();
  
  //Create the Tag for the MeshObj
  csRef<iDocumentNode> factNode = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  factNode->SetValue("meshfact");
  
  //Add the mesh's name to the MeshObj tag
  const char* name = imeshfactw->QueryObject()->GetName();
  if (name && *name)
    factNode->SetAttribute("name", name);
  
  csRef<iFactory> factory = scfQueryInterface<iFactory> (meshfact->GetMeshObjectType());
  
  const char* pluginname = factory->QueryClassID();
  
  if (!(pluginname && *pluginname)) return;
  
  csRef<iDocumentNode> pluginNode = factNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  pluginNode->SetValue("plugin");
  
  //Add the plugin tag
  char loadername[128] = "";
  csReplaceAll(loadername, pluginname, ".object.", ".loader.factory.", sizeof(loadername));
  
  pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(loadername);
  csRef<iPluginManager> plugin_mgr = csQueryRegistry<iPluginManager> (GetObjectRegistry ());
  
  char savername[128] = "";
  
  csReplaceAll(savername, pluginname, ".object.", ".saver.factory.", sizeof(savername));
  
  csRef<iSaverPlugin> saver = csLoadPluginCheck<iSaverPlugin> (plugin_mgr, savername);
  if (saver) 
    saver->WriteDown(meshfact, factNode, 0/*ssource*/);
  
  scfString str;
  doc->Write(&str);
  vfs->WriteFile(filename, str.GetData(), str.Length());
  vfs->Sync();
}

bool Lod::SetupModules ()
{
  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError ("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError ("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError ("Failed to locate Loader!");

  tloader = csQueryRegistry<iThreadedLoader> (GetObjectRegistry());
  if (!tloader) return ReportError("Failed to locate threaded Loader!");
  
  collection = engine->CreateCollection ("lod_region");
   
  CreateLODs("/lev/lodtest/lodbarrel", "/lev/lodtest/lodbarrel_lod");
  //CreateLODs("/lev/lodtest/genMesh.002", "/lev/lodtest/genMesh.002_lod");
  //CreateLODs("/lev/lodtest/lodbox", "/lev/lodtest/lodbox_lod");
  //CreateLODs("/lev/lodtest/genbment2_tables", "/lev/lodtest/genbment2_tables_lod");
  //CreateLODs("/lev/lodtest/simple", "/lev/lodtest/simple_lod");
  //CreateLODs("/lev/lodtest/kwartz.lib", "/lev/lodtest/kwartz_lod.lib");
  //CreateLODs("/lev/lodtest/submeshtest", "/lev/lodtest/submeshtest_lod");

  Quit();
  return true;
}

int main (int argc, char* argv[])
{
  return csApplicationRunner<Lod>::Run (argc, argv);
}
