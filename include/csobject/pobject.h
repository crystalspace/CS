/*
    Copyright (C) 1998 by Jorrit Tyberghein
    csObject library (C) 1999 by Ivan Avramovic <ivan@avramovic.com>
  
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

#ifndef __POBJECT_H_
#define __POBJECT_H_

#include "csobject/csobj.h"

/**
 * A csObject which maintains a reference to the parent csObject.
 */
class csPObject : public csObject
{
protected:
  ///
  csObject* parent;
  ///
  virtual void SetObjectParent(csObject* p) { parent = p; }

public:
  /// 
  csPObject() : csObject(), parent(NULL) {}
  ///
  virtual csObject* GetObjectParent() const { return parent; }
  
  CSOBJTYPE;
};

#endif /* __POBJECT_H_ */
