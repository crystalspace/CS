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
#include <stdarg.h>

#include "cssysdef.h"
#include "csws/cswsutil.h"
#include "csws/csstddlg.h"
#include "csws/csapp.h"
#include "csws/csbutton.h"
#include "csws/csdialog.h"
#include "csws/cswindow.h"
#include "csws/cslistbx.h"
#include "csws/csstatic.h"
#include "csws/csradbut.h"
#include "csws/csiline.h"
#include "csws/cscwheel.h"
#include "csws/cswsaux.h"
#include "csutil/csstring.h"
#include "ivaria/reporter.h"
#include "iutil/vfs.h"
#include "csutil/stringarray.h"
#include "iutil/stringarray.h"

#define MSGBOX_TEXTURE "csws::MessageBoxIcons"

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//- Message boxes --//--

struct MessageBoxData : public iMessageBoxData
{
  int id;
  iBase* userdata;
  SCF_DECLARE_IBASE;
  MessageBoxData () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~MessageBoxData ()
  {
    if (userdata)
      userdata->DecRef ();
    SCF_DESTRUCT_IBASE ();
  }
  virtual int GetPressedButton () { return id; }
  virtual iBase* GetUserData () { return userdata; }
};

SCF_IMPLEMENT_IBASE (MessageBoxData)
  SCF_IMPLEMENTS_INTERFACE (iMessageBoxData)
SCF_IMPLEMENT_IBASE_END

void csMessageBox (csComponent *iParent, const char *iTitle,
	const char *iMessage, iBase* userdata, int iFlags, ...)
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

  int i, overallheight = 0;
  csWindow *MsgBox = new csWindow (iParent, iTitle,
    CSWS_BUTSYSMENU | CSWS_BUTCLOSE | CSWS_TITLEBAR,
    cswfs3D);
  csDialog *Dialog = new csDialog (MsgBox);

  csPixmap *img = 0;
  csStatic *bmp = 0;

  va_list arg;
  va_start (arg, iFlags);
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
    case CSMBS_CUSTOMICON:
    {
      char *name = va_arg (arg, char *);
      int coord [4];
      for (i = 0; i < 4; i++)
        coord [i] = va_arg (arg, int);
      img = NewBitmap (iParent->app, name, coord [0], coord [1], coord [2], coord [3]);
      break;
    }
  } /* endswitch */

  if (img)
    bmp = new csStatic (Dialog, img);

  if (iFlags & CSMBS_USEHEIGHT)
    overallheight = va_arg (arg, int);

  va_end (arg);

  // Create static objects for all text lines
  csStatic *L [100];
  int L_count = 0, L_maxw = 0;
  const char *MsgStart = iMessage;
  for (;;)
  {
    if ((*iMessage == '\n') || (*iMessage == 0))
    {
      char line [200];
      size_t count = iMessage - MsgStart;
      if (count > sizeof (line) - 1)
        count = sizeof (line) - 1;
      strncpy (line, MsgStart, count);
      line [count] = 0;

      L [L_count] = new csStatic (Dialog, 0, line, csscsText);

      int w,h;
      L [L_count]->SuggestSize (w, h);
      L [L_count]->SetSize (w, h);
      if (L_maxw < w) L_maxw = w;

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
      B [B_count] = new csButton (Dialog, ButtonDesc [i].Command,
        ButtonDesc [i].Style);
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
    int deltax = (iFlags & CSMBS_CENTER) ? (L_maxw - L [i]->bound.Width ()) / 2 : 0;
    L [i]->SetPos (left + deltax, cy);
    if (iFlags & CSMBS_USEHEIGHT)
      cy = DIST_TEXTY + (overallheight * (i + 1)) / L_count;
    else
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

  MessageBoxData* mbdata = new MessageBoxData ();
  mbdata->userdata = userdata;
  if (userdata) userdata->IncRef ();
  iParent->app->StartModal (MsgBox, mbdata);
  mbdata->DecRef ();

  //int ret = iParent->app->Execute (MsgBox);
  //delete MsgBox;
  //return ret;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//-- File open dialog --//--

#define FILEDLG_TEXTURE_NAME "csws::FileDialog"

// private class
class cspFileDialog : public csDialog
{
  bool busy;
  bool usevfs;
  iVFS *vfs;
public:
  char *path;
  cspFileDialog (csComponent *iParent);
  virtual ~cspFileDialog ();
  virtual bool HandleEvent (iEvent &Event);

  void UseVFS(bool toggle);
  void Reread ();
  // set file path
  bool SetPath (const char *iPath);
  // set file name
  void SetName (const char *iName);
  // set path and file name
  void SetFileName (const char *iName);
  // build path from directory listbox and set it
  bool BuildAndSetPath ();
  // returns the selected filename
  bool GetFileName(char *buf,size_t bufsize);
};

#define CSFDI_PATHCOMPONENT 0x9999
static int fdref = 0;
static csPixmap *fdspr[2] = { 0, 0 };

cspFileDialog::cspFileDialog (csComponent *iParent)
  : csDialog (iParent)
{
  path = 0;
  vfs = 0;
  busy = false;
  fdref++;
  if (app)
  {
    // If images are not loaded, load them
    if (!fdspr [0])
      fdspr [0] = new csSimplePixmap (app->GetTexture (
        FILEDLG_TEXTURE_NAME), 0, 0, 16, 13);
    if (!fdspr [1])
      fdspr [1] = new csSimplePixmap (app->GetTexture (
        FILEDLG_TEXTURE_NAME), 16, 0, 16, 13);
  } /* endif */
}

cspFileDialog::~cspFileDialog ()
{
  if (--fdref == 0)
  {
    delete fdspr [0]; fdspr [0] = 0;
    delete fdspr [1]; fdspr [1] = 0;
  }
  if (path)
    delete [] path;
  if (vfs)
    vfs->DecRef();
}

void cspFileDialog::UseVFS(bool toggle)
{
  usevfs=toggle;
  if (usevfs)
  {
    vfs = app->VFS;
    vfs->IncRef();
  }
}

bool cspFileDialog::HandleEvent (iEvent &Event)
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

static bool is_checked (csComponent *child, intptr_t param)
{
  (void)child; (void)param;
  return true;
}

bool cspFileDialog::BuildAndSetPath ()
{
  csListBox *dp = (csListBox *)GetChild (CSWID_DIRLIST);
  csComponent *cur = dp->ForEachItem (is_checked, 0, true);
  char buff [CS_MAXPATHLEN + 1];

  if (cur->id != CSFDI_PATHCOMPONENT)
  {
    strcpy (buff, cur->GetText ());
    while (cur->id != CSFDI_PATHCOMPONENT)
      cur = cur->prev;
  }
  else
    buff [0] = 0;

  char pathsep = usevfs ? VFS_PATH_SEPARATOR : CS_PATH_SEPARATOR;
  int maxlen = usevfs ? VFS_MAX_PATH_LEN : CS_MAXPATHLEN;

  while (cur->id == CSFDI_PATHCOMPONENT)
  {
    char *tmp = new char[maxlen + 1];
    strcpy (tmp, buff);
    strcpy (buff, cur->GetText ());
    size_t sl = strlen (buff);
    if ((tmp [0] != 0)
   	&& (buff [sl - 1] != '/')
      	&& (buff [sl - 1] != pathsep))
      buff [sl++] = pathsep;
    strncpy (&buff [sl], tmp, maxlen - sl);
    cur = cur->prev;
    delete[] tmp;
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
  if (!usevfs)
    path = csExpandName (iPath);
  else
    path = csStrNew(iPath);
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
  char path [CS_MAXPATHLEN + 1], name [CS_MAXPATHLEN + 1];
  csSplitPath (iName, path, sizeof (path), name, sizeof (name));
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
  csComponent *activate = 0;

  // Clear "file name" field
  SetName ("");

  char pathsep = usevfs ? VFS_PATH_SEPARATOR : CS_PATH_SEPARATOR;
  int maxlen = usevfs ? VFS_MAX_PATH_LEN : CS_MAXPATHLEN;

  // Now decompose path into components
  char *curp = path;
  int level = 0;
  while (*curp)
  {
    char *sep = curp;
    while ((*sep)
        && (*sep != '/')
        && (*sep != pathsep))
      sep++;
    char *name = new char [maxlen + 1];
    if ((sep == curp)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
     || ((level == 0) && (sep [-1] == ':'))
#endif
       )
      sep++;			// Root directory "/"
    strncpy (name, curp, sep - curp);
    name [sep - curp] = 0;
    csListBoxItem *lbi = new csListBoxItem (dp, name, CSFDI_PATHCOMPONENT);
    lbi->SetBitmap (fdspr [1], false);
    lbi->SetOffset (level * 6);
    level++; curp = sep;
    while ((*curp)
        && ((*curp == '/')
         || (*curp == pathsep)))
      curp++;
    activate = lbi;
    delete[] name;
  } /* endwhile */

  size_t i,n;
  csStringArray dirs;
  csStringArray files;

  if (!usevfs)
  {
    // work on real FS

    DIR *dh;
    struct dirent *de;

    if (!(dh = opendir (path)))
    {
      csMessageBox (app, "Error", "Invalid directory", 0);
      app->Printf (CS_REPORTER_SEVERITY_NOTIFY, "Invalid directory path\n");
    }
    else
    {
      while ((de = readdir (dh)) != 0)
      {
	const char* const name = de->d_name;
	if (strcmp (name, ".") != 0 && strcmp (name, "..") != 0)
	  if (isdir (path, de))
	    dirs.Push (csStrNew(name));
	  else
	    files.Push(csStrNew(name));
      }
      closedir (dh);
    }
  }
  else
  {
    // work on vfs
    csString fpath=path;
    fpath.Append(VFS_PATH_SEPARATOR);
    csRef<iStringArray> filelist = vfs->FindFiles(fpath);

    if (filelist->Length() != 0)
    {
      for (size_t i=0; i<filelist->Length(); i++)
      {
	// extract filename from complete path
	char *fname = (char*) filelist->Get(i);
	size_t dirlen = strlen(fname);
	if (dirlen)
	  dirlen--;
	while (dirlen && fname[dirlen-1]!= VFS_PATH_SEPARATOR)
	  dirlen--;
	fname=fname+dirlen;

	if (fname[strlen(fname)-1] == VFS_PATH_SEPARATOR)
	{
	  fname[strlen(fname)-1]='\0';
	  dirs.Push(csStrNew(fname));
	}
	else
	  files.Push(csStrNew(fname));
      }
    }
  }

  dirs.Sort (false);
  for (i = 0, n = dirs.Length(); i < n; i++)
  {
    csListBoxItem *lbi = new csListBoxItem (dp, dirs.Get(i));
    lbi->SetBitmap (fdspr [0], false);
    lbi->SetOffset (level * 6);
  }

  files.Sort (false);
  for (i = 0, n = files.Length(); i < n; i++)
    (void)new csListBoxItem (fp, files.Get(i));

  busy = false;
  // Place listbox items
  // Activate current directory item
  if (activate)
    activate->SendCommand (cscmdListBoxItemSet, (intptr_t)true);
}

bool cspFileDialog::GetFileName(char *buf, size_t bufsize)
{
  csComponent *f = GetChild (CSWID_FILENAME);
  csComponent *p = GetChild (CSWID_PATHNAME);

  if (f && p && buf)
  {
    p->GetText (buf, bufsize);
    size_t sl = strlen (buf);

    char separator = usevfs ? VFS_PATH_SEPARATOR : CS_PATH_SEPARATOR;

    if (sl < bufsize)
    {
      if ((buf [sl - 1] != '/')
 	  && (buf [sl - 1] != separator))
      {
	buf [sl++] = separator;
	buf [sl] = 0;
      }

      f->GetText (&buf [sl], bufsize - sl);
    }
    return true;
  }
  return false;
}

csWindow *csFileDialog (csComponent *iParent, const char *iTitle,
    const char *iFileName, const char *iOpenButtonText, bool vfspaths)
{
  csWindow *w = new csWindow (iParent, iTitle,
      CSWS_BUTSYSMENU | CSWS_BUTCLOSE | CSWS_TITLEBAR);
  cspFileDialog *d = new cspFileDialog (w);
  w->SetDragStyle (w->GetDragStyle () & ~CS_DRAG_SIZEABLE);

  d->UseVFS(vfspaths);

  csComponent *c = new csInputLine (d, CS_MAXPATHLEN);
  c->id = CSWID_FILENAME;
  c->SetRect (5, 15, 5+310, 31);

  c = new csStatic (d, c, "File ~name");
  c->SetPos (5, 5);

  c = new csInputLine (d, CS_MAXPATHLEN);
  c->id = CSWID_PATHNAME;
  c->SetRect (5, 45, 5+310, 61);

  c = new csStatic (d, c, "File ~path");
  c->SetPos (5, 35);

  c = new csListBox (d, CSLBS_HSCROLL | CSLBS_VSCROLL);
  c->id = CSWID_DIRLIST;
  c->SetRect (5, 75, 5+150, 245);

  c = new csStatic (d, c, "~Directories");
  c->SetPos (5, 65);

  c = new csListBox (d, CSLBS_HSCROLL | CSLBS_VSCROLL);
  c->id = CSWID_FILELIST;
  c->SetRect (165, 75, 165+150, 245);

  c = new csStatic (d, c, "~Files");
  c->SetPos (165, 65);

  csButton *b[2];
  b [0] = new csButton (d, cscmdOK, CSBS_DEFAULTVALUE | CSBS_DEFAULT);
  b [0]->SetText (iOpenButtonText);
  b [0]->SetSuggestedSize (+16, +2);

  b [1] = new csButton (d, cscmdCancel, CSBS_DEFAULTVALUE | CSBS_DISMISS);
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

  cspFileDialog *d = (cspFileDialog *) iFileDialog->GetChild (CSWID_CLIENT);
  if (d)
  {
    d->GetFileName(iFileName, iFileNameSize);
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
  virtual bool HandleEvent (iEvent &Event);
  void SetColor (int iColor);
  int GetColor ()
  { return color; }
  void GetColor (float &oR, float &oG, float &oB)
  { oR = r; oG = g; oB = b; }
  void SetRGB (float iR, float iG, float iB);
  void UpdateInfo (bool UpdateSlider);
  void SetHLSmode (bool Enable);
  void HLS2RGB ()
  { ::csHLS2RGB (h, l, s, r, g, b); }
  void RGB2HLS ()
  { ::csRGB2HLS (r, g, b, h, l, s); }
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
      c->SendCommand (cscmdScrollBarGetStatus, (intptr_t)&sbs);	\
      sbs.value = int ((val) * 255.9);			\
      if (UpdateSlider)					\
        c->SendCommand (cscmdScrollBarSet, (intptr_t)&sbs);	\
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
  csGetRGB (iColor, app, r, g, b);

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
  GetChild (CSWID_COLORHLS)->SendCommand (cscmdRadioButtonSet, (intptr_t)Enable);
  GetChild (CSWID_COLORRGB)->SendCommand (cscmdRadioButtonSet, (intptr_t)!Enable);

  csComponent *c = GetChild (CSWID_COLORHR_LABEL);
  if (c) c->SetText (hlsmode ? "~H" : "~R");
  c = GetChild (CSWID_COLORLG_LABEL);
  if (c) c->SetText (hlsmode ? "~L" : "~G");
  c = GetChild (CSWID_COLORSB_LABEL);
  if (c) c->SetText (hlsmode ? "~S" : "~B");

  UpdateInfo (true);
}

bool cspColorDialog::HandleEvent (iEvent &Event)
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
          SetHLSmode (Event.Command.Info == (intptr_t)GetChild (CSWID_COLORHLS));
          return true;
        case cscmdScrollBarValueChanged:
        {
          csScrollBarStatus sbs;
          csComponent *c = ((csComponent *)Event.Command.Info);
          c->SendCommand (cscmdScrollBarGetStatus, (intptr_t)&sbs);
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

csWindow *csColorDialog (csComponent *iParent, const char *iTitle, int iColor)
{
  float r,g,b;
  csGetRGB (iColor, iParent->app, r, g, b);
  return csColorDialog (iParent, iTitle, r, g, b);
}

csWindow *csColorDialog (csComponent *iParent, const char *iTitle,
  float iR, float iG, float iB)
{
  csWindow *w = new csWindow (iParent, iTitle,
    CSWS_BUTSYSMENU | CSWS_BUTCLOSE | CSWS_TITLEBAR);
  cspColorDialog *d = new cspColorDialog (w);
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
    sb = new csScrollBar (d, cssfsThinRect);				\
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
    sb->SendCommand (cscmdScrollBarSet, (intptr_t)&sbs);		\
									\
    st = new csStatic (d, 0, "@@@");					\
    st->SetPos (sb->bound.xmax + 4,					\
     sby + (sb->bound.Height () - st->bound.Height ()) / 2);		\
    st->id = wid##_NUM;							\
									\
    st = new csStatic (d, sb, "@");					\
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
  b = new csButton (d, cscmdOK, CSBS_DEFAULTVALUE | CSBS_DEFAULT);
  b->SetText ("~Ok");
  b->SetRect (CD_WIDTH - 65, 5, CD_WIDTH - 5, 25);
  b->SetState (CSS_GROUP, true);

  b = new csButton (d, cscmdCancel, CSBS_DEFAULTVALUE);
  b->SetText ("Cancel");
  b->SetRect (CD_WIDTH - 65, 30, CD_WIDTH - 5, 50);

  st = new csStatic (d);
  st->SetText ("Sample");
  st->SetTextAlign (CSSTA_HCENTER | CSSTA_VCENTER);
  st->SetRect (CD_WIDTH - 65, 55, CD_WIDTH - 5, 112);
  st->id = CSWID_COLORSAMPLE;

  csRadioButton *rb;
  rb = new csRadioButton (d, CSWID_COLORHLS);
  rb->SetPos (CD_WIDTH - 65, 116); rb->SetState (CSS_GROUP, true);
  rb->SetCommandCode (cscmdToggleHLS);
  st = new csStatic (d, rb, "HLS");
  st->SetPos (CD_WIDTH - 50, 118);

  rb = new csRadioButton (d, CSWID_COLORRGB);
  rb->SetPos (CD_WIDTH - 65, 130);
  rb->SetCommandCode (cscmdToggleHLS);
  st = new csStatic (d, rb, "RGB");
  st->SetPos (CD_WIDTH - 50, 132);

  // Set starting color value
  d->SetRGB (iR, iG, iB);
  d->SetHLSmode (true);

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
