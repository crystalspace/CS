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

#include "cssysdef.h"
#include "csplugincommon/script/scriptcommon.h"
#include <stdarg.h>
#include <string.h>

void csScriptCommonParseFormat (const char *fmt, va_list va, 
				csRefArray<iScriptValue> &args, iScript *s)
{
  while (*(fmt++) == '%') switch (*(fmt++))
  {
    case 'c': case 'd': case 'i':
    args.Push (csRef<iScriptValue> (s->RValue (va_arg (va, int))));
    break;

    case 'o': case 'u': case 'x': case 'X':
    args.Push (csRef<iScriptValue> (s->RValue ((int) va_arg (va, unsigned))));
    break;

    case 'f': case 'e': case 'g': case 'E': case 'G':
    args.Push (csRef<iScriptValue> (s->RValue (va_arg (va, double))));
    break;

    case 's':
    args.Push (csRef<iScriptValue> (s->RValue (va_arg (va, char *))));
    break;

    case 'p':
    break;

    case 'h': switch (*(fmt++))
    {
      case 'd': case 'i':
      args.Push (csRef<iScriptValue> (s->RValue (va_arg (va, int))));
      break;

      case 'o': case 'u': case 'x': case 'X':
      args.Push (csRef<iScriptValue> (s->RValue ((int) va_arg (va, unsigned))));
      break;

      case 'f': case 'e': case 'g': case 'E': case 'G':
      args.Push (csRef<iScriptValue> (s->RValue (va_arg (va, double))));
      break;

      default:
      break;
    }
    break;

    case 'l': switch (*(fmt++))
    {
      case 'd': case 'i':
      args.Push (csRef<iScriptValue> (s->RValue ((int) va_arg (va, long))));
      break;

      case 'o': case 'u': case 'x': case 'X':
      args.Push (csRef<iScriptValue> (s->RValue ((int) va_arg (va, unsigned long))));
      break;

      case 'f': case 'e': case 'g': case 'E': case 'G':
      args.Push (csRef<iScriptValue> (s->RValue (va_arg (va, double))));
      break;

      default:
      break;
    }
    break;

    default:
    break;
  }
}

//---- csScriptObjectCommon --------------------------------------------

void csScriptObjectCommon::CallCommon (const char *name,
  csRef<iScriptValue> &ret, va_list va, const char *fmt)
{
  //if (strlen(fmt) % 2) return;

  csRefArray<iScriptValue> args;
  csScriptCommonParseFormat(fmt, va, args, GetScript ());

  ret.AttachNew (Call (name, args));
}

bool csScriptObjectCommon::Call (const char *name, const char *fmt, ...)
{
  csRef<iScriptValue> retval;
  va_list va;
  va_start (va, fmt);
  CallCommon (name, retval, va, fmt);
  va_end (va);
  return retval.IsValid ();
}
#define CS_SCRIPTOBJECTCOMMON_CALL(TYPE)				\
{									\
  csRef<iScriptValue> retval;						\
  va_list va;								\
  va_start (va, fmt);							\
  CallCommon (name, retval, va, fmt);					\
  va_end (va);								\
  if (! (retval && (retval->GetTypes() & iScriptValue::t##TYPE))) return false;\
  ret = retval->Get##TYPE ();						\
  return true;								\
}
bool csScriptObjectCommon::Call (const char *name, int &ret, const char *fmt, ...)
CS_SCRIPTOBJECTCOMMON_CALL(Int)
bool csScriptObjectCommon::Call (const char *name, float &ret, const char *fmt, ...)
CS_SCRIPTOBJECTCOMMON_CALL(Float)
bool csScriptObjectCommon::Call (const char *name, double &ret, const char *fmt, ...)
CS_SCRIPTOBJECTCOMMON_CALL(Double)
bool csScriptObjectCommon::Call (const char *name, csRef<iString> &ret, const char *fmt, ...)
CS_SCRIPTOBJECTCOMMON_CALL(String)
bool csScriptObjectCommon::Call (const char *name, csRef<iScriptObject> &ret, const char *fmt, ...)
CS_SCRIPTOBJECTCOMMON_CALL(Object)

#define CS_SCRIPTOBJECTCOMMON_SET			\
{							\
  csRef<iScriptValue> val (GetScript ()->RValue (data));\
  return Set (name, val);				\
}
bool csScriptObjectCommon::Set (const char *name, int data)
CS_SCRIPTOBJECTCOMMON_SET
bool csScriptObjectCommon::Set (const char *name, float data)
CS_SCRIPTOBJECTCOMMON_SET
bool csScriptObjectCommon::Set (const char *name, double data)
CS_SCRIPTOBJECTCOMMON_SET
bool csScriptObjectCommon::Set (const char *name, char const *data)
CS_SCRIPTOBJECTCOMMON_SET
bool csScriptObjectCommon::Set (const char *name, iScriptObject *data)
CS_SCRIPTOBJECTCOMMON_SET
bool csScriptObjectCommon::SetTruth (const char *name, bool data)
CS_SCRIPTOBJECTCOMMON_SET

#define CS_SCRIPTOBJECTCOMMON_GET(TYPE)					\
{									\
  csRef<iScriptValue> val (((csScriptObjectCommon*) this)->Get (name));	\
  if (! (val && (val->GetTypes() & iScriptValue::t##TYPE))) return false;\
  data = val->Get##TYPE ();						\
  return true;								\
}
bool csScriptObjectCommon::Get (const char *name, int &data) const
CS_SCRIPTOBJECTCOMMON_GET(Int)
bool csScriptObjectCommon::Get (const char *name, float &data) const
CS_SCRIPTOBJECTCOMMON_GET(Float)
bool csScriptObjectCommon::Get (const char *name, double &data) const
CS_SCRIPTOBJECTCOMMON_GET(Double)
bool csScriptObjectCommon::Get (const char *name, csRef<iString> &data) const
CS_SCRIPTOBJECTCOMMON_GET(String)
bool csScriptObjectCommon::Get (const char *name, csRef<iScriptObject> &data) const
CS_SCRIPTOBJECTCOMMON_GET(Object)
bool csScriptObjectCommon::GetTruth (const char *name, bool &data) const
CS_SCRIPTOBJECTCOMMON_GET(Bool)

//---- csScriptCommon --------------------------------------------------

void csScriptCommon::CallCommon (const char *name,
  csRef<iScriptValue> &ret, va_list va, const char *fmt)
{
  //if (strlen(fmt) % 2) return;

  csRefArray<iScriptValue> args;
  csScriptCommonParseFormat(fmt, va, args, this);

  ret.AttachNew (Call (name, args));
}

bool csScriptCommon::Call (const char *name, const char *fmt, ...)
{
  csRef<iScriptValue> retval;
  va_list va;
  va_start (va, fmt);
  CallCommon (name, retval, va, fmt);
  va_end (va);
  return retval.IsValid ();
}
#define CS_SCRIPTCOMMON_CALL(TYPE)					\
{									\
  csRef<iScriptValue> retval;						\
  va_list va;								\
  va_start (va, fmt);							\
  CallCommon (name, retval, va, fmt);					\
  va_end (va);								\
  if (! (retval && (retval->GetTypes() & iScriptValue::t##TYPE))) return false;\
  ret = retval->Get##TYPE ();						\
  return true;								\
}
bool csScriptCommon::Call (const char *name, int &ret, const char *fmt, ...)
CS_SCRIPTCOMMON_CALL(Int)
bool csScriptCommon::Call (const char *name, float &ret, const char *fmt, ...)
CS_SCRIPTCOMMON_CALL(Float)
bool csScriptCommon::Call (const char *name, double &ret, const char *fmt, ...)
CS_SCRIPTCOMMON_CALL(Double)
bool csScriptCommon::Call (const char *name, csRef<iString> &ret, const char *fmt, ...)
CS_SCRIPTCOMMON_CALL(String)
bool csScriptCommon::Call (const char *name, csRef<iScriptObject> &ret, const char *fmt, ...)
CS_SCRIPTCOMMON_CALL(Object)

csRef<iScriptObject> csScriptCommon::NewObject (const char *type, const char *fmt, ...)
{
  //if (strlen(fmt) % 2) return;

  va_list va;
  va_start (va, fmt);
  csRefArray<iScriptValue> args;
  csScriptCommonParseFormat (fmt, va, args, this);
  va_end (va);

  return New (type, args);
}

#define CS_SCRIPTCOMMON_STORE			\
{						\
  csRef<iScriptValue> val (RValue (data));	\
  return Store (name, val);			\
}
bool csScriptCommon::Store (const char *name, int data)
CS_SCRIPTCOMMON_STORE
bool csScriptCommon::Store (const char *name, float data)
CS_SCRIPTCOMMON_STORE
bool csScriptCommon::Store (const char *name, double data)
CS_SCRIPTCOMMON_STORE
bool csScriptCommon::Store (const char *name, char const *data)
CS_SCRIPTCOMMON_STORE
bool csScriptCommon::Store (const char *name, iScriptObject *data)
CS_SCRIPTCOMMON_STORE
bool csScriptCommon::SetTruth (const char *name, bool data)
CS_SCRIPTCOMMON_STORE

#define CS_SCRIPTCOMMON_RETRIEVE(TYPE)					\
{									\
  csRef<iScriptValue> val (((csScriptCommon*) this)->Retrieve (name));	\
  if (! (val && (val->GetTypes() & iScriptValue::t##TYPE))) return false;\
  data = val->Get##TYPE ();						\
  return true;								\
}
bool csScriptCommon::Retrieve (const char *name, int &data) const
CS_SCRIPTCOMMON_RETRIEVE(Int)
bool csScriptCommon::Retrieve (const char *name, float &data) const
CS_SCRIPTCOMMON_RETRIEVE(Float)
bool csScriptCommon::Retrieve (const char *name, double &data) const
CS_SCRIPTCOMMON_RETRIEVE(Double)
bool csScriptCommon::Retrieve (const char *name, csRef<iString> &data) const
CS_SCRIPTCOMMON_RETRIEVE(String)
bool csScriptCommon::Retrieve (const char *name, csRef<iScriptObject> &data) const
CS_SCRIPTCOMMON_RETRIEVE(Object)
bool csScriptCommon::GetTruth (const char *name, bool &data) const
CS_SCRIPTCOMMON_RETRIEVE(Bool)
