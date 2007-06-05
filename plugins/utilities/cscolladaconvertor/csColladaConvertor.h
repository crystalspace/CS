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

// Forward Declarations
struct iObjectRegistry;

/** 
 * This class implements the iColladaConvertor interface.  It is used as a conversion utility
 * between files in the COLLADA digital interchange format, and Crystal Space Library and/or
 * map files.
 */

class csColladaConvertor : public scfImplementation2<csColladaConvertor,iColladaConvertor,iComponent>
{
	private:
		
		/// A smart pointer to the Crystal Space document we will be working on in memory 
		csRef<iDocument> csFile;

		/// A smart pointer to the COLLADA document we will be working from in memory
		csRef<iDocument> colladaFile;

		/// Whether or not the COLLADA file has been loaded and is ready
		bool colladaReady;

		/// Whether or not the Crystal Space file has been loaded and is ready
		bool csReady;

		/// A pointer to the object registry
		iObjectRegistry* obj_reg;

	public:

		/// Constructor
		csColladaConvertor(iBase* parent);

		/// Destructor
		virtual ~csColladaConvertor();

		/// Initializes the plugin
		virtual bool Initialize (iObjectRegistry*);
		
		virtual const char* Load(const char *str, csColladaFileType typeEnum);
		virtual const char* Load(iString *str, csColladaFileType typeEnum); 
		virtual const char* Load(iFile *file, csColladaFileType typeEnum);
		virtual const char* Load(iDataBuffer *db, csColladaFileType typeEnum);

		virtual const char* Convert();
		virtual bool ConvertGeometry(iDocumentNode *geometrySection);
		virtual bool ConvertLighting(iDocumentNode *lightingSection);
		virtual bool ConvertTextureShading(iDocumentNode *textureSection);
		virtual bool ConvertRiggingAnimation(iDocumentNode *riggingSection);
		virtual bool ConvertPhysics(iDocumentNode *physicsSection);
};

#endif