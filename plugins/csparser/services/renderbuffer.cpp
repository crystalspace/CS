/*
    Copyright (C) 2005-2007 by Frank Richter

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
#include <limits.h>

#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"

#include "csgeom/math.h"
#include "csgfx/renderbuffer.h"
#include "csutil/csendian.h"
#include "csutil/databuf.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/stringquote.h"
#include "cstool/rbuflock.h"

#include "syntxldr.h"

CS_PLUGIN_NAMESPACE_BEGIN(SyntaxService)
{

CS_IMPLEMENT_STATIC_VAR (GetBufferParseError, csString, ())

struct vhInt
{
  template <class T>
  static void Get (T& v, iDocumentNode* node, const char* attr)
  { v = node->GetAttributeValueAsInt (attr); }

  template <class T>
  static void Set (const T& v, iDocumentNode* node, const char* attr)
  { node->SetAttributeAsInt (attr, (int)v); }
};


struct vhFloat
{
  template <class T>
  static void Get (T& v, iDocumentNode* node, const char* attr)
  { v = node->GetAttributeValueAsFloat (attr); }

  template <class T>
  static void Set (const T& v, iDocumentNode* node, const char* attr)
  { node->SetAttributeAsFloat (attr, v); }  
};

namespace
{
  /* Wrapper class to provide operations for FillBuffer(), BufferWriter,
     BufferParser */
  struct Half
  {
    uint16 v;
    
    Half () {}
    Half (float f) : v (csIEEEfloat::FromNativeRTZ (f)) {}
    
    operator float() const
    {
      return csIEEEfloat::ToNative (v);
    }
    
    Half& operator= (float f)
    {
      v = csIEEEfloat::FromNativeRTZ (f);
      return *this;
    }
    bool operator< (const Half& other) const
    {
      return csIEEEfloat::ToNative (v) < csIEEEfloat::ToNative (other.v);
    }
  };
}

template <class ValGetter>
struct BufferParser
{
  template <class T>
  static const char* Parse (iDocumentNode* node, int compNum,
    csDirtyAccessArray<T>& buf)
  {
    csString compAttrName;

    csRef<iDocumentNodeIterator> it = node->GetNodes();
    while (it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if (child->GetType() != CS_NODE_ELEMENT) continue;
      if ((strcmp (child->GetValue(), "element") != 0)
	&& (strcmp (child->GetValue(), "e") != 0)
	&& (strcmp (child->GetValue(), "dongledome") != 0))
      {
	GetBufferParseError()->Format ("unexpected node %s", 
	  CS::Quote::Single (child->GetValue()));
	return GetBufferParseError()->GetData();
      }
      for (int c = 0; c < compNum; c++)
      {
	compAttrName.Format ("c%d", c);
	T v;
	ValGetter::Get (v, child, compAttrName);
	buf.Push (v);
      }
    }
    return 0;
  }
};

template<typename T>
static csRef<iRenderBuffer> FillBuffer (const csDirtyAccessArray<T>& buf,
                                        csRenderBufferComponentType compType,
                                        int componentNum,
                                        bool indexBuf)
{
  csRef<iRenderBuffer> buffer;
  size_t bufElems = buf.GetSize() / componentNum;
  if (indexBuf)
  {
    T min;
    T max;
    size_t i = 0;
    size_t n = buf.GetSize(); 
    if (n & 1)
    {
      min = max = csMax (buf[0], T (0));
      i++;
    }
    else
    {
      min = T (INT_MAX);
      max = 0;
    }
    for (; i < n; i += 2)
    {
      T a = buf[i]; T b = buf[i+1];
      if (a < b)
      {
        min = csMin (min, a);
        max = csMax (max, b);
      }
      else
      {
        min = csMin (min, b);
        max = csMax (max, a);
      }
    }
    buffer = csRenderBuffer::CreateIndexRenderBuffer (bufElems, CS_BUF_STATIC,
      compType, size_t (min), size_t (max));
  }
  else
  {
    buffer = csRenderBuffer::CreateRenderBuffer (bufElems,
	  CS_BUF_STATIC, compType, (uint)componentNum);
  }
  buffer->CopyInto (buf.GetArray(), bufElems);
  return buffer;
}


template <class ValSetter>
struct BufferWriter
{
  template<class T>
  static const char* Write (iDocumentNode* node, int compNum,
    const T* buf, size_t bufferSize)
  {
    csString compAttrName;
    for (size_t i = 0; i < bufferSize / compNum; ++i)
    {
      csRef<iDocumentNode> child = node->CreateNodeBefore (CS_NODE_ELEMENT);
      child->SetValue ("e");
      for (int c = 0; c < compNum; ++c)
      {
        compAttrName.Format ("c%d", c);
        ValSetter::Set (buf[i*compNum + c], child, compAttrName);
      }
    }
    return 0;
  }
};

csRef<iRenderBuffer> csTextSyntaxService::ParseRenderBuffer (iDocumentNode* node)
{
  static const char* msgid = "crystalspace.syntax.renderbuffer";

  const char* filename = node->GetAttributeValue ("file");
  if (filename != 0)
  {
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
    csRef<iDataBuffer> data = vfs->ReadFile (filename, false);
    if (!data.IsValid())
    {
      ReportError (msgid, node, "could not read from %s",
		   CS::Quote::Single (filename));
      return 0;
    }
    return ReadRenderBuffer (data, KeepSaveInfo() ? filename : 0);
  }

  const char* componentType = node->GetAttributeValue ("type");
  if (componentType == 0)
  {
    ReportError (msgid, node, "no %s attribute",
		 CS::Quote::Single ("type"));
    return 0;
  }
  int componentNum = node->GetAttributeValueAsInt ("components");
  if (componentNum <= 0)
  {
    ReportError (msgid, node, "bogus %s attribute: %d",
		 CS::Quote::Single ("components"), componentNum);
    return 0;
  }
  
  bool normalized = node->GetAttributeValueAsBool ("normalized", false);

  bool indexBuf = false;
  {
    const char* index = node->GetAttributeValue("indices");
    if (index && *index)
    {
      indexBuf = ((strcmp (index, "yes") == 0)
		  || (strcmp (index, "true") == 0)
		  || (strcmp (index, "on") == 0));
    }
  }

  if (indexBuf && (componentNum != 1))
  {
    ReportError (msgid, node, "index buffers are required to have 1 component");
    return 0;
  }
  
  if (indexBuf && normalized)
  {
    ReportError (msgid, node, "index buffers are required to be unnormalized");
    return 0;
  }
  
  const char* err;
  csRef<iRenderBuffer> buffer;
  if ((strcmp (componentType, "int") == 0) 
    || (strcmp (componentType, "i") == 0))
  {
    csDirtyAccessArray<int> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<int> (buf, 
	normalized ? CS_BUFCOMP_INT_NORM : CS_BUFCOMP_INT, 
	componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "uint") == 0) 
    || (strcmp (componentType, "ui") == 0))
  {
    csDirtyAccessArray<uint> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<uint> (buf,
	normalized ? CS_BUFCOMP_UNSIGNED_INT_NORM : CS_BUFCOMP_UNSIGNED_INT,
        componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "byte") == 0) 
    || (strcmp (componentType, "b") == 0))
  {
    csDirtyAccessArray<char> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<char> (buf,
	normalized ? CS_BUFCOMP_BYTE_NORM : CS_BUFCOMP_BYTE, 
        componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "ubyte") == 0) 
    || (strcmp (componentType, "ub") == 0))
  {
    csDirtyAccessArray<unsigned char> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<unsigned char> (buf,
	normalized ? CS_BUFCOMP_UNSIGNED_BYTE_NORM : CS_BUFCOMP_UNSIGNED_BYTE,
	componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "short") == 0) 
    || (strcmp (componentType, "s") == 0))
  {
    csDirtyAccessArray<short> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<short> (buf,
	normalized ? CS_BUFCOMP_SHORT_NORM : CS_BUFCOMP_SHORT,
	componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "ushort") == 0) 
    || (strcmp (componentType, "us") == 0))
  {
    csDirtyAccessArray<unsigned short> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<unsigned short> (buf,
	normalized ? CS_BUFCOMP_UNSIGNED_SHORT_NORM : CS_BUFCOMP_UNSIGNED_SHORT,
	componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "float") == 0) 
    || (strcmp (componentType, "f") == 0))
  {
    csDirtyAccessArray<float> buf;
    err = BufferParser<vhFloat>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<float> (buf, CS_BUFCOMP_FLOAT, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "double") == 0) 
    || (strcmp (componentType, "d") == 0))
  {
    csDirtyAccessArray<double> buf;
    err = BufferParser<vhFloat>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<double> (buf, CS_BUFCOMP_DOUBLE, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "half") == 0) 
    || (strcmp (componentType, "h") == 0))
  {
    csDirtyAccessArray<Half> buf;
    err = BufferParser<vhFloat>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<Half> (buf, CS_BUFCOMP_HALF, componentNum, indexBuf);
    }
  }
  else
  {
    ReportError (msgid, node, "unknown value for %s: %s",
		 CS::Quote::Single ("type"), componentType);
    return 0;
  }
  if (err != 0)
  {
    ReportError (msgid, node, "%s", err);
    return 0;
  }
  return buffer;
}

bool csTextSyntaxService::ParseRenderBuffer (iDocumentNode* node, iRenderBuffer* buffer)
{
  static const char* msgid = "crystalspace.syntax.renderbuffer";

  const char* filename = node->GetAttributeValue ("file");
  if (filename != 0)
  {
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
    csRef<iDataBuffer> data = vfs->ReadFile (filename, false);
    if (!data.IsValid())
    {
      ReportError (msgid, node, "could not read from %s",
		   CS::Quote::Single (filename));
      return false;
    }
    //return ReadRenderBuffer (data, KeepSaveInfo() ? filename : 0);
    //@@TODO: Implement
    return false;
  }

  const char* componentType = node->GetAttributeValue ("type");
  if (componentType == 0)
  {
    ReportError (msgid, node, "no %s attribute",
		 CS::Quote::Single ("type"));
    return false;
  }  

  int componentNum = node->GetAttributeValueAsInt ("components");
  if (componentNum <= 0)
  {
    ReportError (msgid, node, "bogus %s attribute: %d",
		 CS::Quote::Single ("components"), componentNum);
    return false;
  }
  if (componentNum != buffer->GetComponentCount ())
  {
    ReportError (msgid, node, "%s attribute %d does not match expected %d", 
      CS::Quote::Single ("components"),
      componentNum, buffer->GetComponentCount ());
    return false;
  }

  bool normalized = node->GetAttributeValueAsBool ("normalized", false);

  bool indexBuf = false;
  {
    const char* index = node->GetAttributeValue("indices");
    if (index && *index)
    {
      indexBuf = ((strcmp (index, "yes") == 0)
        || (strcmp (index, "true") == 0)
        || (strcmp (index, "on") == 0));
    }
  }

  if (indexBuf && (componentNum != 1))
  {
    ReportError (msgid, node, "index buffers are required to have 1 component", componentNum);
    return false;
  }
  if (indexBuf != buffer->IsIndexBuffer ())
  {
    ReportError (msgid, node, "wrong buffer type, expected %s buffer", 
      buffer->IsIndexBuffer () ? "index" : "normal");
    return false;
  }  

  const char* err;
  if ((strcmp (componentType, "int") == 0) 
    || (strcmp (componentType, "i") == 0))
  {
    csDirtyAccessArray<int> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);

    if (buffer->GetComponentType () !=
      (normalized ? CS_BUFCOMP_INT_NORM : CS_BUFCOMP_INT))
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "int");
      return false;
    }


    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else if ((strcmp (componentType, "uint") == 0) 
    || (strcmp (componentType, "ui") == 0))
  {
    csDirtyAccessArray<uint> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    
    if (buffer->GetComponentType () !=
      (normalized ? CS_BUFCOMP_UNSIGNED_INT_NORM : CS_BUFCOMP_UNSIGNED_INT))
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "uint");
      return false;
    }

    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else if ((strcmp (componentType, "byte") == 0) 
    || (strcmp (componentType, "b") == 0))
  {
    csDirtyAccessArray<char> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    
    if (buffer->GetComponentType () !=
      (normalized ? CS_BUFCOMP_BYTE_NORM : CS_BUFCOMP_BYTE))
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "byte");
      return false;
    }

    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else if ((strcmp (componentType, "ubyte") == 0) 
    || (strcmp (componentType, "ub") == 0))
  {
    csDirtyAccessArray<unsigned char> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    
    if (buffer->GetComponentType () !=
      (normalized ? CS_BUFCOMP_UNSIGNED_BYTE_NORM : CS_BUFCOMP_UNSIGNED_BYTE))
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "ubyte");
      return false;
    }

    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else if ((strcmp (componentType, "short") == 0) 
    || (strcmp (componentType, "s") == 0))
  {
    csDirtyAccessArray<short> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    
    if (buffer->GetComponentType () !=
      (normalized ? CS_BUFCOMP_SHORT_NORM : CS_BUFCOMP_SHORT))
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "short");
      return false;
    }

    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else if ((strcmp (componentType, "ushort") == 0) 
    || (strcmp (componentType, "us") == 0))
  {
    csDirtyAccessArray<unsigned short> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    
    if (buffer->GetComponentType () != 
      (normalized ? CS_BUFCOMP_UNSIGNED_SHORT_NORM : CS_BUFCOMP_UNSIGNED_SHORT))
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "ushort");
      return false;
    }

    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else if ((strcmp (componentType, "float") == 0) 
    || (strcmp (componentType, "f") == 0))
  {
    csDirtyAccessArray<float> buf;
    err = BufferParser<vhFloat>::Parse (node, componentNum, buf);
    
    if (buffer->GetComponentType () != CS_BUFCOMP_FLOAT)
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "float");
      return false;
    }

    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else if ((strcmp (componentType, "double") == 0) 
    || (strcmp (componentType, "d") == 0))
  {
    csDirtyAccessArray<double> buf;
    err = BufferParser<vhFloat>::Parse (node, componentNum, buf);
    
    if (buffer->GetComponentType () != CS_BUFCOMP_DOUBLE)
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "double");
      return false;
    }

    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else if ((strcmp (componentType, "half") == 0) 
    || (strcmp (componentType, "h") == 0))
  {
    csDirtyAccessArray<Half> buf;
    err = BufferParser<vhFloat>::Parse (node, componentNum, buf);
    
    if (buffer->GetComponentType () != CS_BUFCOMP_HALF)
    {
      ReportError (msgid, node, "component type %s does not match expected %s",
        componentType, "half");
      return false;
    }

    if (err == 0)
    {
      if (buf.GetSize () > buffer->GetElementCount ()*componentNum)
      {
        ReportError (msgid, node, "too many elements: %d", buf.GetSize ());
        return false;
      }

      buffer->CopyInto (buf.GetArray (), buf.GetSize ()/componentNum);
    }
  }
  else
  {
    ReportError (msgid, node, "unknown value for %s: %s",
		 CS::Quote::Single ("type"), componentType);
    return 0;
  }
  if (err != 0)
  {
    ReportError (msgid, node, "%s", err);
    return 0;
  }

  return true;
}


bool csTextSyntaxService::WriteRenderBuffer (iDocumentNode* node, iRenderBuffer* buffer)
{
  static const char* msgid = "crystalspace.syntax.renderbuffer";

  if (buffer == 0)
  {
    ReportError (msgid, node, "no buffer specified");
    return false;
  }

  if (buffer->GetMasterBuffer () != 0)
  {
    ReportError (msgid, node, "cannot save child-buffer");
    return false;
  }

  const char* err = 0;

  // Handle saving as external file
  csRef<iRenderBufferPersistence> bufferPersist = 
    scfQueryInterface<iRenderBufferPersistence> (buffer);
  if (bufferPersist.IsValid())
  {
    const char* filename = bufferPersist->GetFileName();
    if (filename != 0)
    {
      csRef<iDataBuffer> saveData = StoreRenderBuffer (buffer);
      csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
      if (!vfs->WriteFile (filename, saveData->GetData(), saveData->GetSize()))
      {
        ReportError (msgid, node, "could not write to %s",
		     CS::Quote::Single (filename));
        return false;
      }
      node->SetAttribute ("file", filename);
      return true;
    }
  }

  // Common attribute
  int componentCount = buffer->GetComponentCount ();
  node->SetAttributeAsInt ("components", componentCount);
  
  csRenderBufferComponentType compType = buffer->GetComponentType ();
  if (compType & CS_BUFCOMP_NORMALIZED)
    node->SetAttribute ("normalized", "yes");

  switch (compType & ~CS_BUFCOMP_NORMALIZED)
  {
  case CS_BUFCOMP_BYTE:
    {
      node->SetAttribute ("type", "byte");
      csRenderBufferLock<int8> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (int8*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_UNSIGNED_BYTE:
    {
      node->SetAttribute ("type", "ubyte");
      csRenderBufferLock<uint8> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (uint8*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }

  case CS_BUFCOMP_SHORT:
    {
      node->SetAttribute ("type", "short");
      csRenderBufferLock<int16> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (int16*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_UNSIGNED_SHORT:
    {
      node->SetAttribute ("type", "ushort");
      csRenderBufferLock<uint16> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (uint16*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
    
  case CS_BUFCOMP_INT:
    {
      node->SetAttribute ("type", "int");
      csRenderBufferLock<int32> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (int32*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_UNSIGNED_INT:
    {
      node->SetAttribute ("type", "uint");
      csRenderBufferLock<uint32> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (uint32*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }

  case CS_BUFCOMP_FLOAT:
    {
      node->SetAttribute ("type", "float");
      csRenderBufferLock<float> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhFloat>::Write (node, componentCount, (float*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_DOUBLE:
    {
      node->SetAttribute ("type", "double");
      csRenderBufferLock<double> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhFloat>::Write (node, componentCount, (double*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_HALF:
    {
      node->SetAttribute ("type", "half");
      csRenderBufferLock<Half> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhFloat>::Write (node, componentCount, (Half*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
    
  default:
    return false;
  }

  if (buffer->IsIndexBuffer())
    node->SetAttribute ("indices", "yes");

  return true;
}

// Buffer component types, as stored on disk
enum
{
  diskBUFCOMP_BYTE = 0,
  diskBUFCOMP_UNSIGNED_BYTE,
  diskBUFCOMP_SHORT,
  diskBUFCOMP_UNSIGNED_SHORT,
  diskBUFCOMP_INT,
  diskBUFCOMP_UNSIGNED_INT,
  diskBUFCOMP_FLOAT,
  diskBUFCOMP_DOUBLE,
  
  diskBUFCOMP_NORMALIZED = 8,
  
  diskBUFCOMP_BYTE_NORM = CS_BUFCOMP_BYTE | CS_BUFCOMP_NORMALIZED,
  diskBUFCOMP_UNSIGNED_BYTE_NORM =
      CS_BUFCOMP_UNSIGNED_BYTE | CS_BUFCOMP_NORMALIZED,
  diskBUFCOMP_SHORT_NORM =
      CS_BUFCOMP_SHORT | CS_BUFCOMP_NORMALIZED,
  diskBUFCOMP_UNSIGNED_SHORT_NORM =
      CS_BUFCOMP_UNSIGNED_SHORT | CS_BUFCOMP_NORMALIZED,
  diskBUFCOMP_INT_NORM = CS_BUFCOMP_INT | CS_BUFCOMP_NORMALIZED,
  diskBUFCOMP_UNSIGNED_INT_NORM = 
      CS_BUFCOMP_UNSIGNED_INT | CS_BUFCOMP_NORMALIZED,

  /* When this was added value 8 was already pegged as the NORMALIZED flag,
     so the next free bit (16) was used instead */
  diskBUFCOMP_HALF = 16,
  
  diskBUFCOMP_BASE_TYPECOUNT
};

static uint8 ComponentTypeToDisk (unsigned int compType)
{
  uint8 compTypeDisk = 0;
  
  switch (compType & ~(CS_BUFCOMP_NORMALIZED))
  {
  case CS_BUFCOMP_BYTE:			compTypeDisk = diskBUFCOMP_BYTE;		break;
  case CS_BUFCOMP_UNSIGNED_BYTE:	compTypeDisk = diskBUFCOMP_UNSIGNED_BYTE;	break;
  case CS_BUFCOMP_SHORT:		compTypeDisk = diskBUFCOMP_SHORT;		break;
  case CS_BUFCOMP_UNSIGNED_SHORT:	compTypeDisk = diskBUFCOMP_UNSIGNED_SHORT;	break;
  case CS_BUFCOMP_INT:			compTypeDisk = diskBUFCOMP_INT;			break;
  case CS_BUFCOMP_UNSIGNED_INT:		compTypeDisk = diskBUFCOMP_UNSIGNED_INT;	break;
  case CS_BUFCOMP_FLOAT:		compTypeDisk = diskBUFCOMP_FLOAT;		break;
  case CS_BUFCOMP_DOUBLE:		compTypeDisk = diskBUFCOMP_DOUBLE;		break;
  case CS_BUFCOMP_HALF:			compTypeDisk = diskBUFCOMP_HALF;		break;
  }
  
  if (compType & CS_BUFCOMP_NORMALIZED)
    compTypeDisk |= diskBUFCOMP_NORMALIZED;
  
  return compTypeDisk;
}

static unsigned int DiskToComponentType (uint8 compTypeDisk)
{
  unsigned int compType = 0;
  
  switch (compTypeDisk & ~(diskBUFCOMP_NORMALIZED))
  {
  case diskBUFCOMP_BYTE:		compType = CS_BUFCOMP_BYTE;		break;
  case diskBUFCOMP_UNSIGNED_BYTE:	compType = CS_BUFCOMP_UNSIGNED_BYTE;	break;
  case diskBUFCOMP_SHORT:		compType = CS_BUFCOMP_SHORT;		break;
  case diskBUFCOMP_UNSIGNED_SHORT:	compType = CS_BUFCOMP_UNSIGNED_SHORT;	break;
  case diskBUFCOMP_INT:			compType = CS_BUFCOMP_INT;		break;
  case diskBUFCOMP_UNSIGNED_INT:	compType = CS_BUFCOMP_UNSIGNED_INT;	break;
  case diskBUFCOMP_FLOAT:		compType = CS_BUFCOMP_FLOAT;		break;
  case diskBUFCOMP_DOUBLE:		compType = CS_BUFCOMP_DOUBLE;		break;
  case diskBUFCOMP_HALF:		compType = CS_BUFCOMP_HALF;		break;
  }
  
  if (compTypeDisk & diskBUFCOMP_NORMALIZED)
    compType |= CS_BUFCOMP_NORMALIZED;
  
  return compType;
}

struct RenderBufferHeaderCommon
{
  enum 
  { 
    MagicNormal = 0x6272,
    MagicIndex = 0x6269
  };

  uint16 magic;
  uint8 compType;
  uint8 compCount;
  uint32 elementCount;
};

struct RenderBufferHeaderNormal
{
  RenderBufferHeaderCommon common;
};

struct RenderBufferHeaderIndex
{
  RenderBufferHeaderCommon common;
  uint32 rangeStart;
  uint32 rangeEnd;
};

class StoredRenderBuffer :
  public scfImplementation1<StoredRenderBuffer,
                            iRenderBuffer>
{
  /**
   * Keep a reference to the "data source", either the original data buffer or
   * the wrapped render buffer.
   */
  csRef<iBase> dataKeeper;
  /**
   * Buffer data header from the original data buffer, or 0 if in edited mode.
   */
  RenderBufferHeaderCommon* header;
  /** 
   * Holds the buffer data in un-edited mode ot the wrapped iRenderBuffer* in
   * edited mode.
   */
  void* actualData;
  csWeakRef<iRenderBufferCallback> callback;

  void MakeEditable (bool copyData)
  {
    // Create a render buffer that will hold the "edited" data.
    csRef<iRenderBuffer> editedBuffer;
    if (IsIndexBuf())
    {
      editedBuffer = csRenderBuffer::CreateIndexRenderBuffer (
        GetElementCount (), CS_BUF_STATIC, GetComponentType(),
        GetRangeStart(), GetRangeEnd());
    }
    else
    {
      editedBuffer = csRenderBuffer::CreateRenderBuffer (
        GetElementCount (), CS_BUF_STATIC, GetComponentType(),
        GetComponentCount ());
    }

    // Fire the callback so this buffer gets uncached
    editedBuffer->SetCallback (callback);
    if (callback.IsValid()) callback->RenderBufferDestroyed (this);
    callback = 0;
    
    if (copyData)
    {
      editedBuffer->CopyInto (actualData, GetElementCount ());
    }

    // header == 0 means data is "edited".
    header = 0;
    dataKeeper = editedBuffer;
    // A bit nasty: to save QIs when forwarding later...
    actualData = (iRenderBuffer*)editedBuffer;
  }
  inline bool IsEdited() const { return header == 0; }
  inline iRenderBuffer* GetWrappedBuffer() const
  { return reinterpret_cast<iRenderBuffer*> (actualData); }

  inline bool IsIndexBuf() const
  { 
    return csLittleEndian::Convert (header->magic) 
      == RenderBufferHeaderCommon::MagicIndex;
  }
  inline RenderBufferHeaderNormal* GetHeaderNormal() const
  { return reinterpret_cast<RenderBufferHeaderNormal*> (header); }
  inline RenderBufferHeaderIndex* GetHeaderIndex() const
  { return reinterpret_cast<RenderBufferHeaderIndex*> (header); }
public:
  StoredRenderBuffer (iDataBuffer* buf) : 
    scfImplementationType (this), dataKeeper (buf)
  {
    uint8* data = buf->GetUint8();
    header = reinterpret_cast<RenderBufferHeaderCommon*> (data);
    if (IsIndexBuf())
      actualData = data + sizeof (RenderBufferHeaderIndex);
    else
      actualData = data + sizeof (RenderBufferHeaderNormal);
  }

  ~StoredRenderBuffer ()
  {
    if (callback.IsValid()) callback->RenderBufferDestroyed (this);
  }

  /**\name iRenderBuffer implementation
   * @{ */
  void* Lock (csRenderBufferLockType lockType)
  {
    if (!IsEdited() && (lockType <= CS_BUF_LOCK_READ))
      return actualData;
    else
    {
      MakeEditable (true);
      return GetWrappedBuffer()->Lock (lockType);
    }
  }

  void Release()
  {
    if (IsEdited()) GetWrappedBuffer()->Release();
  }

  void CopyInto (const void *data, size_t elementCount,
    size_t elemOffset = 0)
  {
    MakeEditable ((elemOffset == 0) && (elementCount >= GetElementCount()));
    GetWrappedBuffer()->CopyInto (data, elementCount, elemOffset);
  }

  int GetComponentCount () const
  {
    if (IsEdited())
      return GetWrappedBuffer()->GetComponentCount();
    else
      return header->compCount;
  }

  virtual csRenderBufferComponentType GetComponentType () const 
  {
    if (IsEdited())
      return GetWrappedBuffer()->GetComponentType();
    else
      return (csRenderBufferComponentType)DiskToComponentType (header->compType);
  }

  virtual csRenderBufferType GetBufferType() const
  {
    return CS_BUF_STATIC;
  }

  virtual size_t GetSize() const
  {
    if (IsEdited())
      return GetWrappedBuffer()->GetSize();
    else
    {
      unsigned int compType = DiskToComponentType (header->compType);
      return csRenderBufferComponentSizes[compType & ~CS_BUFCOMP_NORMALIZED] 
	* csLittleEndian::Convert (header->elementCount) * header->compCount;
    }
  }

  virtual size_t GetStride() const 
  {
    return 0;
  }

  virtual size_t GetElementDistance() const
  {
    if (IsEdited())
      return GetWrappedBuffer()->GetElementDistance();
    else
    {
      unsigned int compType = DiskToComponentType (header->compType);
      return header->compCount * csRenderBufferComponentSizes[compType & ~CS_BUFCOMP_NORMALIZED];
    }
  }

  virtual size_t GetOffset() const
  { return 0; }

  virtual uint GetVersion ()
  {
    if (IsEdited())
      return GetWrappedBuffer()->GetVersion();
    else
      return (uint)0;
  }

  virtual iRenderBuffer* GetMasterBuffer () const
  {
    return 0;
  }

  virtual bool IsIndexBuffer() const
  { return IsIndexBuf(); }

  virtual size_t GetRangeStart() const
  { 
    if (IsIndexBuf())
      return csLittleEndian::Convert (GetHeaderIndex()->rangeStart);
    else
      return 0; 
  }
  virtual size_t GetRangeEnd() const
  { 
    if (IsIndexBuf())
      return csLittleEndian::Convert (GetHeaderIndex()->rangeEnd);
    else
      return 0; 
  }

  virtual size_t GetElementCount() const
  {
    if (IsEdited())
      return GetWrappedBuffer()->GetElementCount();
    else
      return csLittleEndian::Convert (header->elementCount);
  }

  virtual void SetCallback (iRenderBufferCallback *cb)
  {
    if (IsEdited())
      GetWrappedBuffer()->SetCallback (cb);
    else
      callback = cb;
  }

  void SetData (const void *data)
  {
    MakeEditable (false);
    GetWrappedBuffer()->SetData (data);
  }

  /** @} */
};

class StoredRenderBufferWithPersistence :
  public scfImplementationExt1<StoredRenderBufferWithPersistence,
                               StoredRenderBuffer, 
                               iRenderBufferPersistence>
{
  csString filename;
public:
  StoredRenderBufferWithPersistence (iDataBuffer* buf) : 
    scfImplementationType (this, buf)
  {
  }

  void SetFileName (const char* filename) { this->filename = filename; }
  const char* GetFileName () { return filename; }
};

template<typename T>
static void ConvertBufferDataToLE (const T* source, T* dest, 
                                   size_t totalElementCount)
{
  for (size_t n = 0; n < totalElementCount; n++)
  {
    *dest++ = csLittleEndian::Convert (*source++);
  }
}

template<typename T1, typename T2>
static void ConvertFloatBufferDataToLE (const T1* source, T2* dest, 
                                        size_t totalElementCount)
{
  for (size_t n = 0; n < totalElementCount; n++)
  {
    *dest++ = 
      csLittleEndian::Convert (csIEEEfloat::FromNative (*source++));
  }
}

template<typename T1, typename T2>
static void ConvertFloatBufferDataFromLE (const T1* source, T2* dest, 
                                          size_t totalElementCount)
{
  for (size_t n = 0; n < totalElementCount; n++)
  {
    *dest++ = 
      csIEEEfloat::ToNative (csLittleEndian::Convert (*source++));
  }
}

#if !defined(CS_LITTLE_ENDIAN) || !defined(CS_IEEE_DOUBLE_FORMAT)
static void ConvertBufferDataToLE (csRenderBufferComponentType compType, 
                                   const void* source, void* dest, 
                                   size_t totalElementCount)
{
  switch (compType & ~CS_BUFCOMP_NORMALIZED)
  {
    case CS_BUFCOMP_BYTE:
    case CS_BUFCOMP_UNSIGNED_BYTE:
      memcpy (dest, source, totalElementCount);
      break;
    case CS_BUFCOMP_SHORT:
      ConvertBufferDataToLE ((int16*)source, (int16*)dest, totalElementCount);
      break;
    case CS_BUFCOMP_UNSIGNED_SHORT:
      ConvertBufferDataToLE ((uint16*)source, (uint16*)dest, totalElementCount);
      break;
    case CS_BUFCOMP_INT:
      ConvertBufferDataToLE ((int32*)source, (int32*)dest, totalElementCount);
      break;
    case CS_BUFCOMP_UNSIGNED_INT:
      ConvertBufferDataToLE ((uint32*)source, (uint32*)dest, totalElementCount);
      break;
    default:
      CS_ASSERT_MSG(false,
        "A buffer component type is not supported by this code");
  }
}
#endif

static const size_t RenderBufferComponentSizesOnDisk[CS_BUFCOMP_BASE_TYPECOUNT] = 
{
  sizeof (int8), sizeof (uint8), 
  sizeof (int16), sizeof (uint16),
  sizeof (int32), sizeof (uint32),
  sizeof (uint32),
  sizeof (uint64),
  sizeof (uint16)
};

csRef<iRenderBuffer> csTextSyntaxService::ReadRenderBuffer (iDataBuffer* buf, 
                                                            const char* filename)
{
  if (buf->GetSize() < sizeof (RenderBufferHeaderCommon)) return 0;
  RenderBufferHeaderCommon* header = 
    reinterpret_cast<RenderBufferHeaderCommon*> (buf->GetData());

  const uint16 magic = csLittleEndian::Convert (header->magic);
  if ((magic != RenderBufferHeaderCommon::MagicNormal)
    && (magic != RenderBufferHeaderCommon::MagicIndex))
    return 0;

  const size_t headerSize = (magic == RenderBufferHeaderCommon::MagicNormal)
    ? sizeof (RenderBufferHeaderNormal) : sizeof (RenderBufferHeaderIndex);

  const size_t totalElements = 
    header->compCount * csLittleEndian::Convert (header->elementCount);
  unsigned int compType = DiskToComponentType (header->compType);
  const size_t totalSizeDisk = 
    totalElements * RenderBufferComponentSizesOnDisk[compType & ~CS_BUFCOMP_NORMALIZED];

  // Sanity check
  if (buf->GetSize() < (totalSizeDisk + headerSize))
    return 0;

#if !defined(CS_LITTLE_ENDIAN) || !defined(CS_IEEE_DOUBLE_FORMAT)
  const size_t totalSize = 
    totalElements * csRenderBufferComponentSizes[compType & ~CS_BUFCOMP_NORMALIZED];

  // Buffer data is in little endian format, floats in IEEE, need to convert...
  csRef<iDataBuffer> newData;
  newData.AttachNew (new CS::DataBuffer<> (headerSize + totalSize));
  /* Also copy the header since StoredRenderBuffer will pull the header from
     the data buffer we provide on construction. */
  memcpy (newData->GetData(), header, headerSize);
  void* src = buf->GetData() + headerSize;
  void* dst = newData->GetData() + headerSize;

  switch (compType & ~CS_BUFCOMP_NORMALIZED)
  {
    case CS_BUFCOMP_FLOAT:
      ConvertFloatBufferDataFromLE ((uint32*)src, (float*)dst, totalElements);
      break;
    case CS_BUFCOMP_DOUBLE:
      ConvertFloatBufferDataFromLE ((uint64*)src, (double*)dst, totalElements);
      break;
    case CS_BUFCOMP_HALF:
      ConvertBufferDataToLE ((uint16*)src, (uint16*)dst, totalElements);
      break;
    default:
      ConvertBufferDataToLE ((csRenderBufferComponentType)compType,
        src, dst, totalElements);
  }

  // Use converted data
  buf = newData;
#endif

  csRef<StoredRenderBuffer> newBuffer;
  if (filename != 0)
  {
    StoredRenderBufferWithPersistence* bufPersist = 
      new StoredRenderBufferWithPersistence (buf);
    bufPersist->SetFileName (filename);
    newBuffer.AttachNew (bufPersist);
  }
  else
    newBuffer.AttachNew (new StoredRenderBuffer (buf));
  
  return newBuffer;
}

csRef<iDataBuffer> csTextSyntaxService::StoreRenderBuffer (iRenderBuffer* rbuf)
{
  csRenderBufferLock<uint8> buflock (rbuf, CS_BUF_LOCK_READ);
  void* srcData = buflock.Lock();

  const size_t totalElements = 
    rbuf->GetComponentCount() * rbuf->GetElementCount();
  const size_t totalSizeDisk = 
    totalElements * RenderBufferComponentSizesOnDisk[rbuf->GetComponentType() & ~CS_BUFCOMP_NORMALIZED];

#if !defined(CS_LITTLE_ENDIAN) || !defined(CS_IEEE_DOUBLE_FORMAT)
  // Buffer data is in little endian format, floats in IEEE, need to convert...
  csRef<iDataBuffer> newData;
  newData.AttachNew (new CS::DataBuffer<> (totalSizeDisk));
  void* src = srcData;
  void* dst = newData->GetData();

  switch (rbuf->GetComponentType () & ~CS_BUFCOMP_NORMALIZED)
  {
    case CS_BUFCOMP_FLOAT:
      ConvertFloatBufferDataToLE ((float*)src, (uint32*)dst, totalElements);
      break;
    case CS_BUFCOMP_DOUBLE:
      ConvertFloatBufferDataToLE ((double*)src, (uint64*)dst, totalElements);
      break;
    case CS_BUFCOMP_HALF:
      ConvertBufferDataToLE ((uint16*)src, (uint16*)dst, totalElements);
      break;
    default:
      ConvertBufferDataToLE (rbuf->GetComponentType(), src, dst, 
        totalElements);
  }

  srcData = dst;
#endif

  csRef<iDataBuffer> outBuf;
  size_t headerSize = 
    rbuf->IsIndexBuffer() ? sizeof (RenderBufferHeaderIndex)
      : sizeof (RenderBufferHeaderNormal);
  outBuf.AttachNew (new CS::DataBuffer<> (totalSizeDisk + headerSize));
  uint8* outPtr = outBuf->GetUint8();

  RenderBufferHeaderCommon commonHeader;
  commonHeader.magic = 0;
  commonHeader.compType = ComponentTypeToDisk (rbuf->GetComponentType());
  commonHeader.compCount = rbuf->GetComponentCount();
  commonHeader.elementCount = 
    csLittleEndian::Convert (uint32 (rbuf->GetElementCount()));
  if (rbuf->IsIndexBuffer())
  {
    RenderBufferHeaderIndex indexHeader;
    indexHeader.common = commonHeader;
    indexHeader.common.magic = RenderBufferHeaderCommon::MagicIndex;
    indexHeader.rangeStart = 
      csLittleEndian::Convert (uint32 (rbuf->GetRangeStart()));
    indexHeader.rangeEnd = 
      csLittleEndian::Convert (uint32 (rbuf->GetRangeEnd()));
    memcpy (outPtr, &indexHeader, sizeof (indexHeader));
  }
  else
  {
    RenderBufferHeaderNormal normalHeader;
    normalHeader.common = commonHeader;
    normalHeader.common.magic = RenderBufferHeaderCommon::MagicNormal;
    memcpy (outPtr, &normalHeader, sizeof (normalHeader));
  }
  memcpy (outPtr + headerSize, srcData, totalSizeDisk);
  
  return outBuf;
}

}
CS_PLUGIN_NAMESPACE_END(SyntaxService)
