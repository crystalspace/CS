#include "cssysdef.h"

#include <stdio.h>
#include <string.h>

#include "csutil/scfstr.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "awsprefs.h"
#include "awstex.h"
#include "awsadler.h"
#include "awskcfct.h"
 
extern int awsparse(void *prefscont);

const bool DEBUG_KEYS = false;
const bool DEBUG_INIT = false;

/// The gradient step for the slightly darker and lighter versions of the highlight and shadow.
const unsigned char GRADIENT_STEP=25;

iFile *aws_fileinputvfs;

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


awsPrefManager::awsPrefManager(iBase *iParent):n_win_defs(0), n_skin_defs(0), def_skin(NULL), awstxtmgr(NULL), fontsvr(NULL), default_font(NULL), wmgr(NULL)
{
  SCF_CONSTRUCT_IBASE (iParent);
}

awsPrefManager::~awsPrefManager()
{
  SCF_DEC_REF (default_font);
  SCF_DEC_REF (fontsvr);
  SCF_DEC_REF (vfs);
  delete awstxtmgr;

  // empty the constants list
  int i;
  for(i=0; i<constants.Length(); ++i)
  {
    constant_entry *c = (constant_entry *)constants.Get(i);
    delete c;
  }
}

bool
awsPrefManager::Setup(iObjectRegistry *obj_reg)
{  
  if (DEBUG_INIT) printf("aws-debug: initializing AWS Texture Manager\n");

  if (DEBUG_INIT) printf("aws-debug: creating texture manager.\n"); 

  awstxtmgr = new awsTextureManager();
  if (!awstxtmgr)
      return false;

  if (DEBUG_INIT) printf("aws-debug: initing texture manager\n");
  
  awstxtmgr->Initialize(obj_reg);

  vfs=CS_QUERY_REGISTRY(obj_reg, iVFS);
  if (!vfs)
      return false;

  /* We setup the window's registered constants here, because windows are
   * special components, and do not obey the normal construction rules.
   */

  RegisterConstant("wfsNormal",0x0);
  RegisterConstant("wfsToolbar",0x1);
  RegisterConstant("wfsBitmap",0x2);
  RegisterConstant("wfoControl",0x1);
  RegisterConstant("wfoZoom",0x2);
  RegisterConstant("wfoMin",0x4);
  RegisterConstant("wfoClose",0x8);
  RegisterConstant("wfoTitle",0x10);
  RegisterConstant("wfoGrip",0x20);
  RegisterConstant("wfoRoundBorder",0x0);
  RegisterConstant("wfoBeveledBorder",0x40);

  return true;
}

unsigned long 
awsPrefManager::NameToId(char *n)
{

 if (n) {
    unsigned long id = aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)n, strlen(n));
    
    if (DEBUG_KEYS) printf("aws-debug: mapped %s to %lu\n", n, id);
    
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

iTextureHandle *
awsPrefManager::GetTexture(char *name, char *filename)
{

 if (awstxtmgr)
   return awstxtmgr->GetTexture(name, filename, false);
 else 
   return NULL;
 
}

iFont *
awsPrefManager::GetDefaultFont()
{
  return default_font;
}

iFont *
awsPrefManager::GetFont(char * /*filename*/)
{
  return NULL;
}

void 
awsPrefManager::SetTextureManager(iTextureManager *txtmgr)
{
  if (awstxtmgr)
   awstxtmgr->SetTextureManager(txtmgr);
}

void
awsPrefManager::SetFontServer(iFontServer *fntsvr)
{
  if (default_font)
    default_font->DecRef();

  if (fontsvr)
    fontsvr->DecRef();

  (fontsvr = fntsvr)->IncRef ();

  // kludge for the moment: this will eventually be more intelligent
  default_font = fontsvr->LoadFont(CSFONT_LARGE);
}

void 
awsPrefManager::SetWindowMgr(iAws *_wmgr)
{
  wmgr=_wmgr;
}

void
awsPrefManager::SetupPalette()
{
 printf("aws-debug: setting up global AWS palette...\n");
  
 unsigned char red, green, blue;
 iTextureManager* txtmgr = NULL;

 if (awstxtmgr)
   txtmgr = awstxtmgr->GetTextureManager();

 LookupRGBKey("HighlightColor", red, green, blue); 
 sys_colors[AC_HIGHLIGHT] = txtmgr->FindRGB(red,green,blue);

 // Create a slightly darker highlight
 sys_colors[AC_HIGHLIGHT2] = txtmgr->FindRGB((red   > GRADIENT_STEP ? red-GRADIENT_STEP   : 0),
                                             (green > GRADIENT_STEP ? green-GRADIENT_STEP : 0),
                                             (blue  > GRADIENT_STEP ? blue-GRADIENT_STEP  : 0));
                   
 LookupRGBKey("ShadowColor", red, green, blue); 
 sys_colors[AC_SHADOW] = txtmgr->FindRGB(red,green,blue);

 // Create a slightly lighter shadow
 sys_colors[AC_SHADOW2]    = txtmgr->FindRGB((255-red   > GRADIENT_STEP ? red+GRADIENT_STEP   : 255),
                                             (255-green > GRADIENT_STEP ? green+GRADIENT_STEP : 255),
                                             (255-blue  > GRADIENT_STEP ? blue+GRADIENT_STEP  : 255));
 
 LookupRGBKey("FillColor", red, green, blue); 
 sys_colors[AC_FILL] = txtmgr->FindRGB(red,green,blue);
  
 // Create a slightly darker fill
 sys_colors[AC_DARKFILL] = txtmgr->FindRGB((red   > GRADIENT_STEP ? red-GRADIENT_STEP   : 0),
                                           (green > GRADIENT_STEP ? green-GRADIENT_STEP : 0),
                                           (blue  > GRADIENT_STEP ? blue-GRADIENT_STEP  : 0));
 
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
 
 sys_colors[AC_BLACK] = txtmgr->FindRGB(0,0,0);
 sys_colors[AC_WHITE] = txtmgr->FindRGB(255,255,255);
 sys_colors[AC_RED] = txtmgr->FindRGB(128,0,0);
 sys_colors[AC_GREEN] = txtmgr->FindRGB(0,128,0);
 sys_colors[AC_BLUE] = txtmgr->FindRGB(0,0,128);
  
 printf("aws-debug: finished palette setup.\n"); 
}

bool 
awsPrefManager::Load(const char *def_file)
{

  if (wmgr==NULL) 
  {
    printf("\tunable to load definitions because of an internal error: no window manager.\n");
    return false;
  }

  printf("\tloading definitions file %s...\n", def_file);

  aws_fileinputvfs = vfs->Open(def_file, VFS_FILE_READ);
  if (!aws_fileinputvfs)
      return false;
  
  unsigned int ncw = n_win_defs,
               ncs = n_skin_defs;

  if(awsparse(wmgr)) {
      printf("\tsyntax error in definition file, load failed.\n");
      return false;
  }
  
  printf("\tload successful (%i windows, %i skins loaded.)\n", n_win_defs-ncw,
	  n_skin_defs-ncs);
  
  SCF_DEC_REF(aws_fileinputvfs);
  aws_fileinputvfs = NULL;

  return true;
}

bool
awsPrefManager::SelectDefaultSkin(char *skin_name)
{
  awsSkinNode *skin = (awsSkinNode *)skin_defs.GetFirstItem();
  unsigned long id  = NameToId(skin_name);

  while (skin)
  {
    if (skin->Name() == id) {
      def_skin=skin;

      // Set the AWS global palette
      SetupPalette();
  
      // Get the default textures into the texture manager.
	  int i;
      for(i=0; i<def_skin->Length(); ++i)
      {
        awsKey *k = def_skin->GetAt(i);

        if (k->Type() == KEY_STR)
        {
          awsStringKey *sk = (awsStringKey *)(k);
          
          if (awstxtmgr) 
            (void)awstxtmgr->GetTexturebyID(sk->Name(), sk->Value()->GetData(), true);
        }
      }

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
awsPrefManager::LookupPointKey(char *name, csPoint &val)
{
    return LookupPointKey(NameToId(name),val);
}


bool
awsPrefManager::LookupPointKey(unsigned long id, csPoint &val)
{
    awsKey *k = ((awsKeyContainer *)def_skin)->Find(id);

    if (k)
    {
      if (k->Type() == KEY_POINT) 
      {
        val = ((awsPointKey *)k)->Value();
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
   
   if (DEBUG_KEYS) printf("aws-debug: Getting \"%s\" from %p\n", name, node);
   
   awsKey *k = ((awsKeyContainer *)node)->Find(NameToId(name));

   if (DEBUG_KEYS) printf("aws-debug: Node retrieved: %p [%s]\n", node, name);
   
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
  awsComponentNode *win = (awsComponentNode *) win_defs.GetFirstItem();
  unsigned long id = NameToId(name);
  
  while (win)
  {
    if (win && win->Name() == id)
      return win;
    
    win=(awsComponentNode *) win_defs.GetNextItem();
  }
  
  return NULL;
}

void 
awsPrefManager::RegisterConstant(char *name, int value)
{
  constant_entry *c = new constant_entry;

  c->name = NameToId(name);
  c->value = value;

  constants.Push(c);
}

int
awsPrefManager::GetConstantValue(char *name)
{
  unsigned int namev = NameToId(name);

  int i;
  for(i=0; i<constants.Length(); ++i)
  {
    constant_entry *c = (constant_entry *)constants.Get(i);

    if (c->name == namev)
      return c->value;
  }

  return 0;
}

bool 
awsPrefManager::ConstantExists(char *name)
{
  unsigned int namev = NameToId(name);

  int i;
  for(i=0; i<constants.Length(); ++i)
  {
    constant_entry *c = (constant_entry *)constants.Get(i);

    if (c->name == namev)
      return true;
  }

  return false;
}

iAwsKeyFactory *
awsPrefManager::CreateKeyFactory()
{
  return new awsKeyFactory();
}
