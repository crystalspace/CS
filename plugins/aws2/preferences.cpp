#include "cssysdef.h"

#include "preferences.h"
#include "xml_def.h"

#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/reporter.h"
#include "iutil/vfs.h"
#include "iutil/databuff.h"

namespace aws
{

bool preferences::load(iObjectRegistry* objreg, const std::string& filename)
{
	csPrintf("aws: Loading definitions file \"%s\"...\n", filename.c_str());

	csRef<iVFS> vfs (CS_QUERY_REGISTRY (objreg, iVFS));

	if (!vfs)
	{
		csPrintf("aws: Unable to load VFS plugin.\n");
		return false;
	}

	csRef<iFile> input = vfs->Open (filename.c_str(), VFS_FILE_READ);

	if (!input)
	{
		csPrintf("aws: Unable to open file \"%s\".\n", filename.c_str());
		return false;
	}

	// Read the whole file.
	csRef<iDataBuffer> buf = input->GetAllData();

	defFile df;

	// Parse the definition file into the root registry object.
	df.Parse(buf->GetData(), root);

    return true;
}



}