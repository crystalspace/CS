/*
    Copyright (C) 1998 by Ayal Zwi Pinkus
  
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
 
#include <math.h>
#include "sysdef.h"
#include "csengine/bezier.h"

//////////////////////////////////////////////
//////////////////////////////////////////////
// Cache interface
//////////////////////////////////////////////
//////////////////////////////////////////////
#define BEZ_NR1 4
#define BEZ_NR2 9
#define BEZ_NR3 16
#define BEZ_NR4 25
#define BEZ_NR5 36
#define BEZ_NR6 49
#define BEZ_NR7 64
#define BEZ_NR8 81
#define BEZ_NR9 100

#define BEZ_IND1 0
#define BEZ_IND2 (BEZ_IND1+BEZ_NR1)
#define BEZ_IND3 (BEZ_IND2+BEZ_NR2)
#define BEZ_IND4 (BEZ_IND3+BEZ_NR3)
#define BEZ_IND5 (BEZ_IND4+BEZ_NR4)
#define BEZ_IND6 (BEZ_IND5+BEZ_NR5)
#define BEZ_IND7 (BEZ_IND6+BEZ_NR6)
#define BEZ_IND8 (BEZ_IND7+BEZ_NR7)
#define BEZ_IND9 (BEZ_IND8+BEZ_NR8)
#define BEZ_IND_ (BEZ_IND9+BEZ_NR9)

#define BEZ_OFFS1 (BEZ_IND1*9)
#define BEZ_OFFS2 (BEZ_IND2*9)
#define BEZ_OFFS3 (BEZ_IND3*9)
#define BEZ_OFFS4 (BEZ_IND4*9)
#define BEZ_OFFS5 (BEZ_IND5*9)
#define BEZ_OFFS6 (BEZ_IND6*9)
#define BEZ_OFFS7 (BEZ_IND7*9)
#define BEZ_OFFS8 (BEZ_IND8*9)
#define BEZ_OFFS9 (BEZ_IND9*9)
#define BEZ_OFFS_ (BEZ_IND_*9)

#define BEZ_LUT_SIZE  BEZ_OFFS_ // Doubles

// This should be approx. less than 82K
TDtDouble bfactmap[BEZ_LUT_SIZE];
TDtDouble bfactdumap[BEZ_LUT_SIZE];
TDtDouble bfactdvmap[BEZ_LUT_SIZE];

TDtInt bez_offsets[10] = 
{
  BEZ_OFFS1,
  BEZ_OFFS2,
  BEZ_OFFS3,
  BEZ_OFFS4,
  BEZ_OFFS5,
  BEZ_OFFS6,
  BEZ_OFFS7,
  BEZ_OFFS8,
  BEZ_OFFS9,
  BEZ_OFFS_,
};


// Generate the Binomials
static TDtDouble bez3bin[3] = {1,2,1};

TDtDouble bfact(TDtDouble u, TDtInt j, TDtDouble v, TDtInt k)
{
  return bez3bin[j]*bez3bin[k] *pow(u,j)*pow(1-u,2-j)*pow(v,k)*pow(1-v,2-k);
}

static TDtDouble bfact_du(TDtDouble u, TDtInt j, TDtDouble v, TDtInt k)
{
  TDtDouble left=0;
  TDtDouble right=0;
  if (j!=0)
  {
    left = j*pow(u,j-1)*pow(1-u,2-j);
  }
  if (j != 2)
  {
    right = pow(u,j)*(2-j)*pow(1-u,2-j-1);
  }
  return bez3bin[j]*bez3bin[k] *pow(v,k)*pow(1-v,2-k) *( left - right );
}


static TDtDouble bfact_dv(TDtDouble u, TDtInt j, TDtDouble v, TDtInt k)
{
  TDtDouble left=0;
  TDtDouble right=0;

  if (k!=0)
  {
    left = k*pow(v,k-1)*pow(1-v,2-k);
  }
  if (k != 2)
  {
    right = pow(v,k)*(2-k)*pow(1-v,2-k-1);
  }
  return bez3bin[j]*bez3bin[k] *pow(u,j)*pow(1-u,2-j) *( left - right );
}




// Call once!!!
void BuildBezierLuts()
{
  TDtInt res;
  TDtInt index=0;
  for (res=1;res<=9;res++)
  {
    // Test code
    //TODO remove ? TDtInt indexshouldbe = bez_offsets[res-1];
    
    TDtInt i,j,k,l;
    for (i=0;i<=res;i++)
    for (j=0;j<=res;j++)
    for (k=0;k<3;k++)
    for (l=0;l<3;l++)
    {
      TDtDouble u=(1.0*i)/res, v=(1.0*j)/res;
      bfactmap[index] = bfact(u,k,v,l);
      bfactdumap[index] = bfact_du(u,k,v,l);
      bfactdvmap[index] = bfact_dv(u,k,v,l);
      index++;
    }
  }
}



TDtDouble *BinomiumMap()
{
  return bfactmap;
}


void CrossProduct(TDtDouble *aDestination, TDtDouble* aSource1, TDtDouble* aSource2)
{
  aDestination[0] =   aSource1[1] * aSource2[2] - aSource1[2] * aSource2[1];
  aDestination[1] = -(aSource1[0] * aSource2[2] - aSource1[2] * aSource2[0]);
  aDestination[2] =   aSource1[0] * aSource2[1] - aSource1[1] * aSource2[0];
}
TDtDouble InProduct(TDtDouble* aSource1, TDtDouble* aSource2)
{
  TDtDouble result=0;
  result += aSource1[0] * aSource2[0];
  result += aSource1[1] * aSource2[1];
  result += aSource1[2] * aSource2[2];
  return result;
}
void Normalize(TDtDouble* aVertex)
{
  TDtDouble len;
  len = sqrt(InProduct(aVertex,aVertex));
  if (fabs(len)<0.00001)
    return;
  aVertex[0] /= len;
  aVertex[1] /= len;
  aVertex[2] /= len;
}  


/* Formula:                      /2\  /2\
// vtx = sum(j=0->2) sum(k=0->2) \j/  \k/ u^j(1-u)^(2-j) v^k(1-v)^(2-k) *P_jk
*/
void BezierNormal(TDtDouble *aResult, TDtDouble** aControls, TDtInt iu, TDtInt iv,TDtInt resolution)
{
  TDtDouble ddu[5], ddv[5];
  BezierPoint(ddu,aControls,iu,iv,resolution,bfactdumap);
  BezierPoint(ddv,aControls,iu,iv,resolution,bfactdvmap);
  CrossProduct(aResult,ddu,ddv);
  Normalize(aResult);
}

void BezierPoint(TDtDouble *aResult, TDtDouble** aControls, TDtInt iu, TDtInt iv, TDtInt resolution, 
                          TDtDouble *map )
{
  TDtInt j,k;
  TDtDouble *localmap = &map[bez_offsets[resolution-1] + 9*(iu + (resolution+1)*iv) ];

  aResult[0] = aResult[1] = aResult[2] = aResult[3] = aResult[4] = 0;

  for (j=0;j<3;j++)
  {
    for (k=0;k<3;k++)
    {
      TDtInt ctrlindex = j+3*k;
      TDtDouble *ctrl = aControls[ ctrlindex ]; 
      TDtDouble fact = localmap[ctrlindex];
      aResult[0] += ctrl[0] * fact;
      aResult[1] += ctrl[1] * fact;
      aResult[2] += ctrl[2] * fact;
      aResult[3] += ctrl[3] * fact;
      aResult[4] += ctrl[4] * fact;
    }
  }
}




void BezierPoint(double *aResult, double** aControls, double u, double v,
                          double (*func)(double, int, double, int) )

{
  aResult[0] = aResult[1] = aResult[2] = 0;
  int j,k;
  for (j=0;j<3;j++)
  {
    for (k=0;k<3;k++)
    {
      int l;
      double *ctrl = aControls[j+3*k];
      for (l=0;l<3;l++)
      {
        aResult[l] += ctrl[l] * func(u, j, v, k);
      }
    }
  }
}


void BezierNormal(double *aResult, double** aControls, double u, double v)
{
  double ddu[3], ddv[3];
  BezierPoint(ddu,aControls,u,v,bfact_du);
  BezierPoint(ddv,aControls,u,v,bfact_dv);
  CrossProduct(aResult,ddu,ddv);
  Normalize(aResult);
}
