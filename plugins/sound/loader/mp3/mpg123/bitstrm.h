#ifndef _CS_BITSTREAMINFO_H_
#define _CS_BITSTREAMINFO_H_

#define MAXFRAMESIZE 1792

class csBitstreamInfo
{
 protected:
  int skipsize;
  int framesize;

 public:

  csBitstreamInfo ();

  void Init ();

  void FrameSize (int fsize)
  { framesize = fsize; }

  void SkipSize (int size)
  { skipsize = size; }

  void FlipBuffer ();
  void Rewind ();

  void SetPointer (long backstep);
  unsigned int Get1Bit ();
  unsigned int GetBitsFast (int number_of_bits);
  unsigned int GetBits (int number_of_bits);
  int GetByte ();

  int GetOffset ();
  void Back (int number_of_bits);

  unsigned char bsspace[2][MAXFRAMESIZE+512];
  unsigned char *bsbuf, *bsbufold;
  int bsnum;
  int index;
  unsigned char *wordpointer;
};

#endif
