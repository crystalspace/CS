#include "cssysdef.h"
#include "csutil/scfstr.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "awsprefs.h"
#include <stdio.h>
#include <string.h>
 
extern int awsparse(void *prefscont);
extern FILE *awsin;
unsigned long aws_adler32(unsigned long adler,  const unsigned char *buf,  unsigned int len);


const bool DEBUG_KEYS = false;
const bool DEBUG_INIT = true;

/***************************************************************************************************************
*   This constructor converts the text-based name into a fairly unique numeric ID.  The ID's are then used for *
* comparison.  The method of text-to-id mapping may be somewhat slower than a counter, but it does not have to *
* worry about wrapping or collisions or running out during long execution cycles.                              *
***************************************************************************************************************/
awsKey::awsKey(iString *n)
{
  if (n) {
    name = aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)n->GetData(), n->Length());

    if (DEBUG_KEYS) printf("aws-debug: new key %s mapped to %lu\n", n->GetData(), name);

    n->DecRef();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////


awsPrefManager::awsPrefManager(iBase *iParent):n_win_defs(0), n_skin_defs(0), def_skin(NULL), awstxtmgr(NULL)
{
  SCF_CONSTRUCT_IBASE (iParent);
}

awsPrefManager::~awsPrefManager()
{
}

void 
awsPrefManager::Setup(iObjectRegistry *obj_reg)
{  
  if (DEBUG_INIT) printf("aws-debug: Initializing AWS Texture Manager\n");

  awstxtmgr = new awsTextureManager();
  awstxtmgr->Initialize(obj_reg);   
}

unsigned long 
awsPrefManager::NameToId(char *n)
{
 if (n) {
    unsigned long id = aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)n, strlen(n));
    
    printf("aws-debug: mapped %s to %lu\n", n, id);
    
    return id;
 }
 else 
    return 0;
}

void
awsPrefManager::SetColor(int index, int color)
{
  sys_colors[index] = color;  
}

int 
awsPrefManager::GetColor(int index)
{
  return sys_colors[index]; 
}

void
awsPrefManager::SetupPalette(iGraphics3D *g3d)
{
 printf("aws-debug: setting up global AWS palette...\n");
  
 unsigned char red, green, blue;
 iTextureManager* txtmgr = g3d->GetTextureManager();

 LookupRGBKey("HighlightColor", red, green, blue); 
 sys_colors[AC_HIGHLIGHT] = txtmgr->FindRGB(red,green,blue);
 
 LookupRGBKey("ShadowColor", red, green, blue); 
 sys_colors[AC_SHADOW] = txtmgr->FindRGB(red,green,blue);
 
 LookupRGBKey("FillColor", red, green, blue); 
 sys_colors[AC_FILL] = txtmgr->FindRGB(red,green,blue);
 
 LookupRGBKey("TextForeColor", red, green, blue); 
 sys_colors[AC_TEXTFORE] = txtmgr->FindRGB(red,green,blue);
 
 LookupRGBKey("TextBackColor", red, green, blue); 
 sys_colors[AC_TEXTBACK] = txtmgr->FindRGB(red,green,blue);
 
 LookupRGBKey("TextDisabledColor", red, green, blue); 
 sys_colors[AC_TEXTDISABLED] = txtmgr->FindRGB(red,green,blue);
 
 LookupRGBKey("ButtonTextColor", red, green, blue); 
 sys_colors[AC_BUTTONTEXT] = txtmgr->FindRGB(red,green,blue);
 
 if (LookupRGBKey("TransparentColor", red, green, blue)) 
   sys_colors[AC_TRANSPARENT] = txtmgr->FindRGB(red,green,blue);
 else
   sys_colors[AC_TRANSPARENT] = txtmgr->FindRGB(255,0,255); 
  
 printf("aws-debug: finished palette setup.\n"); 
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

 do 
 {
    if (skin->Name() == id) {
      def_skin=skin;
      return true;
    }

    skin = (awsSkinNode *)skin_defs.GetNextItem();
 } while(skin!=(awsSkinNode *)skin_defs.PeekFirstItem());

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
awsPrefManager::LookupRGBKey(char *name, unsigned char &red, unsigned char &green, unsigned char &blue)
{
  return LookupRGBKey(NameToId(name), red, green, blue);
}   

bool 
awsPrefManager::LookupRGBKey(unsigned long id, unsigned char &red, unsigned char &green, unsigned char &blue)
{
  
   awsKey *k = ((awsKeyContainer *)def_skin)->Find(id);
  
    if (k)
    {
      if (k->Type() == KEY_RGB) 
      {
	awsRGBKey::RGB rgb;
        rgb = ((awsRGBKey *)k)->Value();
	
	red=rgb.red;
	green=rgb.green;
	blue=rgb.blue;
	
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
   
   printf("aws-debug: Getting \"%s\" from %x\n", name, node);
   
   awsKey *k = ((awsKeyContainer *)node)->Find(NameToId(name));

   printf("aws-debug: Node retrieved.\n", name, node);
   
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
  
  do 
  {
    awsComponentNode *win = (awsComponentNode *)p;
    
    if (win->Name() == id)
      return win;
    else
      p=win_defs.GetNextItem();
    
  } while(p!=win_defs.PeekFirstItem());
  
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

