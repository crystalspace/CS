#ifndef __AWS_WINDOW_H__
#define __AWS_WINDOW_H__
/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson 
    
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
*****************************************************************************/
#include "ivaria/aws.h"
#include "isys/plugin.h"
#include "csgeom/csrect.h"
#include "awscomp.h"

/***************************************************************************************************************************
*   This file details the interface of awsWindow components.  Windows, while they are just normal components, have some    *
* special properties that most other components will not have.  For example, windows are moveable, hideable, maximizeable, *
* windows can be minimized, closed, and they can sit on top of or below other windows.  Windows, while being peers, also   *
* have a depth ordering that is implicit in their hierarchy.                                                               *
*                                                                                                                          *
***************************************************************************************************************************/

class awsWindow : public awsComponent
{
private:
    /// Pointer to the window above this one.  Is null if there isn't one.
    awsWindow *above;

    /// Pointer to the window below this one.  Is null if there isn't one.
    awsWindow *below;

    /// Unlinks this window from the window hierarchy.
    void Unlink();

    /// Links this window in above the passed in window.  This window must be unlinked!
    void LinkAbove(awsWindow *win);

    /// Links this window in below the passed in window.  This window must be unlinked!
    void LinkBelow(awsWindow *win);

public:
    /// Raises a window to the top.
    void Raise();

    /// Lowers a window to the bottom.
    void Lower();

    /// Get's the window above this one, NULL if there is none.
    awsWindow *WindowAbove()
    { return above; }

    /// Get's the window below this one, NULL if there is none.
    awsWindow *WindowBelow()
    { return below; }

public:
    /// Event triggered when a window is about to be raised
    virtual bool OnRaise();

    /// Event triggered when a window is about to be lowered
    virtual bool OnLower();

};



#endif
