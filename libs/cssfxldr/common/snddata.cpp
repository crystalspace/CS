/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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

#include "sysdef.h"
#include "qint.h"
#include "cssfxldr/common/snddata.h"

csSoundData::csSoundData(int frequency, bool bit16, bool stereo, bool sign, long size, void *data)
{
  Data = data;
  Size = size;
  Frequency = frequency;
  Bit16 = bit16;
  Stereo = stereo;
  Sign = sign;
}


csSoundData::~csSoundData()
{
  Clean();
}

void csSoundData::Clean()
{
  if(Data) 
  {
	  Data = NULL;
	  CHKB (delete Data);
  }
  Size = 0;
  Frequency = 0;
  Bit16 = false;
  Stereo = false;
  Sign = false;
}

int csSoundData::getFrequency()
{
  return Frequency;
}

bool csSoundData::is16Bits()
{
  return Bit16;
}

bool csSoundData::isStereo()
{
  return Stereo;
}

void * csSoundData::getData()
{
  return Data;
}

long csSoundData::getSize()
{
  return Size;
}

bool csSoundData::convertTo(int toFrequency, bool toBit16, bool toStereo)
{
  convertStereoTo(toStereo);
  convert16bitTo(toBit16);
  convertFrequencyTo(toFrequency);

  return true;
}

bool csSoundData::convertFrequencyTo (int toFrequency)
{
  int i;

  if (toFrequency!=Frequency)
  {
    double coeff=(double)(Frequency)/(double)(toFrequency);

    if(!Bit16)
    {
      unsigned char *convert;
      unsigned char *from=(unsigned char *)Data;
      long newsize=(unsigned long)((double)(Size)/coeff);
      int nextchan=(Stereo)?2:1;
      double step;

      CHK (convert=new unsigned char [newsize]);
      if (convert == NULL) return 0;

      for(i = 0, step = 0.0; i < newsize; i++, step += coeff)
      {
        int intstep = QInt (step);
        double interstep = step - intstep;
        convert [i] = QInt (from [intstep] +
          ((int (from [intstep + nextchan]) - int (from [intstep]))) * interstep);
      }

      CHK (delete Data);
      Data = (unsigned char *)convert;
      Size = newsize;
      Frequency = toFrequency;
    }
    else // if (bit16)
    {
      short *convert;
      short *from=(short *)Data;
      long newsize;
      int nextchan=(Stereo)?2:1;
      double step;

      newsize=(unsigned long)((double)(Size)/coeff);

      CHK (convert=new short [newsize]);
      if (convert == NULL) return 0;

      for (i = 0, step = 0.0; i < newsize; i++, step += coeff)
      {
        int intstep = QInt (step);
        double interstep = step - intstep;

        convert [i] = QInt (from [intstep] +
          (int (from [intstep + nextchan]) - int (from [intstep])) * interstep);
      }

      CHK (delete Data);
      Data=(unsigned char *)convert;
      Size=newsize;
      Frequency=toFrequency;
    }
  }

  return true;
}

bool csSoundData::convert16bitTo(bool toBit16)
{
  int i;

  if(toBit16!=Bit16)
  {
    if(toBit16)
    {
      short *convert;
      unsigned char *from=(unsigned char *)Data;

      CHK (convert=new short [Size]);
      if(convert==NULL) return 0;

      for(int j=0; j<Size; j++)
        convert [j] = (from [j] * 256) - 32768;

      CHK (delete Data);
      Data=(unsigned char *)convert;
      Bit16=true;
    }
    else // if(!toBit16)
    {
      unsigned char *convert;
      short *from=(short *)Data;

      CHK (convert = new unsigned char [Size]);
      if(convert == NULL) return 0;

      for (i = 0; i < Size; i++)
        convert [i] = (from [i] - 32768) / 256;

      CHK (delete Data);
      Data = (unsigned char *)convert;
      Bit16 = false;
    }
  }

  return true;
}

bool csSoundData::convertStereoTo(bool toStereo)
{
  int i;

  if(toStereo!=Stereo)
  {
    if(!toStereo)
    {
      if(!Bit16)
      {
        unsigned char *convert;
        unsigned char *from=(unsigned char *)Data;
        long newsize=Size/2;

        CHK (convert=new unsigned char [newsize]);
        if(convert==NULL) return 0;

        for(i=0; i<newsize; i++)
          convert[i]=(from[i*2]+from[(i*2)+1])/2;

        CHK (delete Data);
        Data=(unsigned char *)convert;
        Stereo=false;
        Size=newsize;
      } 
      else // if(bit16)
      {
        short *convert;
        short *from=(short *)Data;
        long newsize=Size/2;

        CHK (convert=new short [newsize]);
        if(convert==NULL) return 0;

        for(i=0; i<newsize; i++)
          convert[i]=(from[i*2]+from[(i*2)+1])/2;

        CHK (delete Data);
        Data=(unsigned char *)convert;
        Stereo=false;
        Size=newsize;
      } 
    }
    else // if(toStereo)
    {
      if(!Bit16)
      {
        unsigned char *convert;
        unsigned char *from=(unsigned char *)Data;
        long newsize=Size*2;

        CHK (convert=new unsigned char [newsize]);
        if(convert==NULL) return 0;

        for(i=0; i<Size; i++)
          convert[i*2]=convert[(i*2)+1]=from[i];

        CHK (delete Data);
        Data=(unsigned char *)convert;
        Size=newsize;
        Stereo=true;
      } 
      else // if(bit16)
      {
        short *convert;
        short *from=(short *)Data;
        long newsize=Size*2;

        CHK (convert=new short [newsize]);
        if(convert==NULL) return 0;

        for(i=0; i<Size; i++)
          convert[i*2]=convert[(i*2)+1]=from[i];

        CHK (delete Data);
        Data=(unsigned char *)convert;
        Size=newsize;
        Stereo=true;
      } 
    }
  }

  return true;
}

void csSoundData::forceMute()
{
  if(Bit16)
  {
    short *ptr=(short *)Data;
    for(int i=0; i<Size; i++)
      *ptr++=0;
  }
  else
  {
    unsigned char *ptr=(unsigned char *)Data;
    for(int i=0; i<Size; i++)
      *ptr++=128;
  }
}
