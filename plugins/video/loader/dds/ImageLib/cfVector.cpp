/*

	cfVector.cpp - Float Codebook Vector routines

*/

#include "common.h"
#include "cfVector.h"
#include <string.h>
#include <math.h>

CS_PLUGIN_NAMESPACE_BEGIN(DDSImageIO)
{
namespace ImageLib
{

cfVector &cfVector::operator=(cfVector &Vect)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] = Vect.pData[i];

	return *this;
}

cfVector &cfVector::operator+=(cfVector &Vect)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] += Vect.pData[i];
	return *this;
}

cfVector &cfVector::operator-=(cfVector &Vect)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] -= Vect.pData[i];
	return *this;
}

cfVector &cfVector::operator*=(cfVector &Vect)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] *= Vect.pData[i];
	return *this;
}

cfVector &cfVector::operator/=(cfVector &Vect)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] /= Vect.pData[i];
	return *this;
}

cfVector &cfVector::operator*=(float f)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] *= f;
	return *this;
}

cfVector &cfVector::operator/=(float f)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] /= f;
	return *this;
}

cfVector &cfVector::operator=(float f)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] = f;
	return *this;
}

int cfVector::operator==(cfVector &Vect)
{
float *p1, *p2;

	p1 = pData;
	p2 = Vect.pData;

	for(int i=0; i<fCodeSize; i++)
	{
		union
		{
		  float f;
		  uint32 u;
		} u1, u2;
		u1.f = p1[i]; u2.f = p2[i];
		if(u1.u != u2.u)
			return 0;
	}

	return 1;
}

void cfVector::Sum(cfVector &Vect1, cfVector &Vect2)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] = Vect1[i] + Vect2[i];
}

void cfVector::Diff(cfVector &Vect1, cfVector &Vect2)
{
	for(int i=0; i<fCodeSize; i++)
		pData[i] = Vect1[i] - Vect2[i];
}


float cfVector::DiffMag(cfVector &Vect)
{
float	diff, mag;

	mag = 0.0f;
	for(int i=0; i<fCodeSize; i++)
	{
		diff = pData[i] - Vect.pData[i];
		mag += diff * diff;
	}

	return mag;
}

float cfVector::Distance(cfVector &Vect)
{
	return (float)sqrt(DiffMag(Vect));
}

void cfVector::Min(cfVector &Test1)
{
	for(int i=0; i<fCodeSize; i++)
	{
		if(Test1.pData[i] < pData[i])
			pData[i] = Test1.pData[i];
	}
}

void cfVector::Max(cfVector &Test1)
{
	for(int i=0; i<fCodeSize; i++)
	{
		if(Test1.pData[i] > pData[i])
			pData[i] = Test1.pData[i];
	}
}


void cfVector::MinMax(cfVector &Min, cfVector &Max)
{
	for(int i=0; i<fCodeSize; i++)
	{
		if(pData[i] > Max[i]) Max[i] = pData[i];
		if(pData[i] < Min[i]) Min[i] = pData[i];
	}
}

float cfVector::Mag(void)
{
float Val, Result = 0.0f;

	for(int i=0; i<fCodeSize; i++)
	{
		Val = pData[i];
		Result += Val * Val;
	}
	return Result;
}

float cfVector::InvMag(void)
{
float Val, Result = 0.0f;

	for(int i=0; i<fCodeSize; i++)
	{
		Val = 255.0f - pData[i];
		Result += Val * Val;
	}
	return Result;
}

} // end of namespace ImageLib
}
CS_PLUGIN_NAMESPACE_END(DDSImageIO)
