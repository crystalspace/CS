/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#include "util/ddgutil.h"
// ----------------------------------------------------------------------
//
// ddgUtil:
//
//  See if we are running on a Pentium III.
//
bool ddgUtil::DetectSIMD(void)
{
bool found_simd = false;
#if defined(WIN32) && !defined(__CRYSTAL_SPACE__)
_asm
{
pushfd
pop eax // get EFLAGS into eax
mov ebx,eax // keep a copy
xor eax,0x200000 
// toggle CPUID bit

push eax
popfd // set new EFLAGS
pushfd
pop eax // EFLAGS back into eax

xor eax,ebx 
// have we changed the ID bit?

je NO_SIMD 
// No, no CPUID instruction

// we could toggle the 
// ID bit so CPUID is present
mov eax,1

cpuid // get processor features
test edx,1<<25 // check the SIMD bit
jz NO_SIMD
mov found_simd,1 
jmp DONE
NO_SIMD:
mov found_simd,0
DONE:
}
#endif
return found_simd;
}

// ----------------------------------------------------------------------
float ddgAngle::_cosTable[180*ddgAngle_res+1];
float ddgAngle::_acosTable[180*ddgAngle_res+1];

ddgAngle ddg_init;	// Ensure table gets initialized.

ddgAngle::ddgAngle()
{
	unsigned int i;
	// Calculate from 0.0 to 180.0 degrees.
	for (i = 0; i <= 180*ddgAngle_res; i++)
	{
		_cosTable[i] = ::cos(M_PI*(float)i/(float)(ddgAngle_res*180));
	}
	_cosTable[90*ddgAngle_res] = 0;
	for (i = 0; i <= 180.0*ddgAngle_res; i++)
	{
		_acosTable[i] = ddgAngle::radtodeg(::acos((float)i/(180.0*ddgAngle_res)));
	}
	_acosTable[90*ddgAngle_res] = 1;
}

// This should be inlined, but W32 wont let me use _cosTable in a DLL
// if it is inlined.
float ddgAngle::cos( float angle)
{
	float a = mod(angle);
	return a < 180.0f ?
		_cosTable[((int)a*ddgAngle_res)]:
		-_cosTable[((int)(a-180.0f)*ddgAngle_res)];
}

float ddgAngle::acos( float value)
{
	int i = (int)(value*180*ddgAngle_res), k;
	k = i;
	if (k < 0) k = -1 *k;
	if (k > 2880) k = 2880 - k;
	float angle = _acosTable[k];
	if (value < 0.0f)
		angle += 180;
//	if (i > 2880)
//		angle = 360.0 - angle;
	return angle;
}

static unsigned int ddgFileHelper_dataDirLen = 0;
static char *ddgFileHelper_dataDir = NULL;
/// Define the data directory;
void ddgFileHelper::setDataDirectory( const char *dataDir )
{
	if (ddgFileHelper_dataDir)
		delete [] ddgFileHelper_dataDir;
	ddgFileHelper_dataDirLen = 0;
	unsigned l = 0;
	while(dataDir[l] != 0) l++;
	ddgFileHelper_dataDir = new char[l+1];
	ddgFileHelper_dataDirLen = l;
	ddgFileHelper_dataDir[l] = 0;
	while (l)
	{
		l--;
		ddgFileHelper_dataDir[l] = dataDir[l];
	}
}
/// Get full filename.
/// The caller doesnt have to free this string we reuse it each time.
static char *ddg_FullFileNameStr = NULL;
char *ddgFileHelper::getFullFileName( const char *filename )
{
	unsigned l = 0, l2 = 0;
	while(filename[l] != 0) l++;
	if (ddg_FullFileNameStr)
	{
		delete [] ddg_FullFileNameStr;
	}
	ddg_FullFileNameStr = new char[ddgFileHelper_dataDirLen+l+1];

	while (l2 < ddgFileHelper_dataDirLen+l)
	{
		ddg_FullFileNameStr[l2] = (l2 < ddgFileHelper_dataDirLen)
			? ddgFileHelper_dataDir[l2]: filename[l2-ddgFileHelper_dataDirLen];
		l2++;
	} 

	ddg_FullFileNameStr[l2] = 0;

	return ddg_FullFileNameStr;
}

void ddgFileHelper::clear(void)
{
	delete [] ddg_FullFileNameStr;
	ddg_FullFileNameStr = 0;
	delete [] ddgFileHelper_dataDir;
	ddgFileHelper_dataDir = 0;
	ddgFileHelper_dataDirLen = 0;
}

int ddgConsole::_last = -1;
void ddgConsole::progress( char *s, int p, int total)
{
#ifdef DDG
	if (100*p/total != _last)
	{
		_last = 100*p/total;
		cerr << '\r' << s << ": " << _last << "%";
	}
#else
	(void)s; (void)p; (void)total;
#endif
}
void ddgConsole::end(void)
{
#ifdef DDG
	cerr << endl;
	_last = -1;
#endif
}
void ddgConsole::s( char *s )
{
#ifdef DDG
	cerr << s;
#else
	(void)s;
#endif
}
/// Send an integer to the console.
void ddgConsole::i( int i )
{
#ifdef DDG
	cerr << i;
#else
	(void)i;
#endif
}
/// Send an float to the console.
void ddgConsole::f( float f )
{
#ifdef DDG
	cerr << f;
#else
	(void)f;
#endif
}
