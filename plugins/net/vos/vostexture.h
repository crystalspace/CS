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

#ifndef _VOSTEXTURE_H_
#define _VOSTEXTURE_H_

#include <vos/vos/vos.hh>
#include <vos/metaobjects/property/property.hh>
#include <vos/metaobjects/property/propertylistener.hh>
#include <vos/metaobjects/a3dl/a3dl.hh>

#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iutil/objreg.h"
#include "csvosa3dl.h"

class ConstructTextureTask;
struct iImageIO;

class csMetaTexture :
  public A3DL::Texture,
  public VOS::ChildChangeListener,
  public VOS::PropertyListener
{
private:
  iObjectRegistry* object_reg;
  csRef<iTextureWrapper> texturewrapper;
  bool alreadyLoaded;
  csVosA3DL* vosa3dl;
  bool needListener;

public:
  csMetaTexture(VOS::VobjectBase* superobject);
  virtual ~csMetaTexture();

  /** Fully load this object into Crystal Space */
  virtual void Setup(csVosA3DL* va);

  /** Return CS iTextureWrapper interface for this object */
  csRef<iTextureWrapper> GetTextureWrapper();

  virtual void notifyPropertyChange(const VOS::PropertyEvent& event);
  virtual void notifyChildInserted(VOS::VobjectEvent& event);
  virtual void notifyChildReplaced(VOS::VobjectEvent& event);
  virtual void notifyChildRemoved(VOS::VobjectEvent& event);

  static VOS::MetaObject* new_csMetaTexture(
    VOS::VobjectBase* superobject, const std::string& type);

  friend class ConstructTextureTask;
  friend class ConstructMozTextureTask;
};

#endif
