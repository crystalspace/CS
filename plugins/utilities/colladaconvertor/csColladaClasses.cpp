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
#include "csColladaClasses.h"
#include "csColladaConvertor.h"

#include "csutil/custom_new_disable.h"

// Standard Headers
/// @todo Remove STL dependencies
#include <string>
#include <sstream>

#include "csutil/custom_new_enable.h"

using std::string;
using std::stringstream;
using std::skipws;

CS_PLUGIN_NAMESPACE_BEGIN (ColladaConvertor)
{
  // =============== Auxiliary Class: csColladaAccessor ===============
  csColladaAccessor::csColladaAccessor()
  {
    stride = 0;
    count = 0;
    accessorNames = 0;
  }

  csColladaAccessor::csColladaAccessor(iDocumentNode* source, csColladaConvertor* par)
  {
    stride = 0;
    count = 0;
    parent = par;
    Process(source);
  }

  csColladaAccessor::~csColladaAccessor()
  {
    delete accessorNames;
  }

  bool csColladaAccessor::Process(iDocumentNode* src)
  {
    csRef<iDocumentNode> currentTechniqueElement;
    csRef<iDocumentNode> currentAccessorElement;
    csRef<iDocumentNode> currentParamsElement;
    csRef<iDocumentNodeIterator> paramsIterator;

    sourceElement = src;

    if (!sourceElement.IsValid() && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_ERROR, "Specified source element is invalid.");
      return 0;
    }

    currentTechniqueElement = sourceElement->GetNode("technique_common");
    if (!currentTechniqueElement.IsValid() && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_ERROR, "Unable to find technique_common element in given source element.");
      return 0;
    }

    // technique_common must contain exactly ONE accessor element, by the COLLADA
    // specification
    currentAccessorElement = currentTechniqueElement->GetNode("accessor");
    if (!currentAccessorElement.IsValid() && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire accessor element from technique_common element.");
      return 0;
    }

    // retrieve size from accessor element
    count = currentAccessorElement->GetAttributeValueAsInt("count");
    if (count == 0 && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire number of parameters in accessor element");
      return 0;
    }

    stride = currentAccessorElement->GetAttributeValueAsInt("stride");
    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_WARNING, "Stride value of accessor: %d", stride);
    }

    if (stride == 0 && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire stride parameter in accessor element.");
      return 0;
    }

    paramsIterator = currentAccessorElement->GetNodes("param");

    if (!paramsIterator.IsValid() && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire an iterator of params elements.");
      return 0;
    }

    accessorNames = new csStringArray();

    while(paramsIterator->HasNext())
    {
      currentParamsElement = paramsIterator->Next();

      // retrieve name of current element
      scfString currentName(currentParamsElement->GetAttributeValue("name"));
      currentName.Downcase();
      accessorNames->Push(currentName);
    }

    return 1;
  }

  void csColladaAccessor::SetAccessorName(size_t x, const char* str)
  {
    if (x >= accessorNames->GetSize())
    {
      return;
    }

    accessorNames->Put(x, str);
  }

  const char* csColladaAccessor::Get(int index)
  {
    return accessorNames->Get(index);
  }

  void csColladaAccessor::SetOffset(int newOffset)
  {
    accessorOffset = newOffset;
  }

  // =============== Auxiliary Class: csColladaMesh ===============

  csColladaMesh::csColladaMesh(iDocumentNode* element, csColladaConvertor* par, csString plugType)
  {
    meshElement = element;
    pluginType = plugType;
    numberOfVertices = 0;
    numVertexElements = 0;
    normalId = 0;
    vertexId = 0;
    parent = par;
    triangles = new csTriangleMesh();
    materials = 0;
    normalAccessor = 0;
    vertexAccessor = 0;
    Process(meshElement);
  }

  csColladaMesh::~csColladaMesh()
  {
    if (materials != 0)
    {
      delete materials;
    }

    delete triangles;
    if ( vertexAccessor ) {
      delete vertexAccessor;
    }
    if ( normalAccessor ) {
      delete normalAccessor;
    }
  }

  csRef<iDocumentNode> csColladaMesh::FindNumericArray(const csRef<iDocumentNode>& node)
  {
    csRef<iDocumentNode> retVal;

    // search first for a <float_array> element
    retVal = node->GetNode("float_array");
    if (retVal.IsValid())
    {
      vType = CS_COLLADA_FLOAT;
      return retVal;
    }

    // then for a <int_array> element
    else
    {
      retVal = node->GetNode("int_array");
      vType = CS_COLLADA_INTEGER;
      return retVal;
    }
  }

  void csColladaMesh::RetrieveArray(iDocumentNode* sourceElement, csColladaAccessor* accessPtr, csArray<csVector3>& toStore)
  {
    csRef<iDocumentNode> numericArrayElement = FindNumericArray(sourceElement);

    // accessor needs to be defined outside of this method
    //vertexAccessor = new csColladaAccessor(sourceElement, this);
    stringstream conver;
    int countArray = numericArrayElement->GetAttributeValueAsInt("count");
    std::string arrayElement(numericArrayElement->GetContentsValue());

    if (parent->warningsOn)
    {
      // check for string correctness
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Current value of array string: %s", arrayElement.c_str());
    }

    conver.str(arrayElement.c_str());

    //if (vertices != 0)
    if (!toStore.IsEmpty())
    {
      toStore.Empty();
    }

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Assigning values to array...");
    }


    // get the number of accessor strings
    int accessorStride = accessPtr->GetStride();

    float* temp = new float[accessorStride];

    int numItemsInArray = countArray/accessorStride;

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "numItemsInArray: %d", numItemsInArray);
    }

    for (int i = 0; i < numItemsInArray; i++)
    {
      for (int j = 0; j < accessorStride; j++)
      {
        conver >> temp[j];
        if (parent->warningsOn)
        {
          parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Value of temp[%d] is: %f", j, temp[j]);
        }
      }

      csVector3 t;
      t.x = temp[0];
      t.y = temp[1];
      t.z = temp[2];

      toStore.Put(toStore.GetSize(), t);
    }

    delete[] temp;
  }

  void csColladaMesh::RetrieveArray(iDocumentNode* sourceElement, csColladaAccessor* accessPtr, csArray<csVector2>& toStore)
  {
    csRef<iDocumentNode> numericArrayElement = FindNumericArray(sourceElement);

    // accessor needs to be defined outside of this method
    //vertexAccessor = new csColladaAccessor(sourceElement, this);
    stringstream conver;
    int countArray = numericArrayElement->GetAttributeValueAsInt("count");
    std::string arrayElement(numericArrayElement->GetContentsValue());

    if (parent->warningsOn)
    {
      // check for string correctness
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Current value of array string: %s", arrayElement.c_str());
    }

    conver.str(arrayElement.c_str());

    //if (vertices != 0)
    if (!toStore.IsEmpty())
    {
      toStore.Empty();
    }

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Assigning values to array...");
    }


    // get the number of accessor strings
    int accessorStride = accessPtr->GetStride();

    float* temp = new float[accessorStride];

    int numItemsInArray = countArray/accessorStride;

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "numItemsInArray: %d", numItemsInArray);
    }

    for (int i = 0; i < numItemsInArray; i++)
    {
      for (int j = 0; j < accessorStride; j++)
      {
        conver >> temp[j];
        if (parent->warningsOn)
        {
          parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Value of temp[%d] is: %f", j, temp[j]);
        }
      }

      csVector2 t;
      t.x = temp[0];
      t.y = temp[1];

      toStore.Put(toStore.GetSize(), t);
    }

    delete[] temp;
  }


  void csColladaMesh::RetrieveInfo(iDocumentNode *element)
  {
    // find the offset of the normals and vertices
    csRef<iDocumentNodeIterator> inputElements = element->GetNodes("input");
    csRef<iDocumentNode> currentInputElement;
    int currentElementOffset=-1;
    nextElementOffset = -1;
    normalOffset = -1;
    textureOffset = -1;
    vertexOffset = -1;

    bool foundN = false, foundT = false, foundP = false;

    while (inputElements->HasNext())
    {
      currentInputElement = inputElements->Next();
      currentElementOffset = currentInputElement->GetAttributeValueAsInt("offset");
      if ( currentElementOffset > nextElementOffset ) {
        nextElementOffset = currentElementOffset;
      }
      scfString semVal(currentInputElement->GetAttributeValue("semantic"));
      if (semVal.Compare("NORMAL"))
      {
        foundN = true;
        normalId = currentInputElement->GetAttributeValue("source");
        if (normalId.GetAt(0) == '#')
        {
          normalId.DeleteAt(0, 1);
        }

        normalOffset = currentElementOffset;
      }
      else if (semVal.Compare("TEXCOORD"))
      {
        foundT = true;
        textureId = currentInputElement->GetAttributeValue("source");
        if (textureId.GetAt(0) == '#')
        {
          textureId.DeleteAt(0, 1);
        }

        textureOffset = currentElementOffset;
      }

      else if (semVal.Compare("VERTEX"))
      {
        foundP = true;
        vertexId = currentInputElement->GetAttributeValue("source");
        if (vertexId.GetAt(0) == '#')
        {
          vertexId.DeleteAt(0, 1);
        }

        vertexOffset = currentElementOffset;
      }
    }
    nextElementOffset = nextElementOffset + 1;
  }

  int csColladaMesh::GetNumInputElements(iDocumentNode* element)
  {
    csRef<iDocumentNodeIterator> inputElements = element->GetNodes("input");
    csRef<iDocumentNode> currentInputElement;
    int counter = 0;
    while (inputElements->HasNext())
    {
      currentInputElement = inputElements->Next();
      counter++;
    }

    return counter;

  }


  const csArray<csVector3>& csColladaMesh::Process(iDocumentNode* element)
  {
    csRef<iDocumentNode> currentInputElement;
    csRef<iDocumentNodeIterator> inputElements;
    csRef<iDocumentNodeIterator> sourceElements;
    csRef<iDocumentNode> currentNumericArrayElement;
    csRef<iDocumentNode> currentSourceElement;
    csRef<iDocumentNode> currentNormalArrayElement;
    std::string inputElementSemantic = "";
    csString positionSource = "";
    //scfString normalSource = "";

    meshElement = element;

    // a sanity check, in the event someone tries to do something foolish
    numberOfVertices = 0;
    numVertexElements = 0;

    // find <input> child element of currentVerticesElement with
    // attribute semantic="POSITION" and store this element as positionElement
    inputElements = meshElement->GetNode("vertices")->GetNodes("input");
    while (inputElements->HasNext())
    {
      currentInputElement = inputElements->Next();
      inputElementSemantic = currentInputElement->GetAttribute("semantic")->GetValue();
      if (inputElementSemantic.compare("POSITION") == 0)
      {
        positionSource = currentInputElement->GetAttribute("source")->GetValue();
        break;
      }

    }

    // remove '#' from positionSource
    if (!positionSource.IsEmpty())
    {
      positionSource.DeleteAt(0, 1);
    }

    /*
    if (!normalSource.IsEmpty())
    {
    normalSource.DeleteAt(0, 1);
    }
    */
    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Found position semantic.  Source value: %s", positionSource.GetData());
    }

    // find child <source> element of currentMeshElement
    // with id equal to positionSource and store as currentSourceElement
    sourceElements = meshElement->GetNodes("source");
    while (sourceElements->HasNext())
    {
      currentSourceElement = sourceElements->Next();

      name = currentSourceElement->GetParent()->GetParent()->GetAttribute("name")->GetValue();

      csString newId = currentSourceElement->GetAttribute("id")->GetValue();

      // this if statement controls the finding of the position
      // array
      if (newId.Compare(positionSource))
      {
        positionId = newId;
        vertexAccessor = new csColladaAccessor(currentSourceElement, parent);

        // find child numeric array element of currentSourceElement
        // store as currentNumericArrayElement
        currentNumericArrayElement = FindNumericArray(currentSourceElement);

        if (!currentNumericArrayElement.IsValid())
        {
          if (parent->warningsOn)
          {
            parent->Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Unable to find a numeric array element under source node!");
            vertices.Empty();
            return vertices;
          }
        }

        stringstream conver(currentNumericArrayElement->GetAttribute("count")->GetValue());

        conver >> numVertexElements;
        //numVertexElements = numVertexElements/3;
        numberOfVertices = numVertexElements/3;

        if (parent->warningsOn)
        {
          parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Numeric array found.  Type: %s, Number of elements: %d", currentNumericArrayElement->GetValue(), numVertexElements);
        }

        RetrieveArray(currentSourceElement, vertexAccessor, vertices);

        if (positionId.Compare(currentNumericArrayElement->GetParent()->GetAttributeValue("id")))
        {
          for (int i = 0; i < numberOfVertices; i++)
          {
            triangles->AddVertex(vertices[i].x);
            triangles->AddVertex(vertices[i].y);
            triangles->AddVertex(vertices[i].z);
          }
        }
      }

      // we will need another if statement to control the finding of the
      // normal array
    }

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Done.");
    }

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Converting triangle elements to objects...");
    }

    // we need to perform a sanity check, in the event someone forgot to 
    // call ConvertEffects(), otherwise we have the potential for a segfault
    if (parent->materialsList.IsEmpty())
    {
      parent->ConvertEffects();
    }

    /* BEGIN POLYGONS PROCESSING - THIS NEEDS WORK*/
    // test to see if we have any generic polygons
    csRef<iDocumentNodeIterator> polygonsIterator = element->GetNodes("polygons");
    csRef<iDocumentNode> nextPolygonsElement;
    if (polygonsIterator->HasNext() && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Generic polygons detected.  The triangulation method for generic polygons is not yet fully implemented.  Use at your own risk."); 
    }

    while(polygonsIterator->HasNext())
    {
      nextPolygonsElement = polygonsIterator->Next();
      ProcessPolygons(nextPolygonsElement);
    }
    /* END POLYGONS PROCESSING */

    /* BEGIN POLYLIST PROCESSING - THIS NEEDS WORK */
    csRef<iDocumentNodeIterator> polylistElements = element->GetNodes("polylist");
    csRef<iDocumentNode> nextPolylistElement;
    while (polylistElements->HasNext())
    {
      nextPolylistElement = polylistElements->Next();
      ProcessPolyList(nextPolylistElement);
    }
    /* END POLYLIST PROCESSING */

    /* BEGIN TRISTRIPS PROCESSING - THIS NEEDS WORK */
    csRef<iDocumentNodeIterator> tristripsElements = element->GetNodes("tristrips");
    csRef<iDocumentNode> nextTristripsElement;
    while (tristripsElements->HasNext())
    {
      nextTristripsElement = tristripsElements->Next();
      ProcessTriStrips(nextTristripsElement);
    }
    /* END TRISTRIPS PROCESSING */

    /* BEGIN TRIFANS PROCESSING - THIS NEEDS WORK */
    csRef<iDocumentNodeIterator> trifansElements = element->GetNodes("trifans");
    csRef<iDocumentNode> nextTrifansElement;
    while (trifansElements->HasNext())
    {
      nextTrifansElement = trifansElements->Next();
      ProcessTriFans(nextTrifansElement);
    }
    /* END TRIFANS PROCESSING */

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Number of vertices/normals: %d", numberOfVertices);
    }

    /* BEGIN TRIANGLES PROCESSING */
    csRef<iDocumentNodeIterator> triangleMeshes = element->GetNodes("triangles");
    csRef<iDocumentNode> nextTrianglesElement;
    while (triangleMeshes->HasNext())
    {
      nextTrianglesElement = triangleMeshes->Next(); 
      ProcessTriangles(nextTrianglesElement);
    }
    /* END TRIANGLES PROCESSING */

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_WARNING, "Normal id: %s", normalId.GetData());
    }

    if (parent->warningsOn)
    {
      csString appender;
      for (int i = 0; i < (int)normals.GetSize(); i++)
      {
        appender.Append(appender.Format("%f %f %f", normals[i].x, normals[i].y, normals[i].z));
      }

      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Normals data: %s", appender.GetData());
    }

    return vertices;

  }

  void csColladaMesh::RestructureIndices()
  {

    // create a new list of vertex indices
    csArray<csColladaVertexIndex> newIndexList;

    csArray<csColladaVertexIndex>::Iterator indicesIter = polygonIndices.GetIterator();

    while (indicesIter.HasNext())
    {
      csColladaVertexIndex needle = indicesIter.Next();

      int positionFoundAt = -1;
      bool foundMatch = false;
      int counter = 0;

      // search through the newIndexList for a given vertex index
      csArray<csColladaVertexIndex>::Iterator newIndexListIter = newIndexList.GetIterator();

      while (newIndexListIter.HasNext())
      {
        csColladaVertexIndex currentSearchComponent = newIndexListIter.Next();

        if (needle.positionIndex == currentSearchComponent.positionIndex &&
          needle.normalIndex == currentSearchComponent.normalIndex &&
          needle.textureIndex == currentSearchComponent.textureIndex)
        {
          // we've found what we're looking for
          foundMatch = true;
          break;
        }

        else if (needle.positionIndex == currentSearchComponent.positionIndex)
        {
          // we've found a situation where the position index was found, but one of
          // the other two doesn't match up
          positionFoundAt = counter;
        }

        counter++;
      }

      // we've now searched through all of the possible situations 

      // CASE 1: we found a complete match
      if (foundMatch)
      {
        /*
        if (parent->warningsOn)
        {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Case 1 invoked");
        }
        */

        // insert the needle as it is
        newIndexList.Push(needle);
        continue;
      }

      // CASE 2: we found a position that matches, but one of 
      // the other two didn't match
      else if (positionFoundAt != -1)
      {
        /*
        if (parent->warningsOn)
        {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Case 2 invoked");
        }
        */

        // in this case, we need to create a new vertex
        // but it will have the same position index as the 
        // one we found previously
        csVector3 position = vertices[newIndexList[positionFoundAt].positionIndex];
        int positionOfNewVertex = GetNumberOfVertices();
        AddVertex(position);
        needle.positionIndex = positionOfNewVertex;
        newIndexList.Push(needle);
      }

      // CASE 3: we didn't find anything that matched
      else
      {
        /*
        if (parent->warningsOn)
        {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Case 3 invoked");
        }
        */

        // in this case, just re-insert it into the list, as
        // it's a new value
        newIndexList.Push(needle);
      }
    }

    // now, make sure that indices reflects our changes
    polygonIndices.Empty();

    for (int i = 0; i < (int)newIndexList.GetSize(); i++)
    {
      polygonIndices.Push(newIndexList[i]);
    }

  }

  void csColladaMesh::AddVertex(const csVector3& v)
  {
    vertices.Push(v);
    numberOfVertices++;
  }

  bool csColladaMesh::WriteXML(iDocumentNode* xmlDoc)
  { 
    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_WARNING, "Inside WriteXML()");
    }

    csArray<csVector3> vertexArray;
    csArray<csVector3> normalArray;
    csArray<csVector2> textureArray;
    int vertexArraySize = 0;
    //int normalArraySize = 0;

    csRef<iDocumentNode> csTopNode = xmlDoc->GetNode("library");
    if(!csTopNode)
    {
      csTopNode = xmlDoc->GetNode("world");
      if(!csTopNode)
      {
        csTopNode = xmlDoc;
      }
    }    

    // Adding vertices to CS document
    vertexArray = GetVertices();
    if (normalOffset != -1)
    {
      normalArray = GetNormals();
    }

    if (textureOffset != -1)
    {
      textureArray = GetTextures();
    }

    vertexArraySize = GetNumberOfVertices();
    // normalArraySize = GetNumberOfNormals();

    if (parent->warningsOn && vertexArraySize > 0)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Array acquired.  id: %s", GetPositionID().GetData());
    }
    else if (parent->warningsOn && vertexArraySize <= 0)
    {
      parent->Report(CS_REPORTER_SEVERITY_WARNING, "Unable to acquire array.  Array size: %d", vertexArraySize);
      return false;
    }

    // now that we have the vertex array, we need to add it to the
    // crystal space document.
    if (!csTopNode.IsValid() && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire top-level node!");
      return false;
    }

    csRef<iDocumentNode> sourceElement = parent->GetSourceElement(GetPositionID().GetData(), meshElement);
    if (sourceElement == 0)
    {
      parent->Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire source element with id: %s", GetPositionID().GetData());
      return false;
    }

    // create meshfact and plugin (top-level) nodes
    csRef<iDocumentNode> meshFactNode = csTopNode->CreateNodeBefore(CS_NODE_ELEMENT);
    meshFactNode->SetValue("meshfact");
    meshFactNode->SetAttribute("name", GetName());
    csRef<iDocumentNode> pluginNode = meshFactNode->CreateNodeBefore(CS_NODE_ELEMENT);
    pluginNode->SetValue("plugin");
    csRef<iDocumentNode> pluginContents = pluginNode->CreateNodeBefore(CS_NODE_TEXT);
    pluginContents->SetValue(GetPluginType());

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "MeshFact element created.  Creating params element");
    }

    // create the sub params element
    csRef<iDocumentNode> currentCrystalParamsElement = 
      meshFactNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    currentCrystalParamsElement->SetValue("params");

    // output the materials
    if (materials != 0)
    {
      csRef<iDocumentNode> materialNode = currentCrystalParamsElement->CreateNodeBefore(CS_NODE_ELEMENT);
      materialNode->SetValue("material");
      csRef<iDocumentNode> materialContents = materialNode->CreateNodeBefore(CS_NODE_TEXT);
      materialContents->SetValue(GetMaterialPointer()->GetName());
    }

    int counter = 0;
    csRef<iDocumentNode> currentCrystalVElement;

    //int accessorArraySize = 0;
    csColladaAccessor *vertAccess = GetVertexAccessor();
    //csColladaAccessor *normAccess = GetNormalAccessor();
    //csArray<int> normalInds = GetNormalIndices();

    while (counter < vertexArraySize)
    {
      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Current values: counter = %d, vertexArraySize = %d", counter, vertexArraySize);
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Adding the following vertex: %f, %f, %f", vertexArray[counter].x, vertexArray[counter].y, vertexArray[counter].z);
      }

      /// @todo: Add normal information here.
      /* WRITE OUT VERTICES */
      scfString formatter;

      // positions
      currentCrystalVElement = currentCrystalParamsElement->CreateNodeBefore(CS_NODE_ELEMENT);
      currentCrystalVElement->SetValue("v");
      formatter.Format("%f", vertexArray[counter].x);
      currentCrystalVElement->SetAttribute(vertAccess->Get(0), formatter.GetData());
      formatter.Format("%f", vertexArray[counter].y);
      currentCrystalVElement->SetAttribute(vertAccess->Get(1), formatter.GetData());
      formatter.Format("%f", vertexArray[counter].z);
      currentCrystalVElement->SetAttribute(vertAccess->Get(2), formatter.GetData());

      // normals
      if (normalOffset != -1)
      {
        formatter.Format("%f", normalArray[counter].x);
        currentCrystalVElement->SetAttribute("nx", formatter.GetData());
        formatter.Format("%f", normalArray[counter].y);
        currentCrystalVElement->SetAttribute("ny", formatter.GetData());
        formatter.Format("%f", normalArray[counter].z);
        currentCrystalVElement->SetAttribute("nz", formatter.GetData());
      }

      // texture coordinates
      if (textureOffset != -1)
      {
        formatter.Format("%f", textureArray[counter].x);
        currentCrystalVElement->SetAttribute("u", formatter.GetData());
        formatter.Format("%f", textureArray[counter].y);
        currentCrystalVElement->SetAttribute("v", formatter.GetData());
      }


      /* DONE WRITING OUT VERTICES */

      counter++;
    }

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Done adding vertices.");
    }

    // now, output the triangles
    csTriangleMesh* currentTriMesh = GetTriangleMesh();

    size_t triCount = currentTriMesh->GetTriangleCount();
    csTriangle* tris = currentTriMesh->GetTriangles();
    size_t triCounter = 0;
    csTriangle* currentTri;

    while (triCounter < triCount)
    {
      currentTri = &tris[triCounter];

      // create a t element
      csRef<iDocumentNode> currentTElement = currentCrystalParamsElement->CreateNodeBefore(CS_NODE_ELEMENT);

      if (!currentTElement.IsValid())
      {
        if (parent->warningsOn)
        {
          parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Unable to add triangle element!");
        }

        return false;
      }

      currentTElement->SetValue("t");

      // get the vertices associated with this triangle
      currentTElement->SetAttributeAsInt("v1", currentTri->a);
      currentTElement->SetAttributeAsInt("v2", currentTri->b);
      currentTElement->SetAttributeAsInt("v3", currentTri->c); 

      triCounter++;
    }

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Done adding triangles");
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Invalidating document nodes...");
    }

    meshFactNode.Invalidate();
    currentCrystalParamsElement.Invalidate();

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Done");
    }

    return true;
  }

  void csColladaMesh::ProcessTriFans(iDocumentNode* trifansElement)
  {
    csRef<iDocumentNode> nextPElement;
    csRef<iDocumentNodeIterator> pElementIterator = trifansElement->GetNodes("p");

    // get number of triangle fans in the mesh
    //int numTriFans = trifansElement->GetAttributeValueAsInt("count");
    //int numberTriangles = numTriStrips * 

    csString mat = trifansElement->GetAttributeValue("material");
    materials = parent->FindMaterial(mat.GetData());
    //delete mat;  

    while (pElementIterator->HasNext())
    {
      // get next p element
      //nextPElement = trianglesElement->GetNode("p");
      nextPElement = pElementIterator->Next();

      if (!nextPElement.IsValid())
      {
        if (parent->warningsOn)
        {
          parent->Report(CS_REPORTER_SEVERITY_WARNING, "Unable to access <p> element of <trifans> element.");
        }
        //vertices.Empty();
        //return vertices;
      }

      // find the offset of the normals and vertices
      /// @todo Clean this up, so that it doesn't rely on 
      ///       an external counter like this.
      int counter = 0;
      csRef<iDocumentNodeIterator> nodeIter = trifansElement->GetNodes("input");
      while (nodeIter->HasNext())
      {
        nodeIter->Next();
        counter++;
      }

      RetrieveInfo(trifansElement);

      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex offset is: %d, Normal offset is: %d", vertexOffset, normalOffset);
      }

      stringstream vertexConvertor(nextPElement->GetContentsValue());

      //int triCounter = 0;

      //int *linearList = new int[numTris * counter*3];
      csArray<int> linearList;

      //for (int x = 0; x < 3*numTris*counter; x++)
      int tempInt;
      while (vertexConvertor >> tempInt)
      {
        //vertexConvertor >> linearList[x];
        linearList.Push(tempInt);
      }

      // now, the number of triangles will be equal to the number of 
      // vertices in the tristrips mesh, minus two (the starting vertices)
      int listSize = (int)linearList.GetSize();
      int vertexIndex = 0;
      int vertex1 = linearList[(vertexIndex*nextElementOffset)+vertexOffset];
      vertexIndex++;
      int vertex2 = linearList[(vertexIndex*nextElementOffset)+vertexOffset];
      vertexIndex++;
      int vertex3 = linearList[(vertexIndex*nextElementOffset)+vertexOffset];
      vertexIndex++;

      while ((nextElementOffset*vertexIndex) < listSize)
      {
        triangles->AddTriangle(vertex1, vertex2, vertex3);
        // for fans, vertex1 always stays the same
        vertex2 = vertex3;
        vertex3 = linearList[(vertexIndex*nextElementOffset)+vertexOffset];
        vertexIndex++;
      }

      linearList.Empty();
    }
  }

  void csColladaMesh::ProcessPolyList(iDocumentNode* polylistElement)
  {
    // polylists will have a single <p> element
    // and a single vcount element
    int polyCount = polylistElement->GetAttributeValueAsInt("count");

    // sample the vcount element
    csArray<int> vCountElement;

    csString mat = polylistElement->GetAttributeValue("material");
    materials = parent->FindMaterial(mat.GetData());
    //delete mat;

    stringstream polylistConv(polylistElement->GetNode("vcount")->GetContentsValue());

    int tempInt;
    while (polylistConv >> tempInt)
    {
      vCountElement.Push(tempInt);
    }

    polylistConv << polylistElement->GetNode("p")->GetContentsValue();

    int counter = 0;
    while (counter < polyCount)
    {
      CS::Geometry::csContour3 polygonalContour;
      int numVertsInPoly = vCountElement[counter];
      int vertIndex = 0;
      while (vertIndex < numVertsInPoly)
      {
        polylistConv >> vertIndex;
        //vertIndex = vertIndex*3;
        float xVal = vertices[vertIndex].x;
        float yVal = vertices[vertIndex].y;
        float zVal = vertices[vertIndex].z;
        polygonalContour.Push(csVector3(xVal, yVal, zVal));
      }

      // now, this part doesn't work yet...
      csTriangleMesh resultMesh;
      //CS::Geom::Triangulate3D::Process(polygonalContour, resultMesh, iReporter);
      triangles->AddTriangleMesh(resultMesh);

      counter++;
    }
  }

  void csColladaMesh::ProcessPolygons(iDocumentNode* polygonsElement)
  {
    //csRef<iDocumentNode> nextPolygon;
    csRef<iDocumentNode> nextPolyPElement;
    csRef<iDocumentNodeIterator> polyPIterator;
    CS::Geometry::csContour3 polygonalContour;
    int tempVertIndex;
    csArray<int> vertexIndexArrayPolys;

    csString mat = polygonsElement->GetAttributeValue("material");
    materials = parent->FindMaterial(mat.GetData());
    //delete mat;

    polyPIterator = polygonsElement->GetNodes("p");

    // we'll also need one for the <ph> elements
    while (polyPIterator->HasNext())
    {
      nextPolyPElement = polyPIterator->Next();
      stringstream polyConv(nextPolyPElement->GetContentsValue()); 
      while (polyConv >> tempVertIndex)
      {
        vertexIndexArrayPolys.Push(tempVertIndex);
      }

      for (size_t i = 0; i < vertexIndexArrayPolys.GetSize(); i++)
      {
        polygonalContour.Push(csVector3(vertices[i].x, vertices[i].y, vertices[i].z));
      }

      // now, triangulate it.  this is the part that isn't working
      csTriangleMesh result;
      //CS::Geom::Triangulate3D::Process(polygonalContour, result, iReporter*);
      triangles->AddTriangleMesh(result); // add to csTriangleMesh
    }

  }

  void csColladaMesh::ProcessTriStrips(iDocumentNode* tristripsElement)
  {
    csRef<iDocumentNode> nextPElement;
    csRef<iDocumentNodeIterator> pElementIterator = tristripsElement->GetNodes("p");

    // get number of triangle strips in the mesh
    //int numTriStrips = tristripsElement->GetAttributeValueAsInt("count");
    //int numberTriangles = numTriStrips * 

    csString mat = tristripsElement->GetAttributeValue("material");
    materials = parent->FindMaterial(mat.GetData());
    delete mat;

    while (pElementIterator->HasNext())
    {
      // get next p element
      //nextPElement = trianglesElement->GetNode("p");
      nextPElement = pElementIterator->Next();

      if (!nextPElement.IsValid())
      {
        if (parent->warningsOn)
        {
          parent->Report(CS_REPORTER_SEVERITY_WARNING, "Unable to access <p> element of <tristrips> element.");
        }
        //vertices.Empty();
        //return vertices;
      }

      RetrieveInfo(tristripsElement);

      int counter = GetNumInputElements(tristripsElement);

      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex offset is: %d, Normal offset is: %d", vertexOffset, normalOffset);
      }

      stringstream vertexConvertor(nextPElement->GetContentsValue());

      //int triCounter = 0;

      //int *linearList = new int[numTris * counter*3];
      csArray<int> linearList;

      //for (int x = 0; x < 3*numTris*counter; x++)
      int tempInt;
      while (vertexConvertor >> tempInt)
      {
        //vertexConvertor >> linearList[x];
        linearList.Push(tempInt);
      }

      // now, the number of triangles will be equal to the number of 
      // vertices in the tristrips mesh, minus two (the starting vertices)
      int listSize = (int)linearList.GetSize();
      int vCount = 0;
      int vertex1 = linearList[(vCount*counter)+vertexOffset];
      vCount++;
      int vertex2 = linearList[(vCount*counter)+vertexOffset];
      vCount++;
      int vertex3 = linearList[(vCount*counter)+vertexOffset];
      vCount++;

      while ((counter*vCount) < listSize)
      {
        triangles->AddTriangle(vertex1, vertex2, vertex3);
        vertex1 = vertex2;
        vertex2 = vertex3;
        vertex3 = linearList[(vCount*counter)+vertexOffset];
        vCount++;
      }

      linearList.Empty();
    }

  }

  void csColladaMesh::ProcessTriangles(iDocumentNode* trianglesElement)
  {
    csRef<iDocumentNode> nextPElement;
    csRef<iDocumentNodeIterator> pElementIterator = trianglesElement->GetNodes("p");

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Initiating processing of triangles");
    }

    // get number of triangles in the mesh
    int numTris = trianglesElement->GetAttributeValueAsInt("count");

    csString mat = trianglesElement->GetAttributeValue("material");

    if (!mat.IsEmpty())
    {
      materials = parent->FindMaterial(mat);

      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_WARNING, "Materials parameter successfully set.");
      }
    }

    //delete mat;


    while (pElementIterator->HasNext())
    {
      // get next p element
      //nextPElement = trianglesElement->GetNode("p");
      nextPElement = pElementIterator->Next();

      if (!nextPElement.IsValid())
      {
        if (parent->warningsOn)
        {
          parent->Report(CS_REPORTER_SEVERITY_WARNING, "Unable to access <p> element of <triangles> element.");
        }
        //vertices.Empty();
        //return vertices;
      }

      // find the offset of the normals and vertices
      RetrieveInfo(trianglesElement);

      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex offset is: %d, Normal offset is: %d, Texture offset is: %d", vertexOffset, normalOffset, textureOffset);
      }

      stringstream vertexConvertor(nextPElement->GetContentsValue());

      int triCounter = 0;

      int counter = GetNumInputElements(trianglesElement);

      // linearlist previously held the vertex indexes for the triangle mesh
      int *linearList = new int[numTris * counter*3];

      for (int x = 0; x < 3*numTris*counter; x++)
      {
        vertexConvertor >> linearList[x];
      }

      // now, convert them to vertex indexes
      for (int x = 0; x < 3*numTris; x++)
      {
        csColladaVertexIndex tempIndex;
        tempIndex.positionIndex = linearList[counter*x+vertexOffset];

        if (normalOffset != -1)
        {
          tempIndex.normalIndex = linearList[counter*x+normalOffset];
        }
        else 
        {
          tempIndex.normalIndex = -1;
        }

        if (textureOffset != -1)
        {
          tempIndex.textureIndex = linearList[counter*x+textureOffset];
        }
        else
        {
          tempIndex.textureIndex = -1;
        }

        polygonIndices.Push(tempIndex);
      }

      delete[] linearList;

      RetrieveOtherData();

      // let's make sure RestructureVertices works...
      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex index list before restructure:");
        for (int i = 0; i < (int)polygonIndices.GetSize(); i++)
        {
          parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Position Index: %d, Normal Index: %d, Texture Index: %d",
            polygonIndices[i].positionIndex,
            polygonIndices[i].normalIndex,
            polygonIndices[i].textureIndex);
        }
      }

      RestructureIndices();

      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex index list after restructure:");
        for (int i = 0; i < (int)polygonIndices.GetSize(); i++)
        {
          parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Position Index: %d, Normal Index: %d, Texture Index: %d",
            polygonIndices[i].positionIndex,
            polygonIndices[i].normalIndex,
            polygonIndices[i].textureIndex);
        }
      }

      // now, indices are restructured, we need to restructure the
      // normal and texture arrays
      RestructureArrays();

      // ok, so let's make sure that the arrays are all accounted for
      if (parent->warningsOn)
      {
        for (int i = 0; i < numberOfVertices; i++)
        {
          if (textureOffset != -1)
          {
            parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex position: (%f, %f, %f), Vertex Normal: (%f, %f, %f), Vertex Texture Coords: (%f, %f)",
              vertices[i].x, vertices[i].y, vertices[i].z, normals[i].x, normals[i].y, normals[i].z, textures[i].x, textures[i].y);
          }
        }
      }

      while (triCounter < numTris)
      {
        csColladaVertexIndex vertex1, vertex2, vertex3;
        vertex1 = polygonIndices[3*triCounter];
        vertex2 = polygonIndices[3*triCounter+1];
        vertex3 = polygonIndices[3*triCounter+2];

        triangles->AddTriangle(vertex1.positionIndex, vertex2.positionIndex, vertex3.positionIndex);

        triCounter++;
      }

    }

    if (parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Done processing triangles");
    }
  }

  void csColladaMesh::RestructureArrays()
  {
    // first, restructure normals (if they exist)
    csArray<csVector3> newNormalArray;
    csArray<csVector2> newTextureArray;

    // let's verify the size of textures
    /*
    if (parent->warningsOn)
    {
    parent->Report(CS_REPORTER_SEVERITY_WARNING, "Size of texture array: %d", textures.GetSize());
    }
    */

    // for each of the vertices
    for (int i = 0; i < numberOfVertices; i++)
    {
      // find the normal index which corresponds to it
      // by searching through the indices array
      for (int x = 0; x < (int)polygonIndices.GetSize(); x++)
      {
        if (polygonIndices[x].positionIndex == i)
        {
          if (polygonIndices[x].normalIndex != -1)
          {
            newNormalArray.Push(normals[polygonIndices[x].normalIndex]);
          }

          if (polygonIndices[x].textureIndex != -1)
          {
            csVector2 vec = textures[polygonIndices[x].textureIndex];
            newTextureArray.Push(vec);
            //newTextureArray.Push(textures[polygonIndices[x].textureIndex]);
          }
          break;
        }
      }
    }

    normals = newNormalArray;
    textures = newTextureArray;
  }

  void csColladaMesh::RetrieveOtherData()
  {
    if (normalOffset != -1)
    {
      // now, we need to retrieve the normals (if there are normals)
      csRef<iDocumentNode> sourceNode = parent->GetSourceElement(normalId.GetData(), meshElement);
      normalAccessor = new csColladaAccessor(sourceNode, parent);
      //csRef<iDocumentNode> normalsNumericArray = FindNumericArray(sourceNode);
      RetrieveArray(sourceNode, normalAccessor, normals);

      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Normal array acquired.");
      }
    }

    // and finally, the textures (if they exist)
    if (textureOffset != -1)
    {
      csRef<iDocumentNode> texSourceNode = parent->GetSourceElement(textureId.GetData(), meshElement);
      textureAccessor = new csColladaAccessor(texSourceNode, parent);
      //csRef<iDocumentNode> textureNumArray = FindNumericArray(texSourceNode);
      RetrieveArray(texSourceNode, textureAccessor, textures);
      if (parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Texture array acquired.");
      }

      // for textures, the default accessor names are 's' and 't'
      // which doesn't work for our purposes
      textureAccessor->SetAccessorName(0, "u");
      textureAccessor->SetAccessorName(1, "v");
    }
  }

  // =============== Auxiliary Class: csColladaEffectProfile ===============

  csColladaEffectProfile::csColladaEffectProfile(iDocumentNode* profileElement, csColladaConvertor* parentObj,
    iDocumentNode* matNode)
  {
    parent = parentObj;
    materialNode = matNode;
    Process(profileElement);
  }

  csColladaEffectProfile::csColladaEffectProfile(const csColladaEffectProfile& copy)
  {
    profileType = copy.profileType;
    element = copy.element;
    parent = copy.parent;
    diffuseColor = copy.diffuseColor;
    specularColor = copy.specularColor;
    ambientColor = copy.ambientColor;
    name = copy.name;
  }

  bool csColladaEffectProfile::Process(iDocumentNode* profileElement)
  {
    csRef<iDocumentNodeIterator> techniquesToProcess;
    csRef<iDocumentNode> nextTechnique;
    csRef<iDocumentNodeIterator> passesToProcess;
    csRef<iDocumentNode> nextPass;

    // Parse 'newparams' nodes.
    csRef<iDocumentNodeIterator> newparams = profileElement->GetNodes("newparam");
    while(newparams->HasNext())
    {
      csRef<iDocumentNode> surface = newparams->Next()->GetNode("surface");
      if(!surface)
      {
        continue;
      }

      if(surface->GetNode("init_from"))
      {
        csRef<iDocumentNode> texture = materialNode->CreateNodeBefore(CS_NODE_ELEMENT);
        texture->SetValue("texture");
        csRef<iDocumentNode> textureContents = texture->CreateNodeBefore(CS_NODE_TEXT);
        textureContents->SetValue(surface->GetNode("init_from")->GetContentsValue());
      }
    }

    // grab the listing of techniques that need to be processed
    techniquesToProcess = profileElement->GetNodes("technique");

    while (techniquesToProcess->HasNext())
    {
      nextTechnique = techniquesToProcess->Next();

      // for the time being, we're just going to convert basic
      // materials
      csRef<iDocumentNode> pbNode = nextTechnique->GetNode("phong");
      if (!pbNode.IsValid())
      {
        pbNode = nextTechnique->GetNode("blinn");
      }

      if (!pbNode.IsValid() && parent->warningsOn)
      {
        parent->Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to find a phong or blinn node to convert.");
        return false;
      }

      // set the ambient, specular, and diffuse colors
      csRef<iDocumentNode> colorElement = pbNode->GetNode("diffuse");
      if (colorElement.IsValid() && colorElement->GetNode("color"))
      {
        diffuseColor = csColladaMaterial::StringToColor(colorElement->GetNode("color")->GetContentsValue());
        if (parent->warningsOn)
        {
          parent->Report(CS_REPORTER_SEVERITY_WARNING, "Diffuse color of instance effect should be: %d, %d, %d", diffuseColor.red, diffuseColor.green, diffuseColor.blue);
        }
      }

      colorElement = pbNode->GetNode("specular");

      if (colorElement.IsValid() && colorElement->GetNode("color"))
      {
        specularColor = csColladaMaterial::StringToColor(colorElement->GetNode("color")->GetValue());
      }
      colorElement = pbNode->GetNode("ambient");

      if (colorElement.IsValid() && colorElement->GetNode("color"))
      {
        ambientColor = csColladaMaterial::StringToColor(colorElement->GetNode("color")->GetValue());
      }

    }

    return true;
  }

  void csColladaEffectProfile::SetAmbientColor(csRGBcolor newAmbient)
  {
    ambientColor = newAmbient;
  }

  void csColladaEffectProfile::SetDiffuseColor(csRGBcolor newDiffuse)
  {
    diffuseColor = newDiffuse;
  }

  void csColladaEffectProfile::SetSpecularColor(csRGBcolor newSpecular)
  {
    specularColor = newSpecular;
  }

  void csColladaEffectProfile::SetName(const char* newName)
  {
    name = newName;
  }

  // =============== Auxiliary Class: csColladaEffect ===============

  csColladaEffect::csColladaEffect(iDocumentNode* effectElement, csColladaConvertor* parentObj,
    iDocumentNode* matNode)
  {
    parent = parentObj;
    materialNode = matNode;
    Process(effectElement);
  }

  csColladaEffect::csColladaEffect(const csColladaEffect& copy)
  {
    element = copy.element;

    /*
    for (size_t i = 0; i < copy.GetNumProfiles(); i++)
    {
    profiles.Push(copy.GetProfileByIndex(i));
    }
    */
    profiles = copy.profiles;

    id = copy.id;
    parent = copy.parent;
  }

  csColladaEffectProfile* csColladaEffect::GetProfile(const char* query)
  {
    csColladaEffectProfile* retVal;
    csArray<csColladaEffectProfile>::Iterator iter = profiles.GetIterator();

    while (iter.HasNext())
    {
      retVal = &(iter.Next());
      if (retVal->GetName().Compare(query))
      {
        return retVal;
      }
    }

    retVal = 0;
    return retVal;
  }

  bool csColladaEffect::Process(iDocumentNode* effectElement)
  {
    csRef<iDocumentNodeIterator> profilesToProcess;
    csRef<iDocumentNode> currentProfile;
    csColladaEffectProfile* currentProfileObject;

    element = effectElement;

    // grab the id of this effect element
    id = element->GetAttributeValue("id");

    // find all of the profile_COMMON elements
    profilesToProcess = element->GetNodes("profile_COMMON");
    while (profilesToProcess->HasNext())
    {
      currentProfile = profilesToProcess->Next();
      currentProfileObject = new csColladaEffectProfile(currentProfile, parent, materialNode);
      currentProfileObject->SetName("profile_COMMON");
      profiles.Push((*currentProfileObject));
    }

    // now, we need to report an error message if someone tries
    // to use profile_GLSL or profile_CG, as they're not currently
    // supported
    profilesToProcess = element->GetNodes("profile_CG");
    if (profilesToProcess->HasNext() && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_WARNING, "Warning: A <profile_CG> element was detected in the COLLADA file.  These elements are not currently supported.  Skipping <profile_CG> element conversion.");
    }

    profilesToProcess = element->GetNodes("profile_GLSL");
    if (profilesToProcess->HasNext() && parent->warningsOn)
    {
      parent->Report(CS_REPORTER_SEVERITY_WARNING, "Warning: A <profile_GLSL> element was detected in the COLLADA file.  These elements are not currently supported.  Skipping <profile_GLSL> element conversion.");
    }

    return true;
  }

  bool csColladaEffect::operator ==(const csColladaEffect &compEffect)
  {
    if (id == compEffect.id)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // =============== Auxiliary Class: csColladaMaterial ===============

  csColladaMaterial::csColladaMaterial(csColladaConvertor *parentEl)
  {
    parent = parentEl;
  }

  csColladaMaterial::~csColladaMaterial()
  {
    // empty body
  }

  void csColladaMaterial::SetID(const char* newId)
  {
    id = newId;
  }

  void csColladaMaterial::SetName(const char* newName)
  {
    name = newName;
  }

  void csColladaMaterial::SetInstanceEffect(iDocumentNode *effectNode)
  {
    instanceEffect = new csColladaEffect(effectNode, parent, materialNode);
  }

  void csColladaMaterial::SetInstanceEffect(csColladaEffect *newInstEffect)
  {
    instanceEffect = newInstEffect;
  }

  void csColladaMaterial::SetMaterialNode(iDocumentNode* node)
  {
    materialNode = node;
  }

  bool csColladaMaterial::operator ==(const csColladaMaterial& comp)
  {
    csString compId, thisId;
    thisId = id;
    compId = comp.id;

    if (thisId.Compare(compId))
    {
      return true;
    }

    return false;
  }

  csRGBcolor csColladaMaterial::StringToColor(const char* toConvert)
  {
    csRGBcolor tempColor;
    stringstream colConver(toConvert);

    float tempVal;

    colConver >> tempVal; // red
    tempVal *= 100; // make sure we have a value between 0-255
    tempColor.red = (unsigned char)tempVal;

    colConver >> tempVal; // green
    tempVal *= 100;
    tempColor.green = (unsigned char)tempVal;

    colConver >> tempVal; // blue
    tempVal *= 100;
    tempColor.blue = (unsigned char)tempVal;

    return tempColor;
  }

} /* End of ColladaConvertor namespace */
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)


