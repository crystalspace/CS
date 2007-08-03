/*
		Copyright	(C)	2007 by	Scott	Johnson

		This application is	free software; you can redistribute	it and/or
		modify it	under	the	terms	of the GNU Library General Public
		License	as published by	the	Free Software	Foundation;	either
		version	2	of the License,	or (at your	option)	any	later	version.

		This application is	distributed	in the hope	that it	will be	useful,
		but	WITHOUT	ANY	WARRANTY;	without	even the implied warranty	of
		MERCHANTABILITY	or FITNESS FOR A PARTICULAR	PURPOSE.	See	the	GNU
		Library	General	Public License for more	details.

		You	should have	received a copy	of the GNU Library General Public
		License	along	with this	application; if	not, write to	the	Free
		Software Foundation, Inc., 675 Mass	Ave, Cambridge,	MA 02139,	USA.
*/

// =============== Auxiliary Classes for csColladaConvertor ===============

#include "csColladaConvertor.h"
#include "ivaria/collada.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csgeom/triangulate3d.h"
#include "csgeom/trimesh.h"

#ifndef	_CS_COLLADA_CLASSES_H_
#define	_CS_COLLADA_CLASSES_H_

CS_PLUGIN_NAMESPACE_BEGIN (ColladaConvertor)
{

class	csColladaAccessor	{
	private:
		csRef<iDocumentNode> sourceElement;
		csColladaConvertor *parent;
		int	stride;
		int	count;

		// the offset of the first element in this accessor 
		// from the first (global) accessor element.  this tells us
		// where in the listing of indices that this accessor begins
		// at. since the crystal space listing of vertices is just a 
		// long list, we need to determine which section starts vertex
		// positions, which starts vertex normals, etc...
		int accessorOffset; 

		iStringArray *accessorNames;

	public:
		csColladaAccessor();
		csColladaAccessor(iDocumentNode* source, csColladaConvertor* parent);

		int GetStride() { return stride; }
		int GetCount() { return count; }
		int GetOffset() { return accessorOffset; }
		iStringArray* GetAccessorNames() { return accessorNames; }
		csColladaConvertor* GetParent() { return parent; }
		csRef<iDocumentNode> GetSourceElement() { return sourceElement; }
		
		void SetOffset(int newOffset);
		bool Process(iDocumentNode* src);
		const char* Get(int index);

}; /* End of class csColladaAccessor */

class	csColladaMesh	{

	private:
		csColladaConvertor* parent;
		csArray<csVector3> vertices; // a list of vertex components (x, y, z)
		int numberOfVertices;  // number of vertices in the mesh
		int numVertexElements; // the number of vertex components (3* numberOfVertices)
		iString	*name;  // what the polygon is actually called
		iString *positionId;    // the id of the position array
		iString *normalId;		// the id of the normal array
		iString *vertexId;		// the id of the vertices
		csRef<iDocumentNode> meshElement;
		csArray<csColladaAccessor> accessorList; // a list of accessors
		csColladaAccessor	vAccess; // the accessor corresponding to the vertices
		csColladaNumericType vType;
		iString *pluginType;  // the type of plugin we're using (default = genmeshfact)
		size_t vertexOffset, normalOffset;  // the offsets of the normals and positions

		csTriangleMesh* triangles;

		/**	\brief Find	the	next numeric array element
		 *
		 * Finds a child numeric array node	within a given node.	This function	will
		 * attempt to	find one of:
		 *	-	<float_array>
		 *	-	<int_array>
		 * within	the	current	node.
		 *
		 * \param	node The current node	to be	searched.	Typically, this	will be	a	<source> node.
		 *						 This	restriction	is not absolutely	enforced,	but	this function	will not 
		 *						 return	anything unless	this is	a	<source> node, since no	other	nodes	
		 *						 will	contain	one	of the aforementioned	array	nodes.
		 * \returns		 The array node, as	a	smart	pointer	to an	iDocumentNode, if	successful;
		 *						 otherwise,	an invalid smart pointer.	 Use csRef<>::IsValid()	to check for
		 *						 validity.
		 */
		csRef<iDocumentNode> FindNumericArray(const	csRef<iDocumentNode>&	node);

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
		void RetrieveArray(iDocumentNode* numericArrayElement, csArray<csVector3>& storeIn); 

		/** \brief Retrieves the vertex and normal offsets
		 *
		 * Retrives values for vertexOffset and normalOffset, given a specific
		 * element.  This function also retrieves the names of the vertex (position)
		 * and normal id values.
		 *
		 * @param element An element with which to retrieve the vertex position and normal
		 *                array with respect to.
		 * @notes This function does NOT retrieve the positionId.
		 */
		void RetrieveOffsets(iDocumentNode* element);

		/** \brief Sets the normals in the vertex array
		 * 
		 * Sets all of the normX, normY, and normZ values in the vertex array.
		 */
		void SetNormals();

	public:
		csColladaMesh(iDocumentNode* element, csColladaConvertor* parent);
		~csColladaMesh();

		const csArray<csVector3>& GetVertices() { return vertices; }
		int GetNumVertexElements() { return numVertexElements; }
		int GetNumberOfVertices() { return numberOfVertices; }
		csColladaNumericType GetVertexType() { return vType; }
		iString* GetName() { return name; }
		iString* GetPositionID() { return positionId; }
		iString* GetPluginType() { return pluginType; }
		int GetNumInputElements(iDocumentNode* element);
		csTriangleMesh* GetTriangleMesh() { return triangles; } 
		csRef<iDocumentNode> GetMeshElement() { return meshElement; }
		const csArray<csVector3>& Process(iDocumentNode* element);

	private:
		/** @todo Make sure these will work for an arbitrary number of
		 *        <p> elements.
		 */       
		void ProcessTriangles(iDocumentNode* trianglesElement);
		void ProcessTriStrips(iDocumentNode* tristripsElement);
		void ProcessTriFans(iDocumentNode* trifansElement);
		void ProcessPolygons(iDocumentNode* polygonsElement);
		void ProcessPolyList(iDocumentNode* polylistElement);

}; /* End of class csColladaMesh */

class csColladaEffectProfile {
	private:
		csColladaEffectProfileType profileType;
		csRef<iDocumentNode> element;
		csColladaConvertor* parent;

	public:
		csColladaEffectProfile(iDocumentNode* profileElement, csColladaConvertor* parentObj);

		bool Process(iDocumentNode* profileElement);
		void SetProfileType(csColladaEffectProfileType newType);

		csColladaEffectProfileType GetProfileType();

}; /* End of class csColladaEffectProfile */

class csColladaEffect {
	private:
		csArray<csColladaEffectProfile> profiles;
		csRef<iDocumentNode> element;
		csColladaConvertor* parent;
		iString* id;

	public:
		csColladaEffect(iDocumentNode* effectElement, csColladaConvertor* parentObj);
		bool Process(iDocumentNode* effectElement);
		bool operator==(const csColladaEffect& compEffect);

}; /* End of class csColladaEffect */

class csColladaMaterial {
	private:
		iString *id;
		csColladaEffect* instanceEffect;
		csColladaConvertor *parent;

	public:
		csColladaMaterial(csColladaConvertor *parentEl);
		~csColladaMaterial();

		void SetID(const char* newId);
		void SetInstanceEffect(iDocumentNode* effectNode);
		void SetInstanceEffect(csColladaEffect *newInstEffect);

		iString* GetID() { return id; }
		csColladaEffect* GetInstanceEffect() { return instanceEffect; }

		bool operator==(const csColladaMaterial& comp);

}; /* End of class csColladaMaterial */

} /* End of ColladaConvertor namespace */
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)

#endif