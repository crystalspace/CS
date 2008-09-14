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

#include "iutil/document.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#include "optimisedata.h"

CS_IMPLEMENT_APPLICATION

OptimiseData::OptimiseData(iObjectRegistry* objReg, iVFS* vfs) : objReg(objReg),
  vfs(vfs)
{
  docSys.AttachNew(new csTinyDocumentSystem());
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
  for(size_t i=0; i<files->GetSize(); i++)
  {
    csRef<iFile> file = vfs->Open(files->Get(i), VFS_FILE_READ);
    if(!file.IsValid())
    {
      CollectData(files->Get(i));
    }
    else
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
        if(top.IsValid())
        {
          libraries.Push(top);
        }
        else
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
                mapName = mapName.Slice(mapName.FindLast("\\"));
            }
            mapNames.Push(mapName);
            maps.Push(top);
          }
          else
          {
            continue;
          }
        }

        csRef<iDocumentNode> node = top->GetNode("textures");
        if(node.IsValid())
        {
          csRef<iDocumentNodeIterator> textureNodes = node->GetNodes("texture");
          while(textureNodes->HasNext())
          {
            textures.PushSmart(textureNodes->Next());
          }            
        }

        node = top->GetNode("materials");
        if(node.IsValid())
        {
          csRef<iDocumentNodeIterator> materialNodes = node->GetNodes("material");
          while(materialNodes->HasNext())
          {
            materials.PushSmart(materialNodes->Next());
          }            
        }

        meshfacts = top->GetNodes("meshfact");
        while(meshfacts->HasNext())
        {
          meshFacts.PushSmart(meshfacts->Next());
        }

        csRef<iDocumentNodeIterator> libraries = top->GetNodes("library");
        while(libraries->HasNext())
        {
          CollectData(libraries->Next()->GetContentsValue());
        }        
      }
    }
  }
}

void OptimiseData::SortData()
{
  // Used to store temp data.
  csRef<iDocument> tempDoc = docSys->CreateDocument();
  csRef<iDocumentNode> tempDocRoot = tempDoc->CreateRoot();

  // Start by working out if we have all the material and texture declarations needed and creating
  // if not. Then add meshfact to the output list as a library.
  for(size_t i=0; i<meshFacts.GetSize(); i++)
  {
    csRef<iDocumentNode> meshFact = meshFacts[i];
    csRefArray<iDocumentNode> tempMats;
    csRef<iDocumentNode> params = meshFact->GetNode("params");
    if(params.IsValid())
    {
      csRef<iDocumentNodeIterator> submeshes = params->GetNodes("submesh");
      bool first = true;

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
          materialName = submeshes->Next()->GetNode("material")->GetContentsValue();
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
          // Assume material name is also texture name.
          material = tempDocRoot->CreateNodeBefore(CS_NODE_ELEMENT);
          material->SetValue("material");
          material->SetAttribute("name", materialName);
          csRef<iDocumentNode> materialTex = material->CreateNodeBefore(CS_NODE_ELEMENT);
          materialTex->SetValue("texture");
          materialTex = materialTex->CreateNodeBefore(CS_NODE_TEXT);
          materialTex->SetValue(materialName);
          materials.Push(material);
        }

        tempMats.Push(material);
      }
    }

    csRef<iDocumentNodeIterator> meshes = meshFact->GetNodes("meshfact");
    while(meshes->HasNext())
    {
      csRef<iDocumentNode> undermesh = meshes->Next();
      csRef<iDocumentNodeIterator> submeshes = undermesh->GetNodes("submesh");
      bool first = true;

      csRefArray<iDocumentNode> tempMats;
      while((first && undermesh->GetNode("material")) || submeshes->HasNext())
      {
        csString materialName;
        if(first && undermesh->GetNode("material"))
        {
          materialName = undermesh->GetNode("material")->GetContentsValue();
          first = false;
        }
        else if(submeshes->HasNext())
        {
          materialName = submeshes->Next()->GetNode("material")->GetContentsValue();
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
          // Assume material name is also texture name.
          material = tempDocRoot->CreateNodeBefore(CS_NODE_ELEMENT);
          material->SetValue("material");
          material->SetAttribute("name", materialName);
          csRef<iDocumentNode> materialTex = material->CreateNodeBefore(CS_NODE_ELEMENT);
          materialTex->SetValue("texture");
          materialTex = materialTex->CreateNodeBefore(CS_NODE_TEXT);
          materialTex->SetValue(materialName);
          materials.Push(material);
        }

        tempMats.Push(material);
      }
    }

    csRefArray<iDocumentNode> tempTexs;
    for(size_t j=0; j<tempMats.GetSize(); j++)
    {
      bool hasTextureDecl = false;
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
        // Assume texture name is also file name.
        texture = tempDocRoot->CreateNodeBefore(CS_NODE_ELEMENT);
        texture->SetValue("texture");
        texture->SetAttribute("name", textureName);
        csRef<iDocumentNode> textureFile = texture->CreateNodeBefore(CS_NODE_ELEMENT);
        textureFile->SetValue("file");
        textureFile = textureFile->CreateNodeBefore(CS_NODE_TEXT);
        textureFile->SetValue(textureName);
        textures.Push(texture);
      }

      tempTexs.Push(texture);
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

    // Remove old stuff.
    world->RemoveNodes(world->GetNodes("textures"));
    world->RemoveNodes(world->GetNodes("materials"));
    world->RemoveNodes(world->GetNodes("meshfact"));
    world->RemoveNodes(world->GetNodes("library"));

    // Check dependencies.
    csRef<iDocumentNodeIterator> sectors = world->GetNodes("sector");
    csArray<csString> libsNeeded;
    while(sectors->HasNext())
    {
      csRef<iDocumentNode> sector = sectors->Next();
      csRef<iDocumentNodeIterator> meshobjs = sector->GetNodes("meshobj");
      while(meshobjs->HasNext())
      {
        csRef<iDocumentNode> meshobjFact = meshobjs->Next();
        if(meshobjFact->GetNode("params"))
        {
          meshobjFact = meshobjFact->GetNode("params")->GetNode("factory");
          libsNeeded.PushSmart(meshobjFact->GetContentsValue()); 
        }
        else if(meshobjFact->GetNode("paramsfile"))
        {
          csString paramsPath = mapInPaths[i] + "/" + meshobjFact->GetNode("paramsfile")->GetContentsValue();
          csRef<iFile> file = vfs->Open(paramsPath, VFS_FILE_READ);
          csRef<iDocument> paramsDoc = docSys->CreateDocument();
          paramsDoc->Parse(file);
          meshobjFact = paramsDoc->GetRoot()->GetNode("params")->GetNode("factory");
          libsNeeded.PushSmart(meshobjFact->GetContentsValue()); 
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
    

    for(size_t j=0; j<libsNeeded.GetSize(); j++)
    {
      csRef<iDocumentNode> lib = world->CreateNodeBefore(CS_NODE_ELEMENT, after);
      lib->SetValue("library");
      lib = lib->CreateNodeBefore(CS_NODE_TEXT);
      lib->SetValue("factories/" + libsNeeded[j] + ".meshfact");
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

  for(size_t i=0; i<mapsOut.GetSize(); i++)
  {
    csString realOut = out + mapNames[i] + ".csworld";
    mapsOut[i]->Write(vfs, realOut);
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
    printf("Usage: optimisedata(.exe) folder1 folder2\n");
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
      }
      
      if(!vfs->Exists(in = "/this/" + in + "/"))
      {
        printf("%s is not a valid path!\n", in.GetData());
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
