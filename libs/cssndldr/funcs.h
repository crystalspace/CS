#ifndef _FUNCS_H
#define _FUNCS_H

/**
 * A static class with only static member functions.
 */
class csSndFunc
{
public:
  ///
  static unsigned long makeWord(int b0, int b1)
  { return ((b0&0xff)<<8)|b1&0xff; }

  ///
  static unsigned long makeDWord(int b0, int b1, int b2, int b3)
  { return ((b0&0xff)<<24)|((b1&0xff)<<16)|((b2&0xff)<<8)|b3&0xff; }

  ///
  static short int ulaw2linear(unsigned char ulawbyte);
};

#endif // _FUNCS_H
