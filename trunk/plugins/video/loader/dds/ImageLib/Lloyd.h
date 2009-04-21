/*

  Lloyd.h - Lloyd codebook generator class

*/

#ifndef LLOYD_H_
#define LLOYD_H_

#include "fCodeBook.h"

CS_PLUGIN_NAMESPACE_BEGIN(DDSImageIO)
{
namespace ImageLib 
{

class Lloyd
{
private:
  // List of codebooks (the centroid of each will ultimately be our result
  // codes)
  ccMinList  Codes;

public:
  long Execute(fCodebook &Source, fCodebook &Dest, long Target);
};

} // end of namespace ImageLib
}
CS_PLUGIN_NAMESPACE_END(DDSImageIO)

#endif
