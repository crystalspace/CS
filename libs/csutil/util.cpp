/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define CS_SYSDEF_PROVIDE_PATH
#define CS_SYSDEF_PROVIDE_GETCWD
#include "cssysdef.h"

// return a random number from minRange to maxRange inclusive
long csRndNum(long minRange, long maxRange)
{
  if (minRange > maxRange)
  {
    long sw = minRange;
    minRange = maxRange;
    maxRange = sw;
  }
  long range = maxRange - minRange + 1;
  return minRange + (long (rand ()) % range);
}

char *csStrNew (const char *s)
{
  if (s)
  {
    size_t sl = strlen (s) + 1;
    char *r = new char [sl];
    memcpy (r, s, sl);
    return r;
  }
  else
    return NULL;
}

static bool RecursivecsCombinations (int *vector, int top, int mask, int m, int n,
  bool (*callback) (int *vector, int count, void *arg), void *arg)
{
  for (int i = 0; i < m; i++)
  {
    if (mask & (1 << i))
      continue;
    vector [top] = i;
    if (top + 1 >= n)
      if (callback (vector, n, arg))
        return true;
      else
        ;
    else if (RecursivecsCombinations (vector, top + 1, mask | (1 << i),
                                    m, n, callback, arg))
      return true;
  } /* endfor */
  return false;
}

void csCombinations (int m, int n, bool (*callback) (int *vector, int count,
  void *arg), void *arg)
{
  int *vector = new int [m];
  RecursivecsCombinations (vector, 0, 0, m, n, callback, arg);
  delete [] vector;
}

#ifdef COMP_BC
static int __getcwd (char drive, char *buffer, int buffersize) {
  _getdcwd(drive, buffer, buffersize);
  return strlen(buffer);
}
#endif

#if defined (OS_OS2) || defined (OS_WIN32) && !defined(COMP_BC)
// We need a function to retrieve current working directory on specific drive

static int __getcwd (char drive, char *buffer, int buffersize)
{
  char old_drive = _getdrive ();
  _chdrive (drive);
  getcwd (buffer, buffersize);
  _chdrive (old_drive);
  return strlen (buffer);
}

#endif // defined (OS_OS2) || defined (OS_WIN32)

#if defined (OS_DOS)
// We need a function to retrieve current working directory on specific drive

static int __getcwd (char drive, char *buffer, int buffersize)
{
  unsigned int old_drive, num_drives;
  _dos_getdrive (&old_drive);
  _dos_setdrive (drive, &num_drives);
  getcwd (buffer, buffersize);
  _dos_setdrive (old_drive, &num_drives);
  return strlen (buffer);
}

#endif // defined (OS_DOS)

char *csExpandName (const char *iName)
{
  char outname [MAXPATHLEN + 1];
  int inp = 0, outp = 0, namelen = strlen (iName);
  while ((outp < MAXPATHLEN)
      && (inp < namelen))
  {
    char tmp [MAXPATHLEN + 1];
    int ptmp = 0;
    while ((inp < namelen)
        && (iName [inp] != '/')
        && (iName [inp] != PATH_SEPARATOR)
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
        && (iName [inp] != ':')
#endif
          )
      tmp [ptmp++] = iName [inp++];
    tmp [ptmp] = 0;

    if ((ptmp > 0)
     && (outp == 0)
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
     && ((inp >= namelen)
      || (iName [inp] != ':'))
#endif
        )
    {
      getcwd (outname, sizeof (outname));
      outp = strlen (outname);
      if (strcmp (tmp, "."))
        outname [outp++] = PATH_SEPARATOR;
    } /* endif */

#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
    // If path starts with '/' (root), get current drive
    if ((ptmp == 0)
     && (outp == 0))
    {
      getcwd (outname, sizeof (outname));
      if (outname [1] == ':')
        outp = 2;
    } /* endif */
#endif

    if (strcmp (tmp, "..") == 0)
    {
      while ((outp > 0)
          && ((outname [outp - 1] == '/')
           || (outname [outp - 1] == PATH_SEPARATOR)
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
           || (outname [outp - 1] == ':')
#endif
             )
            )
        outp--;
      while ((outp > 0)
          && (outname [outp - 1] != '/')
          && (outname [outp - 1] != PATH_SEPARATOR)
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
          && (outname [outp - 1] != ':')
#endif
            )
        outp--;
    }
    else if (strcmp (tmp, ".") == 0)
    {
      // do nothing
    }
    else if (strcmp (tmp, "~") == 0)
    {
      // strip all output path; start from scratch
      strcpy (outname, "~/");
      outp = 2;
    }
    else
    {
      memcpy (&outname [outp], tmp, ptmp);
      outp += ptmp;
      if (inp < namelen)
      {
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
        if ((inp == 1)
         && (iName [inp] == ':'))
          if ((iName [inp + 1] == '/')
           || (iName [inp + 1] == PATH_SEPARATOR))
            outname [outp++] = ':';
          else
            outp += __getcwd (iName [inp - 1], outname + outp - 1, sizeof (outname) - outp + 1) - 1;
        if ((outname [outp - 1] != '/')
         && (outname [outp - 1] != PATH_SEPARATOR))
#endif
        outname [outp++] = PATH_SEPARATOR;
      }
    } /* endif */
    while ((inp < namelen)
        && ((iName [inp] == '/')
         || (iName [inp] == PATH_SEPARATOR)
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
         || (iName [inp] == ':')
#endif
           )
          )
      inp++;
  } /* endwhile */

  char *ret = new char [outp + 1];
  memcpy (ret, outname, outp);
  ret [outp] = 0;
  return ret;
}

void csSplitPath (const char *iPathName, char *oPath, size_t iPathSize,
  char *oName, size_t iNameSize)
{
  size_t sl = strlen (iPathName);
  size_t maxl = sl;
  while (sl
      && (iPathName [sl - 1] != '/')
      && (iPathName [sl - 1] != PATH_SEPARATOR)
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
      && (iPathName [sl - 1] != ':')
#endif
        )
    sl--;

  if (iPathSize)
    if (sl >= iPathSize)
    {
      memcpy (oPath, iPathName, iPathSize - 1);
      oPath [iPathSize - 1] = 0;
    }
    else
    {
      memcpy (oPath, iPathName, sl);
      oPath [sl] = 0;
    }

  if (iNameSize)
    if (maxl - sl >= iNameSize)
    {
      memcpy (oName, &iPathName [sl], iNameSize - 1);
      oName [iNameSize - 1] = 0;
    }
    else
      memcpy (oName, &iPathName [sl], maxl - sl + 1);
}

bool csGlobMatches (const char *fName, const char *fMask)
{
  while (*fName || *fMask)
  {
    if (*fMask == '*')
    {
      while (*fMask == '*')
        fMask++;
      if (!*fMask)
        return true;		// mask = "something*"
      while (*fName && *fName != *fMask)
        fName++;
      if (!*fName)
        return false;		// "*C" - fName does not contain C
    }
    else
    {
      if ((*fMask != '?')
       && (*fName != *fMask))
        return false;
      if (*fMask)
        fMask++;
      if (*fName)
        fName++;
    }
  }
  return (!*fName) && (!*fMask);
}

/*------------------------------------------------------------------------------
  Byte swap 32 bit data buffer
------------------------------------------------------------------------------*/
void csByteSwap32bitBuffer( register unsigned long* place, register unsigned long count )
{
  register unsigned long value;

  for ( ; count > 0; --count, ++place) {
    value = *place;
    *place = ((value >> 24 ) & 0x000000FF ) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | (( value << 24) & 0xFF000000);
  }
}

/*------------------------------------------------------------------------------
	Byte swap 16 bit data in place.
------------------------------------------------------------------------------*/
void csByteSwap16bitBuffer( register unsigned short* place, register unsigned long count )
{
  register unsigned short value;

  for ( ; count > 0; --count, ++place) {
    value = *place;
    *place = (( value >> 8 ) & 0x000000FF ) | (( value << 8 ) & 0x0000FF00 );
  }
}

// finds the smallest number that is a power of two and is larger or equal to n
int csFindNearestPowerOf2 (int n)
{
  int w=1;

  while (n > w)  w <<= 1;

  return w;
}

// returns true if n is a power of two
bool csIsPowerOf2 (int n)
{
  if (n <= 0)
    return false; 
  return !(n & (n - 1));	// (n-1) ^ n >= n;
}

/**
 * given src and dest, which are already allocated, copy source to dest.
 * But, do not copy 'search', instead replace that with 'replace' string.
 * max is size of dest
*/
void csFindReplace(char *dest, const char *src, const char *search,
  const char *replace, int max)
{
  char *found = 0;
  const char *srcpos = src;
  char *destpos = dest;
  int searchlen = strlen(search);
  int replacelen = strlen(replace);
  destpos[0] = 0;
  int sizeleft = max;
  /// find and replace occurrences
  while( (found=strstr(srcpos, search)) != 0 )
  {
    // copy string before it
    int beforelen = found - srcpos;
    sizeleft -= beforelen;
    if(sizeleft <= 0) { destpos[0]=0; return; }
    strncpy(destpos, srcpos, beforelen);
    destpos += beforelen; 
    srcpos += beforelen;
    destpos[0]=0;
    // add replacement
    sizeleft -= replacelen;
    if(sizeleft <= 0) { destpos[0]=0; return; }
    strcpy(destpos, replace); 	
    destpos += replacelen;
    // skip replaced string
    srcpos += searchlen; 			
  }
  // add remainder of string
  int todo = strlen(srcpos);
  sizeleft -= todo;
  if(sizeleft <= 0) { destpos[0]=0; return; }
  strcpy(destpos, srcpos); 	
  destpos += todo;
  destpos[0]=0;
}
