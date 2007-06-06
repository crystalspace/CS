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

#ifndef _CS_IVARIA_COLLADA_H
#define _CS_IVARIA_COLLADA_H

/* These aren't needed - just forward declarations
#include <iutil/string.h>
#include <iutil/document.h>
#include <iutil/vfs.h>
#include <iutil/databuff.h>
*/

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

// Forward Declarations
struct iString;
struct iFile;
struct iDataBuffer;
struct iDocumentNode;

/**
 * Possible file types for the COLLADA plugin to accept.
 */

enum csColladaFileType {

	/// A COLLADA File
	CS_COLLADA_FILE = 1,
	
	/// A Crystal Space library file
	CS_LIBRARY_FILE,

	/// A Crystal Space map file
	CS_MAP_FILE
};

/**
 * Representation of a convertor from COLLADA files to Crystal Space files.
 *
 * Main creators of instances implementing this interface:
 * - Crystal Space COLLADA Conversion System (crystalspace.utilities.colladaconvertor)
 *
 * Main ways to get pointers to this interface:
 * - CS_QUERY_REGISTRY()
 */

struct iColladaConvertor : public virtual iBase
{
  SCF_INTERFACE(iColladaConvertor, 1, 0, 0);
		
	/** 
	 * Load a file from a null-terminated C-string into the COLLADA Conversion System
	 *
     * \param str A string containing the location of the file to be loaded in VFS
	 * \param typeEnum The type of file to load.  This is one of:
	 *                 - CS_COLLADA_FILE
	 *                 - CS_MAP_FILE
	 *                 - CS_LIBRARY_FILE
     * \return 0 if everything is ok; otherwise an error message
	 * \remarks This will replace the current file being used to read data
	 *          from, or store data to, depending on the second parameter.
	 */
	virtual const char* Load(const char *str, csColladaFileType typeEnum) = 0;
	 
	 /** 
	 * Load a file from an iString object into the COLLADA Conversion System
	 *
     * \param str An iString containing the location of the file to be loaded in VFS
	 * \param typeEnum The type of file to load.  This is one of:
	 *                 - CS_COLLADA_FILE
	 *                 - CS_MAP_FILE
	 *                 - CS_LIBRARY_FILE
     * \return 0 if everything is ok; otherwise an error message
	 * \remarks This will replace the current file being used to read data
	 *          from, or store data to, depending on the second parameter.
	 */
	virtual const char* Load(iString *str, csColladaFileType typeEnum) = 0;
	 
	 /** 
	 * Load a file from an iFile object into the COLLADA Conversion System
	 *
     * \param file An iFile object which points to the document to be loaded
	 * \param typeEnum The type of file to load.  This is one of:
	 *                 - CS_COLLADA_FILE
	 *                 - CS_MAP_FILE
	 *                 - CS_LIBRARY_FILE
     * \return 0 if everything is ok; otherwise an error message
	 * \remarks This will replace the current file being used to read data
	 *          from, or store data to, depending on the second parameter.
	 */
	virtual const char* Load(iFile *file, csColladaFileType typeEnum) = 0;

	 /** 
	 * Load a file from an iDataBuffer object into the COLLADA Conversion System
	 *
     * \param db An iDataBuffer object which contains a document to be loaded
	 * \param typeEnum The type of file to load.  This is one of:
	 *                 - CS_COLLADA_FILE
	 *                 - CS_MAP_FILE
	 *                 - CS_LIBRARY_FILE
     * \return 0 if everything is ok; otherwise an error message
	 * \remarks This will replace the current file being used to read data
	 *          from, or store data to, depending on the second parameter.
	 */
	virtual const char* Load(iDataBuffer *db, csColladaFileType typeEnum) = 0;

	virtual const char* Convert() = 0;
	virtual bool ConvertGeometry(iDocumentNode *geometrySection) = 0;
	virtual bool ConvertLighting(iDocumentNode *lightingSection) = 0;
	virtual bool ConvertTextureShading(iDocumentNode *textureSection) = 0;
	virtual bool ConvertRiggingAnimation(iDocumentNode *riggingSection) = 0;
	virtual bool ConvertPhysics(iDocumentNode *physicsSection) = 0;

};

#endif