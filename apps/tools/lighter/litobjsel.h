/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __LITMESHSEL_H__
#define __LITMESHSEL_H__

#include <stdarg.h>
#include <csutil/util.h>
#include <csutil/ref.h>
#include <csutil/refarr.h>
#include <csutil/refcount.h>
#include <csutil/flags.h>
#include "csutil/regexp.h"
#include <iutil/object.h>
#include <iengine/mesh.h>
#include <imesh/object.h>

/**
 * This abstract class indicates when an object is selected or not
 * Subclasses of this class will control how exactly the object is selected.
 */
class litObjectSelect : public csRefCount
{
public:
  virtual ~litObjectSelect () { }

  /**
   * Return 0 if this selector is valid, otherwise returns an error
   * string with error.
   */
  virtual const char* IsValid () { return 0; }

  /**
   * Return true if this object should be selected.
   */
  virtual bool SelectObject (iObject*) = 0;
};

/**
 * Select an object based on name.
 */
class litObjectSelectByName : public litObjectSelect
{
private:
  char* name;

public:
  litObjectSelectByName (const char* name)
  {
    litObjectSelectByName::name = csStrNew (name);
  }
  virtual ~litObjectSelectByName ()
  {
    delete[] name;
  }
  virtual bool SelectObject (iObject* obj)
  {
    return strcmp (name, obj->GetName ()) == 0;
  }
};

/**
 * Select an object based on regexp on name.
 */
class litObjectSelectByNameRE : public litObjectSelect
{
private:
  csRegExpMatcher matcher;

public:
  litObjectSelectByNameRE (const char* name_re) : matcher (name_re) { }
  virtual ~litObjectSelectByNameRE () { }
  virtual const char* IsValid ()
  {
    csRegExpMatchError rc = matcher.Match ("");
    if (rc == NoError || rc == NoMatch) return 0;
    static char error_string[100];
    sprintf (error_string, "Regexp error %d", (int)rc);
    return error_string;
  }
  virtual bool SelectObject (iObject* obj)
  {
    return matcher.Match (obj->GetName ()) == NoError;
  }
};

/**
 * Select an object based on mesh wrapper flags.
 */
class litObjectSelectByMWFlags : public litObjectSelect
{
private:
  uint32 value, mask;

public:
  litObjectSelectByMWFlags (uint32 mask, uint32 value)
  {
    litObjectSelectByMWFlags::mask = mask;
    litObjectSelectByMWFlags::value = value;
  }
  virtual ~litObjectSelectByMWFlags () { }
  virtual bool SelectObject (iObject* obj);
};

/**
 * Select an object based on mesh object flags.
 */
class litObjectSelectByMOFlags : public litObjectSelect
{
private:
  uint32 value, mask;

public:
  litObjectSelectByMOFlags (uint32 mask, uint32 value)
  {
    litObjectSelectByMOFlags::mask = mask;
    litObjectSelectByMOFlags::value = value;
  }
  virtual ~litObjectSelectByMOFlags () { }
  virtual bool SelectObject (iObject* obj);
};

/**
 * Select an object based on mesh type.
 */
class litObjectSelectByType : public litObjectSelect
{
private:
  char* type;

public:
  litObjectSelectByType (const char* type)
  {
    litObjectSelectByType::type = csStrNew (type);
  }
  virtual ~litObjectSelectByType ()
  {
    delete[] type;
  }
  virtual bool SelectObject (iObject* obj);
};

/**
 * Select an object based on the presence of a key value pair.
 */
class litObjectSelectByKeyValue : public litObjectSelect
{
private:
  char* keyname;
  char* keyattrtype;
  char* keyattrvalue;

public:
  litObjectSelectByKeyValue (const char* keyname,
  	const char* keyattrtype, const char* keyattrvalue)
  {
    litObjectSelectByKeyValue::keyname = csStrNew (keyname);
    litObjectSelectByKeyValue::keyattrtype = csStrNew (keyattrtype);
    litObjectSelectByKeyValue::keyattrvalue = csStrNew (keyattrvalue);
  }
  virtual ~litObjectSelectByKeyValue ()
  {
    delete[] keyname;
    delete[] keyattrtype;
    delete[] keyattrvalue;
  }
  virtual bool SelectObject (iObject* obj);
};

/**
 * Select an object based on the presence of a key value pair.
 * Regex version.
 */
class litObjectSelectByKeyValueRE : public litObjectSelect
{
private:
  char* keyname;
  char* keyattrtype;
  csRegExpMatcher matcher;

public:
  litObjectSelectByKeyValueRE (const char* keyname,
  	const char* keyattrtype, const char* keyattrvalue)
	: matcher (keyattrvalue)
  {
    litObjectSelectByKeyValueRE::keyname = csStrNew (keyname);
    litObjectSelectByKeyValueRE::keyattrtype = csStrNew (keyattrtype);
  }
  virtual ~litObjectSelectByKeyValueRE ()
  {
    delete[] keyname;
    delete[] keyattrtype;
  }
  virtual const char* IsValid ()
  {
    csRegExpMatchError rc = matcher.Match ("");
    if (rc == NoError || rc == NoMatch) return 0;
    static char error_string[100];
    sprintf (error_string, "Regexp error %d", (int)rc);
    return error_string;
  }
  virtual bool SelectObject (iObject* obj);
};

/**
 * Mesh selector that can hold children (other object
 * selectors).
 */
class litObjectSelectChildren : public litObjectSelect
{
protected:
  csRefArray<litObjectSelect> a;

public:
  litObjectSelectChildren () { }
  virtual ~litObjectSelectChildren () { }
  void AddMeshSelect (litObjectSelect* ms)
  {
    a.Push (ms);
  }
};

/**
 * Logical and of two other object selectors.
 */
class litObjectSelectAnd : public litObjectSelectChildren
{
public:
  litObjectSelectAnd () { }
  virtual ~litObjectSelectAnd () { }
  virtual bool SelectObject (iObject* obj);
};

/**
 * Logical or of two other object selectors.
 */
class litObjectSelectOr : public litObjectSelectChildren
{
public:
  litObjectSelectOr () { }
  virtual ~litObjectSelectOr () { }
  virtual bool SelectObject (iObject* obj);
};

/**
 * Logical not of other object selector.
 */
class litObjectSelectNot : public litObjectSelect
{
private:
  csRef<litObjectSelect> a;

public:
  litObjectSelectNot (litObjectSelect* a)
  {
    litObjectSelectNot::a = a;
  }
  virtual ~litObjectSelectNot () { }
  virtual bool SelectObject (iObject* obj)
  {
    return !a->SelectObject (obj);
  }
};

/**
 * Select everything.
 */
class litObjectSelectAll : public litObjectSelect
{
public:
  litObjectSelectAll () { }
  virtual ~litObjectSelectAll () { }
  virtual bool SelectObject (iObject*) { return true; }
};

/**
 * Select nothing.
 */
class litObjectSelectNone : public litObjectSelect
{
public:
  litObjectSelectNone () { }
  virtual ~litObjectSelectNone () { }
  virtual bool SelectObject (iObject*) { return false; }
};

#endif // __LITMESHSEL_H__

