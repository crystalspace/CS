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

#ifndef __FAKERTTI_H_
#define __FAKERTTI_H_

/**
 * Pseudo-RTTI, to avoid using "normal" rtti which is a serious size overhead.
 * We need rtti only for a few objects and a limited set of functions only.
 * Because of this we provide a fake RTTI system that does everything we need;
 * it is suggested to compile the engine without RTTI to create much smaller
 * executables.<p>
 * The most noticeable limitation is that it does not support multiple
 * inheritance for objects with fake RTTI. You still can derive classes from
 * multiple parents, but you should take care to not derive a class from more
 * than one class that inherits from csObject.
 */

/**
 * A class that represents the dynamic type of an object.
 * Under the pseudo RTTI system, this class represents the type of
 * an object, which can be retrieved via the Type field or GetType() call.
 * The csIdType is designed to occupy minimal possible storage as there
 * can be lots of RTTI objects (for example, thousands of polygons).
 * In fact, the overhead of our RTTI is just the size of one method entry
 * in each object's vtable plus size of one csIdType object (two pointers)
 * for each existing RTTI class.
 */
class csIdType
{
public:
  /// A pointer to the base type
  const csIdType *Base;
  /// The object type ID string
  const char *ID;

  /// Create a csIdType object given parent and class ID string
  csIdType (const csIdType &iParent, const char *iID) : Base (&iParent), ID (iID)
  { }

  /// Create the top-level csIdType object (with no parent)
  csIdType (const char *iID) : Base (NULL), ID (iID)
  { }

  /// Returns the level of polymorphism of this object
  int Length () const
  {
    int depth = 0;
    for (const csIdType *c = Base; c; c = c->Base, depth++)
      ;
    return depth;
  }

#define COMPARE_TYPE(startval,stopval)			\
  const csIdType *c;					\
  for (c = startval; c && c != stopval; c = c->Base) ;	\
  return (c != NULL);

  /// Compares whether one object's type is derived from the other.
  bool operator < (const csIdType& id) const
  { COMPARE_TYPE (id.Base, this); }

  /// Compares whether one object's type is derived from the other.
  bool operator <= (const csIdType& id) const 
  { COMPARE_TYPE (&id, this); }

  /// Compares whether one object's type is derived from the other.
  bool operator > (const csIdType& id) const 
  { COMPARE_TYPE (Base, &id); }

  /// Compares whether one object's type is derived from the other.
  bool operator >= (const csIdType& id) const
  { COMPARE_TYPE (this, &id); }

#undef COMPARE_TYPE

  /// Compares whether the two objects have the same dynamic type.
  bool operator == (const csIdType& id) const 
  { return ID == id.ID; }

  /// Compares whether neither object is derived from the other.
  bool operator != (const csIdType& id) const
  { return !(*this <= id) && !(id <= *this); }
};

/**
 * Function declarations for a pseudoRTTI class.
 * Include this define in the definition of every class that uses the 
 * psuedoRTTI system.
 */
#define CSOBJTYPE							\
  public:								\
    static const csIdType Type;						\
    virtual const csIdType& GetType () const;

/**
 * The function implementations for a pseudoRTTI class.
 * For every class using the pseudoRTTI system, include this define
 * with the class name and parent class name in the corresponding .cpp file.
 */
#define IMPLEMENT_CSOBJTYPE(thisclass,parentclass)			\
  const csIdType thisclass::Type (parentclass::Type, #thisclass);	\
  const csIdType& thisclass::GetType () const				\
  { return Type; }

#endif
