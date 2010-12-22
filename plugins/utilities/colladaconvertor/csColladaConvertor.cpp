/*
  Copyright (C) 2007 by Scott Johnson

  This application is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This application is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this application; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "csutil/xmltiny.h"
#include "iutil/string.h"
#include "csutil/scfstr.h"
#include "csutil/scfstringarray.h"
#include "csutil/stringconv.h"
#include "csColladaConvertor.h"
#include "csColladaClasses.h"

#include "csutil/custom_new_disable.h"
#include <string>
#include <sstream>
#include "csutil/custom_new_enable.h"

using std::string;
using std::stringstream;

#define CS_COLLADA_NORMAL_MAP_3DS 0x0040


CS_PLUGIN_NAMESPACE_BEGIN (ColladaConvertor)
{

  SCF_IMPLEMENT_FACTORY(csColladaConvertor)

    // =============== Error Reporting and Handling Functions ===============
    void csColladaConvertor::Report(int severity, const char* msg, ...)
  {
    va_list argList;
    va_start(argList, msg);

    csRef<iReporter> rep = csQueryRegistry<iReporter>(obj_reg);
    if (rep.IsValid())
    {
      rep->ReportV(severity, "crystalspace.utilities.colladaconvertor", msg, argList); 
    }
    else
    {
      csPrintfV(msg, argList);
      csPrintf("\n");
    }

    va_end(argList);
  }

  void csColladaConvertor::SetWarnings(bool toggle)
  {
    warningsOn = toggle;
  }

  void csColladaConvertor::SetSectorScene(bool toggle)
  {
    sectorScene = toggle;
  }

  void csColladaConvertor::CheckColladaFilenameValidity(const char* str)
  {
    std::string filePath = str;
    size_t index = filePath.find(".", 0);

    if (index == std::string::npos)
    {
      Report(CS_REPORTER_SEVERITY_WARNING, "Warning:  No file extension detected on filename.  File is possibly not a COLLADA file.");
    }
    else
    {
      std::string ext = filePath.substr(index);

      if (ext != ".dae" && ext != ".DAE")
      {
        Report(CS_REPORTER_SEVERITY_WARNING,
	       "Warning:  File extension %s does not conform to expected COLLADA "
	       "standard file extension %s.  File is possibly not a COLLADA file.",
	       CS::Quote::Single (ext.c_str()), CS::Quote::Single ("dae"));
      }
    }
  }

  const char* csColladaConvertor::CheckColladaValidity(iFile *file)
  {
    Report(CS_REPORTER_SEVERITY_WARNING, "Warning: The function CheckColladaValidity(iFile* file) has not yet been implemented.");
    return 0;
  }

  // =============== Constructors/ Destructors ===============

  csColladaConvertor::csColladaConvertor(iBase* parent) :
  scfImplementationType(this, parent), 
    docSys(0),
    warningsOn(false),
    sectorScene(false),
    obj_reg(0),
    lastEffectId(-1),
    csOutputReady(false),
    outputFileType(CS_NO_FILE),
    colladaReady(false)
  {
  }

  csColladaConvertor::~csColladaConvertor()
  {
    colladaElement.Invalidate();
    colladaFile.Invalidate();
    csFile.Invalidate();
    csTopNode.Invalidate();
    delete docSys;
    materialsList.DeleteAll();
  }

  // =============== Plugin Initialization ===============

  bool csColladaConvertor::Initialize (iObjectRegistry* reg)
  {
    obj_reg = reg;

    // create our own document system, since we will be reading and
    // writing to the XML files
    docSys = new csTinyDocumentSystem();

    // get a pointer to the virtual file system
    fileSys = csQueryRegistryOrLoad<iVFS>(obj_reg, "crystalspace.kernel.vfs");

    return fileSys.IsValid();
  }

  // =============== File Loading ===============

  const char* csColladaConvertor::Load(const char *str)
  {
    csRef<iFile> filePtr;

    // only do a consistency check for collada filename if warnings are on
    if (warningsOn)
    {
      CheckColladaFilenameValidity(str);
    }

    filePtr = fileSys->Open(str, VFS_FILE_READ);

    if (!filePtr.IsValid())
    {
      std::string warningMsg = "Unable to open file: ";
      warningMsg.append(str);
      warningMsg.append(".  File not loaded.");
      Report(CS_REPORTER_SEVERITY_WARNING, warningMsg.c_str());
      return "Unable to open file";
    }

    const char* retVal = Load(filePtr);
    filePtr.Invalidate();
    return retVal;
  }

  const char* csColladaConvertor::Load(iString *str)
  {
    csRef<iFile> filePtr;

    if (!fileSys.IsValid())
    {
      Report(CS_REPORTER_SEVERITY_WARNING,
        "Unable to access file system.  File not loaded.");
      return "Unable to access file system";
    }

    // only do a consistency check for collada file if warnings are on
    if (warningsOn)
    {
      CheckColladaFilenameValidity(str->GetData());
    }

    filePtr = fileSys->Open(str->GetData(), VFS_FILE_READ);

    if (!filePtr.IsValid())
    {
      std::string warningMsg = "Unable to open file: ";
      warningMsg.append(str->GetData());
      warningMsg.append(".  File not loaded.");
      Report(CS_REPORTER_SEVERITY_WARNING, warningMsg.c_str());
      return "Unable to open file";
    }

    const char* retVal = Load(filePtr);
    filePtr.Invalidate();
    return retVal;
  }

  const char* csColladaConvertor::Load(iFile *file)
  {
    colladaFile = docSys->CreateDocument();
    colladaFile->Parse(file);
    csRef<iDocumentNode> rootNode = colladaFile->GetRoot();
    rootNode = rootNode->GetNode("COLLADA");
    if (!rootNode.IsValid())
    {
      Report(CS_REPORTER_SEVERITY_ERROR,
        "Error: Unable to find COLLADA node.  File not loaded.");
      return "Unable to find COLLADA node";
    }

    colladaElement = rootNode;
    csOutputReady = false;
    colladaReady = true;

    return 0;
  }

  const char* csColladaConvertor::Load(iDataBuffer *db)
  {
    colladaFile = docSys->CreateDocument();
    colladaFile->Parse(db);
    csRef<iDocumentNode> rootNode = colladaFile->GetRoot();
    rootNode = rootNode->GetNode("COLLADA");
    if (!rootNode.IsValid())
    {
      Report(CS_REPORTER_SEVERITY_ERROR,
        "Error: Unable to find COLLADA node.  File not loaded.");
      return "Unable to find COLLADA node";
    }  

    colladaElement = rootNode;
    csOutputReady = false;
    colladaReady = true;

    return 0;
  }

  // =============== File Writing ===============

  const char* csColladaConvertor::Write(const char* filepath)
  {
    // sanity check
    if (!csOutputReady)
    {
      if (warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING,
          "Warning: Crystal Space document not ready for writing.");
      }

      return "Crystal Space document not ready for writing";
    }

    const char* errorString = csFile->Write(fileSys, filepath);

    if (errorString)
    {
      std::string errorMsg = "Warning: An error occurred writing to file: ";
      errorMsg.append(errorString);
      Report(CS_REPORTER_SEVERITY_ERROR, errorMsg.c_str()); 
      return "An error occurred writing to file";
    }

    return 0;
  }

  // =============== Accessor Functions =============== 

  csColladaMaterial* csColladaConvertor::FindMaterial(const char* accessorString)
  {
    // Using csArrayCmp would be more efficient I believe, but
    // I have no idea how to use this comparator.  ;)
    //csArrayCmp<csColladaMaterial, const char*> functor;

    if (warningsOn)
    {
      Report(CS_REPORTER_SEVERITY_WARNING, "Inside FindMaterial()");
    }

    csArray<csColladaMaterial>::Iterator matIter = materialsList.GetIterator();
    while (matIter.HasNext())
    {
      csString accessConverted(accessorString);

      if (warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING, "Creating currentMat...");
      }

      csColladaMaterial *currentMat = new csColladaMaterial(matIter.Next());

      if (warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING, "Done. Accessor string: %s", accessConverted.GetData());
      }

      if (currentMat->GetID().Compare(accessConverted.GetData()))
      {
        if (warningsOn)
        {
          Report(CS_REPORTER_SEVERITY_WARNING, "Returning...");
        }
        return currentMat;
      }

      else
      {
        delete currentMat;
      }
    }

    if (warningsOn)
    {
      Report(CS_REPORTER_SEVERITY_WARNING, "Returning...");
    }

    return 0;
  }

  csColladaEffect& csColladaConvertor::GetEffect(size_t index)
  {
    return effectsList.Get(index);
  }

  size_t csColladaConvertor::GetEffectIndex(const csColladaEffect& effect)
  {
    return 0;
  }

  // =============== Mutator Functions ===============

  const char* csColladaConvertor::SetOutputFiletype(csColladaFileType filetype)
  {
    if (filetype == CS_NO_FILE)
    {
      Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to set output file type to CS_NO_FILE.");
      return "Unable to set output file type to CS_NO_FILE";
    }

    outputFileType = filetype;
    return 0;
  }

  bool csColladaConvertor::InitializeCrystalSpaceDocument()
  {
    if (outputFileType == CS_NO_FILE)
    {
      if (warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING, "Warning: No output file type specified.  Use SetOutputFiletype() first");
      }

      return false;
    }

    csFile = docSys->CreateDocument();
    csRef<iDocumentNode> rootNode = csFile->CreateRoot();

    /* Some identifying commentary */
    csRef<iDocumentNode> comment = rootNode->CreateNodeBefore(CS_NODE_COMMENT, 0);
    comment->SetValue("FILE GENERATED AUTOMATICALLY - DO NOT EDIT");
    csRef<iDocumentNode> comment2 = rootNode->CreateNodeBefore(CS_NODE_COMMENT, comment);
    comment2->SetValue("Created with CrystalSpace COLLADA Convertor v1.0");
    comment.Invalidate();
    comment2.Invalidate();

    csRef<iDocumentNode> newNode = rootNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    if(!newNode.IsValid())
    {
      if (warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Unable to create first element.");
      }

      return false;
    }

    if (outputFileType == CS_MAP_FILE)
    {
      newNode->SetValue("world"); 
      csTopNode = csFile->GetRoot()->GetNode("world");
    }

    else 
    {
      newNode->SetValue("library");
      csTopNode = csFile->GetRoot()->GetNode("library");
    }

    csOutputReady = true;
    return true;
  }

  // =============== Basic Utility Functions ===============

  // None, anymore ;)


  // =============== Conversion Functions ===============
  // This is the warp core of the implementation.  ;)
  // Currently, the core has been ejected, so it's missing.  Engineering is still
  // trying to locate it on sensors.  :)

  const char* csColladaConvertor::Convert()
  {
    if (!csOutputReady)
    {
      if (!InitializeCrystalSpaceDocument())
      {
        if (warningsOn)
        {
          Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to initialize output document.");
        }
        return "Unable to initialize output document";
      }
      else
      {
        if (warningsOn)
        {
          Report(CS_REPORTER_SEVERITY_NOTIFY, "Success.");
        }
      }
    }

    if (!colladaReady)
    {
      Report(CS_REPORTER_SEVERITY_ERROR, "Error: COLLADA file has not been loaded.");
      return "COLLADA file not loaded";
    }

    // ConvertEffects() needs to be called first, so that there actually *is* a materials
    // list from which to assign materials in ConvertGeometry()
    csRef<iDocumentNode> materialsNode = colladaElement->GetNode("library_materials");
    if (!materialsNode.IsValid())
    {
      Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to find <library_materials> element");
      return "Unable to find library_materials.";
    }

    if (warningsOn)
    {
      Report(CS_REPORTER_SEVERITY_WARNING, "Beginning to convert effects.");
    }

    ConvertEffects();

    if (warningsOn)
    {
      Report(CS_REPORTER_SEVERITY_WARNING, "Done converting effects.");
    }

    csRef<iDocumentNode> geoNode = colladaElement->GetNode("library_geometries");
    if (!geoNode.IsValid())
    {
      Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to find <library_geometries> element.");
      return "Unable to find library_geometries.";
    }

    if (warningsOn)
    {
      Report(CS_REPORTER_SEVERITY_WARNING, "Beginning to convert geometry");
    }

    ConvertGeometry(geoNode);

    if (warningsOn)
    {
      Report(CS_REPORTER_SEVERITY_WARNING, "Done converting geometry");
    }

    if (outputFileType == CS_MAP_FILE)
    {
      csRef<iDocumentNode> camerasNode, lightsNode, visualScenesNode;
      camerasNode = colladaElement->GetNode("library_cameras");
      lightsNode = colladaElement->GetNode("library_lights");
      visualScenesNode = colladaElement->GetNode("library_visual_scenes");

      if (warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING, "Beginning to convert scene data");
      }

      ConvertScene(camerasNode, lightsNode, visualScenesNode);

      if (warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING, "Done converting scene data");
      }
    }

    return 0;
  }

  bool csColladaConvertor::ConvertGeometry(iDocumentNode *geometrySection)
  {
    // First collect the names of all portals and fill the meshprops hash.
    csRef<iDocumentNode> libVisScenes = colladaElement->GetNode("library_visual_scenes");
    csRef<iDocumentNode> libVisualScenes = colladaElement->GetNode("library_visual_scenes");
    csRef<iDocumentNodeIterator> visualScenes = libVisualScenes->GetNodes("visual_scene");

    csRef<iDocumentNodeIterator> sectors;
    for(int i=0; (visualScenes->HasNext() && !sectorScene) || i<1; i++)
    {
      if(sectorScene)
      {
        sectors = visualScenes;
      }
      else
      {
        sectors = visualScenes->Next()->GetNodes("node");
      }

      while(sectors->HasNext())
      {
        csRef<iDocumentNode> sector = sectors->Next();
        csRef<iDocumentNodeIterator> sectorObjects = sector->GetNodes("node");
        while(sectorObjects->HasNext())
        {
          csRef<iDocumentNode> object = sectorObjects->Next();
          if(object->GetNode("extra"))
          {
            csRef<iDocumentNodeIterator> techniques = object->GetNode("extra")->GetNodes("technique");
            while(techniques->HasNext())
            {
              csRef<iDocumentNode> technique = techniques->Next();
              if(csString(technique->GetAttributeValue("profile")).Compare("FCOLLADA"))
              {
                technique = technique->GetNode("user_properties");
                csStringArray meshProp;
                csStringArray userProp;
                userProp.SplitString(technique->GetContentsValue(), ";");
                for(size_t i=0; i<userProp.GetSize(); i++)
                {
                  meshProp.Push(csString(userProp[i]).Trim().Truncate('&'));
                  csString prop = meshProp[i];
                  if(prop.Truncate(prop.FindFirst('=')).Compare("PORTAL"))
                  {
                    portalNames.Push(object->GetAttributeValue("name"));
                    portalTargets.Push(csString(userProp[i]).Slice(csString(userProp[i]).FindFirst('=')+1));
                  }
                }
                meshProps.Put(object->GetAttributeValue("name"), meshProp);
              }
            }
          }
        }
      }
    }


    csRef<iDocumentNodeIterator> geometryIterator;

    // get an iterator over all <geometry> elements
    geometryIterator = geometrySection->GetNodes("geometry");

    if (!geometryIterator.IsValid())
    {
      if (warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Unable to get iterator over geometry nodes.");
      }

      return false;
    }

    // variable definitions, so we don't have to run the constructors each
    // time during an iteration of the while loops
    csRef<iDocumentNode> currentGeometryElement;
    csRef<iDocumentAttribute> currentGeometryID;
    csRef<iDocumentNodeIterator> meshIterator;
    csRef<iDocumentNode> currentMeshElement;
    csRef<iDocumentNode> currentVerticesElement;
    csRef<iDocumentNodeIterator> convexMeshIterator;
    csColladaMesh *mesh;

    // while the iterator is not empty
    while (geometryIterator->HasNext())
    {
      // retrieve next <geometry> element and store as currentGeometryElement
      currentGeometryElement = geometryIterator->Next();

      // If this is a portal, skip it and continue.
      bool skip = false;
      for(size_t i=0; i<portalNames.GetSize() && !skip; i++)
      {
        skip = portalNames[i].Compare(currentGeometryElement->GetAttributeValue("name"));
      }

      if(skip)
      {
        continue;
      }

      // get value of id attribute and store as currentGeometryID
      currentGeometryID = currentGeometryElement->GetAttribute("id");

      if (currentGeometryID.IsValid())
      {
        if (warningsOn)
        {
          std::string notifyMsg = "Current Geometry Node ID: ";
          notifyMsg.append(currentGeometryID->GetValue());
          Report(CS_REPORTER_SEVERITY_NOTIFY, notifyMsg.c_str());
        }
      }

      // let's make sure that we output a warning in the event that the
      // user attempts to convert a <convex_mesh> element, as this
      // isn't supported yet
      /// @todo Add support for <convex_mesh> elements.
      convexMeshIterator = currentGeometryElement->GetNodes("convex_mesh");
      if (convexMeshIterator->HasNext() && warningsOn)
      {
        Report(CS_REPORTER_SEVERITY_WARNING, "Warning: <convex_mesh> element detected.  This system does not currently support this element type.  It will not be converted.");
      }

      // get an iterator over all <mesh> child elements
      meshIterator = currentGeometryElement->GetNodes("mesh");

      if (!meshIterator.IsValid())
      {
        if (warningsOn)
        {
          Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Unable to get iterator over mesh nodes.");
        }

        return false;
      }

      // while this iterator is not empty
      while (meshIterator->HasNext())
      {
        // get next mesh element as currentMeshElement
        currentMeshElement = meshIterator->Next();

        // retrieve <vertices> element from currentMeshElement as
        // currentVerticesElement
        currentVerticesElement = currentMeshElement->GetNode("vertices");

        if(!currentVerticesElement.IsValid())
        {
          if (warningsOn)
          {
            Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Unable to retrieve vertices element from mesh.");
          }

          return false;
        }

        // Get meshfact type.
        csString meshfactType = GetMeshFactType(csString(""));
        csStringArray meshPropsArr = meshProps.Get(currentGeometryElement->GetAttributeValue("name"), csStringArray());
        for(size_t j=0; j<meshPropsArr.GetSize(); j++)
        {
          if(csString(meshPropsArr[j]).Truncate(csString(meshPropsArr[j]).FindFirst('=')).Compare("MESH"))
          {
            meshfactType = GetMeshFactType(csString(meshPropsArr[j]).Slice(csString(meshPropsArr[j]).FindFirst('=')+1));
          }
        }

        mesh = new csColladaMesh(currentMeshElement, this, meshfactType);

        mesh->WriteXML(csTopNode);

        delete mesh;
      }
    }

    return true;
  }

  csRef<iDocumentNode> csColladaConvertor::GetSourceElement(const char* name, iDocumentNode* parent)
  {
    csRef<iDocumentNode> returnValue;
    csRef<iDocumentNodeIterator> iter;
    bool found = false;
    iter = parent->GetNodes("source");

    while(iter->HasNext())
    {
      returnValue = iter->Next();
      std::string comparisonString(returnValue->GetAttributeValue("id"));
      if (comparisonString.compare(name) == 0)
      {
        found = true;
        break;
      }
    }

    if (!found)
    {
      return 0;
    }

    return returnValue;
  }

  // ConvertEffects() is the generic function for converting textures, materials, shaders
  // and all related items.
  bool csColladaConvertor::ConvertEffects()
  {
    Report(CS_REPORTER_SEVERITY_WARNING, "Warning: ConvertEffects() functionality not fully implemented.  Use at your own risk!");

    // Convert textures
    csRef<iDocumentNode> imagesNode = colladaElement->GetNode("library_images");
    if(imagesNode.IsValid())
    {
      csRef<iDocumentNode> texturesNode = csTopNode->CreateNodeBefore(CS_NODE_ELEMENT);
      texturesNode->SetValue("textures");
      csRef<iDocumentNodeIterator> textureNodes = imagesNode->GetNodes("image");
      while(textureNodes->HasNext())
      {
        csRef<iDocumentNode> texture = textureNodes->Next();
        csRef<iDocumentNode> newTexture = texturesNode->CreateNodeBefore(CS_NODE_ELEMENT);
        newTexture->SetValue("texture");
        newTexture->SetAttribute("name", texture->GetAttributeValue("id"));

        csRef<iDocumentNode> textureFile = newTexture->CreateNodeBefore(CS_NODE_ELEMENT);
        textureFile->SetValue("file");

        csRef<iDocumentNode> textureFileContents = textureFile->CreateNodeBefore(CS_NODE_TEXT);
        textureFileContents->SetValue(texture->GetAttributeValue("id"));

        // TODO: Alpha, Class.

        //checking if this texture is a normalmap, adding <class>normalmap</class> tag if it is.        
        if(CheckForNormalMap(texture))
        {
          csRef<iDocumentNode> classTag = newTexture->CreateNodeBefore(CS_NODE_ELEMENT);
          classTag->SetValue("class");

          csRef<iDocumentNode> normalmapText = classTag->CreateNodeBefore(CS_NODE_TEXT);
          normalmapText->SetValue("normalmap");
        }
      }
    }

    // Get instance materials and make a list of those which are generated by the exporter.
    // These are most likely connected to portals and other objects which have no material attached.
    csRef<iDocumentNode> libVisualScenes = colladaElement->GetNode("library_visual_scenes");
    csRef<iDocumentNodeIterator> visualScenes = libVisualScenes->GetNodes("visual_scene");
    csArray<csString> dummyMaterialIDs;

    csRef<iDocumentNodeIterator> sectors;
    for(int i=0; (visualScenes->HasNext() && !sectorScene) || i<1; i++)
    {
      if(sectorScene)
      {
        sectors = visualScenes;
      }
      else
      {
        sectors = visualScenes->Next()->GetNodes("node");
      }

      while(sectors->HasNext())
      {
        csRef<iDocumentNode> sector = sectors->Next();
        csRef<iDocumentNodeIterator> sectorObjects = sector->GetNodes("node");
        while(sectorObjects->HasNext())
        {
          csRef<iDocumentNode> object = sectorObjects->Next();
          if(object->GetNode("node") && object->GetNode("node")->GetNode("instance_geometry"))
          {
            object = object->GetNode("node")->GetNode("instance_geometry");
            if(object->GetNode("bind_material") && object->GetNode("bind_material")->GetNode("technique_common"))
            {
              object = object->GetNode("bind_material")->GetNode("technique_common");
              csString material("ColorMaterial");
              if(object->GetNode("instance_material") &&
                material.Compare(object->GetNode("instance_material")->GetAttributeValue("symbol")))
              {
                dummyMaterialIDs.Push(csString(object->GetNode("instance_material")->GetAttributeValue("target")).Slice(1));
              }
            }
          }
        }
      }
    }

    // Convert materials
    csRef<iDocumentNode> materialsNode = colladaElement->GetNode("library_materials");
    csRef<iDocumentNode> effectsNode = colladaElement->GetNode("library_effects");
    if(materialsNode.IsValid() && effectsNode.IsValid())
    {
      csRef<iDocumentNode> newMaterialsNode = csTopNode->CreateNodeBefore(CS_NODE_ELEMENT);
      newMaterialsNode->SetValue("materials");

      csRef<iDocumentNodeIterator> materialNodes = materialsNode->GetNodes("material");
      csRef<iDocumentNodeIterator> effectNodes = effectsNode->GetNodes("effect");

      while(materialNodes->HasNext() && effectNodes->HasNext())
      {
        csRef<iDocumentNode> material = materialNodes->Next();
        csRef<iDocumentNode> effect = effectNodes->Next();

        bool nowrite = false;
        for(size_t i=0; i<dummyMaterialIDs.GetSize() && !nowrite; i++)
        {
          nowrite = dummyMaterialIDs[i].Compare(material->GetAttributeValue("id"));
        }

        if(nowrite)
        {
          continue;
        }

        csRef<iDocumentNode> newMaterial = newMaterialsNode->CreateNodeBefore(CS_NODE_ELEMENT);
        newMaterial->SetValue("material");
        newMaterial->SetAttribute("name", material->GetAttributeValue("name"));

        csColladaMaterial nextMaterial = csColladaMaterial(this);
        nextMaterial.SetID(material->GetAttributeValue("id"));
        nextMaterial.SetName(material->GetAttributeValue("name"));
        nextMaterial.SetMaterialNode(newMaterial);
        nextMaterial.SetInstanceEffect(effect);
        materialsList.Push(nextMaterial);
        CreateShaderVarNodes(newMaterial,effect);
      }

      return true;
    }

    return false;
  }

  bool csColladaConvertor::ConvertScene(iDocumentNode *camerasSection, iDocumentNode *lightsSection, iDocumentNode *visualScenesSection)
  {
    Report(CS_REPORTER_SEVERITY_WARNING, "Warning: ConvertScene() functionality not fully implemented.  Use at your own risk!");

    if (outputFileType != CS_MAP_FILE)
    {
      Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Conversion of scenes is invalid except for Crystal Space map files.  Continuing blithely...");
      return false;
    }

    // Get camera IDs
    if(camerasSection)
    {
      csRef<iDocumentNodeIterator> cameraNodes = camerasSection->GetNodes("camera");
      while(cameraNodes->HasNext())
      {
        cameraIDs.Push(cameraNodes->Next()->GetAttributeValue("id"));
      }
    }

    // Get lights
    if(lightsSection)
    {
      csRef<iDocumentNodeIterator> lightNodes = lightsSection->GetNodes("light");
      while(lightNodes->HasNext())
      {
        csRef<iDocumentNode> lightNode = lightNodes->Next();
        lightNode = lightNode->GetNode("technique_common");

        // Point lights.
        if(lightNode &&
          lightNode->GetNode("point"))
        {
          csColladaLight light;

          // TODO: Radius, attenuation, more?

          csStringArray lightColour;
          lightColour.SplitString(lightNode->GetNode("point")->GetNode("color")->GetContentsValue(), " ");
          light.colour = csColor(CS::Utility::strtof(lightColour[0]), CS::Utility::strtof(lightColour[1]), CS::Utility::strtof(lightColour[2]));

          lights.Put(lightNode->GetParent()->GetAttributeValue("id"), light);
        }
      }
    }

    if(sectorScene)
    {
      // Convert each scene (<visual_scene> node) into a sector.
      csRef<iDocumentNodeIterator> sectors = visualScenesSection->GetNodes("visual_scene");
      while(sectors->HasNext())
      {
        csRef<iDocumentNode> sector = sectors->Next();
        WriteSectorInfo(sector);
      }

      // Next, we'll convert the cameras
      // A camera will convert to a start location in the world file
      // A start location consists of: 
      //   * A sector to start in.
      //   * A position to start at.
      //   * An up vector. (optional)
      //   * A forward vector. (optional)

      // For each camera.
      for(size_t i=0; i<cameraIDs.GetSize(); i++)
      {
        // For each scene.
        csRef<iDocumentNodeIterator> sectors = visualScenesSection->GetNodes("visual_scene");
        while(sectors->HasNext())
        {
          csRef<iDocumentNode> sector = sectors->Next();
          WriteCameraInfo(sector, i);
        }
      }
    }
    else
    {
      // For each scene (<visual_scene> node), the first level nodes will be converted to 
      // a sector in crystal space.
      csRef<iDocumentNodeIterator> visualSceneIterator = visualScenesSection->GetNodes("visual_scene");
      while(visualSceneIterator->HasNext())
      {
        csRef<iDocumentNode> visualSceneElement = visualSceneIterator->Next();
        csRef<iDocumentNodeIterator> sectors = visualSceneElement->GetNodes("node");
        while(sectors->HasNext())
        {
          csRef<iDocumentNode> sector = sectors->Next();
          WriteSectorInfo(sector);
        }
      }

      // Next, we'll convert the cameras
      // A camera will convert to a start location in the world file
      // A start location consists of: 
      //   * A sector to start in.
      //   * A position to start at.
      //   * An up vector. (optional)
      //   * A forward vector. (optional)

      // For each camera.
      for(size_t i=0; i<cameraIDs.GetSize(); i++)
      {
        // For each scene.
        visualSceneIterator = visualScenesSection->GetNodes("visual_scene");
        while(visualSceneIterator->HasNext())
        {
          // For each sector;
          csRef<iDocumentNode> visualSceneElement = visualSceneIterator->Next();
          csRef<iDocumentNodeIterator> sectors = visualSceneElement->GetNodes("node");
          while(sectors->HasNext())
          {
            csRef<iDocumentNode> sector = sectors->Next();
            WriteCameraInfo(sector, i);
          }
        }
      }
    }

    return true;
  }

  void csColladaConvertor::WriteSectorInfo(iDocumentNode* sector)
  {
    // Get user props
    if(sector->GetNode("extra"))
    {
      csRef<iDocumentNodeIterator> techniques = sector->GetNode("extra")->GetNodes("technique");
      while(techniques->HasNext())
      {
        csRef<iDocumentNode> technique = techniques->Next();
        if(csString(technique->GetAttributeValue("profile")).Compare("FCOLLADA"))
        {
          technique = technique->GetNode("user_properties");
          csStringArray sectorProp;
          csStringArray userProp;
          userProp.SplitString(technique->GetContentsValue(), ";");
          for(size_t i=0; i<userProp.GetSize(); i++)
          {
            sectorProp.Push(csString(userProp[i]).Trim().Truncate('&'));
          }
          sectorProps.Put(sector->GetAttributeValue("id"), sectorProp);
        }
      }
    }

    // Check that it really is a sector.
    bool cameraTarget = false;
    for(size_t i=0; i<cameraIDs.GetSize(); i++)
    {
      csString id = cameraIDs[i];
      id.Truncate(id.FindLast('-'));
      cameraTarget |= id.Append(".Target-node").Compare(sector->GetAttributeValue("id"));
    }

    if(sector->GetAttribute("name") && !cameraTarget)
    {
      // Write sector.
      csRef<iDocumentNode> currentSectorElement = csTopNode->CreateNodeBefore(CS_NODE_ELEMENT);
      currentSectorElement->SetValue("sector");
      currentSectorElement->SetAttribute("name", sector->GetAttributeValue("name"));

      // Culler (TODO: Support others).
      csRef<iDocumentNode> culler = currentSectorElement->CreateNodeBefore(CS_NODE_ELEMENT);
      culler->SetValue("cullerp");
      culler = culler->CreateNodeBefore(CS_NODE_TEXT);
      csStringArray userProps = sectorProps.Get(sector->GetAttributeValue("id"), csStringArray());
      if(userProps.GetSize() != 0)
      {
        for(size_t i=0; i<userProps.GetSize(); i++)
        {
          if(csString(userProps[i]).Truncate(csString(userProps[i]).FindFirst('=')).Compare("CULLER"))
          {
            culler->SetValue(csString(userProps[i]).Slice(csString(userProps[i]).FindFirst('=')+1));
          }
        }
      }
      else
      {
        culler->SetValue("crystalspace.culling.frustvis");
      }

      // Write children.
      csRef<iDocumentNodeIterator> sectorNodes = sector->GetNodes("node");
      while(sectorNodes->HasNext())
      {
        csRef<iDocumentNode> object = sectorNodes->Next();

        // Write portal.
        size_t portalNum;
        bool isPortal = false;
        for(portalNum=0; portalNum<portalNames.GetSize() && !isPortal; portalNum++)
        {
          isPortal = portalNames[portalNum].Compare(object->GetAttributeValue("name"));
        }

        if(isPortal)
        {
          csRef<iDocumentNode> newPortal = currentSectorElement->CreateNodeBefore(CS_NODE_ELEMENT);
          newPortal->SetValue("portal");
          newPortal->SetAttribute("name", object->GetAttributeValue("name"));
          csRef<iDocumentNode> newPortalSector = newPortal->CreateNodeBefore(CS_NODE_ELEMENT);
          newPortalSector->SetValue("sector");
          csRef<iDocumentNode> newPortalSectorContents = newPortalSector->CreateNodeBefore(CS_NODE_TEXT);
          newPortalSectorContents->SetValue(portalTargets[portalNum-1]);

          csRef<iDocumentNodeIterator> facts = colladaElement->GetNode("library_geometries")->GetNodes("geometry");
          while(facts->HasNext())
          {
            csRef<iDocumentNode> fact = facts->Next();
            if(csString(fact->GetAttributeValue("name")).Compare(object->GetAttributeValue("name")))
            {
              fact = fact->GetNode("mesh")->GetNode("source")->GetNode("float_array");
              csStringArray vertices;
              vertices.SplitString(fact->GetContentsValue(), " ");

              for(int i=0; i<12; i+=3)
              {
                csRef<iDocumentNode> vertex = newPortal->CreateNodeBefore(CS_NODE_ELEMENT);
                vertex->SetValue("v");

                float x = CS::Utility::strtof(vertices[i]);
                float y = CS::Utility::strtof(vertices[i+1]);
                float z = CS::Utility::strtof(vertices[i+2]);

                vertex->SetAttributeAsFloat("x", x);
                vertex->SetAttributeAsFloat("y", y);
                vertex->SetAttributeAsFloat("z", z);
              }
              break;
            }
          }

          continue;
        }

        // Write light.
        if(object->GetNode("instance_light"))
        {
          csRef<iDocumentNode> newLight = currentSectorElement->CreateNodeBefore(CS_NODE_ELEMENT);
          newLight->SetValue("light");
          newLight->SetAttribute("name", object->GetAttributeValue("name"));

          // Calculate centre.
          csStringArray sectorPos;
          csStringArray centerPos;

          sectorPos.SplitString(sector->GetNode("matrix")->GetContentsValue(), " ");
          centerPos.SplitString(object->GetNode("matrix")->GetContentsValue(), " ");

          float x = CS::Utility::strtof(sectorPos[3]) + CS::Utility::strtof(centerPos[3]);
          float y = CS::Utility::strtof(sectorPos[11]) + CS::Utility::strtof(centerPos[11]);
          float z = CS::Utility::strtof(sectorPos[7]) + CS::Utility::strtof(centerPos[7]);

          csRef<iDocumentNode> centreNode = newLight->CreateNodeBefore(CS_NODE_ELEMENT);
          centreNode->SetValue("center");
          centreNode->SetAttributeAsFloat("x", x);
          centreNode->SetAttributeAsFloat("y", y);
          centreNode->SetAttributeAsFloat("z", z);

          // Write colour.
          csString url = object->GetNode("instance_light")->GetAttributeValue("url");
          csColladaLight light = lights.Get(url.Slice(1), csColladaLight());
          csRef<iDocumentNode> lightColour = newLight->CreateNodeBefore(CS_NODE_ELEMENT);
          lightColour->SetValue("color");
          lightColour->SetAttributeAsFloat("red", light.colour.red);
          lightColour->SetAttributeAsFloat("green", light.colour.green);
          lightColour->SetAttributeAsFloat("blue", light.colour.blue);
          continue;
        }

        // Check for camera, deal with it later.
        if(object->GetNode("instance_camera"))
        {
          continue;
        }

        // Write genmesh meshobj (TODO: Other mesh types).
        csRef<iDocumentNode> meshObj = currentSectorElement->CreateNodeBefore(CS_NODE_ELEMENT);
        meshObj->SetValue("meshobj");
        meshObj->SetAttribute("name", object->GetAttributeValue("name"));
        csRef<iDocumentNode> plugin = meshObj->CreateNodeBefore(CS_NODE_ELEMENT);
        plugin->SetValue("plugin");

        csString meshType = GetMeshType(csString(""));
        csStringArray mesh = meshProps.Get(object->GetAttributeValue("name"), csStringArray());
        for(size_t j=0; j<mesh.GetSize(); j++)
        {
          if(csString(mesh[j]).Truncate(csString(mesh[j]).FindFirst('=')).Compare("MESH"))
          {
            meshType = GetMeshType(csString(mesh[j]).Slice(csString(mesh[j]).FindFirst('=')+1));           
            continue;
          }

          if(csString(mesh[j]).Truncate(csString(mesh[j]).FindFirst('=')).Compare("NOSHADOWS"))
          {
            csRef<iDocumentNode> noshadows = meshObj->CreateNodeBefore(CS_NODE_ELEMENT);
            noshadows->SetValue("noshadows");
          }
        }

        plugin = plugin->CreateNodeBefore(CS_NODE_TEXT);
        plugin->SetValue(meshType);

        // Params
        csRef<iDocumentNode> params = meshObj->CreateNodeBefore(CS_NODE_ELEMENT);
        params->SetValue("params");

        // Link to mesh factory and material.
        csRef<iDocumentNode> factory = params->CreateNodeBefore(CS_NODE_ELEMENT);
        factory->SetValue("factory");
        csRef<iDocumentNode> material = params->CreateNodeBefore(CS_NODE_ELEMENT);
        material->SetValue("material");
        csRef<iDocumentNode> factnode = object->GetNode("node")->GetNode("instance_geometry");

        // Factory.
        csString url = factnode->GetAttributeValue("url");
        csRef<iDocumentNodeIterator> facts = colladaElement->GetNode("library_geometries")->GetNodes("geometry");
        while(facts->HasNext())
        {
          csRef<iDocumentNode> fact = facts->Next();
          if(csString(fact->GetAttributeValue("id")).Compare(url.Slice(1)))
          {
            factory = factory->CreateNodeBefore(CS_NODE_TEXT);
            factory->SetValue(fact->GetAttributeValue("name"));
            break;
          }
        }

        // Material
        csRef<iDocumentNode> matnode = factnode->GetNode("bind_material");
        if(matnode.IsValid())
        {
          matnode = matnode->GetNode("technique_common")->GetNode("instance_material");
          material = material->CreateNodeBefore(CS_NODE_TEXT);
          material->SetValue(csString(matnode->GetAttributeValue("target")).Slice(1));          
        }

        // Position
        csRef<iDocumentNode> move = meshObj->CreateNodeBefore(CS_NODE_ELEMENT);
        move->SetValue("move");
        move = move->CreateNodeBefore(CS_NODE_ELEMENT);
        move->SetValue("v");

        csStringArray sectorPos;
        csStringArray meshobjPos;

        sectorPos.SplitString(sector->GetNode("matrix")->GetContentsValue(), " ");
        meshobjPos.SplitString(object->GetNode("matrix")->GetContentsValue(), " ");

        float x = CS::Utility::strtof(sectorPos[3]) + CS::Utility::strtof(meshobjPos[3]);
        float y = CS::Utility::strtof(sectorPos[11]) + CS::Utility::strtof(meshobjPos[11]);
        float z = CS::Utility::strtof(sectorPos[7]) + CS::Utility::strtof(meshobjPos[7]);

        move->SetAttributeAsFloat("x", x);
        move->SetAttributeAsFloat("y", y);
        move->SetAttributeAsFloat("z", z);
      }
    }
  }

  void csColladaConvertor::WriteCameraInfo(iDocumentNode* sector, size_t camera)
  {
    // Not a sector if it's a camera target.
    csString id = cameraIDs[camera];
    if(id.Append(".Target-node").Compare(sector->GetAttributeValue("id")))
    {
      return;
    }

    // For each node (possible camera).
    csRef<iDocumentNodeIterator> nodes = sector->GetNodes("node");
    while(nodes->HasNext())
    {
      csRef<iDocumentNode> node = nodes->Next();
      if(node->GetNode("instance_camera"))
      {
        // Is it the right camera?
        csString url = node->GetNode("instance_camera")->GetAttributeValue("url");
        if(url.Slice(1).Compare(cameraIDs[camera]))
        {
          // Create start node.
          csRef<iDocumentNode> startNode = csTopNode->CreateNodeBefore(CS_NODE_ELEMENT);
          startNode->SetValue("start");
          startNode->SetAttribute("name", node->GetAttributeValue("name"));
          csRef<iDocumentNode> sectorNode = startNode->CreateNodeBefore(CS_NODE_ELEMENT);
          sectorNode->SetValue("sector");
          csRef<iDocumentNode> sectorNodeContents = sectorNode->CreateNodeBefore(CS_NODE_TEXT);
          sectorNodeContents->SetValue(sector->GetAttributeValue("name"));

          // Calculate position.
          csStringArray sectorPos;
          csStringArray cameraPos;

          sectorPos.SplitString(sector->GetNode("matrix")->GetContentsValue(), " ");
          cameraPos.SplitString(node->GetNode("matrix")->GetContentsValue(), " ");

          float x = CS::Utility::strtof(sectorPos[3]) + CS::Utility::strtof(cameraPos[3]);
          float y = CS::Utility::strtof(sectorPos[11]) + CS::Utility::strtof(cameraPos[11]);
          float z = CS::Utility::strtof(sectorPos[7]) + CS::Utility::strtof(cameraPos[7]);

          csRef<iDocumentNode> positionNode = startNode->CreateNodeBefore(CS_NODE_ELEMENT);
          positionNode->SetValue("position");
          positionNode->SetAttributeAsFloat("x", x);
          positionNode->SetAttributeAsFloat("y", y);
          positionNode->SetAttributeAsFloat("z", z);

          // TODO: up and forward vectors.
        }
      }
    }
  }

  csString csColladaConvertor::GetMeshFactType(csString name)
  {
    if(name.CompareNoCase("terrain2"))
    {
      return CS_COLLADA_TERRAIN2FACT_PLUGIN_TYPE;
    }
    else
    {
      return CS_COLLADA_GENMESHFACT_PLUGIN_TYPE;
    }
  }

  csString csColladaConvertor::GetMeshType(csString name)
  {
    if(name.CompareNoCase("terrain2"))
    {
      return CS_COLLADA_TERRAIN2_PLUGIN_TYPE;
    }
    else
    {
      return CS_COLLADA_GENMESH_PLUGIN_TYPE;
    }
  }

  bool csColladaConvertor::ConvertRiggingAnimation(iDocumentNode *riggingSection)
  {
    Report(CS_REPORTER_SEVERITY_WARNING, "Warning: ConvertRiggingAnimation() functionality not fully implemented.  Use at your own risk!");
    return true;
  }

  bool csColladaConvertor::ConvertPhysics(iDocumentNode *physicsSection)
  {
    Report(CS_REPORTER_SEVERITY_WARNING, "Warning: ConvertPhysics() functionality not fully implemented.  Use at your own risk!");
    return true;
  }


  int csColladaConvertor::CheckForNormalMap(csRef<iDocumentNode> texture)
  {
    //add other way to check for normal map here
    if(CheckForNormalMap3DSMaxVersion(texture))
      return CS_COLLADA_NORMAL_MAP_3DS;
    return 0;
  }


  bool csColladaConvertor::CheckForNormalMap3DSMaxVersion(csRef<iDocumentNode> texture)
  {
    csRef<iDocumentNode> effectsNode = colladaElement->GetNode("library_effects");
    csRef<iDocumentNodeIterator> effectNodes = effectsNode->GetNodes("effect");

    csString surfaceID("");

    while(effectNodes->HasNext())
    {
      bool foundSurface = false;

      csRef<iDocumentNode> effect = effectNodes->Next();
      csRef<iDocumentNodeIterator> newparamNodes = effect->GetNode("profile_COMMON")->GetNodes("newparam");

      while(newparamNodes->HasNext())
      {      
        csRef<iDocumentNode> newparam = newparamNodes->Next();

        if(!foundSurface)
        {        
          csRef<iDocumentNode> surfaceNode = newparam->GetNode("surface");

          if(surfaceNode.IsValid())
          {
            csString paramnodeInitValue(surfaceNode->GetNode("init_from")->GetContentsValue());
            csString textureID(texture->GetAttributeValue("id"));

            if(textureID == paramnodeInitValue)
            {
              foundSurface = true;
              surfaceID = newparam->GetAttributeValue("sid");            
            }
          }
        }
        if(foundSurface)
        {
          csString paramNodeSourceValue;
          if(newparam->GetNode("sampler2D").IsValid())
          {
            paramNodeSourceValue = newparam->GetNode("sampler2D")->GetNode("source")->GetContentsValue();
            if(surfaceID == paramNodeSourceValue)
            {
              csString samplerSID;
              samplerSID = newparam->GetAttributeValue("sid");
              csString normalmapTexture;
              csRef<iDocumentNode> normalmapNode = effect->GetNode("profile_COMMON")->GetNode("technique")->GetNode("extra")->GetNode("technique")->GetNode("bump");
              if(normalmapNode.IsValid())
                normalmapTexture = normalmapNode->GetNode("texture")->GetAttributeValue("texture");
              if(normalmapTexture == samplerSID)
              {
                return true;
              }
            }
          }
        }
      }
    }
    return false;
  }


  void csColladaConvertor::CreateShaderVarNodes(csRef<iDocumentNode> newMaterial, csRef<iDocumentNode> effect)
  {
    csRef<iDocumentNodeIterator> surfaceParamNodes = effect->GetNode("profile_COMMON")->GetNodes("newparam");
    while(surfaceParamNodes->HasNext())
    {
      csRef<iDocumentNode> surfaceParamNode = surfaceParamNodes->Next();
      if(!surfaceParamNode->GetNode("surface"))
        continue;

      csString surfaceID(surfaceParamNode->GetAttributeValue("sid"));

      csRef<iDocumentNodeIterator> samplerParamNodes = effect->GetNode("profile_COMMON")->GetNodes("newparam");
      while(samplerParamNodes->HasNext())
      {
        csRef<iDocumentNode> samplerParamNode = samplerParamNodes->Next();

        if(!samplerParamNode->GetNode("sampler2D"))
          continue;

        csString samplerSource = samplerParamNode->GetNode("sampler2D")->GetNode("source")->GetContentsValue();
        if(surfaceID == samplerSource)
        {
          csString samplerID;
          samplerID = samplerParamNode->GetAttributeValue("sid");

          csString shadervarValue = surfaceParamNode->GetNode("surface")->GetNode("init_from")->GetContentsValue();

          if(HasNormalMap(effect, samplerID))
          {
            CreateShaderVarNode( newMaterial, "tex normal", shadervarValue);
          }

          if(HasHeightMap(effect, samplerID))
          {
            CreateShaderVarNode( newMaterial, "tex height", shadervarValue);
          }

          if(HasSpecMap(effect, samplerID))
          {
            CreateShaderVarNode( newMaterial, "tex specular", shadervarValue);
          }
        }
      }
    }
  }


  bool csColladaConvertor::HasNormalMap(csRef<iDocumentNode> effect, csString samplerID)
  {
    csString normalmapString;
    csRef<iDocumentNode> normalmapNode = effect->GetNode("profile_COMMON")->GetNode("technique")->GetNode("extra")->GetNode("technique")->GetNode("bump");
    if(normalmapNode.IsValid())
      normalmapString = normalmapNode->GetNode("texture")->GetAttributeValue("texture");
    if(normalmapString == samplerID)
      return true;

    return false;
  }


  bool csColladaConvertor::HasHeightMap(csRef<iDocumentNode> effect, csString samplerID)
  {
    csString heightmapString;
    csRef<iDocumentNode> heightmapNode = effect->GetNode("profile_COMMON")->GetNode("technique")->GetNode("extra")->GetNode("technique")->GetNode("displacement");
    if(heightmapNode.IsValid())
      heightmapString = heightmapNode->GetNode("texture")->GetAttributeValue("texture");
    if(heightmapString == samplerID)
      return true;

    return false;
  }


  bool csColladaConvertor::HasSpecMap(csRef<iDocumentNode> effect, csString samplerID)
  {
    csString specmapString;
    csRef<iDocumentNode> specmapNode = effect->GetNode("profile_COMMON")->GetNode("technique")->GetNode("extra")->GetNode("technique")->GetNode("shininess");
    if(specmapNode.IsValid())
      specmapString = specmapNode->GetNode("texture")->GetAttributeValue("texture");
    if(specmapString == samplerID)
      return true;
    return false;
  }


  void csColladaConvertor::CreateShaderVarNode(csRef<iDocumentNode> newMaterial, csString name, csString value)
  {
    csRef<iDocumentNode> shadervar = newMaterial->CreateNodeBefore(CS_NODE_ELEMENT);
    shadervar->SetValue("shadervar");

    shadervar->SetAttribute("type", "texture");
    shadervar->SetAttribute("name", name);

    csRef<iDocumentNode> shadervarText = shadervar->CreateNodeBefore(CS_NODE_TEXT);
    shadervarText->SetValue(value);
  }

}
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)
