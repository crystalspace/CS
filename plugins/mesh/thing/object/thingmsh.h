/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef _THINGMESH_H_
#define _THINGMESH_H_

#include "imesh/object.h"
#include "iutil/comp.h"
#include "imesh/thing/thing.h"

/**
 * Thing type. This is the plugin you have to use to create instances
 * of things.
 */
class csThingMeshObjectType : public iMeshObjectType
{
public:
  iObjectRegistry* object_reg;
  iMeshObjectType* parent_type;
  iThingEnvironment* te;

  SCF_DECLARE_IBASE;

  /// Constructor.
  csThingMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csThingMeshObjectType ();
  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg)
  {
    csThingMeshObjectType::object_reg = object_reg;
    return true;
  }
  /// Fetch the thing environment.
  iThingEnvironment* TE ();

  struct eiThingEnvironment : public iThingEnvironment
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingMeshObjectType);
    virtual iPolyTxtPlane* CreatePolyTxtPlane (const char* name = NULL)
    {
      return scfParent->TE ()->CreatePolyTxtPlane (name);
    }
    virtual iPolyTxtPlane* FindPolyTxtPlane (const char* name)
    {
      return scfParent->TE ()->FindPolyTxtPlane (name);
    }
    virtual iCurveTemplate* CreateBezierTemplate (const char* name = NULL)
    {
      return scfParent->TE ()->CreateBezierTemplate (name);
    }
    virtual iCurveTemplate* FindCurveTemplate (const char* name)
    {
      return scfParent->TE ()->FindCurveTemplate (name);
    }
    virtual void RemovePolyTxtPlane (iPolyTxtPlane* pl)
    {
      scfParent->TE ()->RemovePolyTxtPlane (pl);
    }
    virtual void RemoveCurveTemplate (iCurveTemplate* ct)
    {
      scfParent->TE ()->RemoveCurveTemplate (ct);
    }
    virtual void ClearPolyTxtPlanes ()
    {
      scfParent->TE ()->ClearPolyTxtPlanes ();
    }
    virtual void ClearCurveTemplates ()
    {
      scfParent->TE ()->ClearCurveTemplates ();
    }
  } scfiThingEnvironment;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingMeshObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // _THINGMESH_H_

