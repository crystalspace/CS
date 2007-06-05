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

#include <cssysdef.h>
#include <csColladaConvertor.h>
#include <iutil/objreg.h>
#include <iutil/plugin.h>

CS_IMPLEMENT_PLUGIN
SCF_IMPLEMENTATION_FACTORY(csColladaConvertor)

csColladaConvertor::csColladaConvertor(iBase* parent) :
	scfImplementationType(this, parent), 
	obj_reg(0),
	colladaReady(false),
	csReady(false)
{
}

csColladaConvertor::~csColladaConvertor()
{
}

bool csColladaConvertor::Initialize (iObjectRegistry* reg)
{
	obj_reg = reg;
	return true;
}

const char* csColladaConvertor::Load(const char *str, csColladaFileType typeEnum)
{
}

const char* csColladaConvertor::Load(iString *str, csColladaFileType typeEnum)
{
}

const char* csColladaConvertor::Load(iFile *file, csColladaFileType typeEnum)
{
}

const char* csColladaConvertor::Load(iDataBuffer *db, csColladaFileType typeEnum)
{
}

const char* csColladaConvertor::Convert()
{
}

bool csColladaConvertor::ConvertGeometry(iDocumentNode *geometrySection)
{
}

bool csColladaConvertor::ConvertLighting(iDocumentNode *lightingSection)
{
}

bool csColladaConvertor::ConvertTextureShading(iDocumentNode *textureSection)
{
}

bool csColladaConvertor::ConvertRiggingAnimation(iDocumentNode *riggingSection)
{
}

bool csColladaConvertor::ConvertPhysics(iDocumentNode *physicsSection)
{
}