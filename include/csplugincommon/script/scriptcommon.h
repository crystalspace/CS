/*
    Copyright (C) 2007 by Mat Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_PLUGINCOMMON_SCRIPT_SCRIPTCOMMON_H__
#define __CS_PLUGINCOMMON_SCRIPT_SCRIPTCOMMON_H__

#include "csutil/scf_implementation.h"
#include "csutil/array.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/weakref.h"
#include "ivaria/script.h"

#include "csutil/deprecated_warn_off.h"

/**
 * This intermediate class can be subclassed by implementors of iScriptObject.
 * It implements the deprecated methods in iScriptObject by forwarding calls
 * to the new methods which have replaced them.
 *
 * When the deprecated methods are removed, this will no longer be neccessary.
 */
class CS_CRYSTALSPACE_EXPORT csScriptObjectCommon :
  public scfImplementation1<csScriptObjectCommon, iScriptObject>
{
  void CallCommon (const char *name, csRef<iScriptValue> &retval,
    va_list var_args, const char *format);

protected:
  csScriptObjectCommon() : scfImplementationType (this) {}
  csScriptObjectCommon(iBase *parent) : scfImplementationType (this, parent) {}

public:
  virtual ~csScriptObjectCommon() {}

  virtual bool IsType (const char *t) const { return IsA (t); }

  virtual bool SetPointer (void*) { return false; }

  virtual bool Call (const char *name, const char *format, ...);
  virtual bool Call (const char *name, int &ret, const char *fmt, ...);
  virtual bool Call (const char *name, float &ret, const char *fmt, ...);
  virtual bool Call (const char *name, double &ret, const char *fmt, ...);
  virtual bool Call (const char *name, csRef<iString>&, const char *fmt, ...);
  virtual bool Call (const char *name, csRef<iScriptObject>&,
    const char *fmt, ...);

  virtual bool Set (const char *name, int data);
  virtual bool Set (const char *name, float data);
  virtual bool Set (const char *name, double data);
  virtual bool Set (const char *name, char const *data);
  virtual bool Set (const char *name, iScriptObject *data);
  virtual bool SetTruth (const char *name, bool isTrue);

  virtual bool Get (const char *name, int &data) const;
  virtual bool Get (const char *name, float &data) const;
  virtual bool Get (const char *name, double &data) const;
  virtual bool Get (const char *name, csRef<iString>&) const;
  virtual bool Get (const char *name, csRef<iScriptObject>&) const;
  virtual bool GetTruth (const char *name, bool &isTrue) const;

  virtual csPtr<iScriptValue> Call (const char*,
    const csRefArray<iScriptValue>& = csRefArray<iScriptValue> ()) = 0;
  virtual bool Set (const char *name, iScriptValue*) = 0;
  virtual csPtr<iScriptValue> Get (const char*) = 0;
};

/**
 * This intermediate class can be subclassed by implementors of iScript.
 * It implements the deprecated methods in iScript by forwarding calls to the
 * new methods which have replaced them.
 *
 * When the deprecated methods are removed, this will no longer be neccessary.
 */
class CS_CRYSTALSPACE_EXPORT csScriptCommon :
  public scfImplementation1<csScriptCommon, iScript>
{
  void CallCommon (const char *name, csRef<iScriptValue> &retval,
    va_list var_args, const char *format);

protected:
  csScriptCommon() : scfImplementationType (this) {}
  csScriptCommon(iBase *parent) : scfImplementationType (this, parent) {}

public:
  virtual ~csScriptCommon() {}

  virtual csRef<iScriptObject> NewObject (const char *type,
    const char *ctorFormat, ...);

  virtual bool Call (const char *name, const char *format, ...);
  virtual bool Call (const char *name, int &ret, const char *fmt, ...);
  virtual bool Call (const char *name, float &ret, const char *fmt, ...);
  virtual bool Call (const char *name, double &ret, const char *fmt, ...);
  virtual bool Call (const char *name, csRef<iString>&, const char *fmt, ...);
  virtual bool Call (const char *name, csRef<iScriptObject> &ret,
    const char *fmt, ...);

  virtual bool Store (const char *name, int data);
  virtual bool Store (const char *name, float data);
  virtual bool Store (const char *name, double data);
  virtual bool Store (const char *name, char const *data);
  virtual bool Store (const char *name, iScriptObject *data);
  virtual bool SetTruth (const char *name, bool isTrue);

  virtual bool Retrieve (const char *name, int &data) const;
  virtual bool Retrieve (const char *name, float &data) const;
  virtual bool Retrieve (const char *name, double &data) const;
  virtual bool Retrieve (const char *name, csRef<iString>&) const;
  virtual bool Retrieve (const char *name, csRef<iScriptObject>&) const;
  virtual bool GetTruth (const char *name, bool &isTrue) const;

  virtual csPtr<iScriptValue> Call (const char*,
    const csRefArray<iScriptValue>& = csRefArray<iScriptValue> ()) = 0;
  virtual bool Store (const char *name, iScriptValue*) = 0;
  virtual csPtr<iScriptValue> Retrieve (const char*) = 0;
};

#include "csutil/deprecated_warn_on.h"

#endif // __CS_PLUGINCOMMON_SCRIPT_SCRIPTCOMMON_H__
