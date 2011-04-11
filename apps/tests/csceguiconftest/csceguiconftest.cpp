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

CSCEGUIConfTest::CSCEGUIConfTest() : DemoApplication ("CrystalSpace.CSCEGUIConfTest")
{
  myInt = 0;
  myFloat = 0.0f;
  myBool = false;
  myString = "test";
}

CSCEGUIConfTest::~CSCEGUIConfTest()
{
}

void CSCEGUIConfTest::PrintHelp ()
{
  csCommandLineHelper commandLineHelper;

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (GetObjectRegistry (), "csceguiconftest", "csceguiconftest",
     "Application configuration test for CeGUI");
}

void CSCEGUIConfTest::Frame()
{
  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  cegui->Render ();

  hudManager->GetStateDescriptions ()->Put (0, csString ().Format ("myInt     %d", myInt));
  hudManager->GetStateDescriptions ()->Put (1, csString ().Format ("myFloat   %f", myFloat));
  hudManager->GetStateDescriptions ()->Put (2, csString ().Format ("myBool    %s", myBool?"True":"False"));
  hudManager->GetStateDescriptions ()->Put (3, csString ().Format ("myString  %s", myString.GetData()));
}

bool CSCEGUIConfTest::OnInitialize(int argc, char* argv [])
{

  // Default behavior from DemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  return true;
}

bool CSCEGUIConfTest::Application()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  // Initialize the HUD manager
  hudManager->GetKeyDescriptions ()->Empty ();
  hudManager->GetStateDescriptions ()->Push ("myInt");
  hudManager->GetStateDescriptions ()->Push ("myFloat");
  hudManager->GetStateDescriptions ()->Push ("myBool");
  hudManager->GetStateDescriptions ()->Push ("myString");

  configEventNotifier.AttachNew(new CS::Utility::ConfigEventNotifier(GetObjectRegistry()));

  myIntL.AttachNew(new CS::Utility::ConfigListener<int>(GetObjectRegistry(), "CSCEGUIConfTest.myInt", myInt));
  myFloatL.AttachNew(new CS::Utility::ConfigListener<float>(GetObjectRegistry(), "CSCEGUIConfTest.myFloat", myFloat));
  myBoolL.AttachNew(new CS::Utility::ConfigListener<bool>(GetObjectRegistry(), "CSCEGUIConfTest.myBool", myBool));
  myStringL.AttachNew(new CS::Utility::ConfigListener<csString>(GetObjectRegistry(), "CSCEGUIConfTest.myString", myString));

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

  CreateRoom ();

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

/*---------------*
 * Main function
 *---------------*/
int main (int argc, char* argv[])
{
  return CSCEGUIConfTest ().Main (argc, argv);
}
