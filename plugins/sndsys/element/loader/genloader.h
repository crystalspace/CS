/*
    Copyright (C) 2005 by Andrew Mann

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

#ifndef SNDSYS_GENLOADER_H
#define SNDSYS_GENLOADER_H



// generic loader

class SndSysLoader : public iSndSysLoader
{
public:
  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (SndSysLoader);
    virtual bool Initialize (iObjectRegistry *reg)
    {
      return scfParent->Initialize(reg);
    }
  } scfiComponent;

  SndSysLoader (iBase *parent)
  {
    SCF_CONSTRUCT_IBASE (parent);
    SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  }

  virtual ~SndSysLoader ()
  {
    SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
    SCF_DESTRUCT_IBASE();
  }


  virtual bool Initialize (iObjectRegistry *reg);
  virtual csPtr<iSndSysData> LoadSound (iDataBuffer* Buffer);

protected:
  csRef<iSndSysLoader> wavloader;
  csRef<iSndSysLoader> oggloader;
};

#endif // #ifndef SNDSYS_GENLOADER_H




