/*
    Copyright (C) 2000-2001, 2005 by Christopher Nelson

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

#ifndef _AWS_PROPERTY_H__
#define _AWS_PROPERTY_H__

#include "registrar.h"

#include "csutil/scfstr.h"
#include "csutil/snprintf.h"
#include <map>

namespace aws
{
  /** A property is a value which is maintained by some object.  That property can be read or written by
   * external agents ONLY if the requisite permissions are available. */
  class property
  {
    autom::keeper value;

    bool readable;
    bool writeable;

  public:
    // Empty constructor
    property():readable(true), writeable(true) {}

    /// Initialize a property.
    property(const autom::keeper &_value, bool _readable, bool _writeable):value(_value), readable(_readable), writeable(_writeable) {}

    /// Copy property
    property(const property &p):value(p.value), readable(p.readable), writeable(p.writeable) {}

    ~property() {}

    /// Sets the value of the property. Returns true on sucess, false on failure.
    bool Set(autom::keeper new_value)
    {
      if (writeable) { value=new_value; return true; }

      return false;
    }

    /// Gets the value of the property.  Returns true on sucess, false on failure.
    bool Get(autom::keeper &_value)
    {
      if (readable) { _value=value; return true; }
      return false;
    }
  };

  /** Maintains a "bag" of properties.  These can be set through the automation interface, or through the native accessors. */
  class property_bag : public autom::function::slot
  {
    /// The type of the property map.
    typedef std::map<scfString, property> property_map;

    /// The map of names to properties.
    property_map props;

    /// True if automation is allowed to create new properties in this bag.
    bool allow_automation_create;

  public:
    property_bag():allow_automation_create(true) { };
    ~property_bag() {};
    
    /// Creates a new property mapping with the given property. Returns true on sucess, false on failure.
    bool CreateProperty(const scfString &name,  const property& p);

    /// Gets the value of the named property. Returns true on sucess, false on failure.
    bool Set(const scfString &name, const autom::keeper &value);

    /// Gets the value of the named property.  Returns true on sucess, false on failure.
    bool Get(const scfString &name, autom::keeper &value);

    /// Determines whether or not automation calls can create new properties.
    void SetAllowAutomationCreate(bool value) { allow_automation_create=value; }

  public:
    //////////////////// Automation ///////////////////////////

    /// Performs SetProperty via automation
    autom::func_parm _set(autom::function &fn);

    /// Performs GetProperty via automation
    autom::func_parm _get(autom::function &fn);

    /// Performs CreateProperty via automation, NOTE: this will fail if allow_automation_create is set to false!
    autom::func_parm _create(autom::function &fn);

    /// Initializes the automation functions above with the registry.  'oname' must be the name of the object we're registering as.
    void SetupAutomation(const scfString &oname);
  };
}

#endif
