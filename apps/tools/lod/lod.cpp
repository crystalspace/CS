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

#include <crystalspace.h>
#include "lodgen.h"
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

void Lod::Usage()
{
  //        ---------+---------+---------+---------+---------+---------+---------+-------80+
  csPrintf("Sliding Window LOD generation tool\n\n");
  csPrintf("Usage:\n\n");
  csPrintf("cslodgen -i=<input_file> [-o=<output_file>] -mindist=d -maxdist=d [-force] [-v]\n");
  csPrintf("    [-em=<fast|precise>]\n\n");
  csPrintf("-mindist  Minimum LOD distance.\n");
  csPrintf("-maxdist  Maximum LOD distance.\n");
  csPrintf("          For medium-sized objects, try -mindist=5 -maxdist=50.\n");
  csPrintf("          Existing values in input file take precedence, unless -force is used.\n");
  csPrintf("-force    Force use of cmdline-specified mindist and maxdist.\n");
  csPrintf("          Mindist and maxdist *need* to be either in cmdline or input file.\n");
  csPrintf("-v        Verbose console output.\n");
  csPrintf("-em       Error metric (fast or precise).\n");
}

bool Lod::ParseParams(int argc, char* argv[])
{
  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser>(GetObjectRegistry());
  params.input_file = csString(cmdline->GetOption("i"));
  params.output_file = csString(cmdline->GetOption("o"));

  if (params.input_file == "")
  {
    ReportError("Input file not specified.");
    Usage();
    return false;
  }
  if (params.output_file == "")
  {
    csString outfile = params.input_file;
    if (outfile.Find (".zip", outfile.Length () - 4) == (size_t)-1)
    {
      outfile += "_lod";
      ReportInfo ("Output file not specified. Using %s\n", CS::Quote::Single (outfile.GetData ()));
    }
  }

  csString em = cmdline->GetOption("em");
  if (em == "fast")
    params.error_metric_type = ERROR_METRIC_FAST;
  else if (em == "precise")
    params.error_metric_type = ERROR_METRIC_PRECISE;

  if (cmdline->GetOption("v") != 0)
    params.verbose = true;

  csString mind = cmdline->GetOption("mindist");
  csString maxd = cmdline->GetOption("maxdist");
  if ((mind == "" && maxd != "") || (mind != "" && maxd == ""))
  {
    ReportError("mindist and maxdist have to be specified together.");
    Usage();
    return false;
  }
  
  if (mind == "")
  {
    params.dist_specified = false;
  }
  else
  {
    params.dist_specified = true;    
    csScanStr(mind, "%f", &params.min_dist);
    if (params.min_dist < 0.0f)
      params.min_dist = 0.0f;
    csScanStr(maxd, "%f", &params.max_dist);
    if (params.max_dist < 0.0f)
      params.max_dist = 0.0f;
  }
  
  params.override_dist = cmdline->GetBoolOption("force", false);

  return true;
}

bool Lod::OnInitialize (int argc, char* argv[])
{
  //PointTriangleDistanceUnitTests();

  if (!ParseParams(argc, argv))
    return false;
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

template<typename T>
void WriteTriangles(const LodGen& lodgen, csRef<iRenderBuffer> rbindices)
{
  T* data = new T[lodgen.GetTriangleCount() * 3];
  T* pdata = data;
  for (size_t i = 0; i < lodgen.GetTriangleCount(); i++)
  {
    //csPrintf("%d %d %d\n", lodgen.GetTriangle(i)[0], lodgen.GetTriangle(i)[1], lodgen.GetTriangle(i)[2]);
    *pdata++ = (T)lodgen.GetTriangle(i)[0];
    *pdata++ = (T)lodgen.GetTriangle(i)[1];
    *pdata++ = (T)lodgen.GetTriangle(i)[2];
  }
  rbindices->CopyInto(data, lodgen.GetTriangleCount() * 3);
  delete[] data;
}

void Lod::CreateLODs(const char* filename_in, const char* filename_out)
{
  bool bIsZip = false;
  csString filenameIn = filename_in;
  csString filenameOut = filename_out;

  // Check for a zip file.
  if (filenameIn.Find (".zip", filenameIn.Length () - 4) != (size_t)-1)
  {
    bIsZip = true;
    vfs->Mount ("/this/lodzip/", filenameIn);
    filenameIn = "/this/lodzip/world";
    filenameOut = "/this/lodzip/world";
  }

  if (filenameOut.IsEmpty ())
  {
    filenameOut = filenameIn + "_lod";
  }

  csRef<iDataBuffer> buf = vfs->ReadFile (filenameIn);
  if (!buf.IsValid ())
  {
    // Try /this/filename_in
    csString fname = "/this/";
    fname.Append (filenameIn);
    buf = vfs->ReadFile (fname);

    if (!buf.IsValid ())
    {
      ReportError ("Error opening file %s!", CS::Quote::Single (filenameIn.GetData()));
      return;
    }

    filenameIn = fname;
  }

  // Parse the xml document.
  csRef<iDocumentSystem> docsys = csQueryRegistry<iDocumentSystem> (object_reg);
  csRef<iDocument> doc = docsys->CreateDocument ();
  doc->Parse(buf);

  // Create a copy of the document in tiny xml format (we can't write to binary xml files).
  csRef<iDocumentSystem> xml(new csTinyDocumentSystem());
  csRef<iDocument> outDoc = xml->CreateDocument ();
  outDoc->CreateRoot ();
  CS::DocSystem::CloneNode (doc->GetRoot (), outDoc->GetRoot ());

  // Chdir to the file directory.
  CS::Utility::SmartChDir (vfs, filenameIn);

  // Process the mesh factories.
  csRef<iDocumentNode> root = outDoc->GetRoot();
  CreateLODsRecursive(root);

  // Create a backup.
  vfs->WriteFile (filenameIn + ".bak", **buf, buf->GetSize ());
  vfs->Sync ();

  // Save the output file.
  Save(outDoc, filenameOut);

  if (bIsZip)
  {
    // Unmount the zip file if there is one.
    vfs->Unmount ("/this/lodzip/", filename_in);
  }
}

void Lod::CreateLODsRecursive(csRef<iDocumentNode> node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> node = it->Next ();
    csString value(node->GetValue ());

    if (value == "meshfact")
    {
      CreateLODWithMeshFact(node);
    }
    else if (value == "plugins")
    {
      csString cwd = vfs->GetCwd ();
      csRef<iThreadReturn> itr = tloader->LoadNodeWait(cwd, node);
      if (!itr->WasSuccessful())
      {
        ReportError("Error parsing node %s in input file.", CS::Quote::Single (value.GetData ()));
      }
    }
    else
    {
      CreateLODsRecursive(node);
    }
  }
}

void Lod::CreateLODWithMeshFact(csRef<iDocumentNode> node)
{
  csString cwd = vfs->GetCwd ();
  csRef<iThreadReturn> itr = tloader->LoadNodeWait(cwd, node);
  csString name = node->GetAttributeValue ("name");
  if (!itr->WasSuccessful())
  {
    ReportError("Error parsing meshfact %s in input file.\n", CS::Quote::Single (name.GetData ()));
    return;
  }

  csPrintf("Processing factory: %s\n", name.GetData());

  imeshfactw = scfQueryInterface<iMeshFactoryWrapper>(itr->GetResultRefPtr());
    
  csRef<iMeshObjectFactory> fact = imeshfactw->GetMeshObjectFactory();
  CS_ASSERT (fact.IsValid ());
  
  csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(fact);

  // Only process genmesh factories.
  if (!fstate)
  {
    ReportError("Cannot handle meshes that are not genmeshes.\n");
    return;
  }
  
  float mindist_file, maxdist_file;
  fstate->GetProgLODDistances(mindist_file, maxdist_file);
  bool b_have_file_dist = !(mindist_file == 0.0 && maxdist_file == 0.0);
  bool b_have_cmdline_dist = params.dist_specified;
  if (!b_have_file_dist && !b_have_cmdline_dist)
  {
    csPrintf("Factory %s ignored. Distances not specified via the command line or in the factory.\n",
      CS::Quote::Single (name.GetData()));
    return;
  }

  if (!b_have_file_dist || params.override_dist)
    fstate->SetProgLODDistances(params.min_dist, params.max_dist);

  for (unsigned int submesh_index = 0; submesh_index < fstate->GetSubMeshCount(); submesh_index++)
  {
    printf ("Processing submesh number %i...\n", submesh_index);

    LodGen lodgen;
    lodgen.SetErrorMetricType(params.error_metric_type);
    lodgen.SetVerbose(params.verbose);
    
    csVertexListWalker<float, csVector3> fstate_vertices(fstate->GetRenderBuffer(CS_BUFFER_POSITION));
    for (unsigned int i = 0; i < fstate_vertices.GetSize(); i++)
    {
      lodgen.AddVertex(*fstate_vertices);
      ++fstate_vertices;
    }
    
    csRef<iGeneralMeshSubMesh> submesh = fstate->GetSubMesh(submesh_index);
    CS_ASSERT (submesh);

    csRef<iRenderBuffer> index_buffer = submesh->GetIndices();
    CS_ASSERT (index_buffer);

    CS::TriangleIndicesStream<size_t> fstate_triangles(index_buffer, CS_MESHTYPE_TRIANGLES);
    
    while(fstate_triangles.HasNext())
    {
      const CS::TriangleT<size_t> ttri(fstate_triangles.Next());
      csTriangle tri;
      for (int i = 0; i < 3; i++)
        tri[i] = (int)ttri[i];
      lodgen.AddTriangle(tri);
    }
    
    lodgen.GenerateLODs();
  
    iRenderBuffer* rbindices = submesh->GetIndices();
    CS_ASSERT (rbindices);

    csRef<iRenderBuffer> rbindices_new = csRenderBuffer::CreateIndexRenderBuffer(
      lodgen.GetTriangleCount() * 3, rbindices->GetBufferType(), rbindices->GetComponentType(), 0, fstate_vertices.GetSize()-1);
    
    csRenderBufferComponentType compType = rbindices->GetComponentType ();

    switch (compType & ~CS_BUFCOMP_NORMALIZED)
    {
    case CS_BUFCOMP_BYTE:
      WriteTriangles<int8>(lodgen, rbindices_new);
      break;
    case CS_BUFCOMP_UNSIGNED_BYTE:
      WriteTriangles<uint8>(lodgen, rbindices_new);
      break;
    case CS_BUFCOMP_SHORT:
      WriteTriangles<int16>(lodgen, rbindices_new);
      break;
    case CS_BUFCOMP_UNSIGNED_SHORT:
      WriteTriangles<uint16>(lodgen, rbindices_new);
      break;
    case CS_BUFCOMP_INT:
      WriteTriangles<int32>(lodgen, rbindices_new);
      break;
    case CS_BUFCOMP_UNSIGNED_INT:
      WriteTriangles<uint32>(lodgen, rbindices_new);
      break;
    case CS_BUFCOMP_FLOAT:
      WriteTriangles<float>(lodgen, rbindices_new);
      break;
    case CS_BUFCOMP_DOUBLE:
      WriteTriangles<double>(lodgen, rbindices_new);
      break;
    default:
      csPrintf("Bad index buffer type.\n");
      return;
    }

    submesh->SetIndices(rbindices_new);
    csRef<iGeneralFactorySubMesh> fsm = scfQueryInterface<iGeneralFactorySubMesh>(submesh);
    fsm->ClearSlidingWindows();
    for (size_t i = 0; i < lodgen.GetSlidingWindowCount(); i++)
    {
      fsm->AddSlidingWindow(lodgen.GetSlidingWindow(i).start_index*3, lodgen.GetSlidingWindow(i).end_index*3);
    }
  }

  printf ("Saving new mesh...\n");
  SaveToNode(node);
  printf ("DONE!\n");
}

void Lod::SaveToNode(csRef<iDocumentNode> factNode)
{
  factNode->SetValue("meshfact");
  factNode->RemoveNodes();
  
  //Add the mesh's name to the MeshObj tag
  const char* name = imeshfactw->QueryObject()->GetName();
  if (name && *name)
    factNode->SetAttribute("name", name);
  
  csRef<iMeshObjectFactory> meshfact = imeshfactw->GetMeshObjectFactory();
  csRef<iFactory> factory = scfQueryInterface<iFactory> (meshfact->GetMeshObjectType());  
  const char* pluginname = factory->QueryClassID();
  
  csRef<iDocumentNode> pluginNode = factNode->CreateNodeBefore(CS_NODE_ELEMENT);
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
  else ReportError("Failed to find a valid mesh saver.\n");
}

void Lod::Save(csRef<iDocument> doc, const char* filename)
{
  scfString str;
  doc->Write(&str);

  if (!vfs->WriteFile(filename, str.GetData(), str.Length()))
  {
    // Try writing to /this/filename
    csString fname = "/this/";
    fname.Append (filename);

    if (!vfs->WriteFile(fname, str.GetData(), str.Length()))
    {
      ReportError ("Failed to write to output file %s!", CS::Quote::Single (filename));
    }
  }

  vfs->Sync();
}

bool Lod::SetupModules ()
{
  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return ReportError("No VFS.\n");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError ("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError ("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError ("Failed to locate Loader!");

  tloader = csQueryRegistry<iThreadedLoader> (GetObjectRegistry());
  if (!tloader) return ReportError("Failed to locate threaded Loader!");
  tloader->SetFlags (CS_LOADER_CREATE_DUMMY_MATS);
  
  collection = engine->CreateCollection ("lod_region");
   
  CreateLODs(params.input_file, params.output_file);

  Quit();
  return true;
}

int main (int argc, char* argv[])
{
  return csApplicationRunner<Lod>::Run (argc, argv);
}
