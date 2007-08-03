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

// Standard Headers
#include <string>
#include <sstream>

using std::string;
using std::stringstream;

CS_PLUGIN_NAMESPACE_BEGIN (ColladaConvertor)
{

// =============== Auxiliary Class: csColladaAccessor ===============
csColladaAccessor::csColladaAccessor()
{
	stride = 0;
	count = 0;
}

csColladaAccessor::csColladaAccessor(iDocumentNode* source, csColladaConvertor* par)
{
	parent = par;
	Process(source);
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

	paramsIterator = currentAccessorElement->GetNodes("param");

	if (!paramsIterator.IsValid() && parent->warningsOn)
	{
		parent->Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire an iterator of params elements.");
		return 0;
	}

	accessorNames = new scfStringArray();

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

const char* csColladaAccessor::Get(int index)
{
	return accessorNames->Get(index);
}

void csColladaAccessor::SetOffset(int newOffset)
{
	accessorOffset = newOffset;
}

// =============== Auxiliary Class: csColladaMesh ===============

csColladaMesh::csColladaMesh(iDocumentNode* element, csColladaConvertor* par)
{
	meshElement = element;
	pluginType = new scfString(CS_COLLADA_DEFAULT_MESH_PLUGIN_TYPE);
	numberOfVertices = 0;
	numVertexElements = 0;
	normalId = 0;
	vertexId = 0;
	parent = par;
	triangles = new csTriangleMesh();
	Process(meshElement);
}

csColladaMesh::~csColladaMesh()
{
	// we no longer need this - everything will be converted to
	// floating point values.
	/*
	if (vType == CS_COLLADA_FLOAT)
	{
		delete[] ((float*)vertices);
	}

	else
	{
		delete[] ((int*) vertices);
	}
*/

	parent = 0;
	numVertexElements = 0;
	numberOfVertices = 0;

	delete triangles;
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

void csColladaMesh::RetrieveArray(iDocumentNode* numericArrayElement, csArray<csVector3>& toStore)
{
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


	float tempx, tempy, tempz;
	int numVertsInArray = countArray/3;
/*
	while (conver >> temp)
	{
		toStore.Put(toStore.GetSize(), (float)temp);
	}
*/
	for (int i = 0; i < numVertsInArray; i++)
	{
		conver >> tempx;
		conver >> tempy;
		conver >> tempz;

		csVector3 t;
		t.x = tempx;
		t.y = tempy;
		t.z = tempz;

		toStore.Put(toStore.GetSize(), t);
	}

	if (positionId->Compare(&scfString(numericArrayElement->GetParent()->GetAttributeValue("id"))) == 0)
	{
		for (int i = 0; i < numberOfVertices; i++)
		{
				triangles->AddVertex(toStore[i].x);
				triangles->AddVertex(toStore[i].y);
				triangles->AddVertex(toStore[i].z);
		}
	}
}

void csColladaMesh::RetrieveOffsets(iDocumentNode *element)
{
	// find the offset of the normals and vertices
	csRef<iDocumentNodeIterator> inputElements = element->GetNodes("input");
	csRef<iDocumentNode> currentInputElement;
	int counter = 0;

	while (inputElements->HasNext())
	{
		currentInputElement = inputElements->Next();
		scfString semVal(currentInputElement->GetAttributeValue("semantic"));
		if (semVal.Compare("NORMAL"))
		{
			if (normalId != 0)
			{
				delete normalId;
				normalId = 0;
			}

			normalId = new scfString(currentInputElement->GetAttributeValue("source"));
			if (normalId->GetAt(0) == '#')
			{
				normalId->DeleteAt(0, 1);
			}

			normalOffset = counter;
		}
		else if (semVal.Compare("VERTEX"))
		{
			if (vertexId != 0)
			{
				delete vertexId;
				vertexId = 0;
			}

			vertexId = new scfString(currentInputElement->GetAttributeValue("source"));
			if (vertexId->GetAt(0) == '#')
			{
				vertexId->DeleteAt(0, 1);
			}

			vertexOffset = counter;
		}

		counter++;
	}
	
	vertexOffset = vertexOffset * numberOfVertices;
	normalOffset = normalOffset * numberOfVertices;

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
	std::string inputElementSemantic = "";
	scfString positionSource = "";
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
		
		name = new scfString(currentSourceElement->GetParent()->GetParent()->GetAttribute("id")->GetValue());

		scfString *newId = new scfString(currentSourceElement->GetAttribute("id")->GetValue());

		// this if statement controls the finding of the position
		// array
		if (newId->Compare(&positionSource))
		{
			positionId = newId;

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

			RetrieveArray(currentNumericArrayElement, vertices);
		}
		
		// in the event that we aren't looking at the position array,
		// we need to delete the scfString we created, otherwise we will
		// have a memory leak.
		else 
		{
			delete newId;
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

	/* BEGIN POLYGONS PROCESSING */
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

	/* BEGIN POLYLIST PROCESSING */
	csRef<iDocumentNodeIterator> polylistElements = element->GetNodes("polylist");
	csRef<iDocumentNode> nextPolylistElement;
	while (polylistElements->HasNext())
	{
		nextPolylistElement = polylistElements->Next();
		ProcessPolyList(nextPolylistElement);
	}
	/* END POLYLIST PROCESSING */

	/* BEGIN TRISTRIPS PROCESSING */
	csRef<iDocumentNodeIterator> tristripsElements = element->GetNodes("tristrips");
	csRef<iDocumentNode> nextTristripsElement;
	while (tristripsElements->HasNext())
	{
		nextTristripsElement = tristripsElements->Next();
		ProcessTriStrips(nextTristripsElement);
	}
	/* END TRISTRIPS PROCESSING */

	/* BEGIN TRIFANS PROCESSING */
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

	return vertices;
	
	}

	void csColladaMesh::ProcessTriFans(iDocumentNode* trifansElement)
	{
		csRef<iDocumentNode> nextPElement;
		csRef<iDocumentNodeIterator> pElementIterator = trifansElement->GetNodes("p");

		// get number of triangle fans in the mesh
		int numTriFans = trifansElement->GetAttributeValueAsInt("count");
		//int numberTriangles = numTriStrips * 
		
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
			csRef<iDocumentNodeIterator> inputElements = trifansElement->GetNodes("input");
			csRef<iDocumentNode> currentInputElement;
			size_t vertexOffset, normalOffset;
			int counter = 0;

			while (inputElements->HasNext())
			{
				currentInputElement = inputElements->Next();
				scfString semVal(currentInputElement->GetAttributeValue("semantic"));
				if (semVal.Compare("NORMAL"))
				{
					normalOffset = counter;
				}
				else if (semVal.Compare("VERTEX"))
				{
					vertexOffset = counter;
				}

				counter++;
			}
			
			vertexOffset = vertexOffset * numberOfVertices;
			normalOffset = normalOffset * numberOfVertices;

			if (parent->warningsOn)
			{
				parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex offset is: %d, Normal offset is: %d", vertexOffset, normalOffset);
			}

			stringstream vertexConvertor(nextPElement->GetContentsValue());

			int triCounter = 0;

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
				// for fans, vertex1 always stays the same
				vertex2 = vertex3;
				vertex3 = linearList[(vCount*counter)+vertexOffset];
				vCount++;
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
			CS::Geom::csContour3 polygonalContour;
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
		CS::Geom::csContour3 polygonalContour;
		int tempVertIndex;
		csArray<int> vertexIndexArrayPolys;

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
		int numTriStrips = tristripsElement->GetAttributeValueAsInt("count");
		//int numberTriangles = numTriStrips * 
		
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
			
			RetrieveOffsets(tristripsElement);

			int counter = GetNumInputElements(tristripsElement);

			if (parent->warningsOn)
			{
				parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex offset is: %d, Normal offset is: %d", vertexOffset, normalOffset);
			}

			stringstream vertexConvertor(nextPElement->GetContentsValue());

			int triCounter = 0;

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

		// get number of triangles in the mesh
		int numTris = trianglesElement->GetAttributeValueAsInt("count");
		
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
			RetrieveOffsets(trianglesElement);

			if (parent->warningsOn)
			{
				parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex offset is: %d, Normal offset is: %d", vertexOffset, normalOffset);
			}

			stringstream vertexConvertor(nextPElement->GetContentsValue());

			int triCounter = 0;
			
			int counter = GetNumInputElements(trianglesElement);

			int *linearList = new int[numTris * counter*3];
			for (int x = 0; x < 3*numTris*counter; x++)
			{
				vertexConvertor >> linearList[x];
			}

			while (triCounter < numTris)
			{
				int vertex1, vertex2, vertex3;
				vertex1 = linearList[3*triCounter*counter+vertexOffset];
				vertex2 = linearList[3*triCounter*counter+counter+vertexOffset];
				vertex3 = linearList[3*triCounter*counter + 2*counter + vertexOffset];
				triangles->AddTriangle(vertex1, vertex2, vertex3);

				triCounter++;
			}

			delete[] linearList;
		}
	}
  // =============== Auxiliary Class: csColladaEffectProfile ===============

	csColladaEffectProfile::csColladaEffectProfile(iDocumentNode* profileElement, csColladaConvertor* parentObj)
	{
		parent = parentObj;
		Process(profileElement);
	}

	bool csColladaEffectProfile::Process(iDocumentNode* profileElement)
	{
		csRef<iDocumentNodeIterator> techniquesToProcess;
		csRef<iDocumentNode> nextTechnique;
		csRef<iDocumentNodeIterator> passesToProcess;
		csRef<iDocumentNode> nextPass;

		// grab the listing of techniques that need to be processed
		techniquesToProcess = profileElement->GetNodes("technique");

		while (techniquesToProcess->HasNext())
		{
			nextTechnique = techniquesToProcess->Next();

			// for each technique, we can have multiple passes
			passesToProcess = nextTechnique->GetNodes("pass");
			while (passesToProcess->HasNext())
			{
				nextPass = passesToProcess->Next();
			}

		}

		return true;
	}

	// =============== Auxiliary Class: csColladaEffect ===============

	csColladaEffect::csColladaEffect(iDocumentNode* effectElement, csColladaConvertor* parentObj)
	{
		parent = parentObj;
		Process(effectElement);
	}

	bool csColladaEffect::Process(iDocumentNode* effectElement)
	{
		csRef<iDocumentNodeIterator> profilesToProcess;
		csRef<iDocumentNode> currentProfile;
		csColladaEffectProfile* currentProfileObject;

		element = effectElement;
		
		// grab the id of this effect element
		id = new scfString(element->GetAttributeValue("id"));

		// find all of the profile_COMMON elements
		profilesToProcess = element->GetNodes("profile_COMMON");
		while (profilesToProcess->HasNext())
		{
			currentProfile = profilesToProcess->Next();
			currentProfileObject = new csColladaEffectProfile(currentProfile, parent);
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

	csColladaMaterial::csColladaMaterial(csColladaConvertor *parentEl)
	{
		parent = parentEl;
	}

	csColladaMaterial::~csColladaMaterial()
	{
		delete id;
	}

	void csColladaMaterial::SetID(const char* newId)
	{
		id = new scfString(newId);
	}

	void csColladaMaterial::SetInstanceEffect(iDocumentNode *effectNode)
	{
		instanceEffect = new csColladaEffect(effectNode, parent);
	}

	void csColladaMaterial::SetInstanceEffect(csColladaEffect *newInstEffect)
	{
		instanceEffect = newInstEffect;
	}

	bool csColladaMaterial::operator ==(const csColladaMaterial& comp)
	{
		iString *compId, *thisId;
		thisId = id;
		compId = comp.id;

		if (thisId->Compare(compId))
		{
			return true;
		}

		return false;
	}



} /* End of ColladaConvertor namespace */
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)
