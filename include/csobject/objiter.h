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

#ifndef __OBJITER_H__
#define __OBJITER_H__

class csObject;
class csObjContainer;

/**
 * An iterator for a node in a csObject tree.
 * This object behaves similar to a pointer, and is used to traverse
 * through several csObjects in the tree.  
 * <p>
 * Example usage:
 * <pre>
 * csObjIterator i;
 * *i;   (returns a csObject&)
 * ++i;  (moves to the next csObject)
 * i.IsNull() (returns true if i has reached the end of the list)
 * </pre>
 */
class csObjIterator
{
protected:
  /// The type of objects this iterator handles
  const csIdType *Type; 
  /// A pointer to the container of the object which we are iterating
  csObjContainer *Container;
  /// The last index of child we examined
  int Index;

public:
  /// Create the iterator
  csObjIterator (const csIdType &iType, const csObject &iObject);

  /// Reuse the iterator for an other search
  void Reset(const csIdType &iType, const csObject &iObject);
  /// Get the object we are pointing at
  csObject* GetObj() const;
  /// Move forward
  void Next();
  /// Check if we have any children of requested type
  bool IsFinished () const { return Container == NULL; }
  /**
    * traverses all csObjects and looks for an object with the given name
    * returns true, if found, false if not found. You can then get the
    * object by calling GetObj, and can continue search by calling Next and
    * then do an other FindName, if you like.
    */
  bool FindName(const char* name);

  /// Check if we have any children of requested type
  bool IsNull () const {return IsFinished();}
  /// Get the object we are pointing at
  csObject& operator* () const {return *GetObj();}
  /// Move forward
  csObjIterator& operator++ () {Next(); return *this;}
};

#endif // __OBJITER_H__
