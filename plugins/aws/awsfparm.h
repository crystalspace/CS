/*
    Copyright (C) 2001 by Christopher Nelson

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

#ifndef __CS_AWS_FPARM_H__
#define __CS_AWS_FPARM_H__

#include "csgeom/cspoint.h"
#include "csgeom/csrect.h"
#include "csutil/parray.h"
#include "csutil/scfstr.h"
#include "iaws/awsparm.h"
#include "iutil/stringarray.h"
#include "iutil/strset.h"
struct iAws;

/**
 * Provides support for safely passing named parameters through to different
 * functions in a portable manner.  Note that awsParmList does not utilize
 * copy semantics. In the interests of space and speed, it simply takes a
 * reference to the pointers passed in. This means that you should NOT use
 * an awsParmList if any parm it references has gone out of scope!
 */

class awsParmList : public iAwsParmList
{
public:
  static const int INT;
  static const int FLOAT;
  static const int STRING;
  static const int STRINGVECTOR;
  static const int RECT;
  static const int POINT;
  static const int BOOL;
  static const int VOPAQUE;

  struct parmItem
  {
    int type;
    unsigned long name;
    union parmValue
    {
      int i;
      float f;
      bool b;
      iString *s;
      iStringArray *sv;
      csRect *r;
      csPoint *p;
      intptr_t v;
    } parm;

    parmItem () : type (INT) { parm.i = 0; }

    ~parmItem ()
    {
      if (type == STRING) parm.s->DecRef ();
      else if (type == STRINGVECTOR) parm.sv->DecRef ();
    }
  };

private:
  csRef<iStringSet> strset;
  csPDelArray<parmItem> parms;
  unsigned long NameToID (const char *name) const;
  parmItem *FindParm (const char *name, int type) const;

public:
  awsParmList (iAws*);
  virtual ~awsParmList ();

  SCF_DECLARE_IBASE;

  /// Adds an integer to the parmeter list.
  virtual void AddInt (const char *name, int value);

  /// Adds a float to the parmeter list.
  virtual void AddFloat (const char *name, float value);

  /// Adds a bool to the parmeter list.
  virtual void AddBool (const char *name, bool value);

  /// Adds a string to the parmeter list.
  virtual void AddString (const char *name, const char* value);

  /// Adds a string vector to the parmeter list.
  virtual void AddStringVector (const char *name, iStringArray *value);

  /// Adds a rect to the parmeter list.
  virtual void AddRect (const char *name, csRect *value);

  /// Adds a point to the parmeter list.
  virtual void AddPoint (const char *name, csPoint *value);

  /**
   * Adds an opaque, undefined value to the parm list. This is stored as
   * an intptr_t, but should never be assumed to be anything at all, except
   * some value that fits in sizeof(intptr_t).
   */
  virtual void AddOpaque (const char *name, intptr_t value);

  /**
   * Returns the int named "name" in value. True if it was found,
   * otherwise false.
   */
  virtual bool GetInt (const char *name, int *value) const;

  /**
   * Returns the float named "name" in value. True if it was found,
   * otherwise false.
   */
  virtual bool GetFloat (const char *name, float *value) const;

  /**
   * Returns the bool named "name" in value. True if it was found,
   * otherwise false.
   */
  virtual bool GetBool (const char *name, bool *value) const;

  /**
   * Returns the string named "name" in value. True if it was found,
   * otherwise false. The reference count on the returned iString is not
   * incremented on behalf of the caller, so you do not DecRef() it, and it is
   * guaranteed to exist only as long as the awsParmList exists. If you need to
   * claim a reference to it, then invoke IncRef().
   */
  virtual bool GetString (const char *name, iString **value) const;

  /**
   * Returns the string named "name" in value. True if it was found,
   * otherwise false.
   */
  virtual bool GetString (const char *name, csRef<iString> &value) const;

  /**
   * Returns the string vector named "name" in value. True if it was found,
   * otherwise false. The reference count on the returned iStringArray is not
   * incremented on behalf of the caller, so you do not DecRef() it, and it is
   * guaranteed to exist only as long as the awsParmList exists. If you need to
   * claim a reference to it, then invoke IncRef().
   */
  virtual bool GetStringVector (const char *name, iStringArray **value) const;

  /**
   * Returns the string vector named "name" in value. True if it was found,
   * otherwise false.
   */
  virtual bool GetStringVector (const char *name, csRef<iStringArray> &value)
    const;

  /**
   * Returns the rect named "name" in value.  True if it was found,
   * otherwise false.
   */
  virtual bool GetRect (const char *name, csRect **value) const;

  /**
   * Returns the point named "name" in value. True if it was found,
   * otherwise false.
   */
  virtual bool GetPoint (const char *name, csPoint **value) const;

  /**
   * Returns the opaque value named "name" in value.  True if it was found,
   * otherwise false.
   */
  virtual bool GetOpaque (const char *name, intptr_t *value) const;

  /// Clears the parameter list.
  virtual void Clear ();
};

#endif // __CS_AWS_FPARM_H__
