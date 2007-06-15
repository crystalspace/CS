/*
    Copyright (C) 2007 by Scott Johnson

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this application; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _CS_COLLADA_CONVERTOR_H_
#define _CS_COLLADA_CONVERTOR_H_

#include <ivaria/collada.h>
#include <iutil/comp.h>
#include <iutil/vfs.h>
#include <iutil/document.h>
#include <iutil/objreg.h>
#include <ivaria/reporter.h>

// Standard Headers
#include <cstdarg>
#include <string>

// Forward Declarations (probably not needed)
//struct iObjectRegistry;
//struct iDocumentSystem;

/** 
 * This class implements the iColladaConvertor interface.  It is used as a conversion utility
 * between files in the COLLADA digital interchange format, and Crystal Space Library and/or
 * map files.
 *
 * \remarks This class requires writeable XML documents, and thus utilizes the TinyXML plugin.
 *          The TinyXML plugin will be loaded on initialization of this plugin.
 */

class csColladaConvertor : public scfImplementation2<csColladaConvertor,iColladaConvertor,iComponent>
{
	private:
		
		// =============== System Attributes ===============

		/// A smart pointer to the document system
		iDocumentSystem* docSys;

		/// A smart pointer to the virtual file system
		csRef<iVFS> fileSys;

		/// Whether or not we have warnings turned on.  Warnings are off by default.
		bool warningsOn;

		/// A pointer to the object registry
		iObjectRegistry* obj_reg;

		// =============== Crystal Space Attributes ===============

		/// A smart pointer to the Crystal Space document we will be working on in memory 
    csRef<iDocument> csFile;

		/// Whether or not the Crystal Space file has been loaded and is ready
		bool csReady;

		/// The output file type.  Initially, this is set to CS_FILE_NONE.
		csColladaFileType outputFileType;

		// =============== COLLADA Attributes ===============

		/// A smart pointer to the COLLADA document we will be working from in memory
		csRef<iDocument> colladaFile;

		/// Whether or not the COLLADA file has been loaded and is ready
		bool colladaReady;

		/// A smart pointer to the <COLLADA> element
		csRef<iDocumentNode> colladaElement;

		/**
		 * Report various things back to the application
		 */
		void Report(int severity, const char* msg, ...);

		/**
		 * Checks for validity of the file name to see if it conforms to COLLADA standards.
		 */
		void CheckColladaFilenameValidity(const char* str);

		/**
		 * Checks for validity of the COLLADA file.
		 *
		 * Right now, this only checks to see if the file is valid XML.
		 * @todo Add some abilities to validate the XML.
		 */
		const char* CheckColladaValidity(iFile *file);

		/**
		 * \brief Initialization routine for the output document.
		 *
		 * Constructs a new Crystal Space document.  This function requires that 
		 * SetOutputFileType(csColladaFileType filetype) has already been called.
		 * 
		 * \returns true, if initialization went ok; false otherwise
		 */
		bool InitializeCrystalSpaceDocument();

	public:

		/// Constructor
		csColladaConvertor(iBase* parent);

		/// Destructor
		virtual ~csColladaConvertor();

		/**
		 * Initializes the plugin.
		 * 
		 * \warning This will reload the iDocumentSystem interface so that it uses the TinyXML
		 *          plugin as an implementation.
		 */
		virtual bool Initialize (iObjectRegistry*);
		
		/**
		 * Turn debugging warnings on or off.  This will turn on all possible debug information for the 
		 * plugin.  It also will check to verify that files and data structures conform to specified standards.
		 *
		 * \param toggle If true, turns on debug warnings.
		 * 
		 * \notes Debug warnings are off by default.
		 */
		void SetWarnings(bool toggle);

		virtual const char* Load(const char *str);
		virtual const char* Load(iString *str); 
		virtual const char* Load(iFile *file);
		virtual const char* Load(iDataBuffer *db);

		csRef<iDocument> GetCrystalDocument();
		csRef<iDocument> GetColladaDocument();

		virtual const char* Write(const char* filepath);
		virtual const char* SetOutputFiletype(csColladaFileType filetype);
		virtual const char* Convert();
		virtual bool ConvertGeometry(iDocumentNode *geometrySection);
		virtual bool ConvertLighting(iDocumentNode *lightingSection);
		virtual bool ConvertTextureShading(iDocumentNode *textureSection);
		virtual bool ConvertRiggingAnimation(iDocumentNode *riggingSection);
		virtual bool ConvertPhysics(iDocumentNode *physicsSection);
};

#endif