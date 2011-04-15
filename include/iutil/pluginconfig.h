/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_PLUGINCONFIG_H__
#define __CS_IUTIL_PLUGINCONFIG_H__

/**\file
 * Plugin configuration interface and variant types
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf.h"
#include "csutil/scfstr.h"

/// Type of the values that can be contained within a csVariant.
enum csVariantType
{
  /// Long type, obviously also valid for integers
  CSVAR_LONG,
  /// Boolean type
  CSVAR_BOOL,
  /// A command. A command has no value, it is just a flag which can be set or not.
  CSVAR_CMD,
  /// Float type
  CSVAR_FLOAT,
  /// String type
  CSVAR_STRING
};

/**
 * Variant, ie a value whose type is set at runtime.
 * \warning Requesting another type than the contained one
 * will trigger an assertion
 */
struct csVariant
{
private:
  csVariantType type;
  union value
  {
    long l;
    bool b;
    float f;
    iString* s;
  } v;
  void Clear()
  {
    if ((type == CSVAR_STRING) && (v.s != 0)) v.s->DecRef();
  }
public:
  /// Constructor initialized with a value of type CSVAR_CMD
  csVariant () { type = CSVAR_CMD; memset (&v, 0, sizeof (v)); }
  /// Constructor initialized with a value of type CSVAR_LONG
  csVariant (int i) { type = CSVAR_LONG; v.l = i; }
  /// Constructor initialized with a value of type CSVAR_LONG
  csVariant (long l) { type = CSVAR_LONG; v.l = l; }
  /// Constructor initialized with a value of type CSVAR_BOOL
  csVariant (bool b) { type = CSVAR_BOOL; v.b = b; }
  /// Constructor initialized with a value of type CSVAR_FLOAT
  csVariant (float f) { type = CSVAR_FLOAT; v.f = f; }
  /// Constructor initialized with a value of type CSVAR_STRING
  csVariant (const char* s) { type = CSVAR_STRING; v.s = s ? new scfString (s) : nullptr; }

  /// Copy constructor.
  csVariant (const csVariant& var)
  {
    memset (&v, 0, sizeof (v));
    
    type = var.type;
    v = var.v;
    if ((type == CSVAR_STRING) && (v.s != 0)) v.s->IncRef(); 
  }

  ~csVariant () { Clear(); }

  /// Assignment operator.
  const csVariant& operator = (const csVariant& var)
  {
    Clear ();
    type = var.type;
    v = var.v;
    if ((type == CSVAR_STRING) && (v.s != 0)) v.s->IncRef ();
    return var;
  }
  
  /// Assign a long
  void SetLong (long l)
  {
    Clear();
    type = CSVAR_LONG;
    v.l = l;
  }
  /// Assign a bool
  void SetBool (bool b)
  {
    Clear();
    type = CSVAR_BOOL;
    v.b = b;
  }
  /// Assign a float
  void SetFloat (float f)
  {
    Clear();
    type = CSVAR_FLOAT;
    v.f = f;
  }
  /// Assign a string
  void SetString (const char* s)
  {
    Clear();
    type = CSVAR_STRING;
    if (s)
      v.s = new scfString (s);
    else
      v.s = 0;
  }
  /// Assign a command. A command has no value, it is just a flag which can be set or not.
  void SetCommand ()
  {
    Clear();
    type = CSVAR_CMD;
  }

  /// Retrieve a long
  long GetLong () const
  {
    CS_ASSERT (type == CSVAR_LONG);
    return v.l;
  }
  /// Retrieve a bool
  bool GetBool () const
  {
    CS_ASSERT (type == CSVAR_BOOL);
    return v.b;
  }
  /// Retrieve a float
  float GetFloat () const
  {
    CS_ASSERT (type == CSVAR_FLOAT);
    return v.f;
  }
  /// Retrieve a string
  const char* GetString () const
  {
    CS_ASSERT (type == CSVAR_STRING);
    return v.s->GetData();
  }

  /// Get the type of the contained value. The default value is CSVAR_LONG.
  csVariantType GetType () const { return type; }
};

/// Description of a configuration option, to be used by the iPluginConfig interfaces
struct csOptionDescription
{
  /// Description index (or ID)
  int id;
  /// Short name of this option.
  csString name;
  /// Description for this option.
  csString description;
  /// Type to use for this option.
  csVariantType type;

  /// Constructor
  csOptionDescription () {}

  /**
   * Constructor
   * \param id Description index (or ID)
   * \param name Short name of this option.
   * \param description Description for this option.
   * \param type Type to use for this option.
   */
  csOptionDescription (int id, const char* name, const char* description, csVariantType type)
  : id (id), name (name), description (description), type (type) {}

  /**
   * Constructor
   * \param name Short name of this option.
   * \param description Description for this option.
   * \param type Type to use for this option.
   * \warning The \a id is initialized to 0 in this constructor!
   */
  csOptionDescription (const char* name, const char* description, csVariantType type)
  : id (0), name (name), description (description), type (type) {}

  ~csOptionDescription () {}
};

/**
 * Interface to a configurator object. If a SCF module
 * has an object implementing this interface then this can
 * be used to query or set configuration options.
 *
 * Main creators of instances implementing this interface:
 * - Some plugins implement this.
 * 
 * Main ways to get pointers to this interface:
 * - scfQueryInterface() from a plugin instance.
 * 
 * Main users of this interface:
 * - csCommandLineHelper
 * - csPluginManager
 */
struct iPluginConfig : public virtual iBase
{
  SCF_INTERFACE (iPluginConfig,2,1,0);

  /**
   * Get the description of the option of index \a idx. Return \a false if this
   * option does not exist, true otherwise.
   * \param index The index of the option
   * \param option The returned description of the option
   */
  virtual bool GetOptionDescription (int index, csOptionDescription* option) = 0;

  /**
   * Set the value of the option of index \a idx. Return \a false if this
   * option does not exist, true otherwise.
   * \param index The index of the option
   * \param value The new value to be set for the option
   */
  virtual bool SetOption (int index, csVariant* value) = 0;

  /**
   * Get the value of the option of index \a idx. Return \a false if this
   * option does not exist, true otherwise.
   * \param index The index of the option
   * \param value A variant where to store the value of the option
   */
  virtual bool GetOption (int index, csVariant* value) = 0;
};
/** @} */

#endif // __CS_IUTIL_PLUGINCONFIG_H__
