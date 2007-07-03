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

#ifndef	_CS_COLLADA_CONVERTOR_H_
#define	_CS_COLLADA_CONVERTOR_H_

#include "ivaria/collada.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

// Standard	Headers
#include <cstdarg>
#include <string>
#include <sstream>

CS_PLUGIN_NAMESPACE_BEGIN (ColladaConvertor)
{

// Forward Declarations	(probably	not	needed)
//struct iObjectRegistry;
//struct iDocumentSystem;

/**	
 * This	class	implements the iColladaConvertor interface.	 It	is used	as a conversion	utility
 * between files in	the	COLLADA	digital	interchange	format,	and	Crystal	Space	Library	and/or
 * map files.
 *
 * \remarks	This class requires	writeable	XML	documents, and thus	utilizes the TinyXML plugin.
 *					The	TinyXML	plugin will	be loaded	on initialization	of this	plugin.
 */

class	csColladaConvertor : public	scfImplementation2<csColladaConvertor,iColladaConvertor,iComponent>
{
	friend class csColladaAccessor;
	friend class csColladaMesh;

	private:
		
		// =============== System	Attributes ===============

		///	A	smart	pointer	to the document	system
		iDocumentSystem* docSys;

		///	A	smart	pointer	to the virtual file	system
		csRef<iVFS>	fileSys;

		///	Whether	or not we	have warnings	turned on.	Warnings are off by	default.
		bool warningsOn;

		///	A	pointer	to the object	registry
		iObjectRegistry* obj_reg;

		/**
		 * Report	various	things back	to the application
		 */
		void Report(int	severity,	const	char*	msg, ...);


		// =============== Crystal Space Attributes	===============

		///	A	smart	pointer	to the Crystal Space document	we will	be working on	in memory	
		csRef<iDocument> csFile;

		///	A	smart	pointer	to the 'world' or	'library'	node
		csRef<iDocumentNode> csTopNode;

		///	Whether	or not the Crystal Space file	has	been loaded	and	is ready
		bool csReady;

		///	The	output file	type.	 Initially,	this is	set	to CS_FILE_NONE.
		csColladaFileType	outputFileType;

		// =============== COLLADA Attributes	===============

		///	A	smart	pointer	to the COLLADA document	we will	be working from	in memory
		csRef<iDocument> colladaFile;

		///	Whether	or not the COLLADA file	has	been loaded	and	is ready
		bool colladaReady;

		///	A	smart	pointer	to the <COLLADA> element
		csRef<iDocumentNode> colladaElement;

		/**
		 * Checks	for	validity of	the	file name	to see if	it conforms	to COLLADA standards.
		 */
		void CheckColladaFilenameValidity(const	char*	str);

		/**
		 * Checks	for	validity of	the	COLLADA	file.
		 *
		 * Right now,	this only	checks to	see	if the file	is valid XML.
		 * @todo Add some	abilities	to validate	the	XML.
		 */
		const	char*	CheckColladaValidity(iFile *file);

		/**
		 * \brief	Initialization routine for the output	document.
		 *
		 * Constructs	a	new	Crystal	Space	document.	 This	function requires	that 
		 * SetOutputFileType(csColladaFileType filetype) has already been	called.
		 * 
		 * \returns	true,	if initialization	went ok; false otherwise
		 */
		bool InitializeCrystalSpaceDocument();

		// =============== Basic Utility Functions ===============

		/**	\brief Chops a single	character	out	of a string
		 * 
		 * This	function will	remove a single	character	in a string.
		 *
		 * \param	str	The	string to	chop out of
		 * \param	index	An index into	the	string representing	the	location of	the
		 *							character	to remove
		 *
		 * \warning	This function	changes	the	string sent	into the function.
		 * @todo Place this	into iString?
		 */
		
		/*
		CS_DEPRECATED_METHOD_MSG("Use	iString::DeleteAt()	instead");
		std::string& Chop(iString& str,	int	index);
		*/

	public:

		///	Constructor
		csColladaConvertor(iBase*	parent);

		///	Destructor
		virtual	~csColladaConvertor();

		/**
		 * Initializes the plugin.
		 * 
		 * \warning	This will	reload the iDocumentSystem interface so	that it	uses the TinyXML
		 *					plugin as	an implementation.
		 */
		virtual	bool Initialize	(iObjectRegistry*);
		
		/**
		 * Turn	debugging	warnings on	or off.	 This	will turn	on all possible	debug	information	for	the	
		 * plugin.	It also	will check to	verify that	files	and	data structures	conform	to specified standards.
		 *
		 * \param	toggle If	true,	turns	on debug warnings.
		 * 
		 * \notes	Debug	warnings are off by	default.
		 */
		void SetWarnings(bool	toggle);

		virtual	const	char*	Load(const char	*str);
		virtual	const	char*	Load(iString *str);	
		virtual	const	char*	Load(iFile *file);
		virtual	const	char*	Load(iDataBuffer *db);

		csRef<iDocument> GetCrystalDocument();
		csRef<iDocument> GetColladaDocument();

		virtual	const	char*	Write(const	char*	filepath);
		virtual	const	char*	SetOutputFiletype(csColladaFileType	filetype);
		
		// =============== Conversion	Functions	===============

		virtual	const	char*	Convert();
		virtual	bool ConvertGeometry(iDocumentNode *geometrySection);
		virtual	bool ConvertLighting(iDocumentNode *lightingSection);
		virtual	bool ConvertTextureShading(iDocumentNode *textureSection);
		virtual	bool ConvertRiggingAnimation(iDocumentNode *riggingSection);
		virtual	bool ConvertPhysics(iDocumentNode	*physicsSection);

	private:

		/**	\brief Gets	the	array	of vertices	from the mesh
		 *
		 * Retrieves an	array	of float values	from the COLLADA document	which	represent
		 * the vertices	of the mesh.
		 *
		 * \param	verticesElement	An iDocumentNode representing	the	<vertices> element of	the	
		 *												COLLADA	file.
		 * \param	id A reference to	a	variable which will	store	the	id of	the	mesh object.
		 * \param	arraySize	A	reference	to a variable	where	the	array	size will	be stored.
		 *
		 * \returns	A	pointer	to an	array	of float values.	If the <vertices>	element	contained	
		 *					an <int_array>,	the	integers will	be cast	to floating	point	values.
		 *
		 * \warning	The	caller is	responsible	for	deallocating the array of	floating point
		 *					values,	once finished.
		 */
		// float* GetVertexArray(iDocumentNode* verticesElement,	iString	**name,	int& size, int&	stride);

		/**	\brief Retrieves the array of	accessor strings
		 *
		 * Used	to retrieve	the	values of	the	<accessor> elements, in	order	to access
		 * the vertex	array.	This function	is mainly	used for swizzling,	as red/green/blue
		 * values	could	be dereferenced	as x/y/x.
		 *
		 * \param	currentSourceElement A pointer to	the	<source> tag containing	the	
		 *														 <accessor>	element	to be	parsed.	
		 * \param	size The size	of the accessor	array.
		 *
		 * \returns	The	array	of strings depicting the accessor	values on	success;
		 *					0	on failure.
		 *
		 * \remarks	This function	greatly	depends	on the context of	it's call.
		 *
		 * \warning	It is	the	caller's reponsibility to	deallocate the array when
		 *					finished using it.
		 */
	//iStringArray*	GetAccessorArray(iDocumentNode*	currentSourceElement,	int& size);

		 /** \brief	Returns	a	<source> element
		*
		*	Retrieves	a	<source> element associated	with a given element.
		*
		*	\param name	The	name of	the	source element (id)	to retrieve.
		*	\param parent	The	parent node
		*
		*	\returns The source	element	associated with	given	element.
		*/
		csRef<iDocumentNode> GetSourceElement(const	char*	name,	iDocumentNode* parent);

}; /* End of class csColladaConvertor */

// =============== Auxiliary Classes ===============

class	csColladaAccessor	{
	private:
		csRef<iDocumentNode> sourceElement;
		csColladaConvertor *parent;
		int	stride;
		int	count;
		iStringArray *accessorNames;

	public:
		csColladaAccessor();
		csColladaAccessor(iDocumentNode* source, csColladaConvertor* parent);

		int GetStride();
		int GetCount();
		iStringArray* GetAccessorNames();
		csColladaConvertor* GetParent();
		csRef<iDocumentNode> GetSourceElement();
		bool Process(iDocumentNode* src);
		const char* Get(int index);

}; /* End of class csColladaAccessor */

class csColladaPolygon {
	private:
		int* vertexIndices;
	public:
		virtual bool Process(iDocumentNode* element) = 0;
}; /* End of class csColladaPolygon */

class csColladaSimplePolygon : public csColladaPolygon {
	public:
		virtual bool Process(iDocumentNode* element);

}; /* End of class csColladaSimplePolygon */

class	csColladaMesh	{

	private:
		csColladaConvertor* parent;
		void *vertices;
		int numVertices;
		iString	*name;  // what the polygon is actually called
		iString *id;    // what the position ID is
		csRef<iDocumentNode> meshElement;
		csColladaAccessor	vAccess;
		csColladaNumericType vType;

		csColladaPolygon* polygons;

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

	public:
		csColladaMesh(iDocumentNode* element, csColladaConvertor* parent);
		~csColladaMesh();

		void *GetVertices();
		int GetNumVertices();
		csColladaNumericType GetVertexType();
		iString* GetName();
		iString* GetID();
		csRef<iDocumentNode> GetMeshElement();
		void* Process(iDocumentNode*	element);

}; /* End of class csColladaMesh */


} /* End of ColladaConvertor namespace */
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)

#endif