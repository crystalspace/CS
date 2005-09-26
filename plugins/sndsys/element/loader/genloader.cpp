/*
    Copyright (C) 2005 by Andrew Mann

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "isndsys/ss_structs.h"
#include "isndsys/ss_data.h"
#include "isndsys/ss_loader.h"


#include "genloader.h"


CS_IMPLEMENT_PLUGIN


SCF_IMPLEMENT_IBASE (SndSysLoader)
  SCF_IMPLEMENTS_INTERFACE (iSndSysLoader)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (SndSysLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (SndSysLoader);



csPtr<iSndSysData> SndSysLoader::LoadSound (iDataBuffer* Buffer)
{
  csRef<iSndSysData> data;
  if (wavloader)
  {
    data=wavloader->LoadSound(Buffer);
    if (data.IsValid())
      return csPtr<iSndSysData> (data);
  }
  if (oggloader)
  {
    data=oggloader->LoadSound(Buffer);
    if (data.IsValid())
      return csPtr<iSndSysData> (data);
  }
  return 0;
}


bool SndSysLoader::Initialize (iObjectRegistry *reg)
{
  csRef<iPluginManager> mgr=CS_QUERY_REGISTRY(reg, iPluginManager);
  wavloader=CS_LOAD_PLUGIN(mgr, "crystalspace.sndsys.element.wav", iSndSysLoader);
  oggloader=CS_LOAD_PLUGIN(mgr, "crystalspace.sndsys.element.ogg", iSndSysLoader);
  return true;
}



