/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
   $Id$

    This file is part of Crystal Space Virtual Object System Abstract
    3D Layer plugin (csvosa3dl).

    Copyright (C) 2004 Peter Amstutz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _VOSSECTOR_H_
#define _VOSSECTOR_H_

#include "inetwork/vosa3dl.h"
#include "iutil/objreg.h"
#include "iengine/engine.h"
#include "iengine/sector.h"

#include <vos/metaobjects/a3dl/a3dl.hh>

class csMetaSector;

class csVosSector : public iVosSector, public iVosApi,
                    public VOS::ChildChangeListener
{
private:
  bool didLoad;
  iObjectRegistry* objreg;
  csRef<iEngine> engine;
  csRef<iSector> sector;
  csVosA3DL* vosa3dl;
  csRef<iProgressMeter> meter;
  VUtil::vRef<csMetaSector> sectorvobj;

  csSet < csPtrKey<iVosObject3D> > loadedObjects;

  bool isLit;
  int waitingForChildren;

  virtual void notifyChildInserted (VOS::VobjectEvent &event);
  virtual void notifyChildRemoved (VOS::VobjectEvent &event);
  virtual void notifyChildReplaced (VOS::VobjectEvent &event);

public:
  SCF_DECLARE_IBASE;

  csVosSector(iObjectRegistry *o, csVosA3DL* vosa3dl, csMetaSector* sec);
  virtual ~csVosSector();

  virtual void Load(iProgressMeter* progress = 0);
  virtual csRef<iSector> GetSector();

  virtual VUtil::vRef<VOS::Vobject> GetVobject();

  virtual const csSet< csPtrKey<iVosObject3D> > &GetObject3Ds();

  void addObject3D (iVosObject3D *obj);
  void removeObject3D (iVosObject3D *obj);

  //void addLight (iVosLight *light);
  //void removeLight (iVosLight *light);

  friend class LoadSectorTask;
  friend class RelightTask;
};

class csMetaSector : public virtual A3DL::Sector
{
private:
  csRef<csVosSector> csvossector;

public:
  csMetaSector(VOS::VobjectBase* superobject);
  virtual ~csMetaSector() { }

  static VOS::MetaObject* new_csMetaSector(VOS::VobjectBase* superobject,
    const std::string& type);

  csRef<csVosSector> GetCsVosSector() { return csvossector; }
  void SetCsVosSector(csRef<csVosSector> s) { csvossector = s; }
};


#endif
