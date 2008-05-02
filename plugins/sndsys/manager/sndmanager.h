/*
    Copyright (C) 2005-2006 by Jorrit Tyberghein

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

#ifndef __CS_SNDMANAGER_H__
#define __CS_SNDMANAGER_H__

#include "csutil/cfgacc.h"
#include "csutil/parray.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "csutil/csobject.h"
#include "iutil/comp.h"
#include "iutil/csinput.h"
#include "iutil/eventh.h"
#include "iutil/plugin.h"
#include "iutil/selfdestruct.h"
#include "isndsys/ss_manager.h"
#include "isndsys/ss_stream.h"

class csSndSysManager;

#include "csutil/deprecated_warn_off.h"

/**
 * Sound wrapper.
 */
class csSndSysWrapper : public scfImplementationExt2<csSndSysWrapper,
						     csObject,
						     iSndSysWrapper,
						     iSelfDestruct>
{
private:
  csSndSysManager* mgr;
  csRef<iSndSysData> data;

protected:
  void InternalRemove() { SelfDestruct(); }

public:
  csSndSysWrapper (csSndSysManager* mgr, const char* name)
  	: scfImplementationType (this), mgr (mgr)
  {
    SetName (name);
  }
  virtual ~csSndSysWrapper () { }

  virtual iObject* QueryObject () { return (iObject*)this; }
  virtual iSndSysData* GetData () { return data; }
  /// Set the sound data associated with this wrapper.
  virtual void SetData (iSndSysData* data)
  {
    csSndSysWrapper::data = data;
  }

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();
};

#include "csutil/deprecated_warn_on.h"

/**
 * Sound manager plugin.
 */
class csSndSysManager : public scfImplementation2<csSndSysManager,
	iSndSysManager, iComponent>

{
private:
  iObjectRegistry* object_reg;

  csRefArray<csSndSysWrapper> sounds;

public:
  csSndSysManager (iBase* parent) : scfImplementationType (this, parent)
  {
  }

  virtual ~csSndSysManager ()
  {
  }

  /// For iComponent.
  virtual bool Initialize (iObjectRegistry *object_reg);

  virtual iSndSysWrapper* CreateSound (const char* name);
  virtual void RemoveSound (iSndSysWrapper* snd);
  virtual void RemoveSound (size_t idx);
  virtual void RemoveSounds ();
  virtual size_t GetSoundCount () const { return sounds.GetSize (); }
  virtual iSndSysWrapper* GetSound (size_t idx);
  virtual iSndSysWrapper* FindSoundByName (const char* name);
};

#endif // __CS_SNDMANAGER_H__
