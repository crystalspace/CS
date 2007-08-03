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
#include "csColladaClasses.h"

using std::string;
using std::stringstream;

CS_IMPLEMENT_PLUGIN

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
	Report(CS_REPORTER_SEVERITY_WARNING, "Warning: The function CheckColladaValidity(iFile* file) has not yet been implemented.");
	return 0;
}

// =============== Constructors/ Destructors ===============

csColladaConvertor::csColladaConvertor(iBase*	parent)	:
	scfImplementationType(this,	parent), 
	obj_reg(0),
	colladaReady(false),
	csReady(false),
	outputFileType(CS_NO_FILE),
	warningsOn(false),
	lastEffectId(-1)
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

	csReady = true;
	return true;
}

// =============== Basic Utility Functions ===============

// None, anymore ;)


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
		Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to find <library_geometries> element.");
		return "Unable to find library_geometries.";
	}

	ConvertGeometry(geoNode);

	csRef<iDocumentNode> materialsNode = colladaElement->GetNode("library_materials");
	if (!materialsNode.IsValid())
	{
		Report(CS_REPORTER_SEVERITY_ERROR, "Error: Unable to find <library_materials> element");
		return "Unable to find library_materials.";
	}

	ConvertMaterials(materialsNode);



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
	csRef<iDocumentNodeIterator> convexMeshIterator;
	//iString *idValue;
	csArray<csVector3> vertexArray;
	//int *normalArray;
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
			
			mesh = new csColladaMesh(currentMeshElement, this);

			// Adding vertices to CS document
			vertexArray = mesh->GetVertices();
						
			vertexArraySize = mesh->GetNumberOfVertices();

			if (warningsOn && vertexArraySize > 0)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "Array acquired.  id: %s", mesh->GetPositionID()->GetData());
			}
			else if (warningsOn && vertexArraySize <= 0)
			{
				Report(CS_REPORTER_SEVERITY_WARNING, "Unable to acquire array.  Array size: %d", vertexArraySize);
				return false;
			}
			
			// now that we have the vertex array, we need to add it to the
			// crystal space document.
			if (!csTopNode.IsValid() && warningsOn)
			{
				Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire top-level node!");
				return false;
			}
			
			csRef<iDocumentNode> sourceElement = GetSourceElement(mesh->GetPositionID()->GetData(), currentMeshElement);
			if (sourceElement == 0)
			{
				Report(CS_REPORTER_SEVERITY_ERROR, "Unable to acquire source element with id: %s", mesh->GetPositionID()->GetData());
				return false;
			}
		
			// create meshfact and plugin (top-level) nodes
			csRef<iDocumentNode> meshFactNode = csTopNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
			meshFactNode->SetValue("meshfact");
			meshFactNode->SetAttribute("name", mesh->GetName()->GetData());
			csRef<iDocumentNode> pluginNode = meshFactNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
			pluginNode->SetValue("plugin");
			csRef<iDocumentNode> pluginContents = pluginNode->CreateNodeBefore(CS_NODE_TEXT, 0);
			pluginContents->SetValue(mesh->GetPluginType()->GetData());

			if (warningsOn)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "MeshFact element created.  Creating params element");
			}

			// create the sub params element
			csRef<iDocumentNode> currentCrystalParamsElement = 
								meshFactNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
			currentCrystalParamsElement->SetValue("params");

			int counter = 0;
			csRef<iDocumentNode> currentCrystalVElement;

			int accessorArraySize = 0;
			csColladaAccessor *accessorArray = new csColladaAccessor(sourceElement, this);

			while (counter < vertexArraySize)
			{
				if (warningsOn)
				{
					Report(CS_REPORTER_SEVERITY_NOTIFY, "Current values: counter = %d, vertexArraySize = %d", counter, vertexArraySize);
					Report(CS_REPORTER_SEVERITY_NOTIFY, "Adding the following vertex: %f, %f, %f", vertexArray[counter].x, vertexArray[counter].y, vertexArray[counter].z);
				}
			
				/* WRITE OUT VERTICES */
				scfString formatter;
				currentCrystalVElement = currentCrystalParamsElement->CreateNodeBefore(CS_NODE_ELEMENT, 0);
				currentCrystalVElement->SetValue("v");
				formatter.Format("%f", vertexArray[counter].x);
				currentCrystalVElement->SetAttribute(accessorArray->Get(0), formatter.GetData());
				formatter.Format("%f", vertexArray[counter].y);
				currentCrystalVElement->SetAttribute(accessorArray->Get(1), formatter.GetData());
				formatter.Format("%f", vertexArray[counter].z);
				currentCrystalVElement->SetAttribute(accessorArray->Get(2), formatter.GetData());
				/* DONE WRITING OUT VERTICES */

				counter++;
			}

			if (warningsOn)
			{
				Report(CS_REPORTER_SEVERITY_NOTIFY, "Done adding vertices.");
			}

			// now, output the triangles
			csTriangleMesh* currentTriMesh = mesh->GetTriangleMesh();
			
			size_t triCount = currentTriMesh->GetTriangleCount();
			csTriangle* tris = currentTriMesh->GetTriangles();
			size_t triCounter = 0;
			csTriangle* currentTri;

			while (triCounter < triCount)
			{
				currentTri = &tris[triCounter];

				// create a t element
				csRef<iDocumentNode> currentTElement = currentCrystalParamsElement->CreateNodeBefore(CS_NODE_ELEMENT, 0);
				
				if (!currentTElement.IsValid())
				{
					if (warningsOn)
					{
						Report(CS_REPORTER_SEVERITY_NOTIFY, "Unable to add triangle element!");
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

bool csColladaConvertor::ConvertEffects(iDocumentNode *effectsSection)
{
	Report(CS_REPORTER_SEVERITY_WARNING, "Warning: ConvertEffects() functionality not fully implemented.  Use at your own risk!");

	csRef<iDocumentNodeIterator> effectsToProcess = effectsSection->GetNodes("effect");
	csRef<iDocumentNode> currentEffect;

	csColladaEffect* currentEffectObject;

	while (effectsToProcess->HasNext())
	{
		currentEffect = effectsToProcess->Next();
		currentEffectObject = new csColladaEffect(currentEffect, this);
	}

	return true;
}

bool csColladaConvertor::ConvertMaterials(iDocumentNode *materialsSection)
{
	Report(CS_REPORTER_SEVERITY_WARNING, "Warning: ConvertMaterials() functionality not fully implemented.  Use at your own risk!");

	// get an iterator over all the <material> elements
	csRef<iDocumentNodeIterator> materialsElements = materialsSection->GetNodes("material");
	csRef<iDocumentNode> nextMaterialElement;
	csColladaMaterial* nextMaterial;
	csRef<iDocumentNode> effectNode;
	csRef<iDocumentNode> libraryEffects = colladaElement->GetNode("library_effects");
	csRef<iDocumentNodeIterator> effectIter;

	while (materialsElements->HasNext())
	{
		nextMaterialElement = materialsElements->Next();
		nextMaterial = new csColladaMaterial(this);
		nextMaterial->SetID(nextMaterialElement->GetAttributeValue("id"));

		scfString url = nextMaterialElement->GetNode("instance_effect")->GetAttributeValue("url");
		
		// chop off the '#'
		if (url.GetAt(0) == '#')
		{
			url.DeleteAt(0, 1);
		}

		// now, find the same effect in the <library_effects> list
		effectIter = libraryEffects->GetNodes("effect");
		while (effectIter->HasNext())
		{
			effectNode = effectIter->Next();
			scfString newUrl = effectNode->GetAttributeValue("id");
			if (newUrl.Compare(url))
			{
				break;
			}
		}

		nextMaterial->SetInstanceEffect(effectNode);
		materialsList.Push(*nextMaterial);

	}

	return true;
}


bool csColladaConvertor::ConvertScene(iDocumentNode *camerasSection, iDocumentNode *lightsSection, iDocumentNode *nodesSection, iDocumentNode *visualScenesSection)
{
	if (outputFileType != CS_MAP_FILE)
	{
		Report(CS_REPORTER_SEVERITY_WARNING, "Warning: Conversion of scenes is invalid except for Crystal Space map files.  Continuing blithely...");
		return false;
	}

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

}
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)