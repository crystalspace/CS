
#include "cssysdef.h"
#include "qint.h"
#include "csutil/rng.h"

#include <string.h>
#include <math.h>
#include <malloc.h>

/**
 * Idea for this kind of halo has been partially borrowed from GIMP:
 * SuperNova plug-in
 * Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>,
 *                     Spencer Kimball, Federico Mena Quintero
 */
unsigned char *GenerateNova (int iSize, int iSeed, int iNumSpokes, float iRoundness)
{
  csRandomGen rnd (iSeed);
  unsigned char *image = new unsigned char [iSize * iSize];

  const int radius = iSize / 2;
  const int center = radius;

  float *spoke = (float *)alloca ((iNumSpokes + 2) * sizeof (float));
  for (int i = 0; i < iNumSpokes; i++)
    spoke [i] = iRoundness + rnd.Get () * (1 - iRoundness);
  spoke [iNumSpokes] = spoke [0];
  spoke [iNumSpokes + 1] = spoke [1];

  for (int y = 0; y < iSize; y++)
    for (int x = 0; x < iSize; x++)
    {
      // u is in 0..1 interval
      float u = float (x - center) / radius;
      // v is in 0..1 interval
      float v = float (y - center) / radius;
      // l will never exceed sqrt(2) e.g. 1.4142...
      float l = sqrt (u * u + v * v);

      // c is a float from 0..iNumSpokes
      float c = (atan2 (u, v) / (2 * M_PI) + .5) * iNumSpokes;
      // i is the spoke number
      int i = QInt (c);
      // c is the float part of former c
      c -= i;

      // w1 is the pixel intensity depending on spokes, 0..1
      float w1 = spoke [i] * (1 - c) + spoke [i + 1] * c;
      // w is the inverse of distance between center and current point
      float w = 0.9 / (l + 0.001);
      c = w1 * w1 * w * w;
      if (c > 4.0) c = 4.0;
      image [x + y * iSize] = QInt (c * 63.9);
    }

  return image;
}
