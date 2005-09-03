/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "cssysdef.h"

#include "iutil/databuff.h"
#include "iutil/objreg.h"

#include "ceguiresourceprovider.h"

#include "CEGUIExceptions.h"

csCEGUIResourceProvider::csCEGUIResourceProvider (iObjectRegistry *reg) :
  CEGUI::ResourceProvider ()
{
  obj_reg = reg;
  vfs = CS_QUERY_REGISTRY(obj_reg, iVFS);
}
csCEGUIResourceProvider::~csCEGUIResourceProvider ()
{
}

void csCEGUIResourceProvider::loadRawDataContainer (const CEGUI::String& filename,
    CEGUI::RawDataContainer& output, const CEGUI::String& resourceGroup)
{
  csRef<iDataBuffer> buffer = vfs->ReadFile (filename.c_str());

  // Reading failed
  if (!buffer.IsValid ())
  {
    CEGUI::String msg= (uint8*)"csCEGUIResourceProvider::loadRawDataContainer - "
      "Filename supplied for loading must be valid";
    msg += (uint8*)" ["+filename+(uint8*)"]";
    throw CEGUI::InvalidRequestException(msg);
  }
  else
  {
    uint8* data = new uint8[buffer->GetSize ()];
    memcpy (data, buffer->GetUint8 (), sizeof(uint8) * buffer->GetSize ());
    output.setData(data);
    output.setSize(buffer->GetSize ());
  }
}

void csCEGUIResourceProvider::unloadRawDataContainer (CEGUI::RawDataContainer& data)
{
  if (data.getDataPtr())
  {
    delete[] data.getDataPtr();
    data.setData(0);
    data.setSize(0);
  }
}
