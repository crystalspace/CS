/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
   $Id$

    This file is part of the VOS plugin for Crystal Space.
    For more information about VOS, go to http://interreality.org.

    Copyright (C) 2004 Peter Amstutz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _VOSLIGHT_H_
#define _VOSLIGHT_H_

#include <vos/metaobjects/a3dl/a3dl.hh>
#include <vos/metaobjects/property/propertylistener.hh>

#include "inetwork/vosa3dl.h"
#include "csvosa3dl.h"
#include "vossector.h"

class csMetaLight : public A3DL::Light, public VOS::PropertyListener
{
protected:
  iObjectRegistry *object_reg;
  csRef<iSector> sector;
  csRef<iLight> light;

  // light already created & listening to property updates
  bool alreadyLoaded;
  // True if a static light. Updates to subproperties will have no effect.
  bool isStatic;
public:
  csMetaLight(VOS::VobjectBase* superobject);
  virtual ~csMetaLight();

  virtual void Setup(csVosA3DL* vosa3dl, csVosSector* sect);

  virtual void notifyPropertyChange(const VOS::PropertyEvent& event);
  virtual void doExcise();

  static VOS::MetaObject* new_csMetaLight(VOS::VobjectBase* superobject,
  	const std::string& type);

  friend class ConstructLightTask;
};

#endif
