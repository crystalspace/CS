/*
  Crystal Space Event Queue
  Copyright (C) 1998-2004 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>, Eric Sunshine, Jonathan Tarbox,
    Frank Richter

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
#include "csutil/array.h"
#include "csutil/csevent.h"
#include "csutil/cseventq.h"
#include "csutil/memfile.h"
#include "csutil/util.h"
#include "csutil/sysfunc.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csEvent)
  SCF_IMPLEMENTS_INTERFACE (iEvent)
  SCF_IMPLEMENTS_INTERFACE (csEvent)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_STATIC_VAR(GetEventStrSet, csStringSet, ())

char const* csEvent::GetTypeName (csEventAttributeType t)
{
  switch (t)
  {
    case csEventAttrInt:	    return "int";
    case csEventAttrUInt:	    return "uint";
    case csEventAttrFloat:	    return "double";
    case csEventAttrDatabuffer:	    return "databuffer";
    case csEventAttrEvent:	    return "event";
    case csEventAttriBase:	    return "iBase";
    default:
      break;
  }
  return "unknown";
}

csStringID csEvent::GetKeyID (const char* key)
{
  return GetEventStrSet()->Request (key);
}

const char* csEvent::GetKeyName (csStringID id)
{
  return GetEventStrSet()->Request (id);
}

csEvent::csEvent ()
{
  SCF_CONSTRUCT_IBASE (0);

  count = 0;
}

csEvent::csEvent (csTicks iTime, int eType, int mx, int my,
		  int mButton, int mModifiers) : attributes (53)
{
  SCF_CONSTRUCT_IBASE (0);
  Time = iTime;
  Type = eType;
  Category = SubCategory = Flags = 0;
  Mouse.x = mx;
  Mouse.y = my;
  Mouse.Button = mButton;
  Mouse.Modifiers = mModifiers;

  count = 0;
}

csEvent::csEvent (csTicks iTime, int eType, int jn, int jx, int jy,
		  int jButton, int jModifiers) : attributes (53)
{
  SCF_CONSTRUCT_IBASE (0);
  Time = iTime;
  Type = eType;
  Category = SubCategory = Flags = 0;
  Joystick.number = jn;
  Joystick.x = jx;
  Joystick.y = jy;
  Joystick.Button = jButton;
  Joystick.Modifiers = jModifiers;

  count = 0;
}

csEvent::csEvent (csTicks iTime, int eType, int cCode, intptr_t cInfo) :
  attributes (53)
{
  SCF_CONSTRUCT_IBASE (0);
  Time = iTime;
  Type = eType;
  Category = SubCategory = Flags = 0;
  Command.Code = cCode;
  Command.Info = cInfo;
  if (eType == csevBroadcast)
    Flags = CSEF_BROADCAST;

  count = 0;
}

csEvent::csEvent (csEvent const& e) : iEvent(), attributes (53)
{
  SCF_CONSTRUCT_IBASE (0);
  count = 0;

  Type = e.Type;
  Category = e.Category;
  SubCategory = e.SubCategory;
  Flags = e.Flags;
  Time = e.Time;
  attributes = e.attributes;

  if ((Type & CSMASK_Mouse) != 0)
  {
    Mouse.x = e.Mouse.x;
    Mouse.y = e.Mouse.y;
    Mouse.Button = e.Mouse.Button;
    Mouse.Modifiers = e.Mouse.Modifiers;
  }
  else if ((Type & CSMASK_Joystick) != 0)
  {
    Joystick.number = e.Joystick.number;
    Joystick.x = e.Joystick.x;
    Joystick.y = e.Joystick.y;
    Joystick.Button = e.Joystick.Button;
    Joystick.Modifiers = e.Joystick.Modifiers;
  }
  else
  {
    Command.Code = e.Command.Code;
    Command.Info = e.Command.Info;
  }
}

csEvent::~csEvent ()
{
  RemoveAll();
  SCF_DESTRUCT_IBASE ();
}

bool csEvent::Add (const char *name, float v)
{
  if (attributes.In (GetKeyID (name))) return false;
  attribute* object = new attribute (csEventAttrFloat);
  object->doubleVal = v;
  attributes.Put (GetKeyID (name), object);
  count++;
  return true;
}

bool csEvent::Add (const char *name, double v)
{
  if (attributes.In (GetKeyID (name))) return false;
  attribute* object = new attribute (csEventAttrFloat);
  object->doubleVal = v;
  attributes.Put (GetKeyID (name), object);
  count++;
  return true;
}

bool csEvent::Add (const char *name, bool v)
{
  if (attributes.In (GetKeyID (name))) return false;
  attribute* object = new attribute (csEventAttrInt);
  object->intVal = v ? 1 : 0;
  attributes.Put (GetKeyID (name), object);
  count++;
  return true;
}

bool csEvent::Add (const char *name, const char *v)
{
  if (attributes.In (GetKeyID (name))) return false;
  attribute* object = new attribute (csEventAttrDatabuffer);
  object->dataSize = strlen(v);
  object->bufferVal = csStrNew(v);
  attributes.Put (GetKeyID (name), object);
  count++;
  return true;
}

bool csEvent::Add (const char *name, const void *v, size_t size)
{
  if (attributes.In (GetKeyID (name))) return false;
  attribute* object = new attribute (csEventAttrDatabuffer);
  object->bufferVal = new char[size + 1];
  memcpy (object->bufferVal, v, size);
  object->bufferVal[size] = 0;
  object->dataSize = size;
  attributes.Put (GetKeyID (name), object);
  count++;
  return true;
}

bool csEvent::CheckForLoops (iEvent* current, iEvent* e)
{
  csRef<iEventAttributeIterator> iter (current->GetAttributeIterator());

  while (iter->HasNext())
  {
    const char* attr = iter->Next();

    if (current->GetAttributeType (attr) == csEventAttrEvent)
    {
      csRef<iEvent> ev;
      if (current->Retrieve (attr, ev) != csEventErrNone) continue;
      if (ev == e)
	return false;
      return CheckForLoops(ev, e);
    }
  }
  return true;
}

bool csEvent::Add (const char *name, iEvent *v)
{
  if (attributes.In (GetKeyID (name))) return false;
  if (this == v)
    return false;
  if (v && CheckForLoops(v, this))
  {
    attribute* object = new attribute (csEventAttrEvent);
    (object->ibaseVal = (iBase*)v)->IncRef();
    attributes.Put (GetKeyID (name), object);
    count++;
    return true;
  }
  return false;
}

bool csEvent::Add (const char *name, iBase* v)
{
  if (attributes.In (GetKeyID (name))) return false;
  if (v)
  {
    attribute* object = new attribute (csEventAttriBase);
    (object->ibaseVal = v)->IncRef();
    attributes.Put (GetKeyID (name), object);
    count++;
    return true;
  }
  return false;
}

csEventError csEvent::Retrieve (const char *name, float &v) const
{
  attribute* object = attributes.Get (GetKeyID (name), 0);
  if (!object) return csEventErrNotFound;
  if (object->type == csEventAttrFloat)
  {
    v = object->doubleVal;
    return csEventErrNone;
  }
  else
  {
    return InternalReportMismatch (object);
  }
}

csEventError csEvent::Retrieve (const char *name, double &v) const
{
  attribute* object = attributes.Get (GetKeyID (name), 0);
  if (!object) return csEventErrNotFound;
  if (object->type == csEventAttrFloat)
  {
    v = object->doubleVal;
    return csEventErrNone;
  }
  else
  {
    return InternalReportMismatch (object);
  }
}

csEventError csEvent::Retrieve (const char *name, const char *&v) const
{
  attribute* object = attributes.Get (GetKeyID (name), 0);
  if (!object) return csEventErrNotFound;
  if (object->type == csEventAttrDatabuffer)
  {
    v = object->bufferVal;
    return csEventErrNone;
  }
  else
  {
    return InternalReportMismatch (object);
  }
}

csEventError csEvent::Retrieve (const char *name, void const *&v,
  size_t &size) const
{
  attribute* object = attributes.Get (GetKeyID (name), 0);
  if (!object) return csEventErrNotFound;
  if (object->type == csEventAttrDatabuffer)
  {
    v = object->bufferVal;
    size = object->dataSize;
    return csEventErrNone;
  }
  else
  {
    return InternalReportMismatch (object);
  }
}

csEventError csEvent::Retrieve (const char *name, bool &v) const
{
  attribute* object = attributes.Get (GetKeyID (name), 0);
  if (!object) return csEventErrNotFound;
  if (object->type == csEventAttrInt)
  {
    v = object->intVal != 0;
    return csEventErrNone;
  }
  else
  {
    return InternalReportMismatch (object);
  }
}

csEventError csEvent::Retrieve (const char *name, csRef<iEvent> &v) const
{
  attribute* object = attributes.Get (GetKeyID (name), 0);
  if (!object) return csEventErrNotFound;
  if (object->type == csEventAttrEvent)
  {
    v = (iEvent*)object->ibaseVal;
    return csEventErrNone;
  }
  else
  {
    return InternalReportMismatch (object);
  }
}

csEventError csEvent::Retrieve (const char *name, csRef<iBase> &v) const
{
  attribute* object = attributes.Get (GetKeyID (name), 0);
  if (!object) return csEventErrNotFound;
  if (object->type == csEventAttriBase)
  {
    v = object->ibaseVal;
    return csEventErrNone;
  }
  else
  {
    return InternalReportMismatch (object);
  }
}

bool csEvent::AttributeExists (const char* name)
{
  return attributes.In (GetKeyID (name));
}

csEventAttributeType csEvent::GetAttributeType (const char* name)
{
  attribute* object = attributes.Get (GetKeyID (name), 0);
  if (object)
  {
    return object->type;
  }
  return csEventAttrUnknown;
}

bool csEvent::Remove(const char *name)
{
  csStringID id = GetKeyID (name);
  if (!attributes.In (id)) return false;
  attribute* object = attributes.Get (id, 0);
  bool result = attributes.Delete (id, object);
  delete object;
  return result;
}

bool csEvent::RemoveAll()
{
  csHash<attribute*, csStringID>::GlobalIterator iter (
    attributes.GetIterator ());

  while (iter.HasNext())
  {
    csStringID name;
    attribute* object = iter.Next (name);
    delete object;
  }

  attributes.DeleteAll();
  count = 0;
  return true;
}

csRef<iEventAttributeIterator> csEvent::GetAttributeIterator()
{
  csHash<csEvent::attribute*, csStringID>::GlobalIterator attrIter (
    attributes.GetIterator());
  return csPtr<iEventAttributeIterator> (new csEventAttributeIterator (
    attrIter));
}

static void IndentLevel(int level)
{
  for (int i = 0; i < level; i++)
    csPrintf("\t");
}

bool csEvent::Print (int level)
{
  csHash<attribute*, csStringID>::GlobalIterator iter (
    attributes.GetIterator ());

  while (iter.HasNext())
  {
    csStringID name;
    attribute* object = iter.Next (name);

    IndentLevel(level); csPrintf ("------\n");
    IndentLevel(level); csPrintf ("Name: %s\n", GetKeyName (name));
    IndentLevel(level); csPrintf (" Datatype: %s\n",
	  GetTypeName(object->type));
    if (object->type == csEventAttrEvent)
    {
      IndentLevel(level); csPrintf(" Sub-Event Contents:\n");
      csRef<csEvent> csev = SCF_QUERY_INTERFACE (object->ibaseVal, csEvent);
      if (csev)
	csev->Print(level+1);
      else
      {
	IndentLevel(level+1); csPrintf(" (Not an event!):\n");
      }

    }
    if (object->type == csEventAttrInt)
    {

      IndentLevel(level);
      char const* fmt = " Value: %" PRId64 "\n"; // Pacify Mingw/gcc for PRId64
      csPrintf (fmt, object->intVal);
    }
    else if (object->type == csEventAttrUInt)
    {
      IndentLevel(level);
      char const* fmt = " Value: %" PRIu64 "\n"; // Pacify Mingw/gcc for PRIu64
      csPrintf (fmt, (ulonglong) object->intVal);
    }
    else if (object->type == csEventAttrFloat)
    {
      IndentLevel(level);
      csPrintf (" Value: %f\n", object->doubleVal);
    }
    else if (object->type == csEventAttrDatabuffer)
    {
      IndentLevel(level); csPrintf(" Value: 0x%p\n", object->bufferVal);
      IndentLevel(level); csPrintf(" Length: %zu\n", object->dataSize);
    }
  }

  return true;
}

csRef<iEvent> csEvent::CreateEvent()
{
  return csPtr<iEvent>(new csEvent());
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csEventAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE (iEventAttributeIterator)
SCF_IMPLEMENT_IBASE_END

const char* csEventAttributeIterator::Next()
{
  csStringID key;
  iterator.Next (key);
  return csEvent::GetKeyName (key);
}

//*****************************************************************************
// csPoolEvent
//*****************************************************************************
csPoolEvent::csPoolEvent(csEventQueue *q)
{
    pool = q;
    next = 0;
}

void csPoolEvent::DecRef()
{
  if (scfRefCount == 1)
  {
    if (!pool.IsValid())
      return;

    next = pool->EventPool;
    pool->EventPool = this;
    RemoveAll();
    Type = 0;
    Category = 0;
    Flags = 0;
    Time = 0;
    SubCategory = 0;
    Command.Code = 0;
    Command.Info = 0;
  }
  else
  {
    scfRefCount--;
  }
}

csRef<iEvent> csPoolEvent::CreateEvent()
{
  if (pool.IsValid())
    return pool->CreateEvent(0);
  return superclass::CreateEvent();
}
