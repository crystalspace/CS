/*
    Copyright (C) 2005 by Christopher Nelson

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

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

bool preferences::load(iObjectRegistry* objreg, const scfString& filename)
{
	csPrintf("aws: Loading definitions file \"%s\"...\n", filename.GetData());

	csRef<iVFS> vfs (CS_QUERY_REGISTRY (objreg, iVFS));

	if (!vfs)
	{
		csPrintf("aws: Unable to load VFS plugin.\n");
		return false;
	}

	csRef<iFile> input = vfs->Open (filename.GetData(), VFS_FILE_READ);

	if (!input)
	{
		csPrintf("aws: Unable to open file \"%s\".\n", filename.GetData());
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
