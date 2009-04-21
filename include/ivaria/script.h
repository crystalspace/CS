/*
    Copyright (C) 1999 by Brandon Ehle <azverkan@yahoo.com>
              (C) 2003-2007 by Mat Sutcliffe <oktal@gmx.co.uk>

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
#include "csutil/ref.h"
#include "csutil/array.h"
#include "csutil/refarr.h"
#include "iutil/string.h"

struct iScript;
struct iScriptObject;

/**
 * This provides the interface to a value stored in the scripting language's
 * native format, be it a numeric or string value or a reference to an object.
 * The script interpreter (iScript) returns one of these from method and
 * subroutine calls, and property and variable gets, and accepts one in method
 * and subroutine arguments, and property and variable sets.
 *
 * This is a constant value such as 3 or "hello, world" and as such you
 * generally cannot change the value stored in an iScriptValue object.
 * To change the value of a variable in the script, construct a new
 * iScriptValue with iScript::RValue() and pass it to iScript::Store() or
 * iScriptObject::Set().
 *
 * Main creators of instances implementing this interface:
 * - iScript::RValue()
 * - iScript::Retrieve()
 * - iScriptObject::Get()
 */
struct iScriptValue : public virtual iBase
{
  SCF_INTERFACE (iScriptValue, 2, 0, 0);

  /// Datatype constants, used to make a bitmask.
  enum
  {
    tInt = 0x01,
    tFloat = 0x02,
    tDouble = 0x04,
    tString = 0x08,
    tBool = 0x10,
    tObject = 0x20
  };

  /// Returns a reference to the iScript to which this value belongs.
  virtual iScript* GetScript () = 0;

  /// Returns a bitmask indicating which types the value can be coerced into.
  virtual unsigned GetTypes () const = 0;

  /// Returns this value as an int.
  virtual int GetInt () const = 0;

  /// Returns this value as a float.
  virtual float GetFloat () const = 0;

  /// Returns this value as a double.
  virtual double GetDouble () const = 0;

  /// Returns this value as a string.
  virtual const csRef<iString> GetString () const = 0;

  /// Returns this value as a bool.
  virtual bool GetBool () const = 0;

  /// Returns this value as a reference to a script object.
  virtual csRef<iScriptObject> GetObject () const = 0;
};

/**
 * This provides the interface to an object in an object-oriented scripting
 * language.
 *
 * Main ways to get pointers to this interface:
 * - iScript::New()
 * - iScriptValue::GetObject()
 */
struct iScriptObject : public virtual iBase
{
  SCF_INTERFACE (iScriptObject, 2, 0, 0);

  /**
   * Returns a reference to the iScript to which this object belongs.
   */
  virtual iScript* GetScript () = 0;

  /**
   * Returns the name of this object's class.
   *
   * If the object is from the cspace module, the name will be returned
   * without the cspace namespace qualifier.
   */
  virtual const csRef<iString> GetClass () const = 0;

  /**
   * Returns a boolean specifying whether or not the object "is a" or is
   * derived from the named class.
   *
   * This may or may not be a class from the cspace module. Also, such classes
   * may be specified with or without the cspace namespace qualifier.
   */
  virtual bool IsA (const char *) const = 0;

  /// Deprecated version of IsA().
  CS_DEPRECATED_METHOD_MSG("use IsA() instead")
  virtual bool IsType (const char *) const = 0;

  /**
   * If the object is from the cspace module, this will return its pointer.
   *
   * If it is not from the cspace module, the behaviour will be undefined.
   *
   * Some implementations may wish to extend the behaviour of this method to
   * return pointers to other types of object, but that would be specific to
   * the implementation.
   */
  virtual void* GetPointer () = 0;

  /**
   * If the object is from the cspace module, this will allow you to change
   * the reference to point to another object of the same class.
   *
   * NOTE: This is not guaranteed to work, is deprecated, and will be removed!
   */
  CS_DEPRECATED_METHOD
  virtual bool SetPointer (void*) = 0;

  /**
   * Calls a method in the object.
   * @param name The name of the method to call.
   * @param args An optional array of arguments to pass to the method.
   * @return The return value of the method. If the named method does not
   *   exist or encounters a runtime error then it returns 0. If the method
   *   returns no value then it returns an iScriptValue with a bitmask of 0.
   * @remarks Scripting languages that support pass-by-reference may change
   *   the values of the arguments.
   */
  virtual csPtr<iScriptValue> Call (const char *name,
    const csRefArray<iScriptValue> &args = csRefArray<iScriptValue> ()) = 0;

  /**
   * Sets the value of a property in the script object.
   * @param name The name of the property.
   * @param value The value that the property will be set to.
   * @return False if the named property does not exist or cannot be set.
   * @remarks The minimum requirement for implementors of this method is that
   *   it work with objects from the cspace module. An implementation may
   *   support other classes, but if not then behaviour may be undefined when
   *   attempting to use such classes.
   */
  virtual bool Set (const char *name, iScriptValue *value) = 0;

  /**
   * Gets the value of a property in the script object.
   * @param name The name of the property.
   * @return The value of the property, or 0 if it does not exist.
   * @remarks The minimum requirement for implementors of this method is that
   *   it work with objects from the cspace module. An implementation may
   *   support other classes, but if not then behaviour may be undefined when
   *   attempting to use such classes.
   */
  virtual csPtr<iScriptValue> Get (const char *name) = 0;

  ////////////////////////////////////////
  //@@@ Lots more deprecated methods below

  #define CS_DEPRECATED_METHOD_MSG_CALL CS_DEPRECATED_METHOD_MSG \
    ("use Call(const char*, const csRefArray<iScriptValue>&) instead")
  #define CS_DEPRECATED_METHOD_MSG_SET(NEW_METHOD) CS_DEPRECATED_METHOD_MSG \
    ("use " NEW_METHOD "(const char*, iScriptValue*) instead")
  #define CS_DEPRECATED_METHOD_MSG_GET(NEW_METHOD) CS_DEPRECATED_METHOD_MSG \
    ("use " NEW_METHOD "(const char*) instead")

  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, const char *format, ...)
    CS_GNUC_PRINTF(3, 4) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, int &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, float &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, double &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, csRef<iString>&, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, csRef<iScriptObject>&,
    const char *fmt, ...) CS_GNUC_PRINTF(4, 5) = 0;

  CS_DEPRECATED_METHOD_MSG_SET("Set")
  virtual bool Set (const char *name, int data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Set")
  virtual bool Set (const char *name, float data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Set")
  virtual bool Set (const char *name, double data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Set")
  virtual bool Set (const char *name, char const *data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Set")
  virtual bool Set (const char *name, iScriptObject *data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Set")
  virtual bool SetTruth (const char *name, bool isTrue) = 0;

  CS_DEPRECATED_METHOD_MSG_GET("Get")
  virtual bool Get (const char *name, int &data) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Get")
  virtual bool Get (const char *name, float &data) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Get")
  virtual bool Get (const char *name, double &data) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Get")
  virtual bool Get (const char *name, csRef<iString>&) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Get")
  virtual bool Get (const char *name, csRef<iScriptObject>&) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Get")
  virtual bool GetTruth (const char *name, bool &isTrue) const = 0;
};

/**
 * This provides the interface to a scripting language interpreter.
 * @remarks The details of any errors encountered in scripts are sent to the
 *   iReporter.
 */
struct iScript : public virtual iBase
{
  SCF_INTERFACE (iScript, 2, 0, 0);

  /**
   * Runs some script in the scripting language.
   * @param text The text of the script to execute.
   * @return False if there was an error in the script.
   */
  virtual bool RunText (const char *text) = 0;

  /**
   * Loads a named module in the script interpreter.
   * @param name Language-dependent name of the module.
   * @return False if there is an error in the module or it cannot be loaded.
   */
  virtual bool LoadModule (const char *name) = 0;

  /**
   * Loads a module in the script intepreter, from a file in VFS.
   * @param path Virtual filesystem path to the script or module file.
   * @param filename Filename of the script or module file.
   * @return False if there is an error in the module or it cannot be loaded.
   */
  virtual bool LoadModule (const char *path, const char *filename) = 0;

  /**
   * Loads a module from a file in the native filesystem.
   * @param path Native filesystem path to the script or module file.
   * @param filename Filename of the script or module file.
   * @return False if there is an error in the module or it cannot be loaded.
   */
  virtual bool LoadModuleNative (const char *path, const char *filename) = 0;

  /**
   * Calls a subroutine in the script.
   * @param name The name of the subroutine.
   * @param args An optional array of arguments to pass to the subroutine.
   * @return The return value of the subroutine. If the subroutine does not
   *   exist or encounters a runtime error then it returns 0. If it returns
   *   no value then it returns an iScriptValue with a bitmask of 0.
   * @remarks Scripting languages that support pass-by-reference may change
   *   the values of the arguments.
   */
  virtual csPtr<iScriptValue> Call (const char *name,
    const csRefArray<iScriptValue> &args = csRefArray<iScriptValue> ()) = 0;

  /// Creates a new script value container object and returns it.
  virtual csPtr<iScriptValue> RValue (int value) = 0;

  /// Creates a new script value container object and returns it.
  virtual csPtr<iScriptValue> RValue (float value) = 0;

  /// Creates a new script value container object and returns it.
  virtual csPtr<iScriptValue> RValue (double value) = 0;

  /// Creates a new script value container object and returns it.
  virtual csPtr<iScriptValue> RValue (const char *value) = 0;

  /// Creates a new script value container object and returns it.
  virtual csPtr<iScriptValue> RValue (bool value) = 0;

  /// Creates a new script value container object and returns it.
  virtual csPtr<iScriptValue> RValue (iScriptObject *value) = 0;

  /**
   * Creates an object in the script.
   * @param type The name of the class to instantiate.
   * @param args An optional array of arguments to pass to the constructor.
   * @return 0 if the constructor fails, or if the named class does not exist.
   * @remarks Scripting languages that support pass-by-reference may change
   *   the values of the arguments.
   */
  virtual csPtr<iScriptObject> New (const char *type,
    const csRefArray<iScriptValue> &args = csRefArray<iScriptValue> ()) = 0;

  /**
   * Sets a variable in the script interpreter.
   * @param name The name of the variable to set.
   * @param value The value to set the variable to.
   * @return False if the variable could not be stored (e.g. illegal name,
   *   or variable already exists and is immutable).
   * @remarks The name is given without any special characters (e.g. the
   *   dollar prefix in Perl).
   */
  virtual bool Store (const char *name, iScriptValue *value) = 0;

  /**
   * Gets the value of a variable in the script interpreter.
   * @param name The name of the variable to retrieve.
   * @return The value of the variable, or 0 if it does not exist.
   * @remarks The name is given without any special characters (e.g. the
   *   dollar prefix in Perl).
   */
  virtual csPtr<iScriptValue> Retrieve (const char *name) = 0;

  /**
   * Removes a variable from the script interpreter.
   * @param name The name of the variable to remove.
   * @return False if the variable does not exist or cannot be removed.
   * @remarks The name is given without any special characters (e.g. the
   *   dollar prefix in Perl).
   */
  virtual bool Remove (const char *name) = 0;

  /////////////////////////////////////////
  //@@@ Lots more deprecated methods below.

  CS_DEPRECATED_METHOD_MSG("use New() instead")
  virtual csRef<iScriptObject> NewObject (const char *type,
    const char *ctorFormat, ...) CS_GNUC_PRINTF(3, 4) = 0;
  
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, const char *format, ...)
    CS_GNUC_PRINTF(3, 4) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, int &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, float &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, double &ret, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, csRef<iString>&, const char *fmt, ...)
    CS_GNUC_PRINTF(4, 5) = 0;
  CS_DEPRECATED_METHOD_MSG_CALL
  virtual bool Call (const char *name, csRef<iScriptObject> &ret,
    const char *fmt, ...) CS_GNUC_PRINTF(4, 5) = 0;
  
  CS_DEPRECATED_METHOD_MSG_SET("Store")
  virtual bool Store (const char *name, int data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Store")
  virtual bool Store (const char *name, float data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Store")
  virtual bool Store (const char *name, double data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Store")
  virtual bool Store (const char *name, char const *data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Store")
  virtual bool Store (const char *name, iScriptObject *data) = 0;
  CS_DEPRECATED_METHOD_MSG_SET("Store")
  virtual bool SetTruth (const char *name, bool isTrue) = 0;

  CS_DEPRECATED_METHOD_MSG_GET("Retrieve")
  virtual bool Retrieve (const char *name, int &data) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Retrieve")
  virtual bool Retrieve (const char *name, float &data) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Retrieve")
  virtual bool Retrieve (const char *name, double &data) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Retrieve")
  virtual bool Retrieve (const char *name, csRef<iString>&) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Retrieve")
  virtual bool Retrieve (const char *name, csRef<iScriptObject>&) const = 0;
  CS_DEPRECATED_METHOD_MSG_GET("Retrieve")
  virtual bool GetTruth (const char *name, bool &isTrue) const = 0;

  #undef CS_DEPRECATED_METHOD_MSG_CALL
  #undef CS_DEPRECATED_METHOD_MSG_SET
  #undef CS_DEPRECATED_METHOD_MSG_GET
};

#endif // __CS_IVARIA_SCRIPT_H__
