/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/databuff.h"
#include "imesh/mdlconv.h"
#include "cstool/mdldata.h"
#include "cstool/sprbuild.h"

class csModelConverterSPR : public iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterSPR (iBase *pBase);

  /// destructor
  virtual ~csModelConverterSPR ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual size_t GetFormatCount();
  virtual const csModelConverterFormat *GetFormat( size_t idx );
  virtual csPtr<iModelData> Load( uint8* Buffer, size_t size );
  virtual csPtr<iDataBuffer> Save( iModelData*, const char *format );

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterSPR);
    virtual bool Initialize (iObjectRegistry *object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

SCF_IMPLEMENT_IBASE (csModelConverterSPR)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterSPR::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterSPR)


CS_IMPLEMENT_PLUGIN

csModelConverterSPR::csModelConverterSPR (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  FormatInfo.Name = "spr";
  FormatInfo.CanLoad = false;
  FormatInfo.CanSave = true;
}

csModelConverterSPR::~csModelConverterSPR ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csModelConverterSPR::Initialize (iObjectRegistry *)
{
  return true;
}

size_t csModelConverterSPR::GetFormatCount ()
{
  return 1;
}

const csModelConverterFormat *csModelConverterSPR::GetFormat (size_t idx)
{
  return (idx == 0) ? &FormatInfo : 0;
}

csPtr<iModelData> csModelConverterSPR::Load (uint8 * /*Buffer*/, size_t /*Size*/)
{
  return 0;
}

/*
  Purpose:

    csModelConverterSPR::Save() writes a standard CS SPR file.

  Modified:

    12 April 2001

  Author:

    Luca Pancallo

  Modified by Martin Geisse to work with the new converter interface.
*/

csPtr<iDataBuffer> csModelConverterSPR::Save (iModelData *Data,
	const char *Format)
{
  if (strcasecmp (Format, "spr"))
    return 0;

  // only the first object is saved
  csRef<iModelDataObject> obj (
  	CS_GET_CHILD_OBJECT (Data->QueryObject (), iModelDataObject));
  if (!obj) return 0;

  csSpriteBuilderFile Builder;
  csRef<iDataBuffer> buf (Builder.Build (obj));
  return csPtr<iDataBuffer> (buf);
}

