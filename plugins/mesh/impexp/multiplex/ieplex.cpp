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
#include "iutil/databuff.h"
#include "csutil/csstring.h"
#include "csutil/typedvec.h"
#include "isys/system.h"
#include "isys/plugin.h"
#include "imesh/mdlconv.h"
#include "imesh/mdldata.h"

CS_DECLARE_TYPED_VECTOR_NODELETE (csModelConverterVector, iModelConverter);
CS_DECLARE_TYPED_VECTOR_NODELETE (csStringVector, csString);

class csModelConverterMultiplexer : iModelConverter
{
public:
  SCF_DECLARE_IBASE;
  csModelConverterVector Converters;
  csStringVector Formats;

  /// constructor
  csModelConverterMultiplexer (iBase *p);

  /// destructor
  virtual ~csModelConverterMultiplexer ();

  bool Initialize (iSystem *System);
  virtual int GetFormatCount ();
  virtual const char *GetFormat (int idx);
  virtual bool SupportsFormat (const char *Format);
  virtual iModelData *Load (UByte* Buffer, ULong Size);
  virtual iDataBuffer *Save (iModelData*, const char *Format);

  struct Plugin : public iPlugin {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterMultiplexer);
    virtual bool Initialize (iSystem *sys)
    { return scfParent->Initialize (sys); }
  } scfiPlugin;
};

SCF_IMPLEMENT_IBASE (csModelConverterMultiplexer)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterMultiplexer::Plugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterMultiplexer);

SCF_EXPORT_CLASS_TABLE (ieplex)
  SCF_EXPORT_CLASS (csModelConverterMultiplexer,
    "crystalspace.modelconverter.multiplexer",
    "Multiplexer for Model Converters")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csModelConverterMultiplexer::csModelConverterMultiplexer (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);
}

csModelConverterMultiplexer::~csModelConverterMultiplexer ()
{
}

bool csModelConverterMultiplexer::Initialize (iSystem *sys)
{
  // @@@ collect converter plugins

  for (int i=0; i<Converters.Length (); i++)
  {
    iModelConverter *mconv = Converters.Get(i);
    for (int j=0; j<mconv->GetFormatCount (); j++)
      Formats.Push (new csString (mconv->GetFormat (j)));
  }

  return true;
}

int csModelConverterMultiplexer::GetFormatCount ()
{
  return Formats.Length ();
}

const char *csModelConverterMultiplexer::GetFormat (int idx)
{
  return Formats.Get (idx)->GetData ();
}

bool csModelConverterMultiplexer::SupportsFormat (const char *Format)
{
  for (int i=0; i<Formats.Length (); i++)
    if (!strcmp(Formats.Get(i)->GetData (), Format))
      return true;
  return false;
}

iModelData *csModelConverterMultiplexer::Load (UByte* Buffer, ULong Size)
{
  for (int i=0; i<Converters.Length (); i++)
  {
    iModelData *mdl = Converters.Get(i)->Load (Buffer, Size);
    if (mdl) return mdl;
  }
  return NULL;
}

iDataBuffer *csModelConverterMultiplexer::Save (iModelData*mdl, const char *Format)
{
  for (int i=0; i<Converters.Length (); i++)
  {
    iDataBuffer *dbuf = Converters.Get(i)->Save (mdl, Format);
    if (dbuf) return dbuf;
  }
  return NULL;
}

