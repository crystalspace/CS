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

#ifndef __NAMEOBJ_H_
#define __NAMEOBJ_H_

#include "csobject/csobj.h"

///
class csNameObject : public csObject
{
protected:
  ///
  char name[30];

public:
  /// Initialize this object with name 'n'
  csNameObject(const char* n);
  /// Retrieve the name of a csObject, if one exists.
  static const char* GetName(csObject& csobj);
  /// Tag a name onto the given csobject.
  static void AddName(csObject& csobj, const char* name);
  ///
  const char* Name() const { return name; }
  
  CSOBJTYPE;
};

#endif /* __NAMEOBJ_H_ */
