#include "cssysdef.h"
#include "csutil/scfstr.h"
#include "awsprefs.h"
#include <stdio.h>

extern int awsparse(void *prefscont);
extern FILE *awsin;
unsigned long aws_adler32(unsigned long adler,  const unsigned char *buf,  unsigned int len);


/***************************************************************************************************************
*   This constructor converts the text-based name into a fairly unique numeric ID.  The ID's are then used for *
* comparison.  The method of text-to-id mapping may be somewhat slower than a counter, but it does not have to *
* worry about wrapping or collisions or running out during long execution cycles.                              *
***************************************************************************************************************/
awsKey::awsKey(iString *n)
{
  if (n) {
    name = aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)n->GetData(), n->Length());
    delete n;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////


awsPrefManager::awsPrefManager(iBase *iParent):n_win_defs(0), n_skin_defs(0)
{
  CONSTRUCT_IBASE (iParent);
}

awsPrefManager::~awsPrefManager()
{
}

void 
awsPrefManager::Load(const char *def_file)
{
  printf("\tloading definitions file %s...\n", def_file);

  awsin = fopen( def_file, "r" );

  unsigned int ncw = n_win_defs,
               ncs = n_skin_defs;

  if(awsparse(this))
      printf("\tsyntax error in definition file, load failed.\n");
  else
  {
     printf("\tload successful (%i windows, %i skins loaded.)\n", n_win_defs-ncw, n_skin_defs-ncs);
  }
  
  fclose(awsin);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

/* ========================================================================= */
unsigned long
aws_adler32(unsigned long adler,  const unsigned char *buf,  unsigned int len)
{
    unsigned long s1 = adler & 0xffff;
    unsigned long s2 = (adler >> 16) & 0xffff;
    int k;

    if (buf == NULL) return 1L;

    while (len > 0) {
        k = len < NMAX ? len : NMAX;
        len -= k;
        while (k >= 16) {
            DO16(buf);
	    buf += 16;
            k -= 16;
        }
        if (k != 0) do {
            s1 += *buf++;
	    s2 += s1;
        } while (--k);
        s1 %= BASE;
        s2 %= BASE;
    }
    return (s2 << 16) | s1;
}

