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
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "imesh/mdlconv.h"
#include "cstool/mdldata.h"
#include "cstool/sprbuild.h"

class csModelConverterSPR : iModelConverter
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
  virtual int GetFormatCount() const;
  virtual const csModelConverterFormat *GetFormat( int idx ) const;
  virtual iModelData *Load( UByte* Buffer, ULong size );
  virtual iDataBuffer *Save( iModelData*, const char *format );

  struct Plugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterSPR);
    virtual bool Initialize (iObjectRegistry *object_reg)
    { 
      return scfParent->Initialize (object_reg);
    }
  } scfiPlugin;
};

SCF_IMPLEMENT_IBASE (csModelConverterSPR)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterSPR::Plugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterSPR)

SCF_EXPORT_CLASS_TABLE (sprie)
  SCF_EXPORT_CLASS (csModelConverterSPR, 
    "crystalspace.modelconverter.spr",
    "SPR Model Converter")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csModelConverterSPR::csModelConverterSPR (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);

  FormatInfo.Name = "spr";
  FormatInfo.CanLoad = false;
  FormatInfo.CanSave = false;
}

csModelConverterSPR::~csModelConverterSPR ()
{
}

bool csModelConverterSPR::Initialize (iObjectRegistry *objreg)
{
  return true;
}

int csModelConverterSPR::GetFormatCount () const
{
  return 1;
}

const csModelConverterFormat *csModelConverterSPR::GetFormat (int idx) const
{
  return (idx == 0) ? &FormatInfo : NULL;
}

iModelData *csModelConverterSPR::Load (UByte * /*Buffer*/, ULong /*Size*/)
{
  return NULL;
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

iDataBuffer *csModelConverterSPR::Save (iModelData *Data, const char *Format)
{
  if (strcasecmp (Format, "spr"))
    return NULL;

  // only the first object is saved
  iModelDataObject *obj = CS_GET_CHILD_OBJECT (Data->QueryObject (), iModelDataObject);
  if (!obj) return NULL;

  csSpriteBuilderFile Builder;
  iDataBuffer *buf = Builder.Build (obj);
  obj->DecRef ();
  return buf;
}
