/*
    Crystal Space Windowing System: Miscelaneous CSWS utilites
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#define SYSDEF_PATH
#define SYSDEF_DIR
#include "sysdef.h"
#include "csws/cswsutil.h"
#include "csws/csapp.h"
#include "csws/csbutton.h"
#include "csws/csdialog.h"
#include "csws/cslistbx.h"
#include "csws/csstatic.h"
#include "csws/csradbut.h"
#include "csws/cscwheel.h"
#include "csws/cswssys.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csinput/csinput.h"
#include "itxtmgr.h"

#define MSGBOX_TEXTURE "csws::MessageBoxIcons"

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//--/ Window list --//--

enum
{
  cscmdWindowListShow = 0x7ffffff0,
  cscmdWindowListMaximize,
  cscmdWindowListClose
};

csWindowList::csWindowList (csComponent *iParent) : csWindow (iParent,
  "Window List", CSWS_BUTCLOSE | CSWS_BUTMAXIMIZE | CSWS_TITLEBAR, cswfs3D)
{
  shouldclose = false;
  SetFont (csFontCourier);
  CHK (dialog = new csDialog (this));
  CHK (list = new csListBox (dialog, CSLBS_MULTIPLESEL | CSLBS_VSCROLL, cslfsThinRect));
  CHK (butshow = new csButton (dialog, cscmdWindowListShow, CSBS_SHIFT
    | CSBS_SELECTABLE | CSBS_NOMOUSEFOCUS | CSBS_NOKEYBOARDFOCUS, csbfsThinRect));
  butshow->SetText ("~Show");
  CHK (butmaximize = new csButton (dialog, cscmdWindowListMaximize, CSBS_SHIFT
    | CSBS_SELECTABLE | CSBS_NOMOUSEFOCUS | CSBS_NOKEYBOARDFOCUS, csbfsThinRect));
  butmaximize->SetText ("~Maximize");
  CHK (butclose = new csButton (dialog, cscmdWindowListClose, CSBS_SHIFT
    | CSBS_SELECTABLE | CSBS_NOMOUSEFOCUS | CSBS_NOKEYBOARDFOCUS, csbfsThinRect));
  butclose->SetText ("~Close");
  focusedwindow = app->focused;
}

bool csWindowList::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csWindow::SetRect (xmin, ymin, xmax, ymax))
  {
    app->SetWindowListSize (bound.Width (), bound.Height ());
    csRect rect (dialog->bound);
    rect.ymax -= TitlebarHeight;
    rect.Move (-dialog->bound.xmin, -dialog->bound.ymin);
    ((csComponent *)list)->SetRect (rect);
    rect.ymin = rect.ymax + 1;
    rect.ymax += TitlebarHeight;
    int x1 = rect.xmin + rect.Width () / 3;
    int x2 = rect.xmin + (rect.Width () * 2) / 3;
    butshow->SetRect (rect.xmin, rect.ymin, x1, rect.ymax);
    butmaximize->SetRect (x1 + 1, rect.ymin, x2, rect.ymax);
    butclose->SetRect (x2 + 1, rect.ymin, rect.xmax, rect.ymax);
    return true;
  } /* endif */
  return false;
}

void csWindowList::SetState (int mask, bool enable)
{
  csComponent::SetState (mask, enable);
  if ((mask & CSS_FOCUSED) && !enable)
    shouldclose = true;
}

static bool do_sendcommand (csComponent *child, void *param)
{
  ((csComponent *)(child->id))->SendCommand ((int)param, NULL);
  return false;
}

static bool do_select (csComponent *child, void *param)
{
  (void)param;
  ((csComponent *)(child->id))->Select ();
  return true;
}

bool csWindowList::HandleEvent (csEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      if (Event.Command.Code == cscmdPostProcess)
      {
        if (shouldclose)
	{
	  Close ();
	  return true;
	}
        if (app->WindowListChanged)
        {
          app->WindowListChanged = false;
          RebuildList ();
        } /* endif */
      } /* endif */
      break;
    case csevKeyDown:
      if (Event.Key.Code == CSKEY_ESC)
      {
        Close ();
        return true;
      } /* endif */
      break;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdWindowListShow:
          list->ForEachItem (do_sendcommand, (void *)cscmdHide);
          return true;
        case cscmdWindowListMaximize:
          list->ForEachItem (do_sendcommand, (void *)cscmdMaximize);
          return true;
        case cscmdWindowListClose:
          list->ForEachItem (do_sendcommand, (void *)cscmdClose);
          return true;
        case cscmdListBoxItemDoubleClicked:
          list->ForEachItem (do_select);
          return true;
      } /* endswitch */
      break;
  } /* endswitch */

  return csWindow::HandleEvent (Event);
}

bool csWindowList::do_addtowindowlist (csComponent *child, void *param)
{
  csWindowList *windowlist = (csWindowList *)param;
  if (windowlist != child)
  {
    const char *title = child->GetText ();
    if (title && title[0])
      CHKB ((void)new csListBoxItem (windowlist->list, title, (int)child,
        child == windowlist->focusedwindow ? cslisEmphasized : cslisNormal));
  } /* endif */
  return false;
}

void csWindowList::RebuildList ()
{
  list->SendCommand (cscmdListBoxClear, NULL);
  app->ForEach (do_addtowindowlist, (void *)this, true);
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//- Utility functions --//--

csButton *csNewToolbarButton (csComponent *iToolbar, int iCommand, char *iText,
  csButtonFrameStyle iFrameStyle, int iButtonStyle)
{
  CHK (csButton *but = new csButton (iToolbar, iCommand, iButtonStyle,
    iFrameStyle));
  but->SetText (iText);
  int w, h;
  but->SuggestSize (w, h);
  but->SetSize (w, h);
  return but;
}

csButton *csNewToolbarButton (csComponent *iToolbar, int iCommand,
  csSprite2D *bmpup, csSprite2D *bmpdn, csButtonFrameStyle iFrameStyle,
  int iButtonStyle)
{
  CHK (csButton *but = new csButton (iToolbar, iCommand, iButtonStyle,
    iFrameStyle));
  but->SetBitmap(bmpup, bmpdn);
  int w, h;
  but->SuggestSize (w, h);
  but->SetSize (w, h);
  return but;
}

csSprite2D *NewBitmap (csApp *app, char *texturename, int tx, int ty,
  int tw, int th)
{
  iTextureHandle *tex = app->GetTexture (texturename);
  csSprite2D *spr;
  if (tex)
    CHKB (spr = new csSprite2D (tex, tx, ty, tw, th))
  else
    spr = NULL;

  return spr;
}

int csMessageBox (csComponent *iParent, char *iTitle, char *iMessage, int iFlags)
{
  #define DIST_BITMAPX		8
  #define DIST_BITMAPY		8
  #define DIST_BITMAPTEXT	8
  #define DIST_TEXTX		8
  #define DIST_TEXTY		8
  #define DIST_TEXTVERT		1
  #define DIST_TEXTBUTT         8
  #define DIST_BUTTONSX		16
  #define DIST_BUTTONSY		8

  #define BUTTON_DESCRIPTIONS	5
  static struct
  {
    int Mask;
    char *Text;
    int Command;
    int Style;
  } ButtonDesc [BUTTON_DESCRIPTIONS] =
  {
    { CSMBS_OK,     "~Ok",     cscmdOK,     CSBS_DEFAULTVALUE | CSBS_DISMISS | CSBS_DEFAULT },
    { CSMBS_CANCEL, "Cancel",  cscmdCancel, CSBS_DEFAULTVALUE | CSBS_DISMISS },
    { CSMBS_ABORT,  "~Abort",  cscmdAbort,  CSBS_DEFAULTVALUE | CSBS_DISMISS },
    { CSMBS_RETRY,  "~Retry",  cscmdRetry,  CSBS_DEFAULTVALUE | CSBS_DISMISS | CSBS_DEFAULT },
    { CSMBS_IGNORE, "~Ignore", cscmdIgnore, CSBS_DEFAULTVALUE | CSBS_DISMISS }
  };

  int i;
  CHK (csWindow *MsgBox = new csWindow (iParent, iTitle,
    CSWS_BUTSYSMENU | CSWS_BUTCLOSE | CSWS_TITLEBAR,
    cswfs3D));
  CHK (csDialog *Dialog = new csDialog (MsgBox));

  csSprite2D *img = NULL;
  csStatic *bmp = NULL;
  switch (iFlags & CSMBS_TYPEMASK)
  {
    case CSMBS_INFORMATION:
      img = NewBitmap (iParent->app, MSGBOX_TEXTURE, 32*2, 0, 32, 32);
      break;
    case CSMBS_WARNING:
      img = NewBitmap (iParent->app, MSGBOX_TEXTURE, 32*1, 0, 32, 32);
      break;
    case CSMBS_QUESTION:
      img = NewBitmap (iParent->app, MSGBOX_TEXTURE, 32*3, 0, 32, 32);
      break;
    case CSMBS_ERROR:
      img = NewBitmap (iParent->app, MSGBOX_TEXTURE, 32*4, 0, 32, 32);
      break;
    case CSMBS_STOP:
      img = NewBitmap (iParent->app, MSGBOX_TEXTURE, 32*0, 0, 32, 32);
      break;
  } /* endswitch */
  if (img)
    CHKB (bmp = new csStatic (Dialog, img));

  // Create static objects for all text lines
  csStatic *L [100];
  int L_count = 0;
  char *MsgStart = iMessage;
  for (;;)
  {
    if ((*iMessage == '\n') || (*iMessage == 0))
    {
      char line [200];
      unsigned int count = (int)iMessage - (int)MsgStart;
      if (count > sizeof (line) - 1)
        count = sizeof (line) - 1;
      strncpy (line, MsgStart, count);
      line [count] = 0;

      CHK (L [L_count] = new csStatic (Dialog, NULL, line, csscsText));

      int w,h;
      L [L_count]->SuggestSize (w, h);
      L [L_count]->SetSize (w, h);

      MsgStart = iMessage + 1;
      L_count++;

      if (*iMessage == 0)
        break;
    } /* endif */
    iMessage++;
  } /* endfor */

  // Create desired buttons
  csButton *B [BUTTON_DESCRIPTIONS];
  int B_count = 0, butw = 0;
  for (i = 0; i < BUTTON_DESCRIPTIONS; i++)
  {
    if (ButtonDesc [i].Mask & iFlags)
    {
      CHK (B [B_count] = new csButton (Dialog, ButtonDesc [i].Command,
        ButtonDesc [i].Style));
      B [B_count]->SetText (ButtonDesc [i].Text);

      int w,h;
      B [B_count]->SuggestSize (w, h);
      w = w * 3 / 2;
      B [B_count]->SetSize (w, h);
      if (B_count)
        butw += DIST_BUTTONSX;
      butw += w;

      if (ButtonDesc [i].Style & CSBS_DEFAULT)
        B [B_count]->Select ();
      B_count++;
    } /* endif */
  } /* endfor */

  // Now place all elements on their places
  int left = DIST_TEXTX, cy = DIST_TEXTY, xmax = 0;
  if (bmp)
    left = DIST_BITMAPX + img->Width () + DIST_BITMAPTEXT;
  for (i = 0; i < L_count; i++)
  {
    L [i]->SetPos (left, cy);
    cy = L [i]->bound.ymax + DIST_TEXTVERT;
    if (L [i]->bound.xmax > xmax)
      xmax = L [i]->bound.xmax;
  } /* endfor */
  xmax += DIST_BITMAPTEXT;
  if (img)
    if (cy < img->Height ())
      cy = img->Height ();
  cy += DIST_TEXTBUTT;
  if (xmax < butw)
    xmax = butw;
  int cx = (xmax - butw) / 2;
  int ymax = 0;
  for (i = 0; i < B_count; i++)
  {
    B [i]->SetPos (cx, cy);
    cx = B [i]->bound.xmax + DIST_BUTTONSX;
    if (B [i]->bound.ymax > ymax)
      ymax = B [i]->bound.ymax;
  } /* endfor */
  if (bmp)
  {
    bmp->SetPos (DIST_BITMAPX, (cy - img->Height ()) / 2);
    bmp->SetSize (img->Width (), img->Height ());
    if (bmp->bound.ymax > ymax)
      ymax = bmp->bound.ymax;
  } /* endif */
  ymax += DIST_BUTTONSY;

  MsgBox->ClientToWindow (xmax, ymax);
  int x = (iParent->bound.Width () - xmax) / 2;
  int y = (iParent->bound.Height () - ymax) / 2;
  MsgBox->SetRect (x, y, x + xmax, y + ymax);
  MsgBox->SetDragStyle (CS_DRAG_MOVEABLE);

  int ret = iParent->app->Execute (MsgBox);

  CHK (delete MsgBox);
  return ret;
}

struct RectUnionRec
{
  csObjVector *rect;
  csRect *result;
};

static bool doRectUnion (int *vector, int count, void *arg)
{
  RectUnionRec *ru = (RectUnionRec *)arg;
  csRect tmp (*(csRect *)(*ru->rect) [vector [0]]);
  int i;
  for (i = 0; i < count; i++)
    tmp.AddAdjanced (*(csRect *)(*ru->rect) [vector [i]]);
  for (i = count; i < ru->rect->Length (); i++)
    tmp.AddAdjanced (*(csRect *)(*ru->rect) [i]);
  if (tmp.Area () > ru->result->Area ())
    ru->result->Set (tmp);
  return false;
}

void RectUnion (csObjVector &rect, csRect &result)
{
  // Sort rectangles by area so that we can compute much less variants
  for (int i = 0; i < rect.Length (); i++)
    for (int j = rect.Length () - 1; j > i; j--)
      if (((csRect *)rect [i])->Area () < ((csRect *)rect [j])->Area ())
      {
        csRect *tmp = (csRect *)rect [i];
        rect [i] = rect [j];
        rect [j] = tmp;
      } /* endif */

  RectUnionRec ru;
  ru.rect = &rect;
  ru.result = &result;
  result.MakeEmpty ();
  int n = rect.Length ();
  // Save user from endless wait
  if (n > 8)
    n = 8;
  if (rect.Length ())
    Combinations (n, n, doRectUnion, &ru);
}

void FindCFGBitmap (cswsSystemDriver* /*System*/, csStrVector &sv, char *id,
  int *x, int *y, int *w, int *h)
{
  char temp[256];
  *x = -1;
  for (int i = 0; i < sv.Length (); i++)
  {
    char *butdef = (char *)(sv[i]);
    if (strncmp (id, butdef, strlen (id)) == 0)
    {
      if (ScanStr (butdef, "%s %d,%d,%d,%d", &temp, x, y, w, h) < 0)
      {
        sprintf (temp, "%s: csws.cfg parse error in string: %s\n", id, butdef);
        System->Printf (MSG_FATAL_ERROR, temp);
        fatal_exit (0, false);
      }
      break;
    }
  } /* endfor */
  if (*x < 0)
  {
    sprintf (temp, "Cannot find titlebar button definition %s in csws.cfg\n", id);
    System->Printf (MSG_FATAL_ERROR, temp);
    fatal_exit (0, false);
  } /* endif */
}

// Multiply with this constant to convert from range -1/3..+1/3 to -PI...+PI
#define CONST_F2A	(M_PI * 3.0)

// HLS -> RGB
void HLS2RGB (float h, float l, float s, float &r, float &g, float &b)
{
  float hr = (h > 2.0 / 3.0) ? h - 1.0 : h;

  r = (hr < 1.0 / 3.0) ?
    1 + s * cos ((hr           ) * CONST_F2A) :
    1 - s;
  g = (h < 2.0 / 3.0) ?
    1 + s * cos ((h - 1.0 / 3.0) * CONST_F2A) :
    1 - s;
  b = (h > 1.0 / 3.0) ?
    1 + s * cos ((h - 2.0 / 3.0) * CONST_F2A) :
    1 - s;

  r *= l; if (r > 1.0) r = 1.0;
  g *= l; if (g > 1.0) g = 1.0;
  b *= l; if (b > 1.0) b = 1.0;
}

// RGB -> HLS
// Phew! It took me two days until I figured how to do
// the reverse transformation :-) -- A.Z.
void RGB2HLS (float r, float g, float b, float &h, float &l, float &s)
{
  float max, mid, min, sign, delta;
  if (r > g)
    if (r > b)
    {
      delta = 0;
      max = r;
      if (g > b)
        mid = g, min = b, sign = 1;
      else
        mid = b, min = g, sign = -1;
    }
    else
    {
      delta = 2.0 / 3.0;
      max = b;
      mid = r;
      min = g;
      sign = 1;
    }
  else
    if (g > b)
    {
      delta = 1.0 / 3.0;
      max = g;
      if (r > b)
        mid = r, min = b, sign = -1;
      else
        mid = b, min = r, sign = 1;
    }
    else
    {
      delta = 2.0 / 3.0;
      max = b;
      mid = g;
      min = r;
      sign = -1;
    }
  l = max;

  if (l == 0)
    s = 0;
  else
    s = 1 - min / l;

  if ((r == g) && (g == b))
    h = 0;
  else
    h = delta + sign * (acos ((max - mid) / (max - min))) / CONST_F2A;
  if (h < 0) 
    h += 1;
}

#undef CONST_F2A

//--//--//--//--//--//--//--//--//--//--//--//--//--//-- File open dialog --//--

#define FILEDLG_TEXTURE_NAME "csws::FileDialog"

// private class
class cspFileDialog : public csDialog
{
  bool busy;
public:
  char *path;
  cspFileDialog (csComponent *iParent);
  virtual ~cspFileDialog ();
  virtual bool HandleEvent (csEvent &Event);
  void Reread ();
  // set file path
  bool SetPath (const char *iPath);
  // set file name
  void SetName (const char *iName);
  // set path and file name
  void SetFileName (const char *iName);
  // build path from directory listbox and set it
  bool BuildAndSetPath ();
};

#define CSFDI_PATHCOMPONENT 0x9999
static int fdref = 0;
static csSprite2D *fdspr[2] = { NULL, NULL };

cspFileDialog::cspFileDialog (csComponent *iParent)
  : csDialog (iParent)
{
  path = NULL;
  busy = false;
  fdref++;
  if (app)
  {
    // If images are not loaded, load them
    if (!fdspr [0])
      CHKB (fdspr [0] = new csSprite2D (app->GetTexture (
        FILEDLG_TEXTURE_NAME), 0, 0, 16, 13));
    if (!fdspr [1])
      CHKB (fdspr [1] = new csSprite2D (app->GetTexture (
        FILEDLG_TEXTURE_NAME), 16, 0, 16, 13));
  } /* endif */
}

cspFileDialog::~cspFileDialog ()
{
  if (--fdref == 0)
  {
    CHK (delete fdspr [0]); fdspr [0] = NULL;
    CHK (delete fdspr [1]); fdspr [1] = NULL;
  }
  if (path)
    delete [] path;
}

bool cspFileDialog::HandleEvent (csEvent &Event)
{
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdOK:
          switch (focused->id)
          {
            case CSWID_DIRLIST:
              if (!BuildAndSetPath ())
                app->Dismiss (Event.Command.Code);
              return true;
            case CSWID_PATHNAME:
              if (!path || (strcmp (focused->GetText (), path)))
              {
                SetPath (focused->GetText ());
                break;
              }
              // otherwise fallback to dismiss
            default:
              app->Dismiss (Event.Command.Code);
          } /* endswitch */
          return true;
        case cscmdListBoxItemSelected:
        case cscmdListBoxItemClicked:
        {
          csListBoxItem *srcitem = (csListBoxItem *)Event.Command.Info;
          csListBox *srcbox = (csListBox *)srcitem->parent;
          if (srcbox == GetChild (CSWID_FILELIST))
            SetName (srcitem->GetText ());
          return true;
        }
        case cscmdListBoxItemDoubleClicked:
        {
          csListBoxItem *srcitem = (csListBoxItem *)Event.Command.Info;
          csListBox *srcbox = (csListBox *)srcitem->parent;
          if (srcbox == GetChild (CSWID_FILELIST))
            app->Dismiss (cscmdOK);
          else if (srcbox == GetChild (CSWID_DIRLIST))
            BuildAndSetPath ();
          return true;
        }
      } /* endswitch */
      break;
  } /* endswitch */
  return csDialog::HandleEvent (Event);
}

static bool is_checked (csComponent *child, void *param)
{
  (void)child; (void)param;
  return true;
}

bool cspFileDialog::BuildAndSetPath ()
{
  csListBox *dp = (csListBox *)GetChild (CSWID_DIRLIST);
  csComponent *cur = dp->ForEachItem (is_checked, NULL, true);
  char buff [MAXPATHLEN + 1];

  if (cur->id != CSFDI_PATHCOMPONENT)
  {
    strcpy (buff, cur->GetText ());
    while (cur->id != CSFDI_PATHCOMPONENT)
      cur = cur->prev;
  }
  else
    buff [0] = 0;

  while (cur->id == CSFDI_PATHCOMPONENT)
  {
    char tmp [MAXPATHLEN + 1];
    strcpy (tmp, buff);
    strcpy (buff, cur->GetText ());
    int sl = strlen (buff);
    if ((tmp [0] != 0)
     && (buff [sl - 1] != '/')
     && (buff [sl - 1] != PATH_SEPARATOR))
      buff [sl++] = PATH_SEPARATOR;
    strncpy (&buff [sl], tmp, MAXPATHLEN - sl);
    cur = cur->prev;
  } /* endwhile */

  return SetPath (buff);
}

bool cspFileDialog::SetPath (const char *iPath)
{
  if (busy || !iPath)
    return false;

  if (path)
  {
    if (strcmp (iPath, path) == 0)
      return false;
    delete [] path;
  }
  path = expandname (iPath);
  Reread ();
  csInputLine *il = (csInputLine *)GetChild (CSWID_PATHNAME);
  if (il && path)
    il->SetText (path);
  return true;
}

void cspFileDialog::SetName (const char *iName)
{
  csComponent *c = GetChild (CSWID_FILENAME);
  if (c && iName)
    c->SetText (iName);
}

void cspFileDialog::SetFileName (const char *iName)
{
  char path [MAXPATHLEN + 1], name [MAXPATHLEN + 1];
  splitpath (iName, path, sizeof (path), name, sizeof (name));
  SetPath (path);
  if (name && *name)
    SetName (name);
}

void cspFileDialog::Reread ()
{
  if (busy)
    return;

  busy = true;
  csListBox *dp = (csListBox *)GetChild (CSWID_DIRLIST);
  csListBox *fp = (csListBox *)GetChild (CSWID_FILELIST);
  dp->SendCommand (cscmdListBoxClear);
  fp->SendCommand (cscmdListBoxClear);
  csComponent *activate = NULL;

  // Clear "file name" field
  SetName ("");

  // Now decompose path into components
  char *curp = path;
  int level = 0;
  while (*curp)
  {
    char *sep = curp;
    while ((*sep)
        && (*sep != '/')
        && (*sep != PATH_SEPARATOR))
      sep++;
    char name [MAXPATHLEN + 1];
    if ((sep == curp)
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
     || ((level == 0) && (sep [-1] == ':'))
#endif
       )
      sep++;			// Root directory "/"
    strncpy (name, curp, sep - curp);
    name [sep - curp] = 0;
    CHK (csListBoxItem *lbi = new csListBoxItem (dp, name, CSFDI_PATHCOMPONENT));
    lbi->SetBitmap (fdspr [1], false);
    lbi->SetOffset (level * 6, 0);
    level++; curp = sep;
    while ((*curp)
        && ((*curp == '/')
         || (*curp == PATH_SEPARATOR)))
      curp++;
    activate = lbi;
  } /* endwhile */

  DIR *dh;
  struct dirent *de;

  if (!(dh = opendir (path)))
  {
    csMessageBox (app, "Error", "Invalid directory");
    System->Printf (MSG_INITIALIZATION, "Invalid directory path\n");
    goto done;
  }

  while ((de = readdir (dh)))
  {
    if ((strcmp (de->d_name, ".") == 0)
     || (strcmp (de->d_name, "..") == 0))
      continue;

    if (isdir (path, de))
    {
      CHK (csListBoxItem *lbi = new csListBoxItem (dp, de->d_name));
      lbi->SetBitmap (fdspr [0], false);
      lbi->SetOffset (level * 6, 0);
    }
    else
      CHKB ((void)new csListBoxItem (fp, de->d_name));
  } /* endwhile */
  closedir (dh);

done:
  busy = false;
  // Place listbox items
  // Activate current directory item
  if (activate)
    activate->SendCommand (cscmdListBoxItemSet, (void *)true);
}

csWindow *csFileDialog (csComponent *iParent, char *iTitle, char *iFileName,
  char *iOpenButtonText)
{
  CHK (csWindow *w = new csWindow (iParent, iTitle,
    CSWS_BUTSYSMENU | CSWS_BUTCLOSE | CSWS_TITLEBAR));
  CHK (cspFileDialog *d = new cspFileDialog (w));
  w->SetDragStyle (w->GetDragStyle () & ~CS_DRAG_SIZEABLE);

  CHK (csComponent *c = new csInputLine (d, MAXPATHLEN));
  c->id = CSWID_FILENAME;
  c->SetRect (5, 15, 5+310, 31);

  CHK (c = new csStatic (d, c, "File ~name"));
  c->SetPos (5, 5);

  CHK (c = new csInputLine (d, MAXPATHLEN));
  c->id = CSWID_PATHNAME;
  c->SetRect (5, 45, 5+310, 61);

  CHK (c = new csStatic (d, c, "File ~path"));
  c->SetPos (5, 35);

  CHK (c = new csListBox (d, CSLBS_HSCROLL | CSLBS_VSCROLL));
  c->id = CSWID_DIRLIST;
  c->SetRect (5, 75, 5+150, 245);

  CHK (c = new csStatic (d, c, "~Directories"));
  c->SetPos (5, 65);

  CHK (c = new csListBox (d, CSLBS_HSCROLL | CSLBS_VSCROLL));
  c->id = CSWID_FILELIST;
  c->SetRect (165, 75, 165+150, 245);

  CHK (c = new csStatic (d, c, "~Files"));
  c->SetPos (165, 65);

  csButton *b[2];
  CHK (b [0] = new csButton (d, cscmdOK, CSBS_DEFAULTVALUE | CSBS_DEFAULT));
  b [0]->SetText (iOpenButtonText);
  b [0]->SetSuggestedSize (+16, +2);

  CHK (b [1] = new csButton (d, cscmdCancel, CSBS_DEFAULTVALUE | CSBS_DISMISS));
  b [1]->SetText ("Cancel");
  b [1]->SetSuggestedSize (+16, +2);

  // Set filename now, before setting window size,
  // in the case "Invalid directory" message will appear
  if (iFileName)
    d->SetFileName (iFileName);
  else
    d->SetFileName ("./");

  // and now set window size and center it.
  w->SetSize (328, 300);
  w->Center ();

  int i, x, bw = 0;
  for (i = 0; i < 2; i++)
    bw += b [i]->bound.Width () + 20;
  x = (d->bound.Width () - bw) / 2 + 10;
  for (i = 0; i < 2; i++)
  {
    b [i]->SetPos (x, 250);
    x += b [i]->bound.Width () + 20;
  } /* endfor */

  d->GetChild (CSWID_FILELIST)->Select ();
  return w;
}

void csQueryFileDialog (csWindow *iFileDialog, char *iFileName,
  size_t iFileNameSize)
{
  if (iFileName && iFileNameSize)
    iFileName [0] = 0;

  csComponent *d = iFileDialog->GetChild (CSWID_CLIENT);
  if (d)
  {
    csComponent *f = d->GetChild (CSWID_FILENAME);
    csComponent *p = d->GetChild (CSWID_PATHNAME);

    if (f && p && iFileName)
    {
      p->GetText (iFileName, iFileNameSize);
      size_t sl = strlen (iFileName);
      if (sl < iFileNameSize)
      {
        if ((iFileName [sl - 1] != '/')
         && (iFileName [sl - 1] != PATH_SEPARATOR))
        {
          iFileName [sl++] = PATH_SEPARATOR;
          iFileName [sl] = 0;
        } /* endif */

        f->GetText (&iFileName [sl], iFileNameSize - sl);
      } /* endif */
    } /* endif */
  } /* endif */
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//--/ Color wheel --//--

// there is not much sense in making these IDs public
#define CSWID_COLORHR_NUM	0x99990000
#define CSWID_COLORHR_LABEL	0x99990001
#define CSWID_COLORLG_NUM	0x99990002
#define CSWID_COLORLG_LABEL	0x99990003
#define CSWID_COLORSB_NUM	0x99990004
#define CSWID_COLORSB_LABEL	0x99990005

#define cscmdToggleHLS		0x99999999

// private class
class cspColorDialog : public csDialog
{
  float r, g, b;
  float h, l, s;
  bool hlsmode;
  int color;
public:
  cspColorDialog (csComponent *iParent);
  virtual ~cspColorDialog ();
  virtual bool HandleEvent (csEvent &Event);
  void SetColor (int iColor);
  int GetColor ()
  { return color; }
  void GetColor (float &oR, float &oG, float &oB)
  { oR = r; oG = g; oB = b; }
  void SetRGB (float iR, float iG, float iB);
  void UpdateInfo (bool UpdateSlider);
  void SetHLSmode (bool Enable);
  void HLS2RGB ()
  { ::HLS2RGB (h, l, s, r, g, b); }
  void RGB2HLS ()
  { ::RGB2HLS (r, g, b, h, l, s); }
};

cspColorDialog::cspColorDialog (csComponent *iParent) : csDialog (iParent)
{
  r = g = b = h = l = s = 0.0;
  hlsmode = false;
}

cspColorDialog::~cspColorDialog ()
{
}

void cspColorDialog::UpdateInfo (bool UpdateSlider)
{
  int tr = 255, tg = 255, tb = 255;
  if (r > 0.5) tr = 0;
  if (g > 0.5) tg = 0;
  if (b > 0.5) tb = 0;
  color = app->FindColor (int (r * 255.9), int (g * 255.9), int (b * 255.9));
  int tcolor = app->FindColor (tr, tg, tb);

  csStatic *sample = (csStatic *)GetChild (CSWID_COLORSAMPLE);
  if (sample)
  {
    sample->SetColor (CSPAL_STATIC_BACKGROUND, color);
    sample->SetColor (CSPAL_STATIC_ITEXT, tcolor);
    sample->Invalidate ();
  }

  csColorWheel *wheel = (csColorWheel *)GetChild (CSWID_COLORWHEEL);
  if (wheel)
    wheel->SetHS (h, s);

  #define CHANGE_SLIDER(wid, val)			\
  {							\
    csScrollBarStatus sbs;				\
    csComponent *c = GetChild (wid);			\
    if (c)						\
    {							\
      c->SendCommand (cscmdScrollBarGetStatus, &sbs);	\
      sbs.value = int ((val) * 255.9);			\
      if (UpdateSlider)					\
        c->SendCommand (cscmdScrollBarSet, &sbs);	\
      char buff [10];					\
      sprintf (buff, "%d", sbs.value);			\
      c = GetChild (wid##_NUM);				\
      if (c)						\
        c->SetText (buff);				\
    }							\
  }

  CHANGE_SLIDER (CSWID_COLORHR, hlsmode ? h : r);
  CHANGE_SLIDER (CSWID_COLORLG, hlsmode ? l : g);
  CHANGE_SLIDER (CSWID_COLORSB, hlsmode ? s : b);

  #undef CHANGE_SLIDER
}

void cspColorDialog::SetColor (int iColor)
{
  csPixelFormat *pfmt = System->G2D->GetPixelFormat ();
  RGBPixel *palette = System->G2D->GetPalette ();
  if (pfmt->PalEntries)
  {
    r = float (palette [iColor].red) / 255;
    g = float (palette [iColor].green) / 255;
    b = float (palette [iColor].blue) / 255;
  }
  else
  {
    r = float ((iColor & pfmt->RedMask  ) >> pfmt->RedShift  ) / float ((1 << pfmt->RedBits  ) - 1);
    g = float ((iColor & pfmt->GreenMask) >> pfmt->GreenShift) / float ((1 << pfmt->GreenBits) - 1);
    b = float ((iColor & pfmt->BlueMask ) >> pfmt->BlueShift ) / float ((1 << pfmt->BlueBits ) - 1);
  }
  RGB2HLS ();
  UpdateInfo (true);
}

void cspColorDialog::SetRGB (float iR, float iG, float iB)
{
  r = iR; g = iG; b = iB;
  RGB2HLS ();
  UpdateInfo (true);
}

void cspColorDialog::SetHLSmode (bool Enable)
{
  hlsmode = Enable;
  GetChild (CSWID_COLORHLS)->SendCommand (cscmdRadioButtonSet, (void *)Enable);
  GetChild (CSWID_COLORRGB)->SendCommand (cscmdRadioButtonSet, (void *)!Enable);

  csComponent *c = GetChild (CSWID_COLORHR_LABEL);
  if (c) c->SetText (hlsmode ? "~H" : "~R");
  c = GetChild (CSWID_COLORLG_LABEL);
  if (c) c->SetText (hlsmode ? "~L" : "~G");
  c = GetChild (CSWID_COLORSB_LABEL);
  if (c) c->SetText (hlsmode ? "~S" : "~B");

  UpdateInfo (true);
}

bool cspColorDialog::HandleEvent (csEvent &Event)
{
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdOK:
        case cscmdCancel:
          app->Dismiss (Event.Command.Code);
          return true;
        case cscmdToggleHLS:
          SetHLSmode (Event.Command.Info == GetChild (CSWID_COLORHLS));
          return true;
        case cscmdScrollBarValueChanged:
        {
          csScrollBarStatus sbs;
          csComponent *c = ((csComponent *)Event.Command.Info);
          c->SendCommand (cscmdScrollBarGetStatus, &sbs);
          float val = float (sbs.value) / 255;
          if (GetChild (CSWID_COLORHR) == c)
            if (hlsmode) h = val; else r = val;
          if (GetChild (CSWID_COLORLG) == c)
            if (hlsmode) l = val; else g = val;
          if (GetChild (CSWID_COLORSB) == c)
            if (hlsmode) s = val; else b = val;
          if (hlsmode)
            HLS2RGB ();
          else
            RGB2HLS ();
          UpdateInfo (false);
          return true;
        }
        case cscmdColorWheelChanged:
        {
          csColorWheel *wheel = (csColorWheel *)Event.Command.Info;
          wheel->GetHS (h, s);
          HLS2RGB ();
          UpdateInfo (true);
          return true;
        }
      } /* endswitch */
      break;
  } /* endswitch */
  return csDialog::HandleEvent (Event);
}

// color dialog width (without frames)
#define CD_WIDTH	(5 + (10 + 128 + CSSB_DEFAULTSIZE * 2 + 30) + 5)

csWindow *csColorDialog (csComponent *iParent, char *iTitle, int iColor)
{
  CHK (csWindow *w = new csWindow (iParent, iTitle,
    CSWS_BUTSYSMENU | CSWS_BUTCLOSE | CSWS_TITLEBAR));
  CHK (cspColorDialog *d = new cspColorDialog (w));
  w->SetDragStyle (w->GetDragStyle () & ~CS_DRAG_SIZEABLE);

  csColorWheel *cw = new csColorWheel (d);
  cw->SetPos (5, 5);
  cw->id = CSWID_COLORWHEEL;

  csScrollBar *sb;
  csStatic *st;
  int sbx = 5;
#define ADD_SLIDER(wid,y)						\
  {									\
    int sby = y + 4;							\
    CHK (sb = new csScrollBar (d, cssfsThinRect));			\
    sb->id = wid;							\
    sb->SetRect (sbx + 10, sby,						\
      sbx + 10 + 128 + CSSB_DEFAULTSIZE * 2, sby + CSSB_DEFAULTSIZE);	\
    sb->SetState (CSS_SELECTABLE, true);				\
    csScrollBarStatus sbs;						\
    sbs.value = 0;							\
    sbs.maxvalue = 255;							\
    sbs.size = 1;							\
    sbs.maxsize = 10;							\
    sbs.step = 1;							\
    sbs.pagestep = 10;							\
    sb->SendCommand (cscmdScrollBarSet, &sbs);				\
									\
    CHK (st = new csStatic (d, NULL, "@@@"));				\
    st->SetPos (sb->bound.xmax + 4,					\
     sby + (sb->bound.Height () - st->bound.Height ()) / 2);		\
    st->id = wid##_NUM;							\
									\
    CHK (st = new csStatic (d, sb, "@"));				\
    st->SetPos (sb->bound.xmin - 10,					\
     sby + (sb->bound.Height () - st->bound.Height ()) / 2);		\
    st->id = wid##_LABEL;						\
  }

  ADD_SLIDER (CSWID_COLORHR, 144);
  sb->SetState (CSS_GROUP, true);
  ADD_SLIDER (CSWID_COLORLG, sb->bound.ymax);
  ADD_SLIDER (CSWID_COLORSB, sb->bound.ymax);

#undef ADD_SLIDER

  csButton *b;
  CHK (b = new csButton (d, cscmdOK, CSBS_DEFAULTVALUE | CSBS_DEFAULT));
  b->SetText ("~Ok");
  b->SetRect (CD_WIDTH - 65, 5, CD_WIDTH - 5, 25);
  b->SetState (CSS_GROUP, true);

  CHK (b = new csButton (d, cscmdCancel, CSBS_DEFAULTVALUE));
  b->SetText ("Cancel");
  b->SetRect (CD_WIDTH - 65, 30, CD_WIDTH - 5, 50);

  st = new csStatic (d);
  st->SetText ("Sample");
  st->SetTextAlign (CSSTA_HCENTER | CSSTA_VCENTER);
  st->SetRect (CD_WIDTH - 65, 55, CD_WIDTH - 5, 112);
  st->id = CSWID_COLORSAMPLE;

  csRadioButton *rb;
  CHK (rb = new csRadioButton (d, CSWID_COLORHLS));
  rb->SetPos (CD_WIDTH - 65, 116); rb->SetState (CSS_GROUP, true);
  rb->SetCommandCode (cscmdToggleHLS);
  CHK (st = new csStatic (d, rb, "HLS"));
  st->SetPos (CD_WIDTH - 50, 118);

  CHK (rb = new csRadioButton (d, CSWID_COLORRGB));
  rb->SetPos (CD_WIDTH - 65, 130);
  rb->SetCommandCode (cscmdToggleHLS);
  CHK (st = new csStatic (d, rb, "RGB"));
  st->SetPos (CD_WIDTH - 50, 132);

  // Set starting color value
  d->SetColor (iColor);
  d->SetHLSmode (false);

  // and now set window size and center it.
  w->SetSize (4 + CD_WIDTH + 4, 230);
  w->Center ();

  d->GetChild (CSWID_COLORWHEEL)->Select ();
  return w;
}

void csQueryColorDialog (csWindow *iColorDialog, int &oColor)
{
  cspColorDialog *d = (cspColorDialog *)iColorDialog->GetChild (CSWID_CLIENT);
  if (d)
    oColor = d->GetColor ();
}

void csQueryColorDialog (csWindow *iColorDialog, float &oR, float &oG, float &oB)
{
  cspColorDialog *d = (cspColorDialog *)iColorDialog->GetChild (CSWID_CLIENT);
  if (d)
    d->GetColor (oR, oG, oB);
}
