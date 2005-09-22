/*
    Copyright (C) 2003 by Odes B. Boatwright.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "cssysdef.h"
#include "cstool/csapplicationframework.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "csutil/cmdhelp.h"
#include "csutil/event.h"

// Static 
iObjectRegistry* csApplicationFramework::object_reg = 0;
csApplicationFramework* csApplicationFramework::m_Ptr = 0;
char* csApplicationFramework::m_ApplicationStringName = 0;
const char* csApplicationFramework::m_FoundationStringName =
  "crystalspace.libcsappframe";


csApplicationFramework::csApplicationFramework () : restartFlag(false)
{
  // It is a fatal error to have more than one csApplicationFramework
  // derived class in an application.
  CS_ASSERT (0 == m_Ptr);
  m_Ptr = this;
}


csApplicationFramework::~csApplicationFramework ()
{
  /*
    On first sight, calling DestroyApplication() here may look fishy. After
    all, before DestroyApplication() is called, all csRef<>s an application has
    have to be cleared, but you may wonder, "is that the case here?"

    Yes, it is. The order of destruction is first subclass (ie the application
    class, derived from csApplicationFramework) and then the superclass 
    (csApplicationFramework itself). All the subclasses csRef<>s are cleared in
    its destructor, after that the csApplicationFramework destructor is
    called, and csApplicationFramework itself has no csRef<>s. So it's safe to
    call DestroyApplication() here.
   */
  if (object_reg != 0)
    DestroyApplication (object_reg);
  object_reg = 0;
  m_Ptr = 0;
  // reset static vars
  m_ApplicationStringName = 0;
  m_FoundationStringName = "crystalspace.libcsappframe";
}


bool csApplicationFramework::Start ()
{
  CS_ASSERT (0 != m_Ptr);
  return m_Ptr->Application ();
}

void csApplicationFramework::End ()
{
  CS_ASSERT (0 != m_Ptr);
  m_Ptr->OnExit ();
}

void csApplicationFramework::OnExit ()
{
}

void csApplicationFramework::Restart()
{
  restartFlag = true;
  Quit();
}
  
bool csApplicationFramework::Initialize (int argc, char *argv[])
{
  object_reg = CreateEnvironment (argc, argv);

  if (object_reg == 0)
  {
    ReportLibError ("Environment could not be created!");
    return false;
  }
  CS_ASSERT (0 != m_Ptr);
  
  return m_Ptr->OnInitialize (argc, argv);
}

void csApplicationFramework::Quit ()
{
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (GetObjectRegistry(), iEventQueue));
  if (q)
    q->GetEventOutlet()->Broadcast (cscmdQuit);
  else
    exit (2);

}

int csApplicationFramework::Main (int argc, char* argv[])
{
  int iReturn = 0;

  if (!Initialize (argc, argv))
    iReturn = 1;
  else if (0 == GetObjectRegistry ())
    iReturn = 1;
  else if (csCommandLineHelper::CheckHelp (GetObjectRegistry ()))
    csCommandLineHelper::Help (GetObjectRegistry ());
  else if (!Start ())
    iReturn = 2;
  End ();
  restartFlag &= (iReturn == 0);

  return iReturn;
}

bool csApplicationFramework::DoRestart()
{
  bool r = restartFlag;
  restartFlag = false;
  return r;
}
