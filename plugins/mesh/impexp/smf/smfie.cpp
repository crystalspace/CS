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

class csModelConverterSMF : iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterSMF (iBase *pBase);

  /// destructor
  virtual ~csModelConverterSMF ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual int GetFormatCount() const;
  virtual const csModelConverterFormat *GetFormat( int idx ) const;
  virtual iModelData *Load( UByte* Buffer, ULong size );
  virtual iDataBuffer *Save( iModelData*, const char *format );

  struct Plugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterSMF);
    virtual bool Initialize (iObjectRegistry *object_reg)
    { 
      return scfParent->Initialize (object_reg);
    }
  } scfiPlugin;
};

SCF_IMPLEMENT_IBASE (csModelConverterSMF)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterSMF::Plugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterSMF)

SCF_EXPORT_CLASS_TABLE (smfie)
  SCF_EXPORT_CLASS (csModelConverterSMF, 
    "crystalspace.modelconverter.smf",
    "SMF Model Converter")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csModelConverterSMF::csModelConverterSMF (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);

  FormatInfo.Name = "smf";
  FormatInfo.CanLoad = false;
  FormatInfo.CanSave = false;
}

csModelConverterSMF::~csModelConverterSMF ()
{
}

bool csModelConverterSMF::Initialize (iObjectRegistry *objreg)
{
  return true;
}

int csModelConverterSMF::GetFormatCount () const
{
  return 1;
}

const csModelConverterFormat *csModelConverterSMF::GetFormat (int idx) const
{
  return (idx == 0) ? &FormatInfo : NULL;
}

iModelData *csModelConverterSMF::Load (UByte * /*Buffer*/, ULong /*Size*/)
{
  return NULL;
}

iDataBuffer *csModelConverterSMF::Save (iModelData *Data, const char *Format)
{
  if (strcasecmp (Format, "smf"))
    return NULL;

  return NULL;
}
