/*
    Copyright (C) 2002 by Norman Kraemer

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

#include <divx4/decore.h>
#include "cssysdef.h"
#include "odivx4.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csDivX4)
  SCF_IMPLEMENTS_INTERFACE (iAVICodec)
  SCF_IMPLEMENTS_INTERFACE (iBase)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csDivX4)

csDivX4::csDivX4 (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  bOK = false;
  decParam.buffers.mp4_edged_ref_buffers = 0;
  decParam.buffers.mp4_edged_for_buffers = 0;
  decParam.buffers.mp4_edged_back_buffers = 0;
  decParam.buffers.mp4_display_buffers = 0;
  decParam.buffers.mp4_state = 0;
  decParam.buffers.mp4_tables = 0;
  decParam.buffers.mp4_stream = 0;
  decParam.buffers.mp4_reference = 0;
  result = 0;
}

csDivX4::~csDivX4 ()
{
  if (bOK)
  {
    // stop current processing
    decore ((long) this, DEC_OPT_RELEASE, 0, 0);
  }
   
  free(decParam.buffers.mp4_display_buffers);
  free(decParam.buffers.mp4_edged_for_buffers);
  free(decParam.buffers.mp4_edged_back_buffers);
  free(decParam.buffers.mp4_edged_ref_buffers);
  free(decParam.buffers.mp4_reference);
  free(decParam.buffers.mp4_state);
  free(decParam.buffers.mp4_stream);
  free(decParam.buffers.mp4_tables);

  delete [] result;
  SCF_DESTRUCT_IBASE();
}

bool csDivX4::Initialize (csStreamDescription *desc, uint8 *, uint32, uint8 *, uint32)
{
  //DEBUG_BREAK;
  csVideoStreamDescription *vd = (csVideoStreamDescription *)desc;
  w = vd->width;
  h = vd->height;
  bOK = false;

  decParam.x_dim = w;
  decParam.y_dim = h;
  decParam.output_format = DEC_RGB32_INV;
  decParam.time_incr = 15; // time_incr default value

  bOK = (decore((long) this, DEC_OPT_MEMORY_REQS, &decParam, &decMemReqs) == DEC_OK);
  if (bOK)
  {
    
    // the application allocates the data structures and the buffers
    decParam.buffers.mp4_edged_ref_buffers = malloc(decMemReqs.mp4_edged_ref_buffers_size);
    decParam.buffers.mp4_edged_for_buffers = malloc(decMemReqs.mp4_edged_for_buffers_size);
    decParam.buffers.mp4_edged_back_buffers = malloc(decMemReqs.mp4_edged_back_buffers_size);
    decParam.buffers.mp4_display_buffers = malloc(decMemReqs.mp4_display_buffers_size);
    decParam.buffers.mp4_state = malloc(decMemReqs.mp4_state_size);
    decParam.buffers.mp4_tables = malloc(decMemReqs.mp4_tables_size);
    decParam.buffers.mp4_stream = malloc(decMemReqs.mp4_stream_size);
    decParam.buffers.mp4_reference = malloc(decMemReqs.mp4_reference_size);

    memset(decParam.buffers.mp4_state, 0, decMemReqs.mp4_state_size);
    memset(decParam.buffers.mp4_tables, 0, decMemReqs.mp4_tables_size);
    memset(decParam.buffers.mp4_stream, 0, decMemReqs.mp4_stream_size);
    memset(decParam.buffers.mp4_reference, 0, decMemReqs.mp4_reference_size);
    
    bOK = (decore ((unsigned long) this, DEC_OPT_INIT, &decParam, 0) == DEC_OK);
    if (bOK)
    {
      dec_set.postproc_level = 0; // no pp
      bOK = (decore ((unsigned long)this, DEC_OPT_SETPP, &dec_set, 0) == DEC_OK);
      result = new char [4*w*h];
    }
  }

  return bOK;
}

void csDivX4::GetCodecDescription (csCodecDescription &desc)
{
  desc.bEncode = false; // not implemented yet
  desc.bDecode = true;
  desc.decodeoutput = CS_CODECFORMAT_RGBA_INTERLEAVED;
  desc.encodeinput = CS_CODECFORMAT_RGBA_INTERLEAVED;
}

bool csDivX4::Decode (char *indata, uint32 inlength, void *&outdata)
{
  bool succ=false;
  if (bOK)
  {
    dec_frame.length = inlength;
    dec_frame.bitstream = indata;
    dec_frame.bmp = result;
    dec_frame.render_flag = 1;
    dec_frame.stride=w;
    succ = decore ((unsigned long)this, DEC_OPT_FRAME, &dec_frame, 0) == DEC_OK;
    outdata = result;
    return true;
  }
  return false;
}

bool csDivX4::Encode (void *indata, char *outdata, uint32 &outlength)
{
  (void)indata;
  (void)outdata;
  (void)outlength;
  return false;
}

