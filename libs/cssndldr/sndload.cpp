/*
    Copyright (C) 1998 by Jorrit Tyberghein    

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

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "sysdef.h"
#include "csutil/csvector.h"
#include "cssndldr/common/sndbuf.h"
#include "cssndldr/sndload.h"
#include "isystem.h"

// Register all file formats we want
#define REGISTER_FORMAT(format) \
  extern bool Register##format (); \
  bool __##format##Support = Register##format ();

#if defined (DO_AIFF)
  REGISTER_FORMAT (AIFF)
#endif
#if defined (DO_AU)
  REGISTER_FORMAT (AU)
#endif
#if defined (DO_WAV)
  REGISTER_FORMAT (WAV)
#endif
#if defined (DO_IFF)
  REGISTER_FORMAT (IFF)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

csVector *csSoundBufferLoader::loaderlist = NULL;

bool csSoundBufferLoader::Register (csSoundBufferLoader* loader)
{
  if (!loaderlist)
    { CHK (loaderlist = new csVector (8, 8)); }
  loaderlist->Push ((csSome)loader);
  return true;
}

csSoundBuffer* csSoundBufferLoader::load (ISystem* system, const char* filename)
{
  FILE* fp;
  system->FOpen (filename, "rb", &fp);
  if (!fp) return NULL;
  csSoundBuffer* i = load(fp);
  system->FClose (fp);
  return i;
}

csSoundBuffer* csSoundBufferLoader::load (FILE* fp)
{
  fseek(fp, 0, SEEK_END);
  ULong size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  CHK (UByte* buf = new UByte [size+1] );
  fread(buf, 1, size+1, fp);
  csSoundBuffer* i = load(buf,size);
  CHK (delete [] buf);
  return i;
}

csSoundBuffer* csSoundBufferLoader::load (UByte* buf, ULong size)
{
  int i=0;
  while (i < loaderlist->Length() )
  {
    csSoundBufferLoader* l = (csSoundBufferLoader*) (loaderlist->Get (i));
    csSoundBuffer* w = l->loadsound (buf,size);
    if (w) return w;
    i++; 
  }
  return NULL;
}
