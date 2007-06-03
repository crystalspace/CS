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

#ifndef _CS_IUTIL_COLLADA_H
#define _CS_IUTIL_COLLADA_H

#include <crystalspace.h>

enum csColladaFileType {
	CS_COLLADA_COLLADA_FILE = 1,
	CS_COLLADA_CRYSTAL_FILE
};

struct iColladaConvertor : public virtual iBase
{
	SCF_INTERFACE(iColladaConvertor, 1, 0, 0);

	virtual const char* Load(const char *str, csColladaFileType typeEnum) = 0;
	virtual const char* Load(iString *str, csColladaFileType typeEnum) = 0; 
	virtual const char* Load(iFile *file, csColladaFileType typeEnum) = 0;
	virtual const char* Load(iDataBuffer *db, csColladaFileType typeEnum) = 0;

	virtual const char* Convert() = 0;
	virtual bool ConvertGeometry(iDocumentNode *geometrySection) = 0;
	virtual bool ConvertLighting(iDocumentNode *lightingSection) = 0;
	virtual bool ConvertTextureShading(iDocumentNode *textureSection) = 0;
	virtual bool ConvertRiggingAnimation(iDocumentNode *riggingSection) = 0;
	virtual bool ConvertPhysics(iDocumentNode *physicsSection) = 0;

};

#endif