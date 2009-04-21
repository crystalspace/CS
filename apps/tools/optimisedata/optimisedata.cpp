/*
  Copyright (C) 2008 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#include <cssysdef.h>

#include "cstool/initapp.h"
#include "csutil/documenthelper.h"
#include "csutil/stringarray.h"
#include "csutil/xmltiny.h"

#include "iutil/cmdline.h"
#include "iutil/document.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#include "optimisedata.h"

CS_IMPLEMENT_APPLICATION

OptimiseData::OptimiseData(iObjectRegistry* objReg, iVFS* vfs) : objReg(objReg),
  vfs(vfs)
{
  addonLib = false;
  docSys.AttachNew(new csTinyDocumentSystem());

  csRef<iCommandLineParser> clp = csQueryRegistry<iCommandLineParser>(objReg);
  compact = clp->GetBoolOption("compact", true);
}

void OptimiseData::Run(csString in, csString out)
{
  printf("Collecting Data!\n");
  CollectData(in);

  printf("Sorting Data!\n");
  SortData();

  printf("Writing Data!\n");
  WriteData(out);
}

void OptimiseData::CollectData(csString in)
{
  csRef<iStringArray> files = vfs->FindFiles(in);
  if(files->IsEmpty())
  {
    files->Push(in);
  }

  vfs->PushDir(in);

  for(size_t i=0; i<files->GetSize(); i++)
  {
    csRef<iFile> file = vfs->Open(files->Get(i), VFS_FILE_READ);
    if(file.IsValid())
    {
      csRef<iDocument> doc = docSys->CreateDocument();
      doc->Parse(file);
      csRef<iDocumentNode> root = doc->GetRoot();
      if(root.IsValid())
      {
        csRef<iDocumentNodeIterator> meshfacts = root->GetNodes("meshfact");
        while(meshfacts->HasNext())
        {
          meshFacts.Push(meshfacts->Next());
        }

        csRef<iDocumentNode> top = root->GetNode("library");
        if(!top.IsValid())
        {
          top = root->GetNode("world");
          if(top.IsValid())
          {
            csString mapName = files->Get(i);
            mapName.Truncate(mapName.FindLast("/"));
            mapInPaths.Push(mapName);
            if(mapName.FindLast("/") != (size_t)-1)
            {
                mapName = mapName.Slice(mapName.FindLast("/"));
            }
            if(mapName.FindLast("\\") != (size_t)-1)
            {
                mapName = mapName.Slice(mapName.FindLast("\\")+1);
            }
            mapNames.Push(mapName);
            maps.Push(top);
          }
          else
          {
            continue;
          }
        }

        csRef<iDocumentNode> node;
        csRef<iDocumentNodeIterator> texsNode = top->GetNodes("textures");
        while(texsNode->HasNext())
        {
          node = texsNode->Next();
          addonLib = false;
          csRef<iDocumentNodeIterator> textureNodes = node->GetNodes("texture");
          while(textureNodes->HasNext())
          {
            textures.PushSmart(textureNodes->Next());
          }            
        }

        csRef<iDocumentNodeIterator> matsNode = top->GetNodes("materials");
        while(matsNode->HasNext())
        {
          node = matsNode->Next();
          addonLib = false;
          csRef<iDocumentNodeIterator> materialNodes = node->GetNodes("material");
          while(materialNodes->HasNext())
          {
            materials.PushSmart(materialNodes->Next());
          }            
        }

        meshfacts = top->GetNodes("meshfact");
        while(meshfacts->HasNext())
        {
          addonLib = false;
          meshFacts.PushSmart(meshfacts->Next());
        }

        csRef<iDocumentNodeIterator> libraries = top->GetNodes("library");
        while(libraries->HasNext())
        {
          csRef<iDocumentNode> libnode = libraries->Next();
          csString lib = libnode->GetContentsValue();
          if(lib.Compare(""))
          {
            lib = csString(libnode->GetAttributeValue("path")) + "/" + libnode->GetAttributeValue("file");
          }
          addonLib = true;
          CollectData(lib);
          if(addonLib)
          {
            addonNames.Push(lib.Slice(lib.FindLast('/')+1));
          }
          addonLib = false;
        }

        csRef<iDocumentNodeIterator> addonNodes = top->GetNodes("addon");
        while(addonLib && addonNodes->HasNext())
        {
          addons.Push(addonNodes->Next());
        }
      }
    }
  }
  vfs->PopDir();
}

void OptimiseData::ParseMeshFact(csRef<iDocumentNode>& meshFact, csRef<iDocumentNode>& tempDocRoot,
                                 csRefArray<iDocumentNode>& tempMats)
{
  csRef<iDocumentNode> params = meshFact->GetNode("params");
  if(params.IsValid())
  {
    bool first = true;
    csRef<iDocumentNodeIterator> submeshes = params->GetNodes("submesh");
    if(!submeshes->HasNext())
    {
      submeshes = params->GetNodes("curve");
    }

    while((first && params->GetNode("material")) || submeshes->HasNext())
    {
      csString materialName;
      if(first && params->GetNode("material"))
      {
        materialName = params->GetNode("material")->GetContentsValue();
        first = false;
      }
      else if(submeshes->HasNext())
      {
        csRef<iDocumentNode> submesh = submeshes->Next();
        if(submesh->GetNode("material"))
        {
          materialName = submesh->GetNode("material")->GetContentsValue();
        }
        else
        {
          continue;
        }
      }

      bool hasMaterialDecl = false;

      csRef<iDocumentNode> material;
      for(size_t j=0; j<materials.GetSize(); j++)
      {
        material = materials[j];
        if(materialName.Compare(material->GetAttributeValue("name")))
        {
          hasMaterialDecl = true;
          break;
        }
      }

      if(!hasMaterialDecl)
      {
        // Print error and mark data as incorrect.
        csFPrintf(stderr, "ERROR: Meshfact %s uses material %s but there is no material declaration!\n",
          meshFact->GetAttributeValue("name"), materialName.GetData());
      }

      tempMats.PushSmart(material);
    }
  }

  csRef<iDocumentNodeIterator> undermeshes = meshFact->GetNodes("meshfact");
  while(undermeshes->HasNext())
  {
    csRef<iDocumentNode> next = undermeshes->Next();
    ParseMeshFact(next, tempDocRoot, tempMats);
  }
}

void OptimiseData::SortData()
{
  // Used to store temp data.
  csRef<iDocument> tempDoc = docSys->CreateDocument();
  csRef<iDocumentNode> tempDocRoot = tempDoc->CreateRoot();

  // Start by working out if we have all the material and texture declarations needed,
  // and erroring for each if not.
  // Then add meshfact to the output list as a library.
  for(size_t i=0; i<meshFacts.GetSize(); i++)
  {
    csRef<iDocumentNode> meshFact = meshFacts[i];
    csRefArray<iDocumentNode> tempMats;
    ParseMeshFact(meshFact, tempDocRoot, tempMats);

    csRefArray<iDocumentNode> tempTexs;
    for(size_t j=0; j<tempMats.GetSize(); j++)
    {
      bool hasTextureDecl = false;
      if(tempMats[j]->GetNode("texture"))
      {
        csString textureName = tempMats[j]->GetNode("texture")->GetContentsValue();
        csRef<iDocumentNode> texture;
        for(size_t k=0; k<textures.GetSize(); k++)
        {
          texture = textures[k];
          if(textureName.Compare(texture->GetAttributeValue("name")))
          {
            hasTextureDecl = true;
            break;
          }
        }

        if(!hasTextureDecl)
        {
          // Print error and mark data as incorrect.
          csFPrintf(stderr, "ERROR: Material %s uses texture %s but there is no texture declaration!\n",
            tempMats[j]->GetAttributeValue("name"), textureName.GetData());
        }

        tempTexs.Push(texture);
      }
    }

    // Create new output doc.
    csRef<iDocument> output = docSys->CreateDocument();
    csRef<iDocumentNode> outputRoot = output->CreateRoot();
    csRef<iDocumentNode> lib = outputRoot->CreateNodeBefore(CS_NODE_ELEMENT);
    lib->SetValue("library");
    csRef<iDocumentNode> newTextureNodes = lib->CreateNodeBefore(CS_NODE_ELEMENT);
    newTextureNodes->SetValue("textures");

    for(size_t j=0; j<tempTexs.GetSize(); j++)
    {
      csRef<iDocumentNode> newTexture = newTextureNodes->CreateNodeBefore(CS_NODE_ELEMENT);
      CS::DocSystem::CloneNode(tempTexs[j], newTexture);
    }

    for(size_t j=0; j<tempMats.GetSize(); j++)
    {
      csRef<iDocumentNodeIterator> shadervars = tempMats[j]->GetNodes("shadervar");
      while(shadervars->HasNext())
      {
        csRef<iDocumentNode> shadervar = shadervars->Next();
        if(!strcmp(shadervar->GetAttributeValue("type"), "texture"))
        {
          for(size_t k=0; k<textures.GetSize(); k++)
          {
            csRef<iDocumentNode> texture = textures[k];
            csString textureName = texture->GetAttributeValue("name");
            if(textureName.Compare(shadervar->GetContentsValue()))
            {
              csRef<iDocumentNode> newTexture = newTextureNodes->CreateNodeBefore(CS_NODE_ELEMENT);
              CS::DocSystem::CloneNode(texture, newTexture);
              break;
            }
          }        
        }
      }
    }

    csRef<iDocumentNode> materials = lib->CreateNodeBefore(CS_NODE_ELEMENT);
    materials->SetValue("materials");

    for(size_t j=0; j<tempMats.GetSize(); j++)
    {
      csRef<iDocumentNode> newMaterial = materials->CreateNodeBefore(CS_NODE_ELEMENT);
      CS::DocSystem::CloneNode(tempMats[j], newMaterial);
    }

    csRef<iDocumentNode> newMeshFact = lib->CreateNodeBefore(CS_NODE_ELEMENT);
    CS::DocSystem::CloneNode(meshFact, newMeshFact);
    meshFactsOut.Push(output);
  }

  // Now process maps.
  for(size_t i=0; i<maps.GetSize(); i++)
  {
    csRef<iDocumentNode> world = maps[i];

    // Get lightmaps.
    csRef<iDocumentNodeIterator> lms = world->GetNode("textures")->GetNodes("texture");
    while(lms->HasNext())
    {
      csRef<iDocumentNode> lm = lms->Next();
      lm = lm->GetNode("class");
      if(lm.IsValid() && csString("lightmap").Compare(lm->GetContentsValue()))
      {
        lightmaps.PushSmart(lm->GetParent());
      }
    }

    // Remove old stuff.
    world->RemoveNodes(world->GetNodes("textures"));
    world->RemoveNodes(world->GetNodes("materials"));
    world->RemoveNodes(world->GetNodes("meshfact"));
    world->RemoveNodes(world->GetNodes("library"));

    // Check dependencies.
    csRef<iDocumentNodeIterator> sectors = world->GetNodes("sector");
    csArray<csString> libsNeeded;
    csArray<csString> materialsNeeded;

    if(compact)
    {
      for(size_t i=0; i<meshFactsOut.GetSize(); i++)
      {
        csRef<iDocumentNode> node = meshFactsOut[i]->GetRoot()->GetNode("library");
        node = node->GetNode("materials");
        if(node.IsValid())
        {
          csRef<iDocumentNodeIterator> mats = node->GetNodes("material");
          while(mats->HasNext())
          {
            materialsNeeded.PushSmart(mats->Next()->GetAttributeValue("name"));
          }
        }
      }
    }

    while(sectors->HasNext())
    {
      csRef<iDocumentNode> sector = sectors->Next();
      csRef<iDocumentNodeIterator> meshobjs = sector->GetNodes("meshobj");
      while(meshobjs->HasNext())
      {
        csRef<iDocumentNode> meshobj = meshobjs->Next();
        ParseMeshObj(libsNeeded, materialsNeeded, mapInPaths[i], meshobj);
      }

      csRef<iDocumentNodeIterator> meshRefs = sector->GetNodes("meshref");
      while(meshRefs->HasNext())
      {
        csRef<iDocumentNode> fact = meshRefs->Next()->GetNode("factory");
        if(fact.IsValid())
        {
          libsNeeded.PushSmart(fact->GetContentsValue());
        }
      }

      csRef<iDocumentNodeIterator> lights = sector->GetNodes("light");
      while(lights->HasNext())
      {
        csRef<iDocumentNode> halo = lights->Next()->GetNode("halo");
        if(halo.IsValid())
        {
          csRef<iDocumentNodeIterator> halomats = halo->GetNodes();
          while(halomats->HasNext())
          {
            csRef<iDocumentNode> halomat = halomats->Next();
            for(size_t j=0; j<materials.GetSize(); j++)
            {
              if(csString(materials[j]->GetAttributeValue("name")).Compare(halomat->GetContentsValue()))
              {
                materialsNeeded.PushSmart(halomat->GetContentsValue());
              }
            }
          }
        }
      }
    }

    // Add meshfact libs after the plugins and shaders.
    csRef<iDocumentNodeIterator> nodes = world->GetNodes();
    csRef<iDocumentNode> node;
    csRef<iDocumentNode> after;
    bool pushed = false;
    while(nodes->HasNext())
    {
      if(!pushed)
      {
        node = nodes->Next();
      }
      else
      {
        node = after;
        pushed = false;
      }

      if(!strcmp(node->GetValue(), "plugins") || !strcmp(node->GetValue(), "shaders"))
      {
        after = nodes->Next();
        pushed = true;
      }
    }

    if(!after.IsValid())
    {
      nodes = world->GetNodes();
      after = nodes->HasNext() ? nodes->Next() : 0;
    }

    // Write textures and materials first.
    csRef<iDocumentNode> texs = world->CreateNodeBefore(CS_NODE_ELEMENT, after);
    texs->SetValue("textures");
    csRef<iDocumentNode> mats = world->CreateNodeBefore(CS_NODE_ELEMENT, after);
    mats->SetValue("materials");
    for(size_t j=0; j<materialsNeeded.GetSize(); j++)
    {
      csRefArray<iDocumentNode> matsNow;
      csRefArray<iDocumentNode> texsNow;

      // Textures first.
      for(size_t k=0; k<materials.GetSize(); k++)
      {
        if(materialsNeeded[j].Compare(materials[k]->GetAttributeValue("name")))
        {
          matsNow.Push(materials[k]);
          csRef<iDocumentNode> tex = materials[k]->GetNode("texture");
          if(tex)
          {
            for(size_t l=0; l<textures.GetSize(); l++)
            {
              if(csString(tex->GetContentsValue()).Compare(textures[l]->GetAttributeValue("name")))
              {
                texsNow.PushSmart(textures[l]);
              }
            }
          }

          csRef<iDocumentNodeIterator> svars = materials[k]->GetNodes("shadervar");
          while(svars->HasNext())
          {
            csRef<iDocumentNode> svar = svars->Next();
            if(csString("texture").Compare(svar->GetAttributeValue("type")))
            {
              for(size_t l=0; l<textures.GetSize(); l++)
              {
                if(csString(svar->GetContentsValue()).Compare(textures[l]->GetAttributeValue("name")))
                {
                  texsNow.PushSmart(textures[l]);
                }
              }
            }
          }
        }
      }

      for(size_t k=0; k<texsNow.GetSize(); k++)
      {
        texs = texs->CreateNodeBefore(CS_NODE_ELEMENT);
        CS::DocSystem::CloneNode(texsNow[k], texs);
        texs = texs->GetParent();
      }

      // Then materials.
      for(size_t k=0; k<matsNow.GetSize(); k++)
      {
        mats = mats->CreateNodeBefore(CS_NODE_ELEMENT);
        CS::DocSystem::CloneNode(matsNow[k], mats);
        mats = mats->GetParent();
      }
    }

    if(compact)
    {
      // Write lightmaps.
      csRef<iDocumentNode> texturesNode = world->GetNode("textures");
      if(!texturesNode.IsValid())
      {
        texturesNode = world->CreateNodeBefore(CS_NODE_ELEMENT, after);
        texturesNode->SetValue("textures");
      }

      for(size_t i=0; i<lightmaps.GetSize(); i++)
      {
        texturesNode = texturesNode->CreateNodeBefore(CS_NODE_ELEMENT);
        CS::DocSystem::CloneNode(lightmaps[i], texturesNode);
        texturesNode = texturesNode->GetParent();
      }
    }

    // Put the 'addon' libraries next.
    for(size_t j=0; j<addons.GetSize(); j++)
    {
      csRef<iDocumentNode> addon = world->CreateNodeBefore(CS_NODE_ELEMENT, after);
      CS::DocSystem::CloneNode(addons[j], addon);
      if(!compact && addons[j]->GetNode("params"))
      {
        addon->RemoveNode(addon->GetNode("params"));
        addon = addon->CreateNodeBefore(CS_NODE_ELEMENT);
        addon->SetValue("paramsfile");
        addon = addon->CreateNodeBefore(CS_NODE_TEXT);
        addon->SetValue("addons/" + addonNames[j] + ".addon");
      }
      else
      {
        addons.DeleteIndex(j);
        addonNames.DeleteIndex(j);
        j--;
      }
    }

    if(compact)
    {
      // Write meshfacts.
      for(size_t i=0; i<meshFactsOut.GetSize(); i++)
      {
        csRef<iDocumentNode> meshfact = world->CreateNodeBefore(CS_NODE_ELEMENT, after);
        CS::DocSystem::CloneNode(meshFactsOut[i]->GetRoot()->GetNode("library")->GetNode("meshfact"), meshfact);
      }
    }
    else
    {
      // Write lightmaps lib.
      csRef<iDocumentNode> lmaps = world->CreateNodeBefore(CS_NODE_ELEMENT, after);
      lmaps->SetValue("library");
      lmaps = lmaps->CreateNodeBefore(CS_NODE_TEXT);
      lmaps->SetValue("lightmaps.cslib");

      // Write the other libs.
      for(size_t j=0; j<libsNeeded.GetSize(); j++)
      {
        csRef<iDocumentNode> lib = world->CreateNodeBefore(CS_NODE_ELEMENT, after);
        lib->SetValue("library");
        lib = lib->CreateNodeBefore(CS_NODE_TEXT);
        lib->SetValue("factories/" + libsNeeded[j] + ".meshfact");
      }
    }

    csRef<iDocument> map = docSys->CreateDocument();
    csRef<iDocumentNode> mapRoot = map->CreateRoot();
    mapRoot = mapRoot->CreateNodeBefore(CS_NODE_ELEMENT);
    CS::DocSystem::CloneNode(world, mapRoot);
    mapsOut.Push(map);
  }
}

void OptimiseData::WriteData(csString out)
{
  if(!compact)
  {
    // Addons.
    for(size_t i=0; i<addons.GetSize(); i++)
    {
      csRef<iDocument> addon = docSys->CreateDocument();
      csRef<iDocumentNode> addonRoot = addon->CreateRoot();
      addonRoot = addonRoot->CreateNodeBefore(CS_NODE_ELEMENT);
      CS::DocSystem::CloneNode(addons[i]->GetNode("params"), addonRoot);
      csString realOut = out + "/addons/" + addonNames[i];
      addon->Write(vfs, realOut.Append(".addon"));
    }

    // Lightmaps.
    csRef<iDocument> lightmapdoc = docSys->CreateDocument();
    csRef<iDocumentNode> lightmapsroot = lightmapdoc->CreateRoot();
    lightmapsroot = lightmapsroot->CreateNodeBefore(CS_NODE_ELEMENT);
    lightmapsroot->SetValue("library");
    lightmapsroot = lightmapsroot->CreateNodeBefore(CS_NODE_ELEMENT);
    lightmapsroot->SetValue("textures");
    for(size_t i=0; i<lightmaps.GetSize(); i++)
    {
      lightmapsroot = lightmapsroot->CreateNodeBefore(CS_NODE_ELEMENT);
      CS::DocSystem::CloneNode(lightmaps[i], lightmapsroot);
      lightmapsroot = lightmapsroot->GetParent();
    }
    lightmapdoc->Write(vfs, out + "/lightmaps.cslib");

    // Meshfacts.
    for(size_t i=0; i<meshFactsOut.GetSize(); i++)
    {
      csRef<iDocument> meshFact = meshFactsOut[i];
      csRef<iDocumentNode> node = meshFact->GetRoot()->GetNode("library")->GetNode("meshfact");
      csStringArray strarr;
      strarr.SplitString(node->GetAttributeValue("name"), "#");
      csString realOut = out + "/factories";
      for(size_t i=0; i<strarr.GetSize(); i++)
      {
        realOut.AppendFmt("/%s", strarr[i]);
      }
      meshFact->Write(vfs, realOut.Append(".meshfact"));
    }
  }

  for(size_t i=0; i<mapsOut.GetSize(); i++)
  {
    csString realOut = out + "/" + mapNames[i] + ".csworld";
    mapsOut[i]->Write(vfs, realOut);
  }
}

void OptimiseData::ParseMeshObj(csArray<csString>& libsNeeded, csArray<csString>& materialsNeeded,
                                csString& mapInPath, csRef<iDocumentNode> meshobj)
{
  csRef<iDocumentNodeIterator> meshobjs = meshobj->GetNodes("meshobj");
  while(meshobjs->HasNext())
  {
    ParseMeshObj(libsNeeded, materialsNeeded, mapInPath, meshobjs->Next());
  }

  if(meshobj->GetNode("params"))
  {
    meshobj = meshobj->GetNode("params");
    if(meshobj->GetNode("factory"))
    {
      bool found = false;
      for(size_t i=0; i<meshFactsOut.GetSize(); i++)
      {
        if(csString(meshFactsOut[i]->GetRoot()->GetNode("library")->GetNode("meshfact")->GetAttributeValue("name")).Compare(meshobj->GetNode("factory")->GetContentsValue()))
        {
          libsNeeded.PushSmart(meshobj->GetNode("factory")->GetContentsValue());
          found = true;
        }
      }

      if(!found)
      {
        // Print error and mark data as incorrect.
        csFPrintf(stderr, "ERROR: Mesh object %s uses mesh factory %s but there is no factory data!\n",
          meshobj->GetAttributeValue("name"), meshobj->GetNode("factory")->GetContentsValue());
      }
    }
    if(meshobj->GetNode("material"))
    {
      bool found = false;
      for(size_t i=0; i<materials.GetSize(); i++)
      {
        if(csString(materials[i]->GetAttributeValue("name")).Compare(meshobj->GetNode("material")->GetContentsValue()))
        {
          materialsNeeded.PushSmart(meshobj->GetNode("material")->GetContentsValue());
          found = true;
        }
      }

      if(!found)
      {
        // Print error and mark data as incorrect.
        csFPrintf(stderr, "ERROR: Mesh object %s uses material %s but there is no such material declaration!\n",
          meshobj->GetAttributeValue("name"), meshobj->GetNode("material")->GetContentsValue());
      }
    }
  }
  else if(meshobj->GetNode("paramsfile"))
  {
    csString paramsPath = mapInPath + "/" + meshobj->GetNode("paramsfile")->GetContentsValue();
    csRef<iFile> file = vfs->Open(paramsPath, VFS_FILE_READ);
    csRef<iDocument> paramsDoc = docSys->CreateDocument();
    paramsDoc->Parse(file);
    meshobj = paramsDoc->GetRoot()->GetNode("params");
    if(meshobj->GetNode("factory"))
    {
      bool found = false;
      for(size_t i=0; i<meshFactsOut.GetSize(); i++)
      {
        if(csString(meshFactsOut[i]->GetRoot()->GetNode("library")->GetNode("meshfact")->GetAttributeValue("name")).Compare(meshobj->GetNode("factory")->GetContentsValue()))
        {
          libsNeeded.PushSmart(meshobj->GetNode("factory")->GetContentsValue());
          found = true;
        }
      }

      if(!found)
      {
        // Print error and mark data as incorrect.
        csFPrintf(stderr, "ERROR: Mesh object %s uses mesh factory %s but there is no factory data!\n",
          meshobj->GetParent()->GetAttributeValue("name"), meshobj->GetNode("factory")->GetContentsValue());
      }
    }
    if(meshobj->GetNode("material"))
    {
      bool found = false;
      for(size_t i=0; i<materials.GetSize(); i++)
      {
        if(csString(materials[i]->GetAttributeValue("name")).Compare(meshobj->GetNode("material")->GetContentsValue()))
        {
          materialsNeeded.PushSmart(meshobj->GetNode("material")->GetContentsValue());
          found = true;
        }
      }

      if(!found)
      {
        // Print error and mark data as incorrect.
        csFPrintf(stderr, "ERROR: Mesh object %s uses material %s but there is no such material declaration!\n",
          meshobj->GetParent()->GetAttributeValue("name"), meshobj->GetNode("material")->GetContentsValue());
      }
    }
  }
}

int main(int argc, char** argv)
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if(!object_reg)
  {
    printf("Object Reg failed to Init!\n");
    return 1;
  }

  iVFS* vfs = csInitializer::SetupVFS(object_reg);
  if(!vfs)
  {
    printf("Object Reg failed to Init!\n");
    return 1;
  }

  printf("Crystal Space Game Data Optimiser.\n\n");

  if(argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-help")))
  {
    printf("Usage: optimisedata(.exe) --[no]compact folder1 folder2\n");
  }
  else
  {
    OptimiseData* od = new OptimiseData(object_reg, vfs);

    for(int i=1; i<argc; i++)
    {
      csString in = argv[i];
      csString out;
      if(argc > i+1)
      {
        out = argv[i+1];
        if(out.Compare("-out"))
        {
          out.Format("/this/%s", argv[i+2]);
          i += 2;
        }
        else
        {
          out.Empty();
        }
      }
      
      if(!vfs->Exists(in = "/this/" + in + "/"))
      {
        continue;
      }

      if(out.IsEmpty())
      {
        out = in;
        out.Append("out");
      }

      od->Run(in, out);
    }

    delete od;
    printf("\nDone!\n");
  }

  return 0;
}
