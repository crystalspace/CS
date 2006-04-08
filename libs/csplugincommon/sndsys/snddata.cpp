/*
    Copyright (C) 2006 by Andrew Mann

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
#include "csplugincommon/sndsys/snddata.h"


SndSysBasicData::SndSysBasicData(iBase *pParent) :
  scfImplementationType(this, pParent),
  m_bInfoReady(false), m_pDescription(0)
{


}

SndSysBasicData::~SndSysBasicData()
{
  delete[] m_pDescription;
}

const csSndSysSoundFormat *SndSysBasicData::GetFormat()
{
  if (!m_bInfoReady)
    Initialize();
  return &m_SoundFormat;
}

size_t SndSysBasicData::GetFrameCount()
{
  if (!m_bInfoReady)
    Initialize();
  return m_FrameCount;
}


void SndSysBasicData::SetDescription(const char *pDescription)
{
  delete[] m_pDescription;
  m_pDescription=0;

  if (!pDescription)
    return;

  m_pDescription=new char[strlen(pDescription)+1];
  strcpy(m_pDescription, pDescription);
}




