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

// =============== Auxiliary Classes for csColladaConvertor ===============

#ifndef _CS_COLLADA_CLASSES_H_
#define _CS_COLLADA_CLASSES_H_

#include "ivaria/collada.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csgeom/triangulate3d.h"
#include "csgeom/trimesh.h"
#include "csgfx/rgbpixel.h"
#include "csutil/cscolor.h"


CS_PLUGIN_NAMESPACE_BEGIN (ColladaConvertor)
{
  class csColladaConvertor;
  class csColladaMesh;
  class csColladaMaterial;

  struct csColladaVertexIndex 
  {
    int positionIndex;
    int normalIndex;
    int textureIndex;
  }; /* End of struct csColladaVertexIndex */

  class csColladaAccessor 
  {
  private:
    csRef<iDocumentNode> sourceElement;
    csColladaConvertor *parent;
    int stride;
    int count;

    // the offset of the first element in this accessor 
    // from the first (global) accessor element.  this tells us
    // where in the listing of indices that this accessor begins
    // at. since the crystal space listing of vertices is just a 
    // long list, we need to determine which section starts vertex
    // positions, which starts vertex normals, etc...
    int accessorOffset; 

    csStringArray *accessorNames;

  public:
    csColladaAccessor();
    csColladaAccessor(iDocumentNode* source, csColladaConvertor* parent);
    virtual ~csColladaAccessor();

    int GetStride() { return stride; }
    int GetCount() { return count; }
    int GetOffset() { return accessorOffset; }
    void SetAccessorName(size_t x, const char* str);
    csStringArray* GetAccessorNames() { return accessorNames; }
    csColladaConvertor* GetParent() { return parent; }
    csRef<iDocumentNode> GetSourceElement() { return sourceElement; }

    void SetOffset(int newOffset);
    bool Process(iDocumentNode* src);
    const char* Get(int index);

  }; /* End of class csColladaAccessor */

  class csColladaMesh 
  {

  private:
    csColladaConvertor* parent;
    csArray<csVector3> vertices; // a list of vertex components (x, y, z)
    csArray<csVector3> normals;  // a list of normal components
    csArray<csVector2> textures; // a list of texture coordinates
    //csArray<int> normalIndices;  // a list of indices into the normals array

    // this is actually sort of a temporary variable
    // it's used to hold the indices for triangles, polygons, etc...
    csArray<csColladaVertexIndex> polygonIndices;

    int numberOfVertices;  // number of vertices in the mesh
    int numVertexElements; // the number of vertex components (3* numberOfVertices)
    csColladaAccessor* vertexAccessor; // the accessor for vertices
    csColladaAccessor* normalAccessor; // the accessor for normals
    csColladaAccessor* textureAccessor; // the accessor for textures
    csString name;  // what the polygon is actually called
    csString positionId;    // the id of the position array
    csString normalId;  // the id of the normal array
    csString vertexId;  // the id of the vertices
    csString textureId;   // the id of the texture coordinates
    csRef<iDocumentNode> meshElement;

    // is this needed?
    //csArray<csColladaAccessor> accessorList; // a list of accessors

    //csColladaAccessor vAccess; // the accessor corresponding to the vertices
    csColladaNumericType vType;
    csString pluginType;  // the type of plugin we're using (default = genmeshfact)

    // the offsets of the normals, positions, and texture coordinates
    int vertexOffset, normalOffset, textureOffset, nextElementOffset ;  

    csColladaMaterial* materials; // the materials applied to the mesh object

    csTriangleMesh* triangles;

    /** \brief Find the next numeric array element
    *
    * Finds a child numeric array node within a given node. This function will
    * attempt to find one of:
    * - <float_array>
    * - <int_array>
    * within the current node.
    *
    * \param node The current node to be searched. Typically, this will be a <source> node.
    *       This restriction is not absolutely enforced, but this function will not 
    *       return anything unless this is a <source> node, since no other nodes 
    *       will contain one of the aforementioned array nodes.
    * \returns   The array node, as a smart pointer to an iDocumentNode, if successful;
    *       otherwise, an invalid smart pointer.  Use csRef<>::IsValid() to check for
    *       validity.
    */
    csRef<iDocumentNode> FindNumericArray(const csRef<iDocumentNode>& node);

    /** \brief Sets the vertex and normal arrays
    * 
    * Retrieves values from the given numeric array element in the COLLADA document
    * and places them in an array for the mesh object.  This also sets the vertices 
    * for the csTriangleMesh object, if it is a vertex position array.
    *
    * @param numericArrayElement A pointer to the XML element which contains the numeric (int
    *                            or float) array.
    * @param storeIn The csArray that we want to use as a destination.
    */
    void RetrieveArray(iDocumentNode* sourceElement, csColladaAccessor* accessPtr, csArray<csVector3>& storeIn);
    void RetrieveArray(iDocumentNode* sourceElement, csColladaAccessor* accessPtr, csArray<csVector2>& storeIn);

    /** \brief Retrieves the vertex and normal information
    *
    * Retrives values for vertexOffset and normalOffset, given a specific
    * element.  This function also retrieves the names of the vertex (position)
    * and normal id values.
    *
    * @param element An element with which to retrieve the vertex position and normal
    *                array with respect to.
    * @notes This function does NOT retrieve the positionId.
    */
    void RetrieveInfo(iDocumentNode* element);

    void RetrieveOtherData();

    void RestructureArrays();

    /** \brief Sets the pointer for the mesh's material list.
    *
    * This is a mutator function for the materials pointer in the
    * class.  Use it to set a pointer to a list of materials on
    * the object.
    */
    void SetMaterialsPointer(csColladaMaterial* matPtr);

  public:
    csColladaMesh(iDocumentNode* element, csColladaConvertor* parent, csString plugType);
    ~csColladaMesh();

    const csArray<csVector3>& GetVertices() { return vertices; }
    const csArray<csVector3>& GetNormals() { return normals; }
    const csArray<csVector2>& GetTextures() { return textures; }
    // const csArray<int>& GetNormalIndices() { return normalIndices; }
    int GetNumVertexElements() { return numVertexElements; }
    int GetNumberOfVertices() { return numberOfVertices; }
    // int GetNumberOfNormals() { return (int)normals.GetSize(); }
    csColladaMaterial* GetMaterialPointer() { return materials; }
    csColladaNumericType GetVertexType() { return vType; }
    csString GetName() { return name; }
    csString GetPositionID() { return positionId; }
    csString GetPluginType() { return pluginType; }
    void AddVertex(const csVector3& v);
    csVector3& GetVertexAt(const int i) { return vertices[i]; }
    int GetNumInputElements(iDocumentNode* element);
    csTriangleMesh* GetTriangleMesh() { return triangles; } 
    csRef<iDocumentNode> GetMeshElement() { return meshElement; }

    csColladaAccessor* GetVertexAccessor() { return vertexAccessor; }
    csColladaAccessor* GetNormalAccessor() { return normalAccessor; }

    bool WriteXML(iDocumentNode* xmlDoc);

    /** \brief Process a COLLADA mesh node and construct a csColladaMesh object
    *
    * Creates a csColladaMesh object from a COLLADA <mesh> element.  COLLADA mesh
    * elements should contain at least one of the following:
    *
    *  - <triangles>
    *  - <trifans>
    *  - <tristrips>
    *  - <polygons> 
    *  - <polylist>
    *
    * Note that <lines> and <linestrips> are not part of this list.  Currently, 
    * lines and linestrips are not considered mesh objects, and are therefore not
    * processed.  Ideally, all COLLADA mesh objects can be added to a csTriangleMesh 
    * object after being triangulated.  Since lines and linestrips cannot be 
    * triangulated, they are not included in this list.
    *
    * @returns A csArray of vertices of the mesh.
    *
    * @param element A pointer to the <mesh> node to be processed
    *
    * @sa CS::Geom::Triangulate3D::Process(CS::Geom::csContour3& polygon, csTriangleMesh& result, iReporter* report2, CS::Geom::csContour3* holes = 0)
    */
    const csArray<csVector3>& Process(iDocumentNode* element);

  private:

    void ProcessTriangles(iDocumentNode* trianglesElement);
    void ProcessTriStrips(iDocumentNode* tristripsElement);
    void ProcessTriFans(iDocumentNode* trifansElement);
    void ProcessPolygons(iDocumentNode* polygonsElement);
    void ProcessPolyList(iDocumentNode* polylistElement);

    /** @brief Restructure the vertices array
    * 
    * This function traverses through the vertex indices of the polygon, and 
    * creates duplicate vertices for those vertices which have a different 
    * normal or texture coordinates.  
    *
    * The reason this even has to be done is because of the way COLLADA files
    * differ from Crystal Space files.  In Crystal Space files, normal and 
    * texture information is local to a vertex, (that is, a <v> element).
    * In COLLADA files, the position, normal, and texture information are
    * all independant.  Normal, position, and texture information are 
    * combined when a polygon is declared.  Thus, a single position could
    * have multiple normals, depending on how the polygon is declared.  
    * In order to simplify this conversion, this system treats vertices as
    * independant if one of: position, normal, or texture coordinates are
    * different.
    *
    * @notes This function is designed to be called ONLY from one of the
    *        following:
    *        - ProcessTriangles(iDocumentNode* trianglesElement)
    *        - ProcessTriStrips(iDocumentNode* tristripsElement)
    *        - ProcessTriFans(iDocumentNode* trifansElement)
    *        - ProcessPolygons(iDocumentNode* polygonsElement)
    *        - ProcessPolyList(iDocumentNode* polylistElement)
    */
    void RestructureIndices();

  }; /* End of class csColladaMesh */

  class csColladaEffectProfile 
  {
  private:
    csColladaEffectProfileType profileType;
    csRef<iDocumentNode> element;
    csRef<iDocumentNode> materialNode;
    csColladaConvertor* parent;
    csRGBcolor diffuseColor, specularColor, ambientColor;
    csString name;

  public:
    csColladaEffectProfile(iDocumentNode* profileElement, csColladaConvertor* parentObj,
      iDocumentNode* matNode);
    csColladaEffectProfile(const csColladaEffectProfile& copy);

    bool Process(iDocumentNode* profileElement);
    void SetProfileType(csColladaEffectProfileType newType);

    csColladaEffectProfileType GetProfileType();

    csRGBcolor GetAmbientColor() { return ambientColor; }
    csRGBcolor GetDiffuseColor() { return diffuseColor; }
    csRGBcolor GetSpecularColor() { return specularColor; }

    csString GetName() { return name; }

    void SetAmbientColor(csRGBcolor newAmbient);
    void SetDiffuseColor(csRGBcolor newDiffuse);
    void SetSpecularColor(csRGBcolor newSpecular);
    void SetName(const char* newName);

  }; /* End of class csColladaEffectProfile */

  class csColladaEffect 
  {
  private:
    csArray<csColladaEffectProfile> profiles;
    csRef<iDocumentNode> element;
    csColladaConvertor* parent;
    csRef<iDocumentNode> materialNode;
    csString id;

  public:
    csColladaEffect(iDocumentNode* effectElement, csColladaConvertor* parentObj, iDocumentNode* matNode);
    csColladaEffect(const csColladaEffect& copy);
    bool Process(iDocumentNode* effectElement);

    //csColladaEffectProfile* GetProfile(const char* query); // probably should be csRef
    csColladaEffectProfile* GetProfile(const char* query);
    csColladaEffectProfile GetProfileByIndex(size_t index) { return profiles.Get(index); }
    size_t GetNumProfiles() { return profiles.GetSize(); }

    bool operator==(const csColladaEffect& compEffect);

  }; /* End of class csColladaEffect */

  class csColladaMaterial 
  {
  private:
    csString id;
    csString name;
    csColladaEffect* instanceEffect;
    csColladaConvertor *parent;
    csRef<iDocumentNode> materialNode;

  public:
    csColladaMaterial(csColladaConvertor *parentEl);
    ~csColladaMaterial();

    void SetID(const char* newId);
    void SetName(const char* newName);
    void SetInstanceEffect(iDocumentNode* effectNode);
    void SetInstanceEffect(csColladaEffect *newInstEffect);
    void SetMaterialNode(iDocumentNode* node);

    csString GetID() { return id; }
    csString GetName() { return name; }
    csColladaEffect* GetInstanceEffect() { return instanceEffect; }

    bool operator==(const csColladaMaterial& comp);

    static csRGBcolor StringToColor(const char* toConvert);

  }; /* End of class csColladaMaterial */

  struct csColladaLight
  {
    csColor colour;
    csColladaLight()
    {
      colour = csColor(0.0f);
    }
  }; /* End of struct csColladaLight */

} /* End of ColladaConvertor namespace */
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)

#endif


