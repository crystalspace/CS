/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CANVAS_COMMON_DRAW_LINE_H__
#define __CS_CANVAS_COMMON_DRAW_LINE_H__

#include "draw_common.h"
#include "csqint.h"

template<class Tpixel, class Tpixmixer>
class csG2DDrawLine
{
public:
  static void DrawLine (csGraphics2D* G2D, float x1, float y1, float x2, float y2,
    Tpixel color, uint8 alpha)
  {
    Tpixmixer mixer (G2D, color, alpha);
    unsigned char* Memory = G2D->Memory;
    const int* LineAddress = G2D->LineAddress;

    int fx1 = csQint (x1), fx2 = csQint (x2),
	fy1 = csQint (y1), fy2 = csQint (y2);
    
    if (fy1 == fy2)
    {
      if (fx2 - fx1)
      {
	if (fx1 > fx2) { int tmp = fx1; fx1 = fx2; fx2 = tmp; }
	int count = fx2 - fx1 + 1;
	register Tpixel* dest = (Tpixel*)G2D->GetPixelAt (fx1, fy1);
	while (count--) mixer.Mix (*dest++);
      }
      else
	mixer.Mix (*((Tpixel*)G2D->GetPixelAt (fx1, fy1)));
    }
    else if (abs (fx2 - fx1) > abs (fy2 - fy1))
    {
      // Transform floating-point format to 16.16 fixed-point
      fy1 = csQint16 (y1); fy2 = csQint16 (y2);
  
      if (fx1 > fx2)
      {
	int tmp = fx1; fx1 = fx2; fx2 = tmp;
	tmp = fy1; fy1 = fy2; fy2 = tmp;
      }
  
      // delta Y can be negative
      int deltay = (fy2 - fy1) / (fx2 - fx1 + 1);
  
      {                             				
	int x, y;  						
	for (x = fx1, y = fy1 + deltay / 2; x <= fx2; x++)  	
	{                               				
	  Tpixel* p = (Tpixel*)(Memory +             		
	    (x * sizeof (Tpixel) + LineAddress [y >> 16]));    	
	  mixer.Mix (*p); y += deltay;                  		
	}                               				
      }
    }
    else
    {
      // Transform floating-point format to 16.16 fixed-point
      fx1 = csQint16 (x1); fx2 = csQint16 (x2);
  
      if (fy1 > fy2)
      {
	int tmp = fy1; fy1 = fy2; fy2 = tmp;
	tmp = fx1; fx1 = fx2; fx2 = tmp;
      }
  
      // delta X can be negative
      int deltax = (fx2 - fx1) / (fy2 - fy1 + 1);
  
      {                             
	int x, y; 
	for (x = fx1 + deltax / 2, y = fy1; y <= fy2; y++)  
	{                               
	  Tpixel* p = (Tpixel*)(Memory +             
	    ((x >> 16) * sizeof (Tpixel) + LineAddress [y]));  
	  mixer.Mix (*p); x += deltax;                  
	}                               
      }
    }
    
  }
};

#endif // __CS_CANVAS_COMMON_DRAW_LINE_H__
