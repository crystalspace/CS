/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "awskcfct.h"
#include "awsparser.h"
#include "awsprefs.h"
#include "awstex.h"

extern int awsparse (void *prefscont);

const bool DEBUG_KEYS = false;
const bool DEBUG_INIT = false;

// The gradient step for the slightly darker and lighter versions of the
// highlight and shadow.
const unsigned char GRADIENT_STEP = 25;

unsigned long awsKey::ComputeKeyID (const char* n) const
{
  CS_ASSERT (n != 0);
  unsigned long const x = strset->Request(n);
  if (DEBUG_KEYS)
    printf ("aws-debug: new key %s mapped to %lu\n", n, x);
  return x;
}

//////////////////////////////////////////////////////////////////////////////
awsPrefManager::awsPrefManager (iBase *iParent) :
  def_skin(0),
  awstxtmgr(0),
  wmgr(0),
  objreg(0)
{
  SCF_CONSTRUCT_IBASE (iParent);
}

awsPrefManager::~awsPrefManager ()
{
  delete awstxtmgr;
  SCF_DESTRUCT_IBASE();
}

bool awsPrefManager::Setup (iObjectRegistry *obj_reg)
{
  if (DEBUG_INIT) printf ("aws-debug: initializing AWS Texture Manager\n");
  if (DEBUG_INIT) printf ("aws-debug: creating texture manager.\n");

  g2d = CS_QUERY_REGISTRY (obj_reg, iGraphics2D);
  if (!g2d)
  {
    printf ("aws-debug: Couldn't find iGraphics2D plugin!!\n");
    return false;
  }
  awstxtmgr = new awsTextureManager ();
  if (!awstxtmgr) return false;
  if (DEBUG_INIT) printf ("aws-debug: initing texture manager\n");

  awstxtmgr->Initialize (obj_reg);

  objreg = obj_reg;

  return true;
}

unsigned long awsPrefManager::NameToId (const char *n)
{
  if (n)
  {
    unsigned long id = wmgr->GetStringTable()->Request(n);
    if (DEBUG_KEYS) printf ("aws-debug: mapped %s to %lu\n", n, id);
    return id;
  }
  else
    return csInvalidStringID;
}

void awsPrefManager::SetColor (int index, int color)
{
  sys_colors[index] = color;
}

int awsPrefManager::GetColor (int index)
{
  return sys_colors[index];
}

int awsPrefManager::FindColor(unsigned char r,
			      unsigned char g,
			      unsigned char b)
{
  return g2d->FindRGB (r, g, b);
}

iTextureHandle *awsPrefManager::GetTexture (const char* name,
					    const char* filename)
{
  if (awstxtmgr)
    return awstxtmgr->GetTexture (name, filename, false);
  else
    return 0;
}

iTextureHandle *awsPrefManager::GetTexture (const char* name,
					    const char* filename, 
                                            unsigned char key_r,
                                            unsigned char key_g,
                                            unsigned char key_b)
{
  if (awstxtmgr)
    return awstxtmgr->GetTexture (name, filename, false, key_r, key_g, key_b);
  else
    return 0;
}

iFont *awsPrefManager::GetDefaultFont ()
{
  return default_font;
}

iFont *awsPrefManager::GetFont (const char *)
{
  return 0;
}

void awsPrefManager::SetTextureManager (iTextureManager *txtmgr)
{
  if (awstxtmgr) awstxtmgr->SetTextureManager (txtmgr);
}

void awsPrefManager::SetFontServer (iFontServer *fntsvr)
{
  fontsvr = fntsvr;

  // kludge for the moment: this will eventually be more intelligent
  default_font = fontsvr->LoadFont (CSFONT_LARGE);
}

void awsPrefManager::SetDefaultFont (iFont *font)
{
  default_font = font;
}

void awsPrefManager::SetWindowMgr (iAws *_wmgr)
{
  wmgr = _wmgr;
}

void awsPrefManager::SetupPalette ()
{
  printf ("aws-debug: setting up global AWS palette...\n");

  unsigned char red, green, blue;
  iTextureManager *txtmgr = 0;

  if (awstxtmgr) txtmgr = awstxtmgr->GetTextureManager ();

  LookupRGBKey ("HighlightColor", red, green, blue);
  sys_colors[AC_HIGHLIGHT] = g2d->FindRGB (red, green, blue);

  // Create a slightly darker highlight
  sys_colors[AC_HIGHLIGHT2] = g2d->FindRGB (
      (red > GRADIENT_STEP ? red - GRADIENT_STEP : 0),
      (green > GRADIENT_STEP ? green - GRADIENT_STEP : 0),
      (blue > GRADIENT_STEP ? blue - GRADIENT_STEP : 0));

  LookupRGBKey ("ShadowColor", red, green, blue);
  sys_colors[AC_SHADOW] = g2d->FindRGB (red, green, blue);

  // Create a slightly lighter shadow
  sys_colors[AC_SHADOW2] = g2d->FindRGB (
      (255 - red > GRADIENT_STEP ? red + GRADIENT_STEP : 255),
      (255 - green > GRADIENT_STEP ? green + GRADIENT_STEP : 255),
      (255 - blue > GRADIENT_STEP ? blue + GRADIENT_STEP : 255));

  LookupRGBKey ("FillColor", red, green, blue);
  sys_colors[AC_FILL] = g2d->FindRGB (red, green, blue);

  // Create a slightly darker fill
  sys_colors[AC_DARKFILL] = g2d->FindRGB (
      (red > GRADIENT_STEP ? red - GRADIENT_STEP : 0),
      (green > GRADIENT_STEP ? green - GRADIENT_STEP : 0),
      (blue > GRADIENT_STEP ? blue - GRADIENT_STEP : 0));



  LookupRGBKey ("TextForeColor", red, green, blue);
  sys_colors[AC_TEXTFORE] = g2d->FindRGB (red, green, blue);

  LookupRGBKey ("TextBackColor", red, green, blue);
  sys_colors[AC_TEXTBACK] = g2d->FindRGB (red, green, blue);

  LookupRGBKey ("TextDisabledColor", red, green, blue);
  sys_colors[AC_TEXTDISABLED] = g2d->FindRGB (red, green, blue);

  if(LookupRGBKey ("TextSelectedForeColor", red, green, blue))
    sys_colors[AC_SELECTTEXTFORE] = g2d->FindRGB (red, green, blue);
  else
    sys_colors[AC_SELECTTEXTFORE] = sys_colors[AC_TEXTBACK];

  if(LookupRGBKey ("TextSelectedBackColor", red, green, blue))
    sys_colors[AC_SELECTTEXTBACK] = g2d->FindRGB (red, green, blue);
  else
    sys_colors[AC_SELECTTEXTBACK] = sys_colors[AC_TEXTFORE];

  LookupRGBKey ("ButtonTextColor", red, green, blue);
  sys_colors[AC_BUTTONTEXT] = g2d->FindRGB (red, green, blue);

  if (LookupRGBKey ("TransparentColor", red, green, blue))
    sys_colors[AC_TRANSPARENT] = g2d->FindRGB (red, green, blue);
  else
    sys_colors[AC_TRANSPARENT] = g2d->FindRGB (255, 0, 255);

  sys_colors[AC_BLACK] = g2d->FindRGB (0, 0, 0);
  sys_colors[AC_WHITE] = g2d->FindRGB (255, 255, 255);
  sys_colors[AC_RED] = g2d->FindRGB (128, 0, 0);
  sys_colors[AC_GREEN] = g2d->FindRGB (0, 128, 0);
  sys_colors[AC_BLUE] = g2d->FindRGB (0, 0, 128);


  if(LookupRGBKey ("BackgroundColor", red, green, blue))
	  sys_colors[AC_BACKFILL] = g2d->FindRGB(red, green, blue);
  else
	  sys_colors[AC_BACKFILL] = sys_colors[AC_FILL];

  printf ("aws-debug: finished palette setup.\n");
}

bool awsPrefManager::Load (const char *def_file)
{
  if (wmgr == 0)
  {
    printf ("\tunable to load definitions because of an internal error: "
	    "no window manager: %s\n", def_file);
    return false;
  }

  printf ("\tloading definitions file %s...\n", def_file);

  delete static_awsparser;
  static_awsparser = new awsParser(objreg, wmgr, this);
  if (!static_awsparser->Initialize (def_file))
  {
    printf ("Couldn't open def file: %s\n", def_file);
    delete static_awsparser;
    static_awsparser = 0;
    return false;
  }

  int ncw = win_defs.Length();
  int ncs = skin_defs.Length();

  if (awsparse (wmgr))
  {
    printf ("\tsyntax error in definition file, load failed: %s\n", def_file);
    return false;
  }

  printf (
    "\tload successful (%lu windows, %lu skins loaded.)\n",
    (unsigned long)(win_defs.Length() - ncw),
    (unsigned long)(skin_defs.Length() - ncs));

  return true;
}

bool awsPrefManager::SelectDefaultSkin (const char* skin_name)
{
  unsigned long id = NameToId (skin_name);

  for (size_t i = 0; i < skin_defs.Length(); i++)
  {
    if (skin_defs[i]->Name () == id)
    {
      def_skin = skin_defs[i];

      // Set the AWS global palette
      SetupPalette ();

      // Get the default textures into the texture manager.
      int i;
      for (i = 0; i < def_skin->Length (); ++i)
      {
        iAwsKey *k = def_skin->GetAt (i);

        if (k->Type () == KEY_STR)
        {
          csRef<iAwsStringKey> sk (SCF_QUERY_INTERFACE(k, iAwsStringKey));

          if (awstxtmgr)
            (void)awstxtmgr->GetTexturebyID (
                sk->Name (),
                sk->Value ()->GetData (),
                true);
        }
      }

      return true;
    }
  }

  return false;
}

bool awsPrefManager::LookupIntKey (const char *name, int &val)
{
  return LookupIntKey (NameToId (name), val);
}

bool awsPrefManager::LookupIntKey (unsigned long id, int &val)
{
  iAwsKey *k = ((iAwsKeyContainer *)def_skin)->Find (id);

  if (k)
  {
    if (k->Type () == KEY_INT)
    {
      csRef<iAwsIntKey> intKey (SCF_QUERY_INTERFACE(k, iAwsIntKey));
      val = intKey->Value ();
      return true;
    }
  }

  return false;
}

bool awsPrefManager::LookupFloatKey (const char *name, float &val)
{
  return LookupFloatKey (NameToId (name), val);
}

bool awsPrefManager::LookupFloatKey (unsigned long id, float &val)
{
  iAwsKey *k = ((iAwsKeyContainer *)def_skin)->Find (id);

  if (k)
  {
    if (k->Type () == KEY_FLOAT)
    {
      csRef<iAwsFloatKey> floatKey (SCF_QUERY_INTERFACE(k, iAwsFloatKey));
      val = floatKey->Value ();
      return true;
    }
  }

  return false;
}


bool awsPrefManager::LookupStringKey (const char *name, iString * &val)
{
  return LookupStringKey (NameToId (name), val);
}

bool awsPrefManager::LookupStringKey (unsigned long id, iString * &val)
{
  iAwsKey *k = ((iAwsKeyContainer *)def_skin)->Find (id);

  if (k)
  {
    if (k->Type () == KEY_STR)
    {
      csRef<iAwsStringKey> stringKey (SCF_QUERY_INTERFACE(k, iAwsStringKey));
      val = stringKey->Value ();
      return true;
    }
  }

  return false;
}

bool awsPrefManager::LookupRectKey (const char *name, csRect &val)
{
  return LookupRectKey (NameToId (name), val);
}

bool awsPrefManager::LookupRectKey (unsigned long id, csRect &val)
{
  iAwsKey *k = ((iAwsKeyContainer *)def_skin)->Find (id);

  if (k)
  {
    if (k->Type () == KEY_RECT)
    {
      csRef<iAwsRectKey> rectKey (SCF_QUERY_INTERFACE(k, iAwsRectKey));
      val = rectKey->Value ();
      return true;
    }
  }

  return false;
}

bool awsPrefManager::LookupRGBKey (
  const char *name,
  unsigned char &red,
  unsigned char &green,
  unsigned char &blue)
{
  return LookupRGBKey (NameToId (name), red, green, blue);
}

bool awsPrefManager::LookupRGBKey (
  unsigned long id,
  unsigned char &red,
  unsigned char &green,
  unsigned char &blue)
{
  iAwsKey *k = ((iAwsKeyContainer *)def_skin)->Find (id);

  if (k)
  {
    if (k->Type () == KEY_RGB)
    {
      iAwsRGBKey::RGB rgb;
      csRef<iAwsRGBKey> rgbKey (SCF_QUERY_INTERFACE(k, iAwsRGBKey));
      rgb = rgbKey->Value ();

      red = rgb.red;
      green = rgb.green;
      blue = rgb.blue;

      return true;
    }
  }

  return false;
}

bool awsPrefManager::LookupPointKey (const char *name, csPoint &val)
{
  return LookupPointKey (NameToId (name), val);
}

bool awsPrefManager::LookupPointKey (unsigned long id, csPoint &val)
{
  iAwsKey *k = ((iAwsKeyContainer *)def_skin)->Find (id);

  if (k)
  {
    if (k->Type () == KEY_POINT)
    {
      csRef<iAwsPointKey> pointKey (SCF_QUERY_INTERFACE(k, iAwsPointKey));
      val = pointKey->Value ();
      return true;
    }
  }

  return false;
}

bool awsPrefManager::GetInt (iAwsComponentNode *node, const char *name,
			     int &val)
{
  if (!node) return false;

  iAwsKey *k = ((iAwsKeyContainer *)node)->Find (NameToId (name));

  if (k)
  {
    if (k->Type () == KEY_INT)
    {
      csRef<iAwsIntKey> ik (SCF_QUERY_INTERFACE(k, iAwsIntKey));
      val = ik->Value ();
      return true;
    }
  }

  return false;
}

bool awsPrefManager::GetFloat (iAwsComponentNode *node, const char *name,
			       float &val)
{
  if (!node) return false;

  iAwsKey *k = ((iAwsKeyContainer *)node)->Find (NameToId (name));

  if (k)
  {
    if (k->Type () == KEY_FLOAT)
    {
      csRef<iAwsFloatKey> fk (SCF_QUERY_INTERFACE(k, iAwsFloatKey));
      val = fk->Value ();
      return true;
    }
  }

  return false;
}

bool awsPrefManager::GetRGB (iAwsComponentNode *node, const char *name, 
                             unsigned char &red,
                             unsigned char &green,
                             unsigned char &blue)
{
  iAwsKey *k = ((iAwsKeyContainer *)node)->Find (NameToId (name));

  if (k)
  {
    if (k->Type () == KEY_RGB)
    {
      iAwsRGBKey::RGB rgb;
      csRef<iAwsRGBKey> rgbKey (SCF_QUERY_INTERFACE(k, iAwsRGBKey));
      rgb = rgbKey->Value ();

      red = rgb.red;
      green = rgb.green;
      blue = rgb.blue;

      return true;
    }
  }

  return false;
}

bool awsPrefManager::GetRect (iAwsComponentNode *node, const char *name,
			      csRect &val)
{
  if (!node) return false;

  if (DEBUG_KEYS)
    printf ("aws-debug: Getting \"%s\" from %p\n", name, node);

  iAwsKey *k = ((iAwsKeyContainer *)node)->Find (NameToId (name));

  if (DEBUG_KEYS)
    printf ("aws-debug: Node retrieved: %p [%s]\n", node, name);

  if (k)
  {
    if (k->Type () == KEY_RECT)
    {
      csRef<iAwsRectKey> rectKey (SCF_QUERY_INTERFACE(k, iAwsRectKey));
      val = rectKey->Value ();
      return true;
    }
  }

  return false;
}

bool awsPrefManager::GetString (
  iAwsComponentNode *node,
  const char *name,
  iString * &val)
{
  if (!node) return false;

  iAwsKey *k = ((iAwsKeyContainer *)node)->Find (NameToId (name));

  if (k)
  {
    if (k->Type () == KEY_STR)
    {
      csRef<iAwsStringKey> stringKey (SCF_QUERY_INTERFACE(k, iAwsStringKey));
      val = stringKey->Value ();
      return true;
    }
  }

  return false;
}

iAwsComponentNode *awsPrefManager::FindWindowDef (const char *name)
{
  unsigned long id = NameToId (name);

  for (size_t i = 0; i < win_defs.Length(); i++)
    if (win_defs[i]->Name () == id)
      return win_defs[i];

  return 0;
}

iAwsKeyContainer *awsPrefManager::FindSkinDef (const char *name)
{
  unsigned long id = NameToId (name);

  for (size_t i = 0; i < skin_defs.Length(); i++)
    if (skin_defs[i]->Name () == id)
      return skin_defs[i];

  return 0;
}

void awsPrefManager::RegisterConstant (const char *name, int value)
{
  constant_entry *c = new constant_entry;

  c->name = NameToId (name);
  c->value = value;

  constants.Push (c);
}

int awsPrefManager::GetConstantValue (const char *name)
{
  unsigned int namev = NameToId (name);

  size_t i;
  for (i = 0; i < constants.Length (); ++i)
  {
    constant_entry *c = constants.Get (i);

    if (c->name == namev) return c->value;
  }

  return 0;
}

bool awsPrefManager::ConstantExists (const char *name)
{
  unsigned int namev = NameToId (name);

  size_t i;
  for (i = 0; i < constants.Length (); ++i)
  {
    constant_entry *c = constants.Get (i);

    if (c->name == namev) return true;
  }

  return false;
}

iAwsKeyFactory *awsPrefManager::CreateKeyFactory ()
{
  return new awsKeyFactory (wmgr);
}

iAwsConnectionNodeFactory *awsPrefManager::CreateConnectionNodeFactory ()
{
  return new awsConnectionNodeFactory (wmgr);
}
