 #ifndef __AWS_H__
 #define __AWS_H__
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
#include "csfx/proctex.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "awscomp.h"
#include "awswin.h"

/****
  
  This is the alternate windowing system plugin.  It defines a simple, lightweight alternative to the current CSWS
windowing system.  It supports simple skinning via the .skn defintions, and creation of windows from .win definitions.

 ****/

class awsManager : public iAws
{
   /// Handle to the preference manager.
   iAwsPrefs *prefmgr;

   /// The rect which maintains what needs to be invalidated, and thus redrawn.
   csRect     dirty;

   /// The current top window
   awsWindow   *top;
  
   /// Handle to our procedural texture, which is what we draw on.
   class awsCanvas : public csProcTexture
   {
    awsManager *wmgr;

    public:
  
        /// Create a new texture.
        awsCanvas (awsManager *_wmgr);
  
        /// Destroy this texture
        virtual ~awsCanvas ();
        
        /// This is actually not used ever.  The window manager doesn't "animate", and only refreshes the canvas when needed.
        virtual void Animate (cs_time current_time);

        /// Get the iGraphics2D interface so that components can use it.
        iGraphics2D *G2D() 
        { return ptG2D; }

        /// Get the iGraphics3D interface so that components can use it.
        iGraphics3D *G3D() 
        { return ptG3D; }

   } canvas;

public:
    DECLARE_IBASE;

    awsManager(iBase *p);
    virtual ~awsManager();
    
    bool Initialize(iSystem *sys);
   
    /// Get a pointer to the preference manager
    virtual iAwsPrefs *GetPrefMgr();

    /// Set the preference manager used by the window system
    virtual void       SetPrefMgr(iAwsPrefs *pmgr);

    /// Get the top window
    virtual awsWindow *GetTopWindow();

    /// Set the top window
    virtual void       SetTopWindow(awsWindow *_top);


    /// Redraw whatever portions of the screen need it.

  //////////////////////////////////////

  // Implement iPlugIn interface.
  struct eiPlugIn : public iPlugIn
  {
    DECLARE_EMBEDDED_IBASE(awsManager);
    virtual bool Initialize(iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent(iEvent&)   { return false; }
  } scfiPlugIn;
};
 
#endif
