/*
    Copyright (C) 1998 by Jorrit Tyberghein
    csObject library (C) 1999 by Ivan Avramovic <ivan@avramovic.com>
    csColliderPointerObject written by Thomas Hieber <thieber@gmx.net>
  
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

#ifndef __CDOBJ_H_
#define __CDOBJ_H_

#include "csobject/csobj.h"

class csCollider;

/**
 * A csColliderPointerObject which contains a pointer to a Collider Object.
 * This object is used to attach colliders to existing csObjects.
 */
class csColliderPointerObject : public csObject
{
public:
  /// Initialize this object
  csColliderPointerObject(csCollider* pCollider, bool AutoDelete);

  /// Remove the according collider
  ~csColliderPointerObject();

  /// Retrieve the collider of a csObject, if one exists.
  static csCollider* GetCollider(csObject& csobj);

  /// Give a collider to the given csobject.
  static void SetCollider(csObject& csobj, csCollider* pCollider, bool AutoDelete);

  CSOBJTYPE;

protected:
  csCollider* m_pCollider;
  bool        m_AutoDelete;
};

#endif /* __NAMEOBJ_H_ */
