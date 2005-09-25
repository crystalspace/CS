/*
    Copyright (C) 1999 by Brandon Ehle <azverkan@yahoo.com>
              (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_IVARIA_SCRIPT_H__
#define __CS_IVARIA_SCRIPT_H__

/**\file
 * General scripting interfaces
 */

#include "csutil/scf.h"

struct iString;

SCF_VERSION (iScriptObject, 0, 0, 1);

/**
 * This provides the interface to an object in an object-oriented scripting
 * language.
 * 
 * Several functions here take a variable-length argument list with a
 * printf-style format string supporting all the argument types supported by
 * printf, except width and precision specifiers, as they have no meaning here.
 * The type specifier "%p" signifies an iScriptObject. Remember to explicitly
 * cast your csRef's to plain pointers in the var arg list.
 */
struct iScriptObject : public iBase
{
  /**
   * Returns a boolean specifying whether or not the object is derived from
   * the given type.
   */
  virtual bool IsType (const char *) const = 0;

  /**
   * If the object is an interface pointer from the cspace module, this will
   * return its value, otherwise 0.
   */
  virtual iBase* GetPointer () const = 0;

  /**
   * If the object is an interface pointer from the cspace module, this will
   * set its value and return true, otherwise false.
   */
  virtual bool SetPointer (iBase *) = 0;

  /**
   * Call a method in the object, with no return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, const char *format, ...)
    CS_GNUC_PRINTF(3, 4) = 0;

  /**
   * Call a method in the object, with int return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, int &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Call a method in the object, with float return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, float &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Call a method in the object, with double return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, double &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Call a method in the object, with string return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, csRef<iString>&, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Call a method in the object, with object return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, csRef<iScriptObject>&,
    const char *fmt, ...) CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Set the value of an int variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Set (const char *name, int data) = 0;

  /**
   * Set the value of a float variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Set (const char *name, float data) = 0;

  /**
   * Set the value of a double variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Set (const char *name, double data) = 0;

  /**
   * Set the value of a string variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Set (const char *name, char const *data) = 0;

  /**
   * Set the value of an object variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Set (const char *name, iScriptObject *data) = 0;

  /**
   * Set the value of a bool variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool SetTruth (const char *name, bool isTrue) = 0;

  /**
   * Get the value of an int variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Get (const char *name, int &data) const = 0;

  /**
   * Get the value of a float variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Get (const char *name, float &data) const = 0;

  /**
   * Get the value of a double variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Get (const char *name, double &data) const = 0;

  /**
   * Get the value of a string variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool Get (const char *name, csRef<iString>&) const = 0;

  /**
   * Get the value of an object variable in the script interpreter.
   * Returns false if the named variable does not exist.
   */
  virtual bool Get (const char *name, csRef<iScriptObject>&) const = 0;

  /**
   * Get the value of a bool variable in the script interpreter.
   * Returns false if the named property does not exist.
   */
  virtual bool GetTruth (const char *name, bool &isTrue) const = 0;
};

struct iObjectRegistry;

SCF_VERSION (iScript, 0, 0, 2);

/**
 * This provides the interface to a scripting language interpreter.
 * 
 * Several functions here take a variable-length argument list with a
 * printf-style format string supporting all the argument types supported by
 * printf, except width and precision specifiers, as they have no meaning here.
 * The specifier "%p" signifies an iScriptObject. Remember to explicitly cast
 * your csRef's to plain pointers in the var arg list.
 */
struct iScript : public iBase
{
  /// This function is deprecated and should no longer be used.
  CS_DEPRECATED_METHOD virtual bool Initialize (iObjectRegistry *object_reg) = 0;

  /// This function is deprecated and should no longer be used.
  CS_DEPRECATED_METHOD virtual bool Store (const char* name, void* data, 
    void* tag) = 0;

  /// Run some script in the scripting language.
  virtual bool RunText (const char *text) = 0;

  /// Load a module in the script interpreter.
  virtual bool LoadModule (const char *name) = 0;

  /**
   * Call a subroutine in the script, with no return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, const char *format, ...)
    CS_GNUC_PRINTF(3, 4) = 0;

  /**
   * Call a subroutine in the script, with int return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, int &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Call a subroutine in the script, with float return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, float &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Call a subroutine in the script, with double return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, double &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Call a subroutine in the script, with string return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, csRef<iString>&, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Call a subroutine in the script, with object return value.
   * Returns false if the subroutine named does not exist.
   * Format is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual bool Call (const char *name, csRef<iScriptObject> &ret,
    const char *fmt, ...) CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Create an object in the script.
   * Returns 0 if the constructor fails.
   * CtorFormat is a printf-style format string for the arguments.
   * \sa \ref FormatterNotes
   */
  virtual csRef<iScriptObject> NewObject (const char *type,
    const char *ctorFormat, ...) CS_GNUC_PRINTF(3, 4) = 0;

  /// Set the value of an int variable in the script interpreter.
  virtual bool Store (const char *name, int data) = 0;

  /// Set the value of a float variable in the script interpreter.
  virtual bool Store (const char *name, float data) = 0;

  /// Set the value of a double variable in the script interpreter.
  virtual bool Store (const char *name, double data) = 0;

  /// Set the value of a string variable in the script interpreter.
  virtual bool Store (const char *name, char const *data) = 0;

  /// Set the value of an object variable in the script interpreter.
  virtual bool Store (const char *name, iScriptObject *data) = 0;

  /// Set the value of a bool variable in the script interpreter.
  virtual bool SetTruth (const char *name, bool isTrue) = 0;

  /**
   * Get the value of an int variable in the script interpreter.
   * Returns false if the named variable does not exist.
   */
  virtual bool Retrieve (const char *name, int &data) const = 0;

  /**
   * Get the value of a float variable in the script interpreter.
   * Returns false if the named variable does not exist.
   */
  virtual bool Retrieve (const char *name, float &data) const = 0;

  /**
   * Get the value of a double variable in the script interpreter.
   * Returns false if the named variable does not exist.
   */
  virtual bool Retrieve (const char *name, double &data) const = 0;

  /**
   * Get the value of a string variable in the script interpreter.
   * Returns false if the named variable does not exist.
   */
  virtual bool Retrieve (const char *name, csRef<iString>&) const = 0;

  /**
   * Get the value of an object variable in the script interpreter.
   * Returns false if the named variable does not exist.
   */
  virtual bool Retrieve (const char *name, csRef<iScriptObject>&) const = 0;

  /**
   * Get the value of a bool variable in the script interpreter.
   * Returns false if the named variable does not exist.
   */
  virtual bool GetTruth (const char *name, bool &isTrue) const = 0;

  /**
   * Remove a variable from the script interpreter.
   * Returns false if the named variable does not exist.
   */
  virtual bool Remove (const char *name) = 0;
};

#endif // __CS_IVARIA_SCRIPT_H__
