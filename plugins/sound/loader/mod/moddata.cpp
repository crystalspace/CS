/*
    Copyright (C) 2001 by Norman Kraemer

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
#include "moddata.h"
#include "isound/loader.h"
#include "iutil/comp.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csModSoundData)
  SCF_IMPLEMENTS_INTERFACE (iSoundData)
SCF_IMPLEMENT_IBASE_END

int ii=0;
/********** mod reader functions ***********/
static int cs_modreader_seek (MREADER* mr, long offset, int whence)
{
  csModSoundData::cs_mod_reader *r = (csModSoundData::cs_mod_reader *)mr;

  long newpos = (whence == SEEK_SET ? offset :
		 whence == SEEK_CUR ? r->ds.pos + offset : r->ds.length + offset);

  //  printf("pos = %ld -> seek %ld -> pos = %ld\n",r->ds.pos, offset, newpos);
  if (newpos < 0 || newpos > r->ds.length)
    return -1;
  else
    r->ds.pos = newpos;
  r->bEof = false;
  return 0;
}

static long cs_modreader_tell (MREADER* mr)
{
  csModSoundData::cs_mod_reader *r = (csModSoundData::cs_mod_reader *)mr;
  return r->ds.pos;
}

static int cs_modreader_read (MREADER* mr, void *dest, size_t length)
{
  csModSoundData::cs_mod_reader *r = (csModSoundData::cs_mod_reader *)mr;
  if (r->ds.data == 0) return 0;

  size_t maxsize = MIN (((size_t)(r->ds.length - r->ds.pos)), length);
  memcpy (dest, r->ds.data + r->ds.pos, maxsize);
  r->bEof = r->ds.pos + (long)length > r->ds.length;

  r->ds.pos += (long)maxsize;

  //  printf ("%ld %ld %d\n", r->ds.pos, r->ds.length, length);
  return (int)maxsize;
}

static int cs_modreader_get (MREADER* mr)
{
  unsigned char c = '\0';
  if (cs_modreader_read (mr, &c, 1) != 1)
    return EOF;
  return (int)c;
}

static int cs_modreader_eof (MREADER* mr)
{
  //printf("eof\n");
  csModSoundData::cs_mod_reader *r = (csModSoundData::cs_mod_reader *)mr;
  //  printf ("eof %d\n", r->ds.pos >= r->ds.length ? 1 : 0);
  //return r->ds.pos > r->ds.length ? 1 : 0;
  //  printf ("eof %d\n", r->bEof ? 1 : 0);
  return r->bEof ? 1 : 0;
}

csModSoundData::cs_mod_reader::cs_mod_reader (uint8 *d, size_t l, bool own)
  : ds (d, l, own)
{
  Seek = cs_modreader_seek;
  Tell = cs_modreader_tell;
  Read = cs_modreader_read;
  Get  = cs_modreader_get;
  Eof  = cs_modreader_eof;
  bEof = false;
}

bool csModSoundData::mikmod_init = true;
bool csModSoundData::mikmod_reinit = false;
int  csModSoundData::bits = 16;
int  csModSoundData::channels = 2;

csModSoundData::csModSoundData (iBase *parent, uint8 *data, size_t len)
{
  SCF_CONSTRUCT_IBASE (parent);

  mod_ok = false;
  buf = 0;
  bytes_left = 0;
  pos = 0;
  this->len = 0;
  fmt.Bits = 16;
  fmt.Channels = 2;
  fmt.Freq = 44100;

  mod_reader = new cs_mod_reader (data, len, true);
  module = 0;
}

csModSoundData::~csModSoundData ()
{
  if (module)
    Player_Free (module);
  if(buf) free(buf);
  delete mod_reader;
  SCF_DESTRUCT_IBASE();
}

bool csModSoundData::Initialize (const csSoundFormat *fmt)
{
  if (fmt->Freq != -1)
    this->fmt.Freq = fmt->Freq;

  if (mikmod_init || mikmod_reinit)
  {
    if (!mikmod_reinit)
    {
      MikMod_RegisterDriver (&drv_nos);
      MikMod_RegisterAllLoaders ();
    }
    //      MikMod_Exit ();
    mikmod_reinit = false;

    if (fmt->Bits != -1)
      this->fmt.Bits = fmt->Bits;

    if (fmt->Channels != -1)
      this->fmt.Channels = fmt->Channels;

    bits = this->fmt.Bits;
    channels = this->fmt.Channels;

    if (this->fmt.Bits == 16)
      md_mode |= DMODE_16BITS;
    else
      md_mode &= ~DMODE_16BITS;
    if (this->fmt.Channels == 2)
      md_mode |= DMODE_STEREO;
    else
      md_mode &= ~DMODE_STEREO;

    if (MikMod_Init ("") != 0)
    {
      printf ("MikMod could not be initialized, reason : %s\n", MikMod_strerror (MikMod_errno));
      return false;
    }
    mikmod_init = false;
  }
  else
  {
    this->fmt.Bits = bits;
    this->fmt.Channels = channels;
  }

  if (mod_reader == 0)
  {
    printf ("csModSoundData: Not enough memory to load sample\n");
    return false;
  }

  module = Player_LoadGeneric ((MREADER*)mod_reader, 64, 0);

  // now we can free the reader already
  delete mod_reader;
  mod_reader = 0;

  if (module == 0) // no mod file
    return false;

  md_mixfreq = this->fmt.Freq;

  Player_Start (module);
  Player_SetPosition (0);

  mod_ok = true;
  return mod_ok;
}

bool csModSoundData::IsMod (void *Buffer, size_t len)
{
  if (mikmod_init)
  {
    MikMod_RegisterDriver (&drv_nos);
    MikMod_RegisterAllLoaders ();

    md_mode |= DMODE_16BITS;
    md_mode |= DMODE_STEREO;

    if (MikMod_Init ("") != 0)
    {
      printf ("MikMod could not be initialized, reason : %s\n", MikMod_strerror (MikMod_errno));
      return false;
    }
    // if it's initialized here then we'll probably get different values for stereo and bits later
    // so we reinit later with final values
    mikmod_reinit = true;
    mikmod_init = false;
  }

  csModSoundData::cs_mod_reader mod_reader ((uint8*)Buffer, len, false);
  MODULE *module = Player_LoadGeneric ((MREADER*)&mod_reader, 64, 0);

  bool is_mod = module != 0;

  if (is_mod)
    Player_Free (module);
  //  else
  //    printf ("error: %s\n", MikMod_strerror(MikMod_errno));

  return is_mod;
}

const csSoundFormat* csModSoundData::GetFormat()
{
  return &fmt;
}

bool csModSoundData::IsStatic()
{
  return false;
}

long csModSoundData::GetStaticSampleCount()
{
  return -1;
}

void *csModSoundData::GetStaticData()
{
  return 0;
}

void csModSoundData::ResetStreamed()
{
  Player_SetPosition (0);
  bytes_left = 0;
  mod_ok = true;
}

void *csModSoundData::ReadStreamed(long &NumSamples)
{
  Player_Start (module);
  if (Player_Active () && mod_ok)
  {
    unsigned char *p;
    long buffersize = NumSamples * (fmt.Bits >> 3) * fmt.Channels;
    if (buffersize > len)
    {
      buf = (uint8*) realloc (buf, buffersize);
      len = buffersize;
    }

    if (!bytes_left)
    {
      buffersize = VC_WriteBytes ((SBYTE*)buf, buffersize);
      if (buffersize)
      {
	bytes_left = buffersize;
	pos = buf;
      }
      else
      {
	NumSamples = 0;
	return 0;
      }
    }

    p = pos;

    long samples = bytes_left / ((fmt.Bits >> 3) * fmt.Channels);

    //    printf ("returning %d samples\n", samples);

    if (samples > NumSamples)
    {
      pos += NumSamples * ((fmt.Bits >> 3) * fmt.Channels);
      bytes_left -= NumSamples * ((fmt.Bits >> 3) * fmt.Channels);
    }
    else
    {
      NumSamples = samples;
      bytes_left = 0;
    }

    return p;
  }

  NumSamples = 0;
  return 0;
}


// mod loader

class csModLoader : public iSoundLoader
{
public:
  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModLoader);
    virtual bool Initialize (iObjectRegistry *){return true;}
  }scfiComponent;

  csModLoader (iBase *parent)
  {
    SCF_CONSTRUCT_IBASE (parent);
    SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  }

  virtual ~csModLoader ()
  {
    if (!csModSoundData::mikmod_init || csModSoundData::mikmod_reinit)
      MikMod_Exit ();
    SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
    SCF_DESTRUCT_IBASE();
  }

  virtual csPtr<iSoundData> LoadSound (void *Buffer, uint32 Size)
  {
    csModSoundData *sd=0;
    if (csModSoundData::IsMod (Buffer, Size))
      sd = new csModSoundData ((iBase*)this, (uint8*)Buffer, Size);
    return csPtr<iSoundData> (sd);
  }
};

SCF_IMPLEMENT_IBASE (csModLoader)
  SCF_IMPLEMENTS_INTERFACE (iSoundLoader)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csModLoader);


