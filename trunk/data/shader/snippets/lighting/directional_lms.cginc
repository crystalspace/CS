<!--
  Copyright (C) 2008 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
-->
<include>
<![CDATA[

#ifndef __DIRECTIONAL_LMS_CG_INC__
#define __DIRECTIONAL_LMS_CG_INC__

float3 ComputeDLMCoeff (float3 direction)
{
  /* These should really match up with those in lighter2... */
  float3 base[3] = 
  { 
    float3 (/* -1/sqrt(6) */ -0.408248, /* 1/sqrt(2) */ 0.707107, /* 1/sqrt(3) */ 0.577350),
    float3 (/* sqrt(2/3) */ 0.816497, 0, /* 1/sqrt(3) */ 0.577350),
    float3 (/* -1/sqrt(6) */ -0.408248, /* -1/sqrt(2) */ -0.707107, /* 1/sqrt(3) */ 0.577350)
  };

  float3 coeff = float3 (saturate (dot (direction, base[0])),
    saturate (dot (direction, base[1])),
    saturate (dot (direction, base[2])));
  return coeff * coeff;
}

float3 ComputeDLMColor (float3 direction, float3 baseColor[3])
{
  float3 coeff = ComputeDLMCoeff (direction);
  return (coeff.x * baseColor[0]
    + coeff.y * baseColor[1]
    + coeff.z * baseColor[2]);
}

#endif // __DIRECTIONAL_LMS_CG_INC__
 
]]>
</include>
