/*
    Copyright (C) 2001 by Norman Krämer
  
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

#ifndef _ODIVX_H_
#define _ODIVX_H_

#include "ividecod.h"

class csOpenDivX : public iVideoCodec
{
 protected:
  bool bOK;
  int w,h;
  char *ydata, *udata, *vdata;
  char *yuvdata[3];

  DEC_PARAM dec_param;
  DEC_SET dec_set;
  DEC_FRAME dec_frame;

 public:
  DECLARE_IBASE;
  csOpenDivX (iBase *pParent);
  virtual ~csOpenDivX ();

  virtual bool Initialize (csVideoDescription &desc);
  virtual void GetCodecDescription (csCodecDescription &desc);
  virtual bool Decode (char *indata, long inlength, void *&outdata);
  virtual bool Encode (void *indata, char *outdata, long &outlength);
};
#endif
