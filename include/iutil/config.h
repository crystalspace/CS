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

#ifndef __IUTIL_CONFIG_H__
#define __IUTIL_CONFIG_H__

#include "csutil/scf.h"
#include "csutil/util.h"

enum csVariantType
{
  CSVAR_LONG,
  CSVAR_BOOL,
  CSVAR_CMD,
  CSVAR_FLOAT,
  CSVAR_STRING
};

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
  csVariant () { type = CSVAR_LONG; v.s = NULL; }
  ~csVariant () { if (type == CSVAR_STRING) delete[] v.s; }
  void SetLong (long l)
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_LONG;
    v.l = l;
  }
  void SetBool (bool b)
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_BOOL;
    v.b = b;
  }
  void SetFloat (float f)
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_FLOAT;
    v.f = f;
  }
  void SetString (const char* s)
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_STRING;
    if (s)
      v.s = csStrNew (s);
    else
      v.s = NULL;
  }
  void SetCommand ()
  {
    if (type == CSVAR_STRING) delete[] v.s;
    type = CSVAR_CMD;
  }

  long GetLong () const
  {
    CS_ASSERT (type == CSVAR_LONG);
    return v.l;
  }
  bool GetBool () const
  {
    CS_ASSERT (type == CSVAR_BOOL);
    return v.b;
  }
  float GetFloat () const
  {
    CS_ASSERT (type == CSVAR_FLOAT);
    return v.f;
  }
  const char* GetString () const
  {
    CS_ASSERT (type == CSVAR_STRING);
    return v.s;
  }
  csVariantType GetType () const { return type; }
};

struct csOptionDescription
{
  int id;
  char* name;		// Short name of this option.
  char* description;	// Description for this option.
  csVariantType type;	// Type to use for this option.
};

SCF_VERSION (iConfig, 1, 0, 0);

/**
 * Interface to a configurator object. If a SCF module
 * has an object implementing this interface then this can
 * be used to query/set configuration options.
 */
struct iConfig : public iBase
{
  /// Get option description; return FALSE if there is no such option
  virtual bool GetOptionDescription (int idx, csOptionDescription *option) = 0;
  /// Set option
  virtual bool SetOption (int id, csVariant* value) = 0;
  /// Get option
  virtual bool GetOption (int id, csVariant* value) = 0;
};

#endif // __IUTIL_CONFIG_H__
