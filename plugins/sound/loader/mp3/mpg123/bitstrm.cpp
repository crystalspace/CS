#include "cssysdef.h"
#include "bitstrm.h"

csBitstreamInfo::csBitstreamInfo ()
{
  Init ();
}

void csBitstreamInfo::Init ()
{
  bsbuf=bsspace[1];
  bsnum = 0;
  index = 0;
  bsbufold = NULL;
  wordpointer = NULL;
  framesize = 0;
}

void csBitstreamInfo::FlipBuffer ()
{
  bsbufold = bsbuf;
  bsbuf = bsspace[bsnum]+512;
  bsnum = bsnum ^ 1;
}

void csBitstreamInfo::Rewind ()
{
  index = 0;
  wordpointer = bsbuf;
}

void csBitstreamInfo::SetPointer (long backstep)
{
  wordpointer = bsbuf + skipsize - backstep;
  if (backstep)
    memcpy (wordpointer, bsbufold+framesize-backstep, backstep);
  index = 0; 
}

void csBitstreamInfo::Back (int number_of_bits)
{
  index    -= number_of_bits;
  wordpointer += (index>>3);
  index    &= 0x7;
}

int csBitstreamInfo::GetOffset () 
{
  return (-index)&0x7;
}

int csBitstreamInfo::GetByte ()
{
  return *wordpointer++;
}

unsigned int csBitstreamInfo::GetBits (int number_of_bits)
{
  unsigned long rval;

  if(!number_of_bits)
    return 0;

  rval = wordpointer[0];
  rval <<= 8;
  rval |= wordpointer[1];
  rval <<= 8;
  rval |= wordpointer[2];
  
  rval <<= index;
  rval &= 0xffffff;

  index += number_of_bits;

  rval >>= (24-number_of_bits);

  wordpointer += (index>>3);
  index &= 7;

  return (unsigned int)rval;
}

unsigned int csBitstreamInfo::GetBitsFast (int number_of_bits)
{
  unsigned int rval;

  rval =  (unsigned char) (wordpointer[0] << index);
  rval |= ((unsigned int) wordpointer[1]<<index)>>8;
  rval <<= number_of_bits;
  rval >>= 8;

  index += number_of_bits;

  wordpointer += (index>>3);
  index &= 7;

  return rval;
}

unsigned int csBitstreamInfo::Get1Bit ()
{
  unsigned char rval;

  rval = *wordpointer << index;

  index++;
  wordpointer += (index>>3);
  index &= 7;
  return rval>>7;
}

