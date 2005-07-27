/**************************************************************************
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
*****************************************************************************/

#ifndef __CS_AWS_FLEXIBLE_PARAMETER_LIST_INTERFACE_H__
#define __CS_AWS_FLEXIBLE_PARAMETER_LIST_INTERFACE_H__

#include "csgeom/csrect.h"
#include "csgeom/vector2.h"
#include "iutil/string.h"
#include "iutil/stringarray.h"

SCF_VERSION (iAwsParmList, 0, 1, 1);

/**
 * Provides support for safely passing named parameters through to different
 * functions in a portable manner.  Note that awsParmList does not utilize
 * copy semantics.  In the interests of space and speed, it simply takes a
 * reference to the pointers passed in.  This means that you should NOT use
 * an awsParmList if any parm it references has gone out of scope!
 */
struct iAwsParmList : public iBase
{
  /// Adds an integer to the parmeter list
  virtual void AddInt(const char *name, int value) = 0;
  /// Adds a float to the parmeter list
  virtual void AddFloat(const char *name, float value) = 0;
  /// Adds a bool to the parmeter list
  virtual void AddBool(const char *name, bool value) = 0;
  /// Adds a string to the parmeter list
  virtual void AddString(const char *name, const char* value) = 0;
  /// Adds a string vector to the parmeter list
  virtual void AddStringVector(const char *name, iStringArray* value) = 0;
  /// Adds a rect to the parmeter list
  virtual void AddRect(const char *name, csRect *value) = 0;
  /// Adds a point to the parmeter list
  virtual void AddPoint(const char *name, csVector2 *value) = 0;
  /**
   * Adds an opaque, undefined value to the parm list. This is stored as a
   * (intptr_t), but should never be assumed to be anything at all, except some
   * value that fits in sizeof(intptr_t), which is guaranteed to be large
   * enough to store a number or a pointer.
   */
  virtual void AddOpaque (const char *name, intptr_t value) = 0;

  /**
   * Returns the int named "name" in value.  True if it was found, otherwise
   * false.
   */
  virtual bool GetInt(const char *name, int *value) const = 0;
  /**
   * Returns the float named "name" in value.  True if it was found,
   * otherwise false.
   */
  virtual bool GetFloat(const char *name, float *value) const = 0;
  /**
   * Returns the bool named "name" in value.  True if it was found,
   * otherwise false.
   */
  virtual bool GetBool(const char *name, bool *value) const = 0;
  /**
   * Returns the string named "name" in value. True if it was found,
   * otherwise false. The reference count on the returned iString is not
   * incremented on behalf of the caller, so you do not DecRef() it, and it is
   * guaranteed to exist only as long as the awsParmList exists. If you need to
   * claim a reference to it, then invoke IncRef().
   */
  virtual bool GetString(const char *name, iString **value) const = 0;
  /**
   * Returns the string named "name" in value. True if it was found,
   * otherwise false.
   */
  virtual bool GetString(const char *name, csRef<iString> &value) const = 0;
  /**
   * Returns the string vector named "name" in value.  True if it was found,
   * otherwise false. The reference count on the returned iStringArray is not
   * incremented on behalf of the caller, so you do not DecRef() it, and it is
   * guaranteed to exist only as long as the awsParmList exists. If you need to
   * claim a reference to it, then invoke IncRef().
   */
  virtual bool GetStringVector(const char *name, iStringArray **value)
    const = 0;
  /**
   * Returns the string vector named "name" in value.  True if it was found,
   * otherwise false.
   */
  virtual bool GetStringVector(const char *name, csRef<iStringArray> &value)
    const = 0;
  /**
   * Returns the rect named "name" in value.  True if it was found, otherwise
   * false.
   */
  virtual bool GetRect(const char *name, csRect **value) const = 0;
  /**
   * Returns the point named "name" in value.  True if it was found, otherwise
   * false.
   */
  virtual bool GetPoint(const char *name, csVector2 **value) const = 0;
  /**
   * Returns the opaque value named "name" in value.  True if it was found,
   * otherwise false.
   */
  virtual bool GetOpaque (const char *name, intptr_t *value) const = 0;

  /// Clears the parameter list
  virtual void Clear() = 0;
};

#endif // __CS_AWS_FLEXIBLE_PARAMETER_LIST_INTERFACE_H__
