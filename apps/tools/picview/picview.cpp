/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cscolor.h"
#include "csutil/cmdline.h"
#include "csutil/cmdhelp.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"
#include "iutil/event.h"
#include "picview.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivaria/reporter.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "igraphic/imageio.h"
#include "csutil/event.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

ceImageView::ceImageView (csComponent *iParent, iGraphics3D * /*G3D*/)
  	: csComponent (iParent)
{
  image = 0;

  SetState (CSS_SELECTABLE, true);
  // set background color to use and make palette
  int palsize = 1;
  int *palette = new int[palsize];
  SetPalette(palette, palsize);
  SetColor(0, cs_Color_Gray_L);
  if (parent)
    parent->SendCommand (cscmdWindowSetClient, (void *)this);
}

ceImageView::~ceImageView ()
{
  delete image;
}

bool ceImageView::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      break;
    case csevKeyboard:
      break;
  }
  return csComponent::HandleEvent (Event);
}

void ceImageView::Draw ()
{
  Box (0, 0, bound.Width(), bound.Height(), 0);
  if (image) Pixmap (image, 0, 0, bound.Width (), bound.Height ());
}

//-----------------------------------------------------------------------------

bool ceControlWindow::HandleEvent (iEvent& Event)
{
  PicViewApp* ceapp = (PicViewApp*)app;
  if (Event.Type == csevCommand)
    switch (Event.Command.Code)
    {
      case cmdQuit:
        app->SendCommand (cscmdQuit);
        break;
      case cmdFirst:
	ceapp->LoadNextImage (1, -1);
	return true;
      case cmdPrev:
	ceapp->LoadNextImage (0, -1);
	return true;
      case cmdNext:
	ceapp->LoadNextImage (0, 1);
	return true;
    }
  return csWindow::HandleEvent (Event);
}

/*---------------------------------------------------------------------*
 * PicViewApp
 *---------------------------------------------------------------------*/

PicViewApp::PicViewApp (iObjectRegistry *object_reg, csSkin &skin)
	: csApp (object_reg, skin)
{
}

PicViewApp::~PicViewApp ()
{
}

bool PicViewApp::Initialize ()
{
  if (!csApp::Initialize ())
    return false;

  image_loader = CS_QUERY_REGISTRY (object_reg, iImageIO);
  if (!image_loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.picview", "No image loader plugin!");
    return false;
  }

  pG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  // Disable double buffering since it kills performance
  pG3D->GetDriver2D ()->DoubleBuffer (false);

  // Change to other directory before doing Prepare()
  // because otherwise precalc_info file will be written into MazeD.zip
  // The /tmp dir is fine for this.
  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  VFS->ChDir ("/tmp");
  files = VFS->FindFiles ("/this/*");
  cur_idx = 0;

  //------------------------------- ok, now initialize the CSWS application ---

  // Initialize the image window ...
  csWindow *w = new csWindow (this, "Image View", CSWS_TITLEBAR, cswfsThin );
  image_view = new ceImageView (w, pG3D);
  image_window = w;
  w->SetRect (140, 0, bound.Width (), bound.Height ());
  w->SetDragStyle(0);
  w->SetResizeMode(CS_LOCK_ALL);
  int bw = 0, bh = 0;
  w->GetBorderSize(bw, bh);
  image_view->SetRect(bw, bh + w->GetTitlebarHeight(), bw + 5,
    bh+w->GetTitlebarHeight() + 5);

  w = new ceControlWindow (this, "", CSWS_DEFAULTVALUE & ~CSWS_MENUBAR);
  w->SetRect (1, 50, 1+140, 50+320);
  csComponent* d = new csDialog (w);
  csButton* but;
  but = new csButton (d, cmdFirst);
  but->SetText ("First");
  int y = 10;
  but->SetPos (1, y); but->SetSize (130, 14); y += 15;

  but = new csButton (d, cmdPrev);
  but->SetText ("Prev");
  but->SetPos (1, y); but->SetSize (130, 14); y += 15;

  but = new csButton (d, cmdNext);
  but->SetText ("Next");
  but->SetPos (1, y); but->SetSize (130, 14); y += 15;

  but = new csButton (d, cmdQuit);
  but->SetText ("~Quit");
  but->SetPos (1, y); but->SetSize (130, 14); y += 15;

  y += 30;

  label1 = new csButton (d, cmdNothing, CSBS_NODEFAULTBORDER|
		  CSBS_TEXTBELOW, csbfsNone);
  label1->SetText ("?");
  label1->SetPos (0, y); label1->SetSize (130, 14); y += 15;
  label2 = new csButton (d, cmdNothing, CSBS_NODEFAULTBORDER|
		  CSBS_TEXTBELOW, csbfsNone);
  label2->SetText ("?");
  label2->SetPos (0, y); label2->SetSize (130, 14); y += 15;

  return true;
}

bool PicViewApp::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case 'q':
	  {
	    ShutDown ();
	    return true;
	  }
	}
      }
      break;
    case csevCommand:
      if (Event.Command.Code == cscmdStopModal)
      {
	csComponent* d = GetTopModalComponent ();
	int rc = (int)Event.Command.Info;
	if (rc == cscmdCancel) { delete d; return true; }

        if (GetTopModalUserdata ())
	{
          csRef<iMessageBoxData> mbd (
	  	SCF_QUERY_INTERFACE (GetTopModalUserdata (),
		iMessageBoxData));
	  if (mbd)
	  {
	    delete d;
	    return true;
	  }
	}
	return true;
      }
      break;
  }
  return csApp::HandleEvent (Event);
}

void PicViewApp::LoadNextImage (int idx, int step)
{
  iTextureManager* txtmgr = pG3D->GetTextureManager ();
  int i;
  if (idx) cur_idx = idx;
  else cur_idx += step;
  if (cur_idx < 0) cur_idx = files->Length ()-1;
  if (cur_idx >= files->Length ()) cur_idx = 0;
  i = cur_idx;
  printf ("loading file '%s' (%d/%d)\n", files->Get (i), i+1, files->Length ());
  char sbuf[255];
  sprintf (sbuf, "%d/%d", i+1, files->Length ());
  label1->SetText (sbuf);
  sprintf (sbuf, "%s", files->Get (i));
  label2->SetText (sbuf+6);
  csRef<iDataBuffer> buf (VFS->ReadFile (files->Get (i)));
  if (!buf) return;
  csRef<iImage> ifile (image_loader->Load (buf->GetUint8 (),
		  buf->GetSize (), txtmgr->GetTextureFormat ()));
  if (image_view->image)
  {
    image_view->image->GetTextureHandle ()->DecRef ();
    delete image_view->image;
    image_view->image = 0;
    image_view->SetSize(5, 5);
  }
  if (ifile)
  {
    int w = ifile->GetWidth ();
    int h = ifile->GetHeight ();
    csRef<iTextureHandle> txt (txtmgr->RegisterTexture (ifile, CS_TEXTURE_2D
    	| CS_TEXTURE_DITHER));
    txt->IncRef ();	// Avoid DecRef from smart pointer.
    txtmgr->PrepareTextures ();
    csSimplePixmap* pm = new csSimplePixmap (txt);
    image_view->image = pm;
    //int w = pm->Width ();
    //int h = pm->Height ();
    int mw = image_window->bound.xmax - image_window->bound.xmin;
    int mh = image_window->bound.ymax - image_window->bound.ymin;
    while (w > mw || h > mh)
    {
      w = w*90/100;
      h = h*90/100;
    }
    printf ("size is %d %d\n", w, h);
    image_view->SetSize(w,h);
  }
  image_view->Invalidate ();
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
CSWS_SKIN_DECLARE_DEFAULT (DefaultSkin);

int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, 0))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.picview",
	"Can't initialize system!");
    return -1;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.picview",
	"Can't initialize system!");
    return -1;
  }

  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));
  cmdline->AddOption ("mode", "1024x768");

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  srand (time (0));

  csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
  iNativeWindow* nw = g3d->GetDriver2D ()->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Picture Viewer");

  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.picview",
	"Can't initialize system!");
    return -1;
  }
  // Create our main class.

  PicViewApp *theApp = new PicViewApp (object_reg, DefaultSkin);

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (theApp->Initialize ())
    csDefaultRunLoop(object_reg);
  else
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.picview", "Error initializing system!");

  delete theApp;
  g3d = 0;	// Release before DestroyApplication().
  cmdline = 0;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}
