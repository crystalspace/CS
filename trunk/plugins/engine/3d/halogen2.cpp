/**
 * Idea for this kind of halo has been partially borrowed from GIMP:
 * SuperNova plug-in
 * Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>,
 *                     Spencer Kimball, Federico Mena Quintero
 */

#include "cssysdef.h"
#include "csqint.h"
#include "csutil/randomgen.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

unsigned char*
csGenerateNova (int iSize, int iSeed, int iNumSpokes, float iRoundness)
{
  csRandomGen rnd (iSeed);
  unsigned char *image = new unsigned char [iSize * iSize];

  const int radius = iSize / 2;
  const int center = radius;

  CS_ALLOC_STACK_ARRAY (float, spoke, iNumSpokes + 2);
  int i;
  for (i = 0; i < iNumSpokes; i++)
    spoke [i] = rnd.Get ();//iRoundness + rnd.Get () * (1 - iRoundness);
  spoke [iNumSpokes] = spoke [0];
  spoke [iNumSpokes + 1] = spoke [1];

  int y;
  for (y = 0; y < iSize; y++)
  {
	int x;
    for (x = 0; x < iSize; x++)
    {
      // u is in -1..1 interval
      float u = float (x - center) / radius;
      // v is in -1..1 interval
      float v = float (y - center) / radius;
      // l will never exceed sqrt(2) e.g. 1.4142...
      float l = u * u + v * v;

      // c is a float from 0..iNumSpokes
      float c = (atan2 (u, v) / TWO_PI + 0.5f) * iNumSpokes;
      // i is the spoke number
      int i = csQint (c);
      // c is the float part of former c
      c -= i;

      // w1 is the pixel intensity depending on spokes, 0..1
      float w1 = spoke [i] * (1 - c) + spoke [i + 1] * c;
      float w = 1.1f - pow (l, iRoundness);
      if (w < 0.0f) w = 0.0f; if (w > 1.0f) w = 1.0f;
      image [x + y * iSize] = csQint (w * (w + (1.0f - w) * w1) * 255.9f);
    }
  }

  return image;
}
