/*

  DXTCGen.h - Bounds & variance based codebook generator class

*/

#ifndef DXTCGEN_H_
#define DXTCGEN_H_

#include "CodeBook.h"

namespace ImageLib 
{

class DXTCGen
{
private:
  CodeBook  Vects;
  cbVector  *pVects;
  cbVector Best[2], Test[2];

  static void BuildCodes3(cbVector *pVects, cbVector &v1, cbVector &v2);
  static void BuildCodes4(cbVector *pVects, cbVector &v1, cbVector &v2);
  static void BuildCodes8(cbVector *pVects, cbVector &v1, cbVector &v2);
  static void BuildCodes6(cbVector *pVects, cbVector &v1, cbVector &v2);

  static void BuildCodes3(cbVector *pVects, long Channel, cbVector &v1,
  	cbVector &v2);
  static void BuildCodes4(cbVector *pVects, long Channel, cbVector &v1,
  	cbVector &v2);

  long ComputeError3(CodeBook &Pixels);
  long ComputeError4(CodeBook &Pixels);
  long ComputeError8(CodeBook &Pixels);

public:
  DXTCGen();

  long Execute3(CodeBook &Source, CodeBook &Pixels, CodeBook &Dest);
  long Execute4(CodeBook &Source, CodeBook &Pixels, CodeBook &Dest);
  long Execute8(CodeBook &Source, CodeBook &Pixels, CodeBook &Dest);
  long Execute6(CodeBook &Source, CodeBook &Pixels, CodeBook &Dest);
};

} // end of namespace ImageLib

#endif
