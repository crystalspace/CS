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

// Pseudo-RTTI, since AFAIK we still allow gcc 2.7.x which has buggy RTTI

#include "sysdef.h"

typedef const char* csIdStr;


///
class csIdType
{
private:
  ///
  unsigned char length;
  ///
  csIdStr* entries;
  ///
  const csIdType* base;
  ///
  static csIdStr default_idstr;

public:
  ///
  csIdType() : length(0), entries(&default_idstr), base(NULL) {}
  ///
  csIdType(const csIdType& id) : 
    length(id.length), entries(id.entries), base(id.base) {}

  ///
  friend class csIdFunc;

  ///
  ~csIdType() {}

  /// Returns the level of polymorphism of this object
  int Length() const { return length; }

  /// Get the base type of this type.
  const csIdType* GetBase() const { return base; }

  ///
  csIdStr operator*() const { return entries[length]; }

  ///
  bool operator< (const csIdType& id) const 
  { return length < id.length && entries[length] == id.entries[length]; }

  ///
  bool operator<= (const csIdType& id) const 
  { return length <= id.length && entries[length] == id.entries[length]; }

  ///
  bool operator> (const csIdType& id) const 
  { return length > id.length && entries[id.length] == id.entries[id.length]; }

  ///
  bool operator>= (const csIdType& id) const
  { return length >= id.length && entries[id.length] == id.entries[id.length]; }

  ///
  bool operator==(const csIdType& id) const 
  { return length==id.length && entries[length]==id.entries[length]; }

  ///
  bool operator!=(const csIdType& id) const 
  { return !(*this<=id) && !(id<=*this); }
};

/**
 * A static class with static member functions, for manipulating csIdType
 */
class csIdFunc
{
public:
/**
 * The following function is used to allocate static csIdType instances.
 * Since it allocates an array that is never freed, it should only be
 * invoked for static variables that exist for the duration of the program.
 */
static csIdType Allocate(csIdStr s, const csIdType& id);

/**
 * Returns the csIdType common to both entries.
 * This function will return the csIdType representing the highest-derived
 * class which is common to both.
 */
static csIdType GetCommon(const csIdType& t1, const csIdType& t2);

};

class NULLCLASS
{ public:  static const csIdType& Type(); };

/**
 * Function declarations for a psuedoRTTI class.
 * Include this define in the definition of every class that uses the 
 * psuedoRTTI system.
 */
#define CSOBJTYPE \
  public:                                                          \
    static csIdStr IdStr();                                        \
    static const csIdType& Type();                                 \
    virtual csIdStr GetIdStr() const { return IdStr(); }           \
    virtual const csIdType& GetType() const { return Type(); } 

/**
 * The function implementations for a psuedoRTTI class.
 * For every class using the psuedoRTTI system, include this define
 * with the class name and parent class name in the corresponding .cpp file.
 */
#define CSOBJTYPE_IMPL(thisclass,parentclass) \
  csIdStr thisclass::IdStr()                                       \
  { static csIdStr t = #thisclass;  return t; }                    \
  const csIdType& thisclass::Type()                                \
  { static csIdType t = csIdFunc::Allocate(IdStr(),                \
    parentclass::Type());  return t; }

#endif
