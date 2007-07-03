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
#include "csColladaConvertor.h"

using std::string;
using std::stringstream;

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN (ColladaConvertor)
{

SCF_IMPLEMENT_FACTORY(csColladaConvertor)

// =============== Error Reporting and Handling ===============

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
			std::string expectedExt = ".dae";

			if (!(ext == expectedExt))
			{
				std::string warningMsg = "Warning:  File extension \'";
				warningMsg.append(ext);
				warningMsg.append("\' does not conform to expected COLLADA standard file extension \'.dae\'.  File is possibly not a COLLADA file.");
				Report(CS_REPORTER_SEVERITY_WARNING, warningMsg.c_str());
			}
		}
}

const char* csColladaConvertor::CheckColladaValidity(iFile *file)
{
	return 0;
}

// =============== Constructors/ Destructors ===============

csColladaConvertor::csColladaConvertor(iBase*	parent)	:
	scfImplementationType(this,	parent), 
	obj_reg(0),
	colladaReady(false),
	csReady(false),
	outputFileType(CS_NO_FILE),
	warningsOn(false)
{
}

csColladaConvertor::~csColladaConvertor()
{
	colladaElement.Invalidate();
	colladaFile.Invalidate();
	csFile.Invalidate();
	csTopNode.Invalidate();
	delete docSys;
}

// =============== Plugin Initialization ===============

bool csColladaConvertor::Initialize	(iObjectRegistry*	reg)
{
	obj_reg	=	reg;

	
	// create	our	own	document system, since we	will be	reading	and
	// writing to	the	XML	files
	docSys = new csTinyDocumentSystem();

	// get a pointer to the virtual file system
	fileSys = csQueryRegistry<iVFS>(obj_reg);

	return true;
}

// =============== File Loading ===============

const char* csColladaConvertor::Load(const char	*str)
{
	csRef<iFile> filePtr;
	
	if (!fileSys.IsValid())
	{
		Report(CS_REPORTER_SEVERITY_WARNING, "Unable to access file system.  File not loaded.");
		return "Unable to access file system";
	}

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
		Report(CS_REPORTER_SEVERITY_WARNING, "Unable to access file system.  File not loaded.");
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
		Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to find COLLADA node.  File not loaded.");
		return "Unable to find COLLADA node";
	}

	colladaElement = rootNode;
	colladaReady = true;

	//	csRef<iDocumentNode> newNode = rootNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	//	if(!newNode.IsValid())
	//	{
	//		if (warningsOn)
	//		{
	//			Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Unable to create first element.");
	//		}

	//		return "Unable to create first element";
	//	}

	//	newNode->SetValue("world");


	return 0;
}

const char*	csColladaConvertor::Load(iDataBuffer *db)
{
	colladaFile = docSys->CreateDocument();
	colladaFile->Parse(db);
	csRef<iDocumentNode> rootNode = colladaFile->GetRoot();
	rootNode = rootNode->GetNode("COLLADA");
	if (!rootNode.IsValid())
	{
		Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to find COLLADA node.  File not loaded.");
		return "Unable to find COLLADA node";
	}		

	colladaElement = rootNode;
	colladaReady = true;

	return 0;
}

// =============== File Writing ===============

const char* csColladaConvertor::Write(const char* filepath)
{
	// sanity check
	if (!csReady)
	{
		if (warningsOn)
		{
			Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Crystal Space document not ready for writing.");
		}

		return "Crystal Space document not ready for writing";
	}
/*
	csRef<iFile> fileToWrite = fileSys->Open(filepath, VFS_FILE_WRITE);
	
	if (!fileToWrite.IsValid())
	{
		if (warningsOn)
		{
			std::string errorMsg = "Warning: Unable to open file: ";
			errorMsg.append(filepath);
			errorMsg.append(" for writing.");
			Report(CS_REPORTER_SEVERITY_WARNING, errorMsg.c_str());
		}

		return "Unable to open file for writing";
	}
*/

/*
	if (csFile->Changeable() == CS_CHANGEABLE_YES)
	{
		Report(CS_REPORTER_SEVERITY_WARNING, "CS File is changeable.");
	}
*/
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
csRef<iDocument> csColladaConvertor::GetCrystalDocument()
{
	return csFile;
}

csRef<iDocument> csColladaConvertor::GetColladaDocument()
{
	return colladaFile;
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

	csReady = true;
	return true;
}

// =============== Basic Utility Functions ===============




// =============== Conversion Functions ===============
// This is the warp core of the implementation.  ;)
// Currently, the core has been ejected, so it's missing.  Engineering is still
// trying to locate it on sensors.  :)

const char*	csColladaConvertor::Convert()
{
	if (!csReady)
	{
		if (warningsOn)
		{
			Report(CS_REPORTER_SEVERITY_NOTIFY, "Crystal Space document not yet initialized.  Attempting to initialize...");
			if (!InitializeCrystalSpaceDocument())
			{
				Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to initialize output document.");
				return "Unable to initialize output document";
			}
			else
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

	csRef<iDocumentNode> geoNode = colladaElement->GetNode("library_geometries");
	if (!geoNode.IsValid())
	{
		Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to find geometry section.");
		return "Unable to find geometry section";
	}

	ConvertGeometry(geoNode);

	return 0;
}

bool csColladaConvertor::ConvertGeometry(iDocumentNode *geometrySection)
{
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
	//iString *idValue;
	float* vertexArray;
	//iStringArray *accessorArray;
	int vertexArraySize = 0;
	csColladaMesh *mesh;

	// while the iterator is not empty
	while (geometryIterator->HasNext())
	{
		// retrieve next <geometry> element and store as currentGeometryElement
		currentGeometryElement = geometryIterator->Next();

		// get value of id attribute and store as currentGeometryID
		currentGeometryID	= currentGeometryElement->GetAttribute("id");

		if (currentGeometryID.IsValid())
		{
			if (warningsOn)
			{
				std::string notifyMsg = "Current Geometry Node ID: ";
				notifyMsg.append(currentGeometryID->GetValue());
				Report(CS_REPORTER_SEVERITY_NOTIFY, notifyMsg.c_str());
			}
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
			
			mesh = new csColladaMesh(currentMeshElement, this);

			//vertexArray = GetVertexArray(currentVerticesElement, &idValue, vertexArraySize);
			if (warningsOn)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "Converting to float* ...");
			}

			vertexArray = (float*)(mesh->GetVertices());
			
			if (warningsOn)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "Done");
				Report(CS_REPORTER_SEVERITY_NOTIFY, "Getting number of vertices...");
			}

			
			vertexArraySize = mesh->GetNumVertices();

			if (warningsOn)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "Done.");
			}

			if (warningsOn && vertexArraySize > 0)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex array acquired.  id: %s", mesh->GetID()->GetData());
			}
			else if (warningsOn && vertexArraySize <= 0)
			{
				Report(CS_REPORTER_SEVERITY_WARNING, "Unable to acquire vertex array.  Vertex array size: %d", vertexArraySize);
				return false;
			}
			
			// now that we have the vertex array, we need to add it to the
			// crystal space document.
			if (!csTopNode.IsValid() && warningsOn)
			{
				Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire top-level node!");
				return false;
			}
			
			csRef<iDocumentNode> sourceElement = GetSourceElement(mesh->GetID()->GetData(), currentMeshElement);
			if (sourceElement == 0)
			{
				Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire source element with id: %s", mesh->GetID()->GetData());
				return false;
			}
		
			csRef<iDocumentNode> meshFactNode = csTopNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
			meshFactNode->SetValue("meshFact");
			meshFactNode->SetAttribute("name", mesh->GetName()->GetData());
			
			if (warningsOn)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "MeshFact element created.  Creating params element");
			}

			csRef<iDocumentNode> currentCrystalParamsElement = 
								meshFactNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
			currentCrystalParamsElement->SetValue("params");

			int counter = 0;
			csRef<iDocumentNode> currentCrystalVElement;

			int accessorArraySize = 0;
			csColladaAccessor *accessorArray = new csColladaAccessor(sourceElement, this);

			//accessorArray = GetAccessorArray(sourceElement, accessorArraySize);
			
			while (counter < vertexArraySize)
			{
				if (warningsOn)
				{
					Report(CS_REPORTER_SEVERITY_NOTIFY, "Adding the following vertex: %f, %f, %f", vertexArray[counter], vertexArray[counter+1], vertexArray[counter+2]);
				}
			
				scfString formatter;
				currentCrystalVElement = currentCrystalParamsElement->CreateNodeBefore(CS_NODE_ELEMENT, 0);
				currentCrystalVElement->SetValue("v");
				formatter.Format("%f", vertexArray[counter]);
				currentCrystalVElement->SetAttribute(accessorArray->Get(0), formatter.GetData());
				formatter.Format("%f", vertexArray[counter+1]);
				currentCrystalVElement->SetAttribute(accessorArray->Get(1), formatter.GetData());
				formatter.Format("%f", vertexArray[counter+2]);
				currentCrystalVElement->SetAttribute(accessorArray->Get(2), formatter.GetData()) ;
				
				counter = counter + 3;
			}

			meshFactNode.Invalidate();
			currentCrystalParamsElement.Invalidate();

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

bool csColladaConvertor::ConvertLighting(iDocumentNode *lightingSection)
{
	return true;
}

bool csColladaConvertor::ConvertTextureShading(iDocumentNode *textureSection)
{
	return true;
}

bool csColladaConvertor::ConvertRiggingAnimation(iDocumentNode *riggingSection)
{
	return true;
}

bool csColladaConvertor::ConvertPhysics(iDocumentNode	*physicsSection)
{
	return true;
}

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

int csColladaAccessor::GetStride()
{
	return stride;
}

int csColladaAccessor::GetCount()
{
	return count;
}

csColladaConvertor* csColladaAccessor::GetParent()
{
	return parent;
}

csRef<iDocumentNode> csColladaAccessor::GetSourceElement()
{
	return sourceElement;
}

iStringArray* csColladaAccessor::GetAccessorNames()
{
	return accessorNames;
}

bool csColladaAccessor::Process(iDocumentNode* src)
{
	csRef<iDocumentNode> currentTechniqueElement;
	csRef<iDocumentNode> currentAccessorElement;
	csRef<iDocumentNode> currentParamsElement;
	csRef<iDocumentNodeIterator> paramsIterator;
	//iStringArray *retArray;

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
		accessorNames->Push(currentParamsElement->GetAttributeValue("name"));
	}

	return 1;
}

const char* csColladaAccessor::Get(int index)
{
	return accessorNames->Get(index);
}

// =============== Auxiliary Class: csColladaMesh ===============

csColladaMesh::csColladaMesh(iDocumentNode* element, csColladaConvertor* par)
{
	meshElement = element;
	parent = par;
	Process(meshElement);
}

csColladaMesh::~csColladaMesh()
{
	if (vType == CS_COLLADA_FLOAT)
	{
		delete[] ((float*)vertices);
	}

	else
	{
		delete[] ((int*) vertices);
	}

	parent = 0;
	numVertices = 0;
}

csRef<iDocumentNode> csColladaMesh::GetMeshElement()
{
	return meshElement;
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

iString* csColladaMesh::GetID()
{
	return id;
}

void* csColladaMesh::GetVertices()
{
	return vertices;
}

int csColladaMesh::GetNumVertices()
{
	return numVertices;
}

csColladaNumericType csColladaMesh::GetVertexType()
{
	return vType;
}

iString* csColladaMesh::GetName()
{
	return name;
}

void* csColladaMesh::Process(iDocumentNode* element)
{
	csRef<iDocumentNode> currentInputElement;
	csRef<iDocumentNodeIterator> inputElements;
	csRef<iDocumentNodeIterator> sourceElements;
	csRef<iDocumentNode> currentNumericArrayElement;
	csRef<iDocumentNode> currentSourceElement;
	//csRef<iDocumentNode> currentMeshElement = verticesElement->GetParent();
	std::string inputElementSemantic = "";
	scfString positionSource = "";
	//(*id) = 0;
	//void* vertexArray = 0;

	meshElement = element;

	// a sanity check, in the event someone tries to do something foolish
	numVertices = 0;

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
	positionSource.DeleteAt(0, 1);

	if (parent->warningsOn)
	{
		parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Found position semantic.  Source value: %s", positionSource.GetData());
	}

	// find child <source> element of currentMeshElement
	// with id equal to positionSource and store as currentSourceElement
	sourceElements = meshElement->GetNodes("source");
	while (sourceElements->HasNext())
	{
/*		
		if (!((*id) == 0))
		{
			delete (*id);
		}
*/
		currentSourceElement = sourceElements->Next();
		
		name = new scfString(currentSourceElement->GetParent()->GetParent()->GetAttribute("id")->GetValue());

		id = new scfString(currentSourceElement->GetAttribute("id")->GetValue());
//		id = new scfString(attrib->GetValue());
/*
		if (warningsOn)
		{
			Report(CS_REPORTER_SEVERITY_NOTIFY, "attrib is: %s", attrib->GetName()); 
			Report(CS_REPORTER_SEVERITY_NOTIFY, "id is: %s", (*id)->GetData());
			Report(CS_REPORTER_SEVERITY_NOTIFY, "positionSource is: %s", positionSource.GetData()); 
		}
*/

		if (id->Compare(&positionSource))
		{
			//parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Breaking...");
			break;
		}

	}

	// find child numeric array element of currentSourceElement
	// store as currentNumericArrayElement
	currentNumericArrayElement = FindNumericArray(currentSourceElement);

	if (!currentNumericArrayElement.IsValid())
	{
		if (parent->warningsOn)
		{
			parent->Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Unable to find a numeric array element under source node!");
			return 0;
		}
	}
	
	stringstream conver(currentNumericArrayElement->GetAttribute("count")->GetValue());

	conver >> numVertices;

	if (parent->warningsOn)
	{
		/*
		std::string notifyMsg = "Numeric array found.  Type: ";
		notifyMsg.append(currentNumericArrayElement->GetValue());
		notifyMsg.append(", Count: ");
		notifyMsg.append(countElements);
		*/
		parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Numeric array found.  Type: %s, Number of elements: %d", currentNumericArrayElement->GetValue(), numVertices);
	}

	std::string arrayElement(currentNumericArrayElement->GetContentsValue());
	
	if (parent->warningsOn)
	{
		// check for string correctness
		parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Current value of vertex array string: %s", arrayElement.c_str());
	}

	conver.clear();
	conver.str(arrayElement.c_str());
	
	if (parent->warningsOn)
	{
		parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Assigning vertices to array...");
	}
	
	if (vType == CS_COLLADA_INTEGER)
	{
		vertices = new int[numVertices];
		int temp;
		int i = 0;
		while (conver >> temp)
		{
			((int*)vertices)[i] = temp; 
			i++;
		}
	}

	else
	{
		vertices = new float[numVertices];
		float temp;
		int i = 0;
		while (conver >> temp)
		{
	
			((float*)vertices)[i] = temp; 
			i++;
		}
	}

	if (parent->warningsOn)
	{
		parent->Report(CS_REPORTER_SEVERITY_NOTIFY, "Done.");
	}

	return vertices;
	//Report(CS_REPORTER_SEVERITY_WARNING, "First element of vertexArray: %f", vertexArray[0]);
}

bool csColladaSimplePolygon::Process(iDocumentNode* element)
{
	// element should represent a <p> or <ph> node
	return true;
}

}
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)