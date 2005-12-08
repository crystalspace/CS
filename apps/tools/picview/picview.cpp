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

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

PicView::PicView ()
{
  SetApplicationName ("PicView");
  pic = 0;
  gui = 0;
  scale = false;
}

PicView::~PicView ()
{
}

void PicView::EventHandler::ProcessFrame ()
{
  iGraphics2D* g2d = parent->g3d->GetDriver2D ();

  if (g2d->GetHeight() != parent->y || g2d->GetWidth() != parent->x)
  {
    parent->x = g2d->GetWidth();
    parent->y = g2d->GetHeight();
    // TODO : why call GetDriver2D here instead of using g2d?
    parent->aws->SetupCanvas(0, parent->g3d->GetDriver2D (), parent->g3d);
    if (parent->gui)
      parent->gui->MoveTo(g2d->GetWidth ()/2-100, 0);
  }
  
  if (!parent->g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  g2d->Clear(0);
  if (parent->pic)
  {
    if (parent->scale)
      parent->pic->DrawScaled(parent->g3d, 0, 0, g2d->GetWidth (), g2d->GetHeight ());
    else
      parent->pic->Draw(parent->g3d, 0, 0);
  }

  parent->aws->Redraw ();
  parent->aws->Print (parent->g3d, 64);
}

void PicView::EventHandler::FinishFrame ()
{
  parent->g3d->FinishDraw ();
  parent->g3d->Print (0);
}

bool PicView::EventHandler::OnKeyboard(iEvent& ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC || code == 'q')
    {
      ButtonQuit(0, (intptr_t)this, 0);
    }
    else if (code == 'f')
    {
      ButtonFirst(0, (intptr_t)this, 0);
    }
    else if (code == 'p')
    {
      ButtonPrev(0, (intptr_t)this, 0);
    }
    else if (code == 'n')
    {
      ButtonNext(0, (intptr_t)this, 0);
    }
    else if (code == 's')
    {
      ButtonScale(0, (intptr_t)this, 0);
    }
  }
  return false;
}

bool PicView::EventHandler::HandleEvent (iEvent &ev)
{
  csBaseEventHandler::HandleEvent(ev);
  if (parent->aws) return parent->aws->HandleEvent (ev);
  return false;
}

bool PicView::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN("crystalspace.window.alternatemanager", iAws),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  Handler = new EventHandler (this, GetObjectRegistry ());

  if (!Handler->RegisterQueue(GetObjectRegistry(), csevAllEvents (GetObjectRegistry())))
    return ReportError("Failed to set up global event handler!");

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

  aws = CS_QUERY_REGISTRY(GetObjectRegistry(), iAws);
  if (!aws) return ReportError("Failed to locate Alternative WindowingSystem!");

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
  aws->SetupCanvas(0, g3d->GetDriver2D (), g3d);

  iAwsSink* sink = aws->GetSinkMgr ()->CreateSink ((intptr_t)this);
  sink->RegisterTrigger ("First", &EventHandler::ButtonFirst);
  sink->RegisterTrigger ("Prev" , &EventHandler::ButtonPrev );
  sink->RegisterTrigger ("Next" , &EventHandler::ButtonNext );
  sink->RegisterTrigger ("Quit" , &EventHandler::ButtonQuit );
  sink->RegisterTrigger ("Scale", &EventHandler::ButtonScale);
  aws->GetSinkMgr ()->RegisterSink ("PicView", sink);

  if (!aws->GetPrefMgr()->Load ("/aws/windows_skin.def"))
    ReportError("couldn't load skin definition file!");
  if (!aws->GetPrefMgr()->Load ("/varia/picview.def"))
    ReportError("couldn't load definition file!");

  aws->GetPrefMgr ()->SelectDefaultSkin ("Windows");

  gui = aws->CreateWindowFrom ("PicView");
  if (gui)
  {
    gui->MoveTo(g3d->GetDriver2D ()->GetWidth ()/2-100, 0);
    gui->Show ();
  }
}

void PicView::LoadNextImage (size_t idx, int step)
{
  size_t startIdx = cur_idx;
  csRef<iImage> ifile;
  iTextureManager* txtmgr = g3d->GetTextureManager();

  if (idx) cur_idx = idx;
  do
  {
    if ((step < 0) && ((size_t)-step > cur_idx))
      cur_idx = files->Length ()-1;
    else
      cur_idx += step;
    if ((size_t)cur_idx >= files->Length ()) cur_idx = 0;

    csRef<iDataBuffer> buf (vfs->ReadFile (files->Get (cur_idx), false));
    if (!buf) return;
  		
    ifile = imgloader->Load (buf, txtmgr->GetTextureFormat ());
  }
  while (!ifile.IsValid() && (cur_idx != startIdx));
  if (!ifile) return;

  delete pic;
  txt = txtmgr->RegisterTexture (ifile, CS_TEXTURE_2D | CS_TEXTURE_DITHER);
  pic = new csSimplePixmap (txt);
}

//---------------------------------------------------------------------------

PicView::EventHandler::EventHandler (PicView *parent, 
				     iObjectRegistry *object_reg) :
  csBaseEventHandler (object_reg),
  parent (parent)
{
}

void PicView::EventHandler::ButtonFirst (unsigned long, intptr_t app, iAwsSource* /*source*/)
{
  PicView* picview = (PicView*)app;
  picview->LoadNextImage (1, -1);
}

void PicView::EventHandler::ButtonPrev (unsigned long, intptr_t app, iAwsSource* /*source*/)
{
  PicView* picview = (PicView*)app;
  picview->LoadNextImage (0, -1);
}

void PicView::EventHandler::ButtonNext (unsigned long, intptr_t app, iAwsSource* /*source*/)
{
  PicView* picview = (PicView*)app;
  picview->LoadNextImage (0, 1);
}

void PicView::EventHandler::ButtonQuit (unsigned long, intptr_t /*app*/, iAwsSource* /*source*/)
{
  csRef<iEventQueue> q = CS_QUERY_REGISTRY(GetObjectRegistry(), iEventQueue);
  if (q.IsValid())
    q->GetEventOutlet()->Broadcast(csevQuit (GetObjectRegistry ()));
}

void PicView::EventHandler::ButtonScale (unsigned long, intptr_t app, iAwsSource* /*source*/)
{
  PicView* picview = (PicView*)app;
  picview->scale ^= true;
}

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<PicView>::Run (argc, argv);
}
