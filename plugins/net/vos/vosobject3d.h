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
#ifndef _CSVOSOBJECT3D_H_
#define _CSVOSOBJECT3D_H_

#include <vos/metaobjects/a3dl/object3d.hh>
#include <vos/metaobjects/property/propertylistener.hh>
#include <vos/vos/listener.hh>

#include "inetwork/vosa3dl.h"
#include "iengine/mesh.h"
#include "csutil/csobject.h"
#include "ivaria/dynamics.h"

#include "csvosa3dl.h"
#include "vossector.h"
#include "vosmaterial.h"

class csMetaObject3D;

class csVosObject3D : public csObject,
                      public iVosObject3D,
                      public iVosApi,
                      public iDynamicsMoveCallback
{
private:
  csRef<iMeshWrapper> meshwrapper;
  csRef<iRigidBody> collider;
  VUtil::vRef<csMetaObject3D> object3d;

public:
  SCF_DECLARE_IBASE;

  csVosObject3D(csMetaObject3D* obj3d, VUtil::RefCounted* rc);
  virtual ~csVosObject3D();

  virtual csRef<iMeshWrapper> GetMeshWrapper();

  void SetMeshWrapper(iMeshWrapper* mw);

  virtual csRef<iRigidBody> GetCollider();

  void SetCollider (iRigidBody *col);

  virtual VUtil::vRef<VOS::Vobject> GetVobject();

  virtual void Execute (csOrthoTransform &t);
  virtual void Execute (iMeshWrapper *mesh, csOrthoTransform &t);
};

class csMetaObject3D : public virtual A3DL::Object3D,
                       public VOS::PropertyListener,
                       public VOS::ChildChangeListener
{
protected:
  bool alreadyLoaded;
  csRef<csVosObject3D> csvobj3d;
  csRef<csVosA3DL> vosa3dl;
  csRef<csVosSector> sector;
  VUtil::vRef<csMetaMaterial> metamaterial;
  bool htvalid;

public:
  csMetaObject3D(VOS::VobjectBase* superobject);
  virtual ~csMetaObject3D();

  static VOS::MetaObject* new_csMetaObject3D(VOS::VobjectBase* superobject,
                                             const std::string& type);

  // Accessor for vosa3dl
  csVosA3DL *getVosA3DL () { return vosa3dl; }

  // Set up the object
  virtual void Setup(csVosA3DL* vosa3dl, csVosSector* sect);
  VUtil::Task* GetSetupTask(csVosA3DL* vosa3dl, csVosSector* sect);

  void updateMaterial ();

  csRef<csVosObject3D> GetCSinterface();
  VUtil::vRef<csMetaMaterial> getMetaMaterial() { return metamaterial; }

  // Child change listener for events
  virtual void notifyChildInserted (VOS::VobjectEvent &event);
  virtual void notifyChildRemoved (VOS::VobjectEvent &event);
  virtual void notifyChildReplaced (VOS::VobjectEvent &event);

  // Property listener and callbacks for object property events
  virtual void notifyPropertyChange(const VOS::PropertyEvent &event);

  // Call these from CS run loop.  Derived objects can override if they
  // do not correctly use movable interface
  virtual void changePosition (const csVector3 &pos);
  virtual void changeOrientation (const csMatrix3 &ori);

  virtual void changeMaterial (iMaterialWrapper* mat);

  // This is commented out because CS does not support dynamically changing
  // scaling of objects
  //virtual void changeScaling (const csMatrix3 &scaling)
};

#endif
