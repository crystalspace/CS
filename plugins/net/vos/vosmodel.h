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

#ifndef _VOSMODEL_H_
#define _VOSMODEL_H_

#include <vos/metaobjects/a3dl/model.hh>
#include "inetwork/vosa3dl.h"
#include "csvosa3dl.h"
#include "vosobject3d.h"

class csMetaModel : public virtual csMetaObject3D, public virtual A3DL::Model
{
private:
  bool alreadyLoaded;
public:
  csMetaModel(VOS::VobjectBase* superobject);

  static VOS::MetaObject* new_csMetaModel(VOS::VobjectBase* superobject,
    const std::string& type);

  virtual void Setup(csVosA3DL* vosa3dl, csVosSector* sect);
};

#endif
