/*
  Crystal Space Windowing System: Event manager
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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
#include "csutil/csevent.h"
#include "csutil/cseventq.h"

SCF_IMPLEMENT_IBASE (csEvent)
  SCF_IMPLEMENTS_INTERFACE (iEvent)
SCF_IMPLEMENT_IBASE_END

typedef struct attribute_tag
{
  union {
    int32 Integer;
	  uint32 Unsigned;
    double Double;
    char *String;
    bool Bool;
	  csEvent *Event;
  };
  enum Type {
    tag_int8,
    tag_uint8,
    tag_int16,
    tag_uint16,
    tag_int32,
    tag_uint32,
    tag_float,
    tag_double,
    tag_string,
    tag_databuffer,
    tag_bool,
    tag_event
  } type;
  uint32 length;
  attribute_tag(Type t) { type = t; }
  ~attribute_tag() 
  { 
    if (type == tag_string) delete String; 
	  if (type == tag_event) Event->DecRef();
  }
} attribute;

char *GetTypeName(attribute::Type t)
{
  switch (t)
  {
    case attribute::tag_int8:
      return "int8";
    case attribute::tag_uint8:
      return "uint8";
    case attribute::tag_int16:
      return "int16";
    case attribute::tag_uint16:
      return "uint16";
    case attribute::tag_int32:
      return "int32";
    case attribute::tag_uint32:
      return "uint32";
    case attribute::tag_float:
      return "float";
    case attribute::tag_double:
      return "double";
    case attribute::tag_bool:
      return "bool";
    case attribute::tag_string:
      return "string";
    case attribute::tag_databuffer:
      return "databuffer";
    case attribute::tag_event:
      return "event";
  }
  return "unknown";
}

csEvent::csEvent ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csEvent::csEvent (csTicks iTime,int eType,int kCode,int kChar,int kModifiers)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Time = iTime;
  Type = eType;
  Category = SubCategory = Flags = 0;
  Key.Code = kCode;
  Key.Char = kChar;
  Key.Modifiers = kModifiers;
}

csEvent::csEvent (csTicks iTime, int eType, int mx, int my,
  int mButton, int mModifiers)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Time = iTime;
  Type = eType;
  Category = SubCategory = Flags = 0;
  Mouse.x = mx;
  Mouse.y = my;
  Mouse.Button = mButton;
  Mouse.Modifiers = mModifiers;
}

csEvent::csEvent (csTicks iTime, int eType, int jn, int jx, int jy,
  int jButton, int jModifiers)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Time = iTime;
  Type = eType;
  Category = SubCategory = Flags = 0;
  Joystick.number = jn;
  Joystick.x = jx;
  Joystick.y = jy;
  Joystick.Button = jButton;
  Joystick.Modifiers = jModifiers;
}

csEvent::csEvent (csTicks iTime, int eType, int cCode, void *cInfo)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Time = iTime;
  Type = eType;
  Category = SubCategory = Flags = 0;
  Command.Code = cCode;
  Command.Info = cInfo;
  if (eType == csevBroadcast)
    Flags = CSEF_BROADCAST;
}

csEvent::csEvent (csEvent const& e) : iEvent()
{
  SCF_CONSTRUCT_IBASE (NULL);
  Type = e.Type;
  Category = e.Category;
  SubCategory = e.SubCategory;
  Flags = e.Flags;
  Time = e.Time;

  if ((Type & CSMASK_Keyboard) != 0)
  {
    Key.Code = e.Key.Code;
    Key.Char = e.Key.Char;
    Key.Modifiers = e.Key.Modifiers;
  }
  else if ((Type & CSMASK_Mouse) != 0)
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
}

bool csEvent::Add(const char *name, int8 v)
{
  attribute *object = new attribute(attribute::tag_int8);
  object->Integer = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::Add(const char *name, uint8 v)
{
  attribute *object = new attribute(attribute::tag_uint8);
  object->Unsigned = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::Add(const char *name, int16 v)
{
  attribute *object = new attribute(attribute::tag_int16);
  object->Integer = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::Add(const char *name, uint16 v)
{
  attribute *object = new attribute(attribute::tag_uint16);
  object->Unsigned = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::Add(const char *name, int32 v, bool force_boolean)
{
  attribute *object = new attribute(attribute::tag_int32);
  if (force_boolean)
  {
    object->type = attribute::tag_bool;
    object->Bool = v;
  }
  else
    object->Integer = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::Add(const char *name, uint32 v)
{
  attribute *object = new attribute(attribute::tag_uint32);
  object->Unsigned = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::Add(const char *name, float v)
{
  attribute *object = new attribute(attribute::tag_float);
  object->Double = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::Add(const char *name, double v)
{
  attribute *object = new attribute(attribute::tag_double);
  object->Double = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

#ifndef CS_USE_FAKE_BOOL_TYPE

bool csEvent::Add(const char *name, bool v, bool force_boolean)
{
  attribute *object = new attribute(attribute::tag_bool);
  if (!force_boolean)
  {
    object->type = attribute::tag_int32;
    object->Integer = v;
  }
  else
    object->Bool = v;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

#endif

bool csEvent::Add(const char *name, char *v)
{
  attribute *object = new attribute(attribute::tag_string);
  object->length = strlen(v);
  object->String = new char[object->length+1];
  strcpy(object->String, v);
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::Add(const char *name, void *v, uint32 size)
{
  attribute *object = new attribute(attribute::tag_databuffer);
  object->String = (char *)v;
  object->length = size;
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (!v1) 
  {
    v1 = new csVector();
    attributes.Put(name, (csHashObject) v1);
  }
  v1->Push((csSome)object);
  return true;
}

bool csEvent::CheckForLoops(csEvent *current, csEvent *e)
{
  csEvent *temp = NULL;
  if (current->Find("_parent", (iEvent **)&temp))
  {
    if (temp == e)
      return false;
    return CheckForLoops(temp, e);
  }
  return true;
}

bool csEvent::Add(const char *name, iEvent *v)
{
  if (this == v)
    return false;
  if (CheckForLoops(this, STATIC_CAST(csEvent*, v)))
  {
    attribute *object = new attribute(attribute::tag_event);
    object->Event = STATIC_CAST(csEvent*, v);
    if (object->Event)
    { 
      if (strcmp(name, "_parent") != 0)
      {
        object->Event->IncRef();
        object->Event->Add("_parent", this);
      }
      csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
      if (!v1) 
      {
        v1 = new csVector();
        attributes.Put(name, (csHashObject) v1);
      }
      v1->Push((csSome)object);
      return true;
    }
  }
  return false;
}

bool csEvent::Find(const char *name, int8 &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_int8)
    {
      v = object->Integer;
      return true;
    }
  }
  return false;
}
  
bool csEvent::Find(const char *name, uint8 &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_uint8)
    {
      v = object->Unsigned;
      return true;
    }
  }
  return false;
}
  
bool csEvent::Find(const char *name, int16 &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_int16)
    {
      v = object->Integer;
      return true;
    }
  }
  return false;
}
  
bool csEvent::Find(const char *name, uint16 &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_uint16)
    {
      v = object->Unsigned;
      return true;
    }
  }
  return false;
}

bool csEvent::Find(const char *name, int32 &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if ((object->type == attribute::tag_int32) || (object->type == attribute::tag_bool))
    {
      v = object->Integer;
      return true;
    }
  }
  return false;
}
  
bool csEvent::Find(const char *name, uint32 &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_uint32)
    {
      v = object->Unsigned;
      return true;
    }
  }
  return false;
}

bool csEvent::Find(const char *name, float &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_float)
    {
      v = (float)object->Double;
      return true;
    }
  }
  return false;
}

bool csEvent::Find(const char *name, double &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_double)
    {
      v = object->Double;
      return true;
    }
  }
  return false;
}

bool csEvent::Find(const char *name, char **v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_string)
    {
      *v = object->String;
      return true;
    }
  }
  return false;
}

bool csEvent::Find(const char *name, void **v, uint32 &size, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_databuffer)
    {
      *v = (void *)object->String;
      size = object->length;
      return true;
    }
  }
  return false;
}

#ifndef CS_USE_FAKE_BOOL_TYPE

bool csEvent::Find(const char *name, bool &v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_bool)
    {
      v = object->Bool;
      return true;
    }
  }
  return false;
}

#endif

bool csEvent::Find(const char *name, iEvent **v, int index)
{
  csVector *v1 = (csVector *) attributes.Get(csHashCompute(name));
  if (v1)
  {
    attribute *object = (attribute *) v1->Get(index);
    if (object->type == attribute::tag_event)
    {
      *v = object->Event;
      return true;
    }
  }
  return false;
}

bool csEvent::Remove(const char *name, int index)
{
  if (index == -1)
  {
    // remove all in the vector, and remove the vector from the hashmap
    csVector *v = (csVector *) attributes.Get(csHashCompute(name));
    if (v)
    {
      attribute *object = NULL;
      while((object = (attribute *) v->Pop()) != NULL)
      {
        if (object)
        {
          if ((object->type == attribute::tag_event) && (strcmp("_parent", name) != 0))
            object->Event->Remove("_parent");
          delete object;
        }
      }
      return true;
    }
    return false;
  }
  else
  {
    // remove only the listed index from the vector, if it's the only one 
    // in the vector, remove the vector from the hashmap
    csVector *v = (csVector *) attributes.Get(csHashCompute(name));
    if (v)
    {
      attribute *object = (attribute *) v->Get(index);
      if (object)
      {
        delete object;
        return true;
      }
    }
    return false;
  }
}

bool csEvent::RemoveAll()
{
  csHashIterator iter(&attributes);
  while (iter.HasNext())
  {
    csVector *v = (csVector *) iter.Next();
    if (v)
    {
      attribute *object = NULL;
      while((object = (attribute *) v->Pop()) != NULL)
      {
        if (object)
        {
          delete object;
        }
      }
    }
  }
  attributes.DeleteAll();
  return true;
}

void IndentLevel(int level)
{
  for (int i = 0; i < level; i++)
    printf("\t");
    
}

bool csEvent::Print(int level)
{
  
  csHashIteratorReversible iter(&attributes);
  while (iter.HasNext())
  {
    csVector *v = (csVector *) iter.Next();
    if (v)
    {
      attribute *object = NULL;
      int index = 0;
      IndentLevel(level); printf ("Event Type: %d\n", Type);
      while(index < v->Length())
      {
        if ((object = (attribute *) v->Get(index)) != NULL)
        {
          IndentLevel(level); printf ("------\n");
          IndentLevel(level); printf ("Name: %s\n", iter.GetKey());
          IndentLevel(level); printf (" Datatype: %s\n",  GetTypeName(object->type));
          if (object->type == attribute::tag_event)
          {
            if (strcmp("_parent", iter.GetKey()) != 0)
            {
              IndentLevel(level); printf(" Sub-Event Contents:\n"); 
              object->Event->Print(level+1);
            }
          }
          if ((object->type == attribute::tag_int8) || (object->type == attribute::tag_uint8)
            || (object->type == attribute::tag_int16) || (object->type == attribute::tag_uint16)
            || (object->type == attribute::tag_int32) || (object->type == attribute::tag_uint32))
          {
            IndentLevel(level); printf (" Value: %ld\n", object->Integer);
          }
          
          if ((object->type == attribute::tag_float) || (object->type == attribute::tag_double))
          {
            IndentLevel(level); printf (" Value: %f\n", object->Double);
          }
          if (object->type == attribute::tag_bool)
          {
            IndentLevel(level); printf (" Value: %s\n", object->Bool ? "true" : "false");
          }
          if (object->type == attribute::tag_databuffer)
          {
            IndentLevel(level); printf (" Value: 0x%X\n", (int)object->String);
            IndentLevel(level); printf (" Length: %ld\n", object->length);
          }
          if (object->type == attribute::tag_string)
          {
            IndentLevel(level); printf (" Value: %s\n", object->String);
          }
          index++;
        }
      }
    }
  }
  return true;
}


char *csEvent::Flatten(uint32 &size)
{
  // TODO
  return NULL;
}

bool csEvent::Unflatten(const char *buffer)
{
  // TODO
  return false;
}

csPoolEvent::csPoolEvent(csEventQueue *q) 
{
    pool = q;
    next = NULL;
}
 
void csPoolEvent::DecRef() 
{
  if (scfRefCount == 1) 
  {
    // while this should never happen, this will prevent a seg fault if some 
    // donut-head decides to create one of these improperly.
    if (!pool) return;
    next = pool->EventPool;
    pool->EventPool = this;
  }
  else
    scfRefCount--;
}
