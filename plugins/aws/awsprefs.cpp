#include "cssysdef.h"
#include "csutil/scfstr.h"
#include "awsprefs.h"
#include <stdio.h>
#include <string.h>
 
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
    n->DecRef();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////


awsPrefManager::awsPrefManager(iBase *iParent):n_win_defs(0), n_skin_defs(0), def_skin(0)
{
  SCF_CONSTRUCT_IBASE (iParent);
}

awsPrefManager::~awsPrefManager()
{
}

unsigned long 
awsPrefManager::NameToId(char *n)
{
 if (n) {
    return aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)n, strlen(n));
 }
 else 
    return 0;
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

bool
awsPrefManager::SelectDefaultSkin(char *skin_name)
{
 awsSkinNode *skin = (awsSkinNode *)skin_defs.GetFirstItem();
 unsigned long id  = NameToId(skin_name);

 while(skin)
 {
    if (skin->Name() == id) {
      def_skin=skin;
      return true;
    }

    skin = (awsSkinNode *)skin_defs.GetNextItem();
}

 return false;
}

bool
awsPrefManager::LookupIntKey(char *name, int &val)
{
   return LookupIntKey(NameToId(name),val);
}

bool
awsPrefManager::LookupIntKey(unsigned long id, int &val)
{
   awsKey *k = ((awsKeyContainer *)def_skin)->Find(id);

   if (k)
   {
     if (k->Type() == KEY_INT) 
     {
       val = ((awsIntKey *)k)->Value();
       return true;
     }
   }
    
   return false;
}

bool
awsPrefManager::LookupStringKey(char *name, iString *&val)
{
   return LookupStringKey(NameToId(name),val);
}

bool
awsPrefManager::LookupStringKey(unsigned long id, iString *&val)
{
    awsKey *k = ((awsKeyContainer *)def_skin)->Find(id);

    if (k)
    {
      if (k->Type() == KEY_STR) 
      {
        val = ((awsStringKey *)k)->Value();
        return true;
      }
    }

    return false;
}

bool
awsPrefManager::LookupRectKey(char *name, csRect &val)
{
    return LookupRectKey(NameToId(name),val);
}


bool
awsPrefManager::LookupRectKey(unsigned long id, csRect &val)
{
    awsKey *k = ((awsKeyContainer *)def_skin)->Find(id);

    if (k)
    {
      if (k->Type() == KEY_RECT) 
      {
        val = ((awsRectKey *)k)->Value();
        return true;
      }
    }

    return false;
}

bool
awsPrefManager::GetInt(awsComponentNode *node, char *name, int &val)
{
    if (!node) return false;

    awsKey *k = ((awsKeyContainer *)node)->Find(NameToId(name));

    if (k)
    {
      if (k->Type() == KEY_INT) 
      {
        val = ((awsIntKey *)k)->Value();
        return true;
      }
    }

    return false;
}

bool 
awsPrefManager::GetRect(awsComponentNode *node, char *name, csRect &val)
{
   if (!node) return false;
   awsKey *k = ((awsKeyContainer *)node)->Find(NameToId(name));

    if (k)
    {
       if (k->Type() == KEY_RECT) 
       {
          val = ((awsRectKey *)k)->Value();
          return true;
       }
    }

  return false;

}

bool 
awsPrefManager::GetString(awsComponentNode *node, char *name, iString *&val)
{
    if (!node) return false;

    awsKey *k = ((awsKeyContainer *)node)->Find(NameToId(name));

    if (k)
    {
       if (k->Type() == KEY_STR) 
       {
          val = ((awsStringKey *)k)->Value();
          return true;
       }
    }

  return false;
}

awsComponentNode *
awsPrefManager::FindWindowDef(char *name)
{
  void *p = win_defs.GetFirstItem();
  unsigned long id = NameToId(name);
  
  while(p)
  {
    awsComponentNode *win = (awsComponentNode *)p;
    
    if (win->Name() == id)
      return win;
    else
      p=win_defs.GetNextItem();
    
  }
  
  return NULL;
 
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

