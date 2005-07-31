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
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf.h"

/// Types that can be contained within a variant.
enum csVariantType
{
  /// long
  CSVAR_LONG,
  /// bool
  CSVAR_BOOL,
  /// a command
  CSVAR_CMD,
  /// float
  CSVAR_FLOAT,
  /// string
  CSVAR_STRING
};

/**
 * Variant, means that type of contained data is set at runtime.
 * Be aware that requesting another type than the containing one
 * will trigger an assertion!
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
    char* s;
  } v;

public:
  csVariant () { type = CSVAR_LONG; v.s = 0; }
  ~csVariant () { if (type == CSVAR_STRING) delete[] v.s; }
  /// Assign a long
  void SetLong (long l)
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_LONG;
    v.l = l;
  }
  /// Assign a bool
  void SetBool (bool b)
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_BOOL;
    v.b = b;
  }
  /// Assign a float
  void SetFloat (float f)
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_FLOAT;
    v.f = f;
  }
  /// Assign a string
  void SetString (const char* s)
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_STRING;
    if (s)
      v.s = csStrNew (s);
    else
      v.s = 0;
  }
  /// Assign a command
  void SetCommand ()
  {
    if (type == CSVAR_STRING) delete[] v.s;
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
    return v.s;
  }
  csVariantType GetType () const { return type; }
};

/// Configuration option description.
struct csOptionDescription
{
  /// Description ID.
  int id;
  /// Short name of this option.
  const char* name;		
  /// Description for this option.
  const char* description;	
  /// Type to use for this option.
  csVariantType type;	
};

//SCF_VERSION (iPluginConfig, 1, 0, 0);
/**
 * Interface to a configurator object. If a SCF module
 * has an object implementing this interface then this can
 * be used to query/set configuration options.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Some plugins implement this.
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() from a plugin instance.
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>csCommandLineHelper
 *   <li>csPluginManager
 *   </ul>
 */
struct iPluginConfig : public virtual iBase
{
  SCF_INTERFACE(iPluginConfig,2,0,0);
  /// Get option description; return FALSE if there is no such option
  virtual bool GetOptionDescription (int idx, csOptionDescription *option) = 0;
  /// Set option
  virtual bool SetOption (int id, csVariant* value) = 0;
  /// Get option
  virtual bool GetOption (int id, csVariant* value) = 0;
};
/** @} */

#endif // __CS_IUTIL_PLUGINCONFIG_H__
