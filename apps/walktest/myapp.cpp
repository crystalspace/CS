/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "walktest/walktest.h"


#define Gfx3D System->G3D
#define Gfx2D System->G2D


// Called at the end of WalkTest's constructor.
void WalkTest::MyAppConstructor(void)
{
}


// Called at the beginning of WalkTest's destructor.
void WalkTest::MyAppDestructor1(void)
{
}


// Called at the end of WalkTest's destructor.
void WalkTest::MyAppDestructor2(void)
{
}


// Load standard libraries here.
void WalkTest::MyAppInitialize1(void)
{
}


// Load standard textures here.
void WalkTest::MyAppInitialize2(void)
{
}


// Load 2D sprites here.
void WalkTest::MyAppInitialize3(void)
{
}


// Do any remaining initializations here.
void WalkTest::MyAppInitialize4(void)
{
}



// Draw the first (background) 2D sprites within this function call.
void WalkTest::MyAppDrawFrame1(void)
{
}


// Draw the last (foreground) 2D sprites within this function call.
void WalkTest::MyAppDrawFrame2(void)
{
}


// Display any additional help commands
void WalkTest::MyAppShowHelp(void)
{
}


// Insert new command handler capabilities here.
bool WalkTest::MyAppCommandHandler(const char *cmd, const char *arg)
{
  cmd; arg;
  return false;   // return false if command wasn't handled here.
}


// Called near the beginning of NextFrame.
void WalkTest::MyAppNextFrame1(time_t elapsed_time, time_t current_time)
{
  elapsed_time; current_time;
}


// Called near the end of NextFrame, just before PrepareFrame call.
void WalkTest::MyAppNextFrame2(time_t elapsed_time, time_t current_time)
{
  elapsed_time; current_time;
}


// Mouse event handler - left button
bool WalkTest::MyAppMouseClick1Handler(csEvent &Event)
{
  Event;
  return false;   // return false to execute WalkTest's default event handler.
}


// Mouse event handler - right button
bool WalkTest::MyAppMouseClick2Handler(csEvent &Event)
{
  Event;
  return false;   // return false to execute WalkTest's default event handler.
}


// Mouse event handler - middle button
bool WalkTest::MyAppMouseClick3Handler(csEvent &Event)
{
  Event;
  return false;   // return false to execute WalkTest's default event handler.
}

