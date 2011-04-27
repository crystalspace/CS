//=============================================================================
//
//	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXConfigFile.cpp
//
//	Plain-C wrapper of a few functions from the SCF iConfigFile interface.
//	Used by the Objective-C OSXMenu.m.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "iutil/cfgfile.h"
#include "OSXConfigFile.h"

#define NCF_PROTO(RET,FUNC) RET OSXConfigFile_##FUNC

NCF_PROTO(char const*,lookup)(
  OSXConfigHandle handle, char const* key, char const* fallback)
  { return ((iConfigFile*)handle)->GetStr(key, fallback); }
NCF_PROTO(int,exists)(OSXConfigHandle handle, char const* section)
  { return ((iConfigFile*)handle)->SubsectionExists(section); }
NCF_PROTO(OSXConfigIterator,new_iterator)(OSXConfigHandle handle,
  char const* section)
  { 
  csRef<iConfigIterator> r = ((iConfigFile*)handle)->Enumerate(section);
  iConfigIterator* p = r;
  p->IncRef();
  return (OSXConfigIterator)p;
  }
NCF_PROTO(void,dispose_iterator)(OSXConfigIterator handle)
  { ((iConfigIterator*)handle)->DecRef(); }
NCF_PROTO(int,iterator_next)(OSXConfigIterator handle)
  { return ((iConfigIterator*)handle)->Next(); }
NCF_PROTO(char const*,iterator_key)(OSXConfigIterator handle)
  { return ((iConfigIterator*)handle)->GetKey(true); }
NCF_PROTO(char const*,iterator_data)(OSXConfigIterator handle)
  { return ((iConfigIterator*)handle)->GetStr(); }

#undef NCF_PROTO
