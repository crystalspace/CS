/* $Id$ */
/*
    This file is part of Ter'Angreal, a 3D VR application
    using VOS and CrystalSpace from interreality.org

    Copyright (C) 2002 Peter Amstutz

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

#ifndef _VOSTEXTURE_H_
#define _VOSTEXTURE_H_

#include <vos/vos/vos.hh>
#include <vos/metaobjects/property/property.hh>
#include <vos/metaobjects/property/propertylistener.hh>
#include <vos/metaobjects/a3dl/a3dl.hh>

#include "iengine/texture.h"
#include "iutil/objreg.h"
#include "csvosa3dl.h"

class ConstructTextureTask;

class csMetaTexture : public A3DL::Texture, VOS::ChildChangeListener, VOS::PropertyListener
{
private:
    csRef<iTextureWrapper> texturewrapper;
    csRef<iObjectRegistry> object_reg;
    bool alreadyLoaded;

public:
    csMetaTexture(VOS::VobjectBase* superobject);
    virtual ~csMetaTexture();

    /** Fully load this object into Crystal Space */
    virtual void setup(csVosA3DL* va);

    /** Return CS iTextureWrapper interface for this object */
    csRef<iTextureWrapper> getTextureWrapper();

    virtual void notifyPropertyChange(const VOS::PropertyEvent& event);
    virtual void notifyChildInserted(VOS::VobjectEvent& event);
    virtual void notifyChildReplaced(VOS::VobjectEvent& event);
    virtual void notifyChildRemoved(VOS::VobjectEvent& event);

    static VOS::MetaObject* new_csMetaTexture(VOS::VobjectBase* superobject, const std::string& type);

    friend class ConstructTextureTask;
};

#endif
