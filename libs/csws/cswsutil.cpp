/*
    Crystal Space Windowing System: Miscelaneous CSWS utilites
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csinput/csinput.h"
#include "csengine/texture.h"

#define MSGBOX_TEXTURE "img/CSWS/msgicons.png"

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
    char *title = child->GetText ();
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
  list->PlaceItems ();
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
  csTextureHandle *tex = app->GetTexture (texturename);
  csSprite2D *spr;
  if (tex)
    CHKB (spr = new csSprite2D (tex, tx, ty, tw, th))
  else
    spr = NULL;

  return spr;
}

int MessageBox (csComponent *iParent, char *iTitle, char *iMessage, int iFlags)
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

void FindCFGBitmap (csStrVector &sv, char *id, int *x, int *y, int *w, int *h)
{
  *x = -1;
  for (int i = 0; i < sv.Length (); i++)
  {
    char *butdef = (char *)(sv[i]);
    if (strncmp (id, butdef, strlen (id)) == 0)
    {
      char temp[256];
      if (ScanStr (butdef, "%s %d,%d,%d,%d", &temp, x, y, w, h) < 0)
      {
        pprintf ("%s: csws.cfg parse error in string: %s\n", id, butdef);
        fatal_exit (0, false);
      }
      break;
    }
  } /* endfor */
  if (*x < 0)
  {
    pprintf ("Cannot find titlebar button definition %s in csws.cfg\n", id);
    fatal_exit (0, false);
  } /* endif */
}

//--//--//--//--//--//--//--//--//--//--//--//--//--// File choose dialog --//--

#define FILEDLG_TEXTURE_NAME "img/CSWS/filedlg.png"

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
  bool SetPath (char *iPath);
  // set file name
  void SetName (char *iName);
  // set path and file name
  void SetFileName (char *iName);
  // build path from directory listbox and set it
  bool BuildAndSetPath ();
};

#define CSFDI_PATHCOMPONENT 0x9999
static int fdref = 0;
static csSprite2D *fdspr[2] = {NULL, NULL};

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

bool cspFileDialog::SetPath (char *iPath)
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

void cspFileDialog::SetName (char *iName)
{
  csComponent *c = GetChild (CSWID_FILENAME);
  if (c && iName)
    c->SetText (iName);
}

void cspFileDialog::SetFileName (char *iName)
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
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
    strncpy (name, curp, sep - curp);
//    if (name [sep - curp - 1] == ':') { sep++;	name [sep - curp - 1] = ' '; }// Root directory "C:\"
#else
    if (sep == curp) sep++;			// Root directory "/"
    strncpy (name, curp, sep - curp);
#endif
    name [sep - curp] = 0;
//  System->Printf (MSG_INITIALIZATION, "Name of file query result %s\n", name);
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

// System->Printf (MSG_INITIALIZATION, "Path is [%s]\n", path);
// prevent the extra '\' when evaluating root directory
#if defined (OS_DOS) || defined (OS_WIN32)
  if (strlen(path) == 3) // root path
	  path[2] = '\0';
#endif
  if (!(dh = opendir (path)))
  {
    MessageBox (app, "Error", "Invalid directory");
	System->Printf (MSG_INITIALIZATION, "Invalid directory path\n");
    goto done;
  }
// Return to original path
#if defined (OS_DOS) || defined (OS_WIN32)
  if (strlen(path) == 4) // root path
	  path[2] = '\\';
#endif
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
  dp->PlaceItems ();
  fp->PlaceItems ();
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
  c->SetRect (5, 5, 5+310, 15);

  CHK (c = new csInputLine (d, MAXPATHLEN));
  c->id = CSWID_PATHNAME;
  c->SetRect (5, 45, 5+310, 61);

  CHK (c = new csStatic (d, c, "File ~path"));
  c->SetRect (5, 35, 5+310, 45);

  CHK (c = new csListBox (d, CSLBS_HSCROLL | CSLBS_VSCROLL));
  c->id = CSWID_DIRLIST;
  c->SetRect (5, 75, 5+150, 245);

  CHK (c = new csStatic (d, c, "~Directories"));
  c->SetRect (5, 65, 5+150, 75);

  CHK (c = new csListBox (d, CSLBS_HSCROLL | CSLBS_VSCROLL));
  c->id = CSWID_FILELIST;
  c->SetRect (165, 75, 165+150, 245);

  CHK (c = new csStatic (d, c, "~Files"));
  c->SetRect (165, 65, 165+150, 75);

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
