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
#include "iutil/objreg.h"
#include "iutil/strvec.h"
#include "csutil/csstring.h"
#include "csutil/typedvec.h"
#include "isys/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imesh/mdlconv.h"
#include "imesh/mdldata.h"

#define MY_CLASSNAME	"crystalspace.modelconverter.multiplexer"

CS_DECLARE_TYPED_VECTOR_NODELETE (csModelConverterVector, iModelConverter);
CS_DECLARE_TYPED_VECTOR_NODELETE (csModelConverterFormatVector, const csModelConverterFormat);

class csModelConverterMultiplexer : iModelConverter
{
public:
  SCF_DECLARE_IBASE;
  csModelConverterVector Converters;
  csModelConverterFormatVector Formats;

  /// constructor
  csModelConverterMultiplexer (iBase *p);

  /// destructor
  virtual ~csModelConverterMultiplexer ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual int GetFormatCount () const;
  virtual const csModelConverterFormat *GetFormat (int idx) const;
  virtual iModelData *Load (UByte* Buffer, ULong Size);
  virtual iDataBuffer *Save (iModelData*, const char *Format);

  struct Component : public iComponent {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterMultiplexer);
    virtual bool Initialize (iObjectRegistry *object_reg)
    { return scfParent->Initialize (object_reg); }
  } scfiComponent;
};

SCF_IMPLEMENT_IBASE (csModelConverterMultiplexer)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterMultiplexer::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterMultiplexer);

SCF_EXPORT_CLASS_TABLE (ieplex)
  SCF_EXPORT_CLASS (csModelConverterMultiplexer,
    MY_CLASSNAME,
    "Multiplexer for Model Converters")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csModelConverterMultiplexer::csModelConverterMultiplexer (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csModelConverterMultiplexer::~csModelConverterMultiplexer ()
{
  // don't delete the elements of the 'formats' vector. We don't own them!

  while (Converters.Length () > 0)
  {
    iModelConverter *conv = Converters.Pop ();
    conv->DecRef ();
  }
}

bool csModelConverterMultiplexer::Initialize (iObjectRegistry *object_reg)
{
  int i, j;
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  // @@@ collect converter plugins
  iStrVector* classlist =
      iSCF::SCF->QueryClassList ("crystalspace.modelconverter.");
  int const nmatches = classlist->Length();
  for (i = 0; i < nmatches; i++)
  {
    char const* classname = classlist->Get(i);
    if (!strcasecmp (classname, MY_CLASSNAME)) continue;

    iModelConverter *ldr = CS_LOAD_PLUGIN (plugin_mgr, classname, 0, iModelConverter);
    if (ldr)
      Converters.Push(ldr);
  }

  for (i=0; i<Converters.Length (); i++)
  {
    iModelConverter *mconv = Converters.Get(i);
    for (j=0; j<mconv->GetFormatCount (); j++)
      Formats.Push (mconv->GetFormat (j));
  }

  return true;
}

int csModelConverterMultiplexer::GetFormatCount () const
{
  return Formats.Length ();
}

const csModelConverterFormat *csModelConverterMultiplexer::GetFormat (int idx) const
{
  return Formats.Get (idx);
}

iModelData *csModelConverterMultiplexer::Load (UByte* Buffer, ULong Size)
{
  int i;
  for (i=0; i<Converters.Length (); i++)
  {
    iModelData *mdl = Converters.Get(i)->Load (Buffer, Size);
    if (mdl) return mdl;
  }
  return NULL;
}

iDataBuffer *csModelConverterMultiplexer::Save (iModelData *mdl, const char *Format)
{
  int i;
  for (i=0; i<Converters.Length (); i++)
  {
    iDataBuffer *dbuf = Converters.Get(i)->Save (mdl, Format);
    if (dbuf) return dbuf;
  }
  return NULL;
}
