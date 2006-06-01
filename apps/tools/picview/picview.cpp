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

#include "picview.h"

/** Event codes for the control bar's widgets. */
enum PICVIEW_EVENTS { PVE_FIRST, PVE_PREV, PVE_NEXT, PVE_QUIT, PVE_SCALE };

/** Callback for picture viewer events. */
struct onPicViewEvent : public iAws2ScriptEvent 
{	
private:
	/** Holds the instance of the picture viewer that we're responding to. */
	PicView *picview;
	
public:
	onPicViewEvent(PicView *_pv):picview(_pv) {}
	virtual ~onPicViewEvent() {};
	
	virtual void operator() (iAws2ScriptObject *info) 
	{
		switch(info->GetIntArg(0))
		{
			case PVE_FIRST:
				picview->LoadNextImage (true, 1);
			break;
			
			case PVE_PREV:
				picview->LoadNextImage (false, -1);
			break;
			
			case PVE_NEXT:
				picview->LoadNextImage (false, 1);
			break;
			
			case PVE_QUIT:
				picview->Quit();
			break;
			
			case PVE_SCALE:
				picview->FlipScale();
				picview->LoadNextImage (false, 0);
			break;							
		}	
	}	

};	



//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

PicView::PicView ()
{
  SetApplicationName ("CrystalSpace.PicView");
  pic = 0;
  scale = false;
}

PicView::~PicView ()
{
}

void PicView::ProcessFrame ()
{
  iGraphics2D* g2d = g3d->GetDriver2D ();

  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  g2d->Clear(0);

  if (pic)
  {
    if (scale)
      pic->DrawScaled(g3d, 0, 0, g2d->GetWidth (), g2d->GetHeight ());
    else
      pic->Draw(g3d, 0, 0);
  }

  aws->Redraw ();
}

void PicView::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool PicView::OnKeyboard(iEvent& ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC || code == 'q')
    {
      picview_events->Exec("picView.Quit()");
    }
    else if (code == 'f')
    {
      picview_events->Exec("picView.First()");
    }
    else if (code == 'p')
    {
      picview_events->Exec("picView.Prev()");
    }
    else if (code == 'n')
    {
      picview_events->Exec("picView.Next()");
    }
    else if (code == 's')
    {
      picview_events->Exec("picView.Scale()");
    }
  }
  return false;
}

bool PicView::HandleEvent (iEvent &ev)
{
  csBaseEventHandler::HandleEvent(ev);
  if (aws) return aws->HandleEvent (ev);
  return false;
}

bool PicView::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
  SetupConfigManager(GetObjectRegistry(), "/config/picview.cfg");	
	
  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN("crystalspace.window.alternatemanager2", iAws2),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());  
  

  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void PicView::OnExit()
{
}

bool PicView::Application()
{
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  g3d = CS_QUERY_REGISTRY(GetObjectRegistry(), iGraphics3D);
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = CS_QUERY_REGISTRY(GetObjectRegistry(), iEngine);
  if (!engine) return ReportError("Failed to locate 3D engine!");

  kbd = CS_QUERY_REGISTRY(GetObjectRegistry(), iKeyboardDriver);
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  vfs = CS_QUERY_REGISTRY(GetObjectRegistry(), iVFS);
  if (!vfs) return ReportError("Failed to locate Virtual FileSystem!");

  imgloader = CS_QUERY_REGISTRY(GetObjectRegistry(), iImageIO);
  if (!imgloader) return ReportError("Failed to locate Image Loader!");

  aws = CS_QUERY_REGISTRY(GetObjectRegistry(), iAws2);
  if (!aws) return ReportError("Failed to locate Alternative Windowing System 2!");

  vfs->ChDir ("/tmp");
  files = vfs->FindFiles ("/this/*");
  cur_idx = 0;

  CreateGui();

  x = g3d->GetDriver2D ()->GetWidth ();
  y = g3d->GetDriver2D ()->GetHeight ();

  Run();

  return true;
}

void PicView::CreateGui ()
{  
  aws->SetDrawTarget(g3d->GetDriver2D (), g3d);

  picview_events = aws->CreateScriptObject("picView", new onPicViewEvent(this));
  
  picview_events->SetProp("cmdFirst", PVE_FIRST);
  picview_events->SetProp("cmdPrev", PVE_PREV);
  picview_events->SetProp("cmdNext", PVE_NEXT);
  picview_events->SetProp("cmdQuit", PVE_QUIT);
  picview_events->SetProp("cmdScale", PVE_SCALE);  

  // Load the normal skin.
  if (aws->Load ("/varia/picview.skin.js")==false)
    ReportError("Couldn't load skin definition file: '/varia/picview.skin.js'!");
   
  // Load the no files we recognize tooltip. 
  if (files->IsEmpty())
  {
	  if (aws->Load ("/varia/picview.nopics.js")==false)
    	ReportError("Couldn't load skin definition file: '/varia/picview.nopics.js'!");	  	  
  }
  // Otherwise, queue up the first picture.
  else
  {
	 picview_events->Exec("picView.First()");	  
  }
}

void PicView::LoadNextImage (bool rewind, int step)
{
  if (rewind) cur_idx = files->Length ();
  size_t startIdx = cur_idx;
  csRef<iImage> ifile;
  iTextureManager* txtmgr = g3d->GetTextureManager();

  do
  {
    if ((step < 0) && ((size_t)-step > cur_idx))
      cur_idx = files->Length ()-1;
    else
      cur_idx += step;
    if ((size_t)cur_idx >= files->Length ()) cur_idx = 0;

    csRef<iDataBuffer> buf (vfs->ReadFile (files->Get (cur_idx), false));
    if (!buf) continue;
  		
    ifile = imgloader->Load (buf, txtmgr->GetTextureFormat ());
  }
  while (!ifile.IsValid() && (cur_idx != startIdx));
  if (!ifile) 
  {
	  picview_events->Exec("nmp.Show()");
	  return;
  }

  delete pic;
  txt = txtmgr->RegisterTexture (ifile, CS_TEXTURE_2D);
  pic = new csSimplePixmap (txt);
}

//---------------------------------------------------------------------------

// void PicView::ButtonFirst (unsigned long, intptr_t app, iAwsSource* /*source*/)
// {
//   PicView* picview = (PicView*)app;
//   picview->LoadNextImage (1, -1);
// }

// void PicView::ButtonPrev (unsigned long, intptr_t app, iAwsSource* /*source*/)
// {
//   PicView* picview = (PicView*)app;
//   picview->LoadNextImage (0, -1);
// }

// void PicView::ButtonNext (unsigned long, intptr_t app, iAwsSource* /*source*/)
// {
//   PicView* picview = (PicView*)app;
//   picview->LoadNextImage (0, 1);
// }

// void PicView::ButtonQuit (unsigned long, intptr_t /*app*/, iAwsSource* /*source*/)
// {
//   csRef<iEventQueue> q = CS_QUERY_REGISTRY(GetObjectRegistry(), iEventQueue);
//   if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
// }

// void PicView::ButtonScale (unsigned long, intptr_t app, iAwsSource* /*source*/)
// {
//   PicView* picview = (PicView*)app;
//   picview->scale ^= true;
// }

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<PicView>::Run (argc, argv);
}
