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

#include <cssysdef.h>
#include <csappframe/csapplicationframework.h>
#include <csutil/cmdhelp.h>

CS_IMPLEMENT_APPLICATION


int main (int argc, char* argv[])
{
  int iReturn = 0;

  if ( ! csApplicationFramework::Initialize (argc, argv) )
  {
    iReturn = 1;
  }
  else if ( NULL == csApplicationFramework::GetObjectRegistry () )
  {
    iReturn = 1;
  }
  // TODO: We may want to provide a way for the developer to disable
  // automatic help checking, but I can't think of a reason to do so.
  else if (csCommandLineHelper::CheckHelp (
  	csApplicationFramework::GetObjectRegistry ()))
  {
    csCommandLineHelper::Help (csApplicationFramework::GetObjectRegistry ());
  }
  else if ( ! csApplicationFramework::Start () )
  {
    iReturn = 2;
  }

	csApplicationFramework::End ();
  return iReturn;
}
