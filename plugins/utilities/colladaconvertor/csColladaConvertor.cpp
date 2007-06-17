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

#include <cssysdef.h>
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iutil/document.h>
#include <csutil/xmltiny.h>
#include <iutil/string.h>
#include <csutil/scfstr.h>
#include "csColladaConvertor.h"

using std::string;
using std::stringstream;

CS_IMPLEMENT_PLUGIN
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

const	char*	csColladaConvertor::Load(const char	*str)
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

const	char*	csColladaConvertor::Load(iString *str)
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

const	char*	csColladaConvertor::Load(iFile *file)
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

const	char*	csColladaConvertor::Load(iDataBuffer *db)
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
	}

	else 
	{
		newNode->SetValue("library");
	}

	csReady = true;
	return true;
}

// =============== Basic Utility Functions ===============

std::string& csColladaConvertor::Chop(std::string& str, int index)
{
	std::string before, after;
	before = str.substr(0, index);
	after = str.substr(index+1);

	before.append(after);
	str = before;

	return str;
}

csRef<iDocumentNode> csColladaConvertor::FindNumericArray(const csRef<iDocumentNode>& node)
{
	csRef<iDocumentNode> retVal;

	// search first for a <float_array> element
	retVal = node->GetNode("float_array");
	if (retVal.IsValid())
	{
		return retVal;
	}

	// then for a <int_array> element
	else
	{
		retVal = node->GetNode("int_array");
		return retVal;
	}
}


// =============== Conversion Functions ===============
// This is the warp core of the implementation.  ;)
// Currently, the core has been ejected, so it's missing.  Engineering is still
// trying to locate it on sensors.  :)

float* csColladaConvertor::GetVertexArray(iDocumentNode* verticesElement, iString& id, int& arraySize)
{
	csRef<iDocumentNode> currentInputElement;
	csRef<iDocumentNodeIterator> inputElements;
	csRef<iDocumentNodeIterator> sourceElements;
	csRef<iDocumentNode> currentNumericArrayElement;
	csRef<iDocumentNode> currentSourceElement;
	csRef<iDocumentNode> currentMeshElement = verticesElement->GetParent();
	std::string inputElementSemantic = "";
	std::string positionSource = "";
	float* vertexArray = 0;
	
	// a sanity check, in the event someone tries to do something foolish
	arraySize = 0;

	// find <input> child element of currentVerticesElement with
	// attribute semantic="POSITION" and store this element as positionElement
	inputElements = verticesElement->GetNodes("input");
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
	Chop(positionSource, 0);

	if (warningsOn)
	{
		std::string notifyMsg = "Found position semantic.  Source value: ";
		notifyMsg.append(positionSource);
		Report(CS_REPORTER_SEVERITY_NOTIFY, notifyMsg.c_str());
	}

	// find child <source> element of currentMeshElement
	// with id equal to positionSource and store as currentSourceElement
	sourceElements = currentMeshElement->GetNodes("source");
	while (sourceElements->HasNext())
	{
		currentSourceElement = sourceElements->Next();
		
		id = currentSourceElement->GetAttribute("id")->GetValue();
		if (id.compare(positionSource) == 0)
		{
			break;
		}
	}

	// find child numeric array element of currentSourceElement
	// store as currentNumericArrayElement
	currentNumericArrayElement = FindNumericArray(currentSourceElement);

	if (!currentNumericArrayElement.IsValid())
	{
		if (warningsOn)
		{
			Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Unable to find a numeric array element under source node!");
			return 0;
		}
	}
	
	stringstream conver(currentNumericArrayElement->GetAttribute("count")->GetValue());
	conver >> arraySize;

	conver << arraySize;

	string countElements = conver.str();

	if (warningsOn)
	{
		std::string notifyMsg = "Numeric array found.  Type: ";
		notifyMsg.append(currentNumericArrayElement->GetValue());
		notifyMsg.append(", Count: ");
		notifyMsg.append(countElements);
		Report(CS_REPORTER_SEVERITY_NOTIFY, notifyMsg.c_str());
	}

	conver << currentNumericArrayElement->GetContentsValue();
	
	vertexArray = new float[arraySize];

	for (int i = 0; i < arraySize; i++)
	{
		conver >> vertexArray[i]; 
	}

	return vertexArray;
}

const	char*	csColladaConvertor::Convert()
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
	iString idValue;
	float* vertexArray;
	int vertexArraySize = 0;

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
		
			vertexArray = GetVertexArray(currentVerticesElement, idValue, vertexArraySize);

			if (warningsOn && vertexArraySize > 0)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "Vertex array acquired.");
			}

			delete[] vertexArray;
		}
	}

	return true;
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