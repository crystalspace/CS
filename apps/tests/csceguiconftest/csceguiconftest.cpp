/*
  Copyright (C) 2010 Jelle Hellemans

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "csceguiconftest.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include <CEGUIWindowManager.h>
#include <CEGUILogger.h>
#include "csutil/custom_new_enable.h"


CS_IMPLEMENT_APPLICATION

CSCEGUIConfTest::CSCEGUIConfTest() : csDemoApplication ("CrystalSpace.CSCEGUIConfTest", "csceguiconftest",
		       "csceguiconftest <OPTIONS>",
		       "")
{
  myBool = false;
  myInt = 0;
  myFloat = 0.0f;
  myString = "test";
}

CSCEGUIConfTest::~CSCEGUIConfTest()
{
}

void CSCEGUIConfTest::Frame()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * 4 * speed);
  }
  else
  {
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      rotY += speed;
    if (kbd->GetKeyState (CSKEY_LEFT))
      rotY -= speed;
    if (kbd->GetKeyState (CSKEY_PGUP))
      rotX += speed;
    if (kbd->GetKeyState (CSKEY_PGDN))
      rotX -= speed;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 4 * speed);
  }

  // We now assign a new rotation transformation to the camera.  You
  // can think of the rotation this way: starting from the zero
  // position, you first rotate "rotY" radians on your Y axis to get
  // the first rotation.  From there you rotate "rotX" radians on
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csXRotMatrix3 (rotX) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  c->SetTransform (ot);
  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw(
    engine->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS))
    return;

  
  // Tell the camera to render into the frame buffer.
  view->Draw ();

  cegui->Render ();

  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  int margin = 15;
  int fontColor = g2d->FindRGB (255, 150, 100);

  //TODO: Remove
  csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  myBool = app_cfg->GetBool("CSCEGUIConfTest.myBool", false);
  myInt = app_cfg->GetInt("CSCEGUIConfTest.myInt", 0);
  myFloat = app_cfg->GetFloat("CSCEGUIConfTest.myFloat", 0.0f);
  myString = app_cfg->GetStr("CSCEGUIConfTest.myString", "");
  //End remove.

  WriteShadow(margin, 0, fontColor,  "myBool    %s", myBool?"True":"False");
  WriteShadow(margin, 15, fontColor, "myInt     %d", myInt);
  WriteShadow(margin, 30, fontColor, "myFloat   %f", myFloat);
  WriteShadow(margin, 45, fontColor, "myString  %s", myString.GetData());
}

bool CSCEGUIConfTest::OnInitialize(int argc, char* argv [])
{

  // Default behavior from csDemoApplication
  //if (!csDemoApplication::OnInitialize (argc, argv))
  //  return false;

  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());

  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

bool CSCEGUIConfTest::Application()
{
  // Default behavior from csDemoApplication
  if (!csDemoApplication::Application ())
    return false;

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return ReportError("Failed to locate VFS!");

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!cegui) return ReportError("Failed to locate CEGUI plugin");

  // Initialize CEGUI wrapper
  cegui->Initialize ();
  
  // Set the logging level
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

  vfs->ChDir ("/cegui/");

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->create("ice.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("ice", "MouseArrow");

  cegui->GetFontManagerPtr ()->createFreeTypeFont("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  vfs->ChDir ("/data/csceguiconftest/");
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("csceguiconftest.layout"));

  CEGUI::Window* info = winMgr->getWindow("Options/Info");
  CEGUI::Listbox* list = static_cast<CEGUI::Listbox*>(winMgr->getWindow("Options/List"));

  list->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::SubscriberSlot(&CSCEGUIConfTest::OnListSelection, this));


  static const char* windows[] = {"options-test", "options-video"};
  for (size_t i = 0; i < 2; i++)
  {
    CEGUI::String s = windows[i];
    CEGUI::Window* w = winMgr->loadWindowLayout(s+".layout");
    if (i) w->setVisible(false);
    info->addChildWindow(w);
    CEGUI::String name = w->getName();
    name = name.substr(strlen("Options/Info/"));
    CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(name, (CEGUI::uint)i);
    item->setTextColours(CEGUI::colour(0.f, 0.f, 0.f)); 
    list->addItem(item);
  }

  // These are used store the current orientation of the camera.
  rotY = rotX = 0;

  CreateRoom();
  view->GetCamera()->SetSector (room);
  view->GetCamera()->GetTransform().SetOrigin(csVector3 (0, 5, 0));

  Run();

  return true;
}

bool CSCEGUIConfTest::OnListSelection (const CEGUI::EventArgs& e)
{
  using namespace CEGUI;

  const WindowEventArgs& ddea = static_cast<const WindowEventArgs&>(e);

  // Get the listbox.
  CEGUI::Listbox* listbox = static_cast<CEGUI::Listbox*>(ddea.window);

  // Get the item.
  CEGUI::ListboxItem* item = listbox->getFirstSelectedItem();
  if (!item) { return true;}

  // And switch.
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();
  CEGUI::Window* info = winMgr->getWindow("Options/Info");
  for (size_t i = 0; i < info->getChildCount(); i++)
  {
    Window* child = info->getChildAtIdx(i);
    if (item->getText() == child->getName().substr(strlen("Options/Info/")))
      child->setVisible(true);
    else if (!child->isAutoWindow())
      child->setVisible(false);
  }

  return true;
}

bool CSCEGUIConfTest::OnKeyboard(iEvent& ev)
{
  csDemoApplication::OnKeyboard (ev);
  return false;
}

bool CSCEGUIConfTest::OnMouseDown(iEvent& ev)
{
  return false;
}

/*---------------*
 * Main function
 *---------------*/
int main (int argc, char* argv[])
{
  //return csApplicationRunner<CSCEGUIConfTest>::Run (argc, argv);
  return CSCEGUIConfTest ().Main (argc, argv);
}
