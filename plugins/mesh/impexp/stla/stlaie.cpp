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

class csModelConverterSTLA : iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterSTLA (iBase *pBase);

  /// destructor
  virtual ~csModelConverterSTLA ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual int GetFormatCount() const;
  virtual const csModelConverterFormat *GetFormat( int idx ) const;
  virtual iModelData *Load( UByte* Buffer, ULong size );
  virtual iDataBuffer *Save( iModelData*, const char *format );

  struct Plugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterSTLA);
    virtual bool Initialize (iObjectRegistry *object_reg)
    { 
      return scfParent->Initialize (object_reg);
    }
  } scfiPlugin;
};

SCF_IMPLEMENT_IBASE (csModelConverterSTLA)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterSTLA::Plugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterSTLA)

SCF_EXPORT_CLASS_TABLE (stlaie)
  SCF_EXPORT_CLASS (csModelConverterSTLA, 
    "crystalspace.modelconverter.stla",
    "STLA Model Converter")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csModelConverterSTLA::csModelConverterSTLA (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);

  FormatInfo.Name = "stla";
  FormatInfo.CanLoad = false;
  FormatInfo.CanSave = false;
}

csModelConverterSTLA::~csModelConverterSTLA ()
{
}

bool csModelConverterSTLA::Initialize (iObjectRegistry *objreg)
{
  return true;
}

int csModelConverterSTLA::GetFormatCount () const
{
  return 1;
}

const csModelConverterFormat *csModelConverterSTLA::GetFormat (int idx) const
{
  return (idx == 0) ? &FormatInfo : NULL;
}

iModelData *csModelConverterSTLA::Load (UByte * /*Buffer*/, ULong /*Size*/)
{
  return NULL;
}

iDataBuffer *csModelConverterSTLA::Save (iModelData *Data, const char *Format)
{
  if (strcasecmp (Format, "stla"))
    return NULL;

  return NULL;
}
