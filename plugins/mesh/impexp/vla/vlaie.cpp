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

class csModelConverterVLA : iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterVLA (iBase *pBase);

  /// destructor
  virtual ~csModelConverterVLA ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual int GetFormatCount() const;
  virtual const csModelConverterFormat *GetFormat( int idx ) const;
  virtual iModelData *Load( UByte* Buffer, ULong size );
  virtual iDataBuffer *Save( iModelData*, const char *format );

  struct Plugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterVLA);
    virtual bool Initialize (iObjectRegistry *object_reg)
    { 
      return scfParent->Initialize (object_reg);
    }
  } scfiPlugin;
};

SCF_IMPLEMENT_IBASE (csModelConverterVLA)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterVLA::Plugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterVLA)

SCF_EXPORT_CLASS_TABLE (vlaie)
  SCF_EXPORT_CLASS (csModelConverterVLA, 
    "crystalspace.modelconverter.vla",
    "VLA Model Converter")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csModelConverterVLA::csModelConverterVLA (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);

  FormatInfo.Name = "vla";
  FormatInfo.CanLoad = false;
  FormatInfo.CanSave = false;
}

csModelConverterVLA::~csModelConverterVLA ()
{
}

bool csModelConverterVLA::Initialize (iObjectRegistry *objreg)
{
  return true;
}

int csModelConverterVLA::GetFormatCount () const
{
  return 1;
}

const csModelConverterFormat *csModelConverterVLA::GetFormat (int idx) const
{
  return (idx == 0) ? &FormatInfo : NULL;
}

iModelData *csModelConverterVLA::Load (UByte * /*Buffer*/, ULong /*Size*/)
{
  return NULL;
}

iDataBuffer *csModelConverterVLA::Save (iModelData *Data, const char *Format)
{
  if (strcasecmp (Format, "vla"))
    return NULL;

  return NULL;
}
