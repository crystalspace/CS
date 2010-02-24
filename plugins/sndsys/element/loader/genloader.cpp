/*
    Copyright (C) 2005 by Andrew Mann

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
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "isndsys/ss_structs.h"
#include "isndsys/ss_data.h"
#include "isndsys/ss_loader.h"


#include "genloader.h"





SCF_IMPLEMENT_FACTORY (SndSysLoader)



csPtr<iSndSysData> SndSysLoader::LoadSound (iDataBuffer* Buffer, const char *pDescription)
{
  csRef<iSndSysData> data;
  if (m_pWavLoader)
  {
    data=m_pWavLoader->LoadSound(Buffer, pDescription);
    if (data.IsValid())
      return csPtr<iSndSysData> (data);
  }
  if (m_pOggLoader)
  {
    data=m_pOggLoader->LoadSound(Buffer, pDescription);
    if (data.IsValid())
      return csPtr<iSndSysData> (data);
  }
  if (m_pSpeexLoader)
  {
    data=m_pSpeexLoader->LoadSound(Buffer, pDescription);
    if (data.IsValid())
      return csPtr<iSndSysData> (data);
  }
  return 0;
}


bool SndSysLoader::Initialize (iObjectRegistry *reg)
{
  csRef<iPluginManager> mgr=csQueryRegistry<iPluginManager> (reg);
  m_pWavLoader=csLoadPlugin<iSndSysLoader> (mgr,
    "crystalspace.sndsys.element.wav");
  m_pOggLoader=csLoadPlugin<iSndSysLoader> (mgr,
    "crystalspace.sndsys.element.ogg");
  m_pSpeexLoader=csLoadPlugin<iSndSysLoader> (mgr,
    "crystalspace.sndsys.element.speex", false);
  return true;
}



