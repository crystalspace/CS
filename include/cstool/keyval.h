/*
    Copyright (C) 2000 by Thomas Hieber
    
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

#ifndef __KEYVAL_H__
#define __KEYVAL_H__

#include "csgeom/vector3.h"
#include "csutil/csobject.h"
#include "ivaria/keyval.h"

/**
 * A Key Value pair. This object contains a 'key' string and a 'value' string.
 * The 'key' string is the same as the name of the object as returned from
 * the iObject.
 */
class csKeyValuePair : public csObject
{
public:
  /// The constructor. Requires both key and value. Data is being copied!
  csKeyValuePair (const char* Key, const char* Value);
  /// The destructor as usual
  virtual ~csKeyValuePair ();

  /// Get the key string of the pair.
  const char *GetKey () const;

  /// Set the key string of the pair.
  void SetKey (const char *s);

  /// Get the value string of the pair
  const char *GetValue () const;

  /// Set the value of a key in an object.
  void SetValue (const char* value);

  DECLARE_IBASE_EXT (csObject);
  //----------------------- iKeyValuePair --------------------------
  struct KeyValuePair : public iKeyValuePair
  {
    DECLARE_EMBEDDED_IBASE (csKeyValuePair);
    virtual iObject *QueryObject() { return scfParent; }
    virtual const char *GetKey () const { return scfParent->GetKey (); }
    virtual void SetKey (const char* s) { scfParent->SetKey (s); }
    virtual const char *GetValue () const { return scfParent->GetValue (); }
    virtual void SetValue (const char* value) { scfParent->SetValue (value); }
  } scfiKeyValuePair;

private:
  char *m_Value;
};

#endif // __KEYVAL_H__
