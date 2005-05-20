/*
  Copyright (C) 2004 by Jorrit Tyberghein
	    (C) 2004 by Jonathan Tarbox
	    (C) 2004 by Frank Richter

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
#include "cstypes.h"
#include "iutil/event.h"
#include "csutil/csevent.h"
#include "csutil/memfile.h"

#include "csutil/cseventflattener.h"

#define CS_CRYSTAL_PROTOCOL	  0x43533031

enum
{
  CS_DATATYPE_INT8 = 0x00,
  CS_DATATYPE_UINT8,
  CS_DATATYPE_INT16,
  CS_DATATYPE_UINT16,
  CS_DATATYPE_INT32,
  CS_DATATYPE_UINT32,
  CS_DATATYPE_INT64,
  CS_DATATYPE_UINT64,
  CS_DATATYPE_DOUBLE,
  CS_DATATYPE_DATABUFFER,
  CS_DATATYPE_EVENT
};


csEventFlattenerError csEventFlattener::FlattenSize (iEvent* event, size_t& size)
{
  // Start count with the initial header
  // Version(4) + packet length(8) + Type(1) + Cat(1) + SubCat(1) 
  //    + Flags(1) + Time(4)
  size = 20;

  csRef<iEventAttributeIterator> iter (event->GetAttributeIterator ());

  while (iter->HasNext())
  {
    const char* name;
    name = iter->Next ();

    csEventAttributeType attrType = event->GetAttributeType (name);

    if (attrType == csEventAttrEvent)
    {
      csRef<iEvent> ev;
      if (event->Retrieve (name, ev) != csEventErrNone)
	return csEventFlattenerErrorAttributeRetrieval;
      // 2 for name length
      // X for name string
      // 1 for type id
      // 8 for data length
      // X for data
      size_t innerSize;
      csEventFlattenerError innerResult;
      if ((innerResult = FlattenSize (ev, innerSize)) 
	!= csEventFlattenerErrorNone)
	return innerResult;
      size += 11 + strlen(name) + innerSize;
    }
    else if (attrType == csEventAttrInt)
    {
      // 2 for name length
      // X for name string
      // 1 for type id
      size += 3 + (uint32)strlen(name);

      int64 val;
      if (event->Retrieve (name, val) != csEventErrNone)
	return csEventFlattenerErrorAttributeRetrieval;
      if ((val > 0x7fffffff) || (val < ((int32)0x80000000)))
      {
	size += 8;
      }
      else if ((val > 0x7fff) || (val < ((int16)0x8000)))
      {
	size += 4;
      }
      else if ((val > 0x7f) || (val < ((int8)0x80)))
      {
	size += 2;
      }
      else
      {
	size += 1;
      }
    }
    else if (attrType == csEventAttrUInt)
    {
      // 2 for name length
      // X for name string
      // 1 for type id
      size += 3 + strlen(name);

      uint64 val;
      if (event->Retrieve (name, val) != csEventErrNone)
	return csEventFlattenerErrorAttributeRetrieval;
      if (val > 0xffffffff)
      {
	size += 8;
      }
      else if (val > 0xffff)
      {
	size += 4;
      }
      else if (val > 0xff)
      {
	size += 2;
      }
      else
      {
	size += 1;
      }
    }
    else if (attrType == csEventAttrDatabuffer)
    {
      const void* data;
      size_t dataSize;
      if (event->Retrieve (name, data, dataSize) != csEventErrNone)
	return csEventFlattenerErrorAttributeRetrieval;

      // 2 for name length
      // X for name string
      // 1 for type id
      // 8 for length
      // X for data
      size += 11 + strlen(name) + dataSize;
    }
    else if (attrType == csEventAttrFloat)
    {
      // 2 for name length
      // X for name string
      // 1 for type id
      // 8 for data
      size += 11 + strlen(name);
    }
    else
      return csEventFlattenerErroriBaseEncountered;
  } 
  return csEventFlattenerErrorNone;
}

csEventFlattenerError csEventFlattener::Flatten (iEvent* event, 
						 char * buffer)
{
  uint8 ui8;
  int8 i8;
  uint16 ui16;
  int16 i16;
  uint32 ui32;
  int32 i32;
  int64 i64;
  uint64 ui64;
  size_t size;
  csEventFlattenerError flattenResult = FlattenSize (event, size);
  if (flattenResult != csEventFlattenerErrorNone)
    return flattenResult;
  csMemFile b (buffer, size, csMemFile::DISPOSITION_IGNORE);
  
  ui32 = CS_CRYSTAL_PROTOCOL;
  ui32 = csConvertEndian(ui32);
  b.Write((char *)&ui32, sizeof(uint32));         // protocol version
  ui64 = size;
  ui64 = csConvertEndian(ui64);
  b.Write((char *)&ui64, sizeof(uint64));         // packet size
  b.Write((char *)&event->Type, sizeof(uint8));          // iEvent.Type
  b.Write((char *)&event->Category, sizeof(uint8));      // iEvent.Category
  b.Write((char *)&event->SubCategory, sizeof(uint8));   // iEvent.SubCategory
  b.Write((char *)&event->Flags, sizeof(uint8));         // iEvent.Flags
  ui32 = csConvertEndian((uint32)event->Time);
  b.Write((char *)&ui32, sizeof(uint32));       // iEvent.Time

  csRef<iEventAttributeIterator> iter (event->GetAttributeIterator ());

  while (iter->HasNext())
  {
    const char* name;
    name = iter->Next ();

    switch (event->GetAttributeType (name))
    {
      case csEventAttriBase:
	return csEventFlattenerErroriBaseEncountered;
      case csEventAttrEvent:
	{
	  // 2 byte name length (little endian)
	  ui16 = (uint16)strlen(name);
	  ui16 = csConvertEndian(ui16);
	  b.Write((char *)&ui16, sizeof(int16));
	  // XX byte name
	  b.Write(name, ui16);
	  // 1 byte datatype id
	  ui8 = CS_DATATYPE_EVENT;
	  b.Write((char *)&ui8, sizeof(uint8));

	  csRef<iEvent> ev;
	  if (event->Retrieve (name, ev) != csEventErrNone)
	    return csEventFlattenerErrorAttributeRetrieval;

	  size_t innerSize;
	  csEventFlattenerError innerResult;
	  if ((innerResult = FlattenSize (ev, innerSize)) 
	    != csEventFlattenerErrorNone)
	    return innerResult;

	  // 8 byte data length
	  ui64 = csConvertEndian ((uint64)innerSize);
	  b.Write((char *)&ui64, sizeof(uint64));

	  // XX byte data
	  innerResult = Flatten (ev, b.GetPos() + buffer);
	  if (innerResult != csEventFlattenerErrorNone)
	    return innerResult;
	  else
	    b.SetPos(b.GetPos() + innerSize);
	  break;
	}
      case csEventAttrDatabuffer:
	{
	  const void* data;
	  size_t dataSize;
	  if (event->Retrieve (name, data, dataSize) != csEventErrNone)
	    return csEventFlattenerErrorAttributeRetrieval;

	  // 2 byte name length (little endian)
	  ui16 = (uint16)strlen(name);
	  ui16 = csConvertEndian(ui16);
	  b.Write((char *)&ui16, sizeof(int16));
	  // XX byte name
	  b.Write(name, ui16);
	  // 1 byte datatype id
	  ui8 = CS_DATATYPE_DATABUFFER;
	  b.Write((char *)&ui8, sizeof(uint8));
	  // 4 byte data length
	  ui64 = dataSize;
	  ui64 = csConvertEndian (ui64);
	  b.Write((char *)&ui64, sizeof(uint64));
	  // XX byte data
	  b.Write ((char*)data, dataSize);
	  break;
	}
      case csEventAttrInt:
	{
	  int64 val;
	  if (event->Retrieve (name, val) != csEventErrNone)
	    return csEventFlattenerErrorAttributeRetrieval;
	  // 2 byte name length (little endian)
	  ui16 = (uint16)strlen(name);
	  ui16 = csConvertEndian(ui16);
	  b.Write((char *)&ui16, sizeof(int16));
	  // XX byte name
	  b.Write(name, ui16);
	  if ((val > 0x7fffffff) || (val < ((int32)0x80000000)))
	  {
	    // 1 byte datatype id
	    ui8 = CS_DATATYPE_INT64;
	    b.Write((char *)&ui8, sizeof(uint8));
	    // 4 byte data
	    i64 = csConvertEndian(val);
	    b.Write((char *)&i64, sizeof(int64));
	    break;
	  }
	  else if ((val > 0x7fff) || (val < ((int16)0x8000)))
	  {
	    // 1 byte datatype id
	    ui8 = CS_DATATYPE_INT32;
	    b.Write((char *)&ui8, sizeof(uint8));
	    // 4 byte data
	    i32 = csConvertEndian((int32)val);
	    b.Write((char *)&i32, sizeof(int32));
	    break;
	  }
	  else if ((val > 0x7f) || (val < ((int8)0x80)))
	  {
	    // 1 byte datatype id
	    ui8 = CS_DATATYPE_INT16;
	    b.Write((char *)&ui8, sizeof(uint8));
	    // 2 byte data
	    i16 = csConvertEndian((int16)val);
	    b.Write((char *)&i16, sizeof(int16));
	    break;
	  }
	  else
	  {
	    // 1 byte datatype id
	    ui8 = CS_DATATYPE_INT8;
	    b.Write((char *)&ui8, sizeof(uint8));
	    // 1 byte data
	    i8 = (int8)val;
	    b.Write((char *)&i8, sizeof(int8));
	    break;
	  }
	}
      case csEventAttrUInt:
	{
	  uint64 val;
	  if (event->Retrieve (name, val) != csEventErrNone)
	    return csEventFlattenerErrorAttributeRetrieval;
	  // 2 byte name length (little endian)
	  ui16 = (uint16)strlen(name);
	  ui16 = csConvertEndian(ui16);
	  b.Write((char *)&ui16, sizeof(int16));
	  // XX byte name
	  b.Write(name, ui16);
	  if (val > 0xffffffff)
	  {
	    // 1 byte datatype id
	    ui8 = CS_DATATYPE_UINT64;
	    b.Write((char *)&ui8, sizeof(uint8));
	    // 4 byte data
	    ui64 = csConvertEndian(val);
	    b.Write((char *)&ui64, sizeof(uint64));
	    break;
	  }
	  else if (val > 0xffff)
	  {
	    // 1 byte datatype id
	    ui8 = CS_DATATYPE_UINT32;
	    b.Write((char *)&ui8, sizeof(uint8));
	    // 4 byte data
	    ui32 = csConvertEndian((uint32)val);
	    b.Write((char *)&ui32, sizeof(uint32));
	    break;
	  }
	  else if (val > 0xff)
	  {
	    // 1 byte datatype id
	    ui8 = CS_DATATYPE_UINT16;
	    b.Write((char *)&ui8, sizeof(uint8));
	    // 2 byte data
	    ui16 = csConvertEndian((uint16)val);
	    b.Write((char *)&ui16, sizeof(uint16));
	    break;
	  }
	  else
	  {
	    // 1 byte datatype id
	    ui8 = CS_DATATYPE_UINT8;
	    b.Write((char *)&ui8, sizeof(uint8));
	    // 1 byte data
	    ui8 = (uint8)val;
	    b.Write((char *)&ui8, sizeof(uint8));
	    break;
	  }
	}
      case csEventAttrFloat:
	{
	  double val;
	  if (event->Retrieve (name, val) != csEventErrNone)
	    return csEventFlattenerErrorAttributeRetrieval;
	  // 2 byte name length (little endian)
	  ui16 = (uint16)strlen(name);
	  ui16 = csConvertEndian(ui16);
	  b.Write((char *)&ui16, sizeof(int16));
	  // XX byte name
	  b.Write(name, ui16);
	  // 1 byte datatype id
	  ui8 = CS_DATATYPE_DOUBLE;
	  b.Write((char *)&ui8, sizeof(uint8));
	  // 8 byte data (longlong fixed format)
	  i64 = csDoubleToLongLong (val);
	  b.Write((char *)&i64, sizeof(int64));
	  break;
	}
      default: 
        break;
    }
  }
  return csEventFlattenerErrorNone;
}

csEventFlattenerError csEventFlattener::Unflatten (iEvent* event, 
						   const char *buffer, 
						   size_t length)
{
  csMemFile b((char *)buffer, length, csMemFile::DISPOSITION_IGNORE);
  uint8 ui8;
  int8 i8;
  uint16 ui16;
  int16 i16;
  uint32 ui32;
  int32 i32;
  uint64 ui64;
  int64 i64;
  double d;
  char *name;
  size_t size;

  b.Read((char *)&ui32, sizeof(ui32));
  ui32 = csConvertEndian(ui32);
  if (ui32 != CS_CRYSTAL_PROTOCOL)
  {
    //csPrintf("protocol version invalid: %" PRIX32 "\n", ui32);
    return csEventFlattenerErrorWrongFormat;
  }
  b.Read((char *)&ui64, sizeof(uint64));
  size = csConvertEndian (ui64);
  b.Read((char *)&event->Type, sizeof(uint8));          // iEvent.Type
  b.Read((char *)&event->Category, sizeof(uint8));      // iEvent.Category
  b.Read((char *)&event->SubCategory, sizeof(uint8));   // iEvent.SubCategory
  b.Read((char *)&event->Flags, sizeof(uint8));         // iEvent.Flags
  b.Read((char *)&ui32, sizeof(uint32));         // iEvent.Time
  event->Time = csConvertEndian(ui32);

  while (b.GetPos() < size)
  {
    b.Read((char *)&ui16, sizeof(uint16));
    ui16 = csConvertEndian(ui16);
    name = new char[ui16+1];
    b.Read(name, ui16);
    name[ui16] = 0;

    b.Read((char *)&ui8, sizeof(uint8));
    switch(ui8)
    {
      case CS_DATATYPE_INT8:
        b.Read((char *)&i8, sizeof(int8));
        event->Add (name, i8);
        break;
      case CS_DATATYPE_UINT8:
        b.Read((char *)&ui8, sizeof(uint8));
        event->Add (name, ui8);
        break;
      case CS_DATATYPE_INT16:
        b.Read((char *)&i16, sizeof(int16));
        i16 = csConvertEndian(i16);
        event->Add (name, i16);
        break;
      case CS_DATATYPE_UINT16:
        b.Read((char *)&ui16, sizeof(uint16));
        ui16 = csConvertEndian(ui16);
        event->Add (name, ui16);
        break;
      case CS_DATATYPE_INT32:
        b.Read((char *)&i32, sizeof(int32));
        i32 = csConvertEndian(i32);
        event->Add (name, i32);
        break;
      case CS_DATATYPE_UINT32:
        b.Read((char *)&ui32, sizeof(uint32));
        ui32 = csConvertEndian(ui32);
        event->Add (name, ui32);
        break;
      case CS_DATATYPE_INT64:
        b.Read((char *)&i64, sizeof(int64));
        i64 = csConvertEndian(i64);
        event->Add (name, i64);
        break;
      case CS_DATATYPE_UINT64:
        b.Read((char *)&ui64, sizeof(uint64));
        ui64 = csConvertEndian(ui64);
        event->Add (name, ui64);
        break;
      case CS_DATATYPE_DOUBLE:
        b.Read((char *)&i64, sizeof(int64));
        d = csLongLongToDouble(i64);
        event->Add (name, d);
        break;
      case CS_DATATYPE_DATABUFFER:
        {
          b.Read((char *)&ui64, sizeof(uint64));
          ui64 = csConvertEndian(ui64);
          char* data = new char[ui64];
          b.Read(data, ui64);
          event->Add (name, data, ui64);
	  delete[] data;
        }
        break;
      case CS_DATATYPE_EVENT:
        {
          b.Read((char *)&ui64, sizeof (uint64));
          ui64 = csConvertEndian (ui64);
	  csRef<iEvent> e;
	  e.AttachNew (new csEvent ());
	  event->Add (name, e);
	  csEventFlattenerError unflattenResult = Unflatten (e, 
	    buffer+b.GetPos(), ui64);
	  if (unflattenResult != csEventFlattenerErrorNone)
	  {
	    delete[] name;
	    return unflattenResult;
	  }
          b.SetPos (b.GetPos() + (size_t)ui64);
        }
        break;
      default:
        break;
    }  
    delete[] name;
  } 
  return csEventFlattenerErrorNone;
}

