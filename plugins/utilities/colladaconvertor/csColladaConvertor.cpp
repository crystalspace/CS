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
#include "csColladaConvertor.h"

CS_IMPLEMENT_PLUGIN
SCF_IMPLEMENT_FACTORY(csColladaConvertor)

void csColladaConvertor::Report(int severity, const char* msg, ...)
{
	va_list argList;
	va_start(argList, msg);

	csRef<iReporter> rep = csQueryRegistry<iReporter>(obj_reg);
	if (rep.IsValid())
	{
		rep->ReportV(severity, "crystalspace.colladaconvertor", msg, argList); 
	}
	else
	{
		csPrintfV(msg, argList);
		csPrintf("\n");
	}

	va_end(argList);
}


csColladaConvertor::csColladaConvertor(iBase*	parent)	:
	scfImplementationType(this,	parent), 
	obj_reg(0),
	colladaReady(false),
	csReady(false)
{
}

csColladaConvertor::~csColladaConvertor()
{
	delete docSys;
}

bool csColladaConvertor::Initialize	(iObjectRegistry*	reg)
{
	obj_reg	=	reg;

	
	// create	our	own	document system, since we	will be	reading	and
	// writing to	the	XML	files
	docSys = new csTinyDocumentSystem();

/*
	// get a pointer to the virtual file system
	fileSys = csQueryRegistry<iVFS>(obj_reg);

	if (!fileSys.IsValid())
	{
		return false;
	}
*/

	return true;
}

const	char*	csColladaConvertor::Load(const char	*str,	csColladaFileType	typeEnum)
{
	//iFile* filePtr;

	Report(CS_REPORTER_SEVERITY_NOTIFY, "Inside Load!");
	/*
	if (!fileSys.IsValid())
	{
		return "Unable to acquire pointer to VFS.";
	}
*/

	return "0";
}

const	char*	csColladaConvertor::Load(iString *str, csColladaFileType typeEnum)
{
	return "0";
}

const	char*	csColladaConvertor::Load(iFile *file,	csColladaFileType	typeEnum)
{
	return "0";
}

const	char*	csColladaConvertor::Load(iDataBuffer *db,	csColladaFileType	typeEnum)
{
	return "0";
}

const	char*	csColladaConvertor::Convert()
{
	return "0";
}

bool csColladaConvertor::ConvertGeometry(iDocumentNode *geometrySection)
{
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