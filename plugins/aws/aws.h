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
#include "cstool/proctex.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "awscomp.h"
#include "awswin.h"

const int awsNumRectBuckets = 16;

/**
 *
 *  This is the alternate windowing system plugin.  It defines a simple, lightweight alternative to the current CSWS
 * windowing system.  It supports simple skinning via the .skn defintions, and creation of windows from .win definitions.
 *
 */
class awsManager : public iAws
{
   /// Handle to the preference manager.
   iAwsPrefs *prefmgr;

   /** The group of rects which maintain what needs to be invalidated, and
    * thus redrawn.  Rectangles which overlap or contain each other will be merged
    * too keep overdraw to a minimum.  Having multiple dirty zones allows us to
    * perform a scatter/gather sort of update.  This is especially useful when
    * updating several small areas.
    */
   csRect     dirty[awsNumRectBuckets];



   /// This contains the index of the highest dirty rect containing valid info.
   int dirty_lid;

   /** This is true if all the rect buckets got full.  It would be nice to
    *  have an algorithm that checks to see if the rect is close enough to
    *  another dirty zone so that including them wouldn't cause too much
    *  hassle, but I have a feeling that that's overkill.  Currently, if we
    *  overrun the rect buckets, all buckets are merged and this flag becomes
    *  set, so that future invalidations are automatically merged into
    *  bucket 0.
    */
   bool all_buckets_full;

   /// The current top window
   awsWindow   *top;

   /// The 2d graphics context
   iGraphics2D *ptG2D;

   /// The 3d graphics context
   iGraphics3D *ptG3D;
  
   /// Handle to our procedural texture, which the user can have us draw on.
   class awsCanvas : public csProcTexture
   {
     public:
  
        /// Create a new texture.
        awsCanvas ();
  
        /// Destroy this texture
        virtual ~awsCanvas ();

        /// This is actually not used ever.  The window manager doesn't "animate", and only refreshes the canvas when needed.
        virtual void Animate (csTime current_time);

        /// Get the iGraphics2D interface so that components can use it.
        iGraphics2D *G2D() 
        { return ptG2D; }

        /// Get the iGraphics3D interface so that components can use it.
        iGraphics3D *G3D() 
        { return ptG3D; }
   };

   /// Procedural texture canvas instantiation
   awsCanvas canvas;

   /** 
    * Defines the mapping between a factory and it's interned name.  Used for window template instantiation.
    */
   struct awsComponentFactoryMap
   {
      awsComponentFactory *factory;
      unsigned long        id;
   };

   /// Contains the list of factory to ID mappings.
   csDLinkList component_factories;

public:
    SCF_DECLARE_IBASE;

    awsManager(iBase *p);
    virtual ~awsManager();
    
    bool Initialize(iSystem *sys);
   
    /// Get a pointer to the preference manager
    virtual iAwsPrefs *GetPrefMgr();

    /// Set the preference manager used by the window system
    virtual void       SetPrefMgr(iAwsPrefs *pmgr);

    /// Register a component factory
    virtual void       RegisterComponentFactory(awsComponentFactory *factory, char *name);

    /// Get the top window
    virtual awsWindow *GetTopWindow();

    /// Set the top window
    virtual void       SetTopWindow(awsWindow *_top);

    /// Returns true if part of this window is inside the dirty zones
    virtual bool       WindowIsDirty(awsWindow *win);

    /// Redraw whatever portions of the screen need it.
    virtual void       Redraw();

    /// Mark a section of the screen dirty
    virtual void       Mark(csRect &rect);

protected:
     /// Redraws a window only if it has areas in the dirtyarea
    void          RedrawWindow(awsWindow *win, csRect &dirtyarea);
                
    ///  Redraws all children recursively, but only if they have an part in dirty area
    void          RecursiveDrawChildren(awsComponent *cmp, csRect &dirtyarea);

public:
    /// Set the contexts however you want
    virtual void SetContext(iGraphics2D *g2d, iGraphics3D *g3d);

    /// Set the context to the procedural texture
    virtual void SetDefaultContext();

    /// Get the iGraphics2D interface so that components can use it.
    iGraphics2D *G2D() 
    { return ptG2D; }

    /// Get the iGraphics3D interface so that components can use it.
    iGraphics3D *G3D() 
    { return ptG3D; }

 
  //////////////////////////////////////

  // Implement iPlugIn interface.
  struct eiPlugIn : public iPlugIn
  {
    SCF_DECLARE_EMBEDDED_IBASE(awsManager);
    virtual bool Initialize(iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent(iEvent&)   { return false; }
  } scfiPlugIn;
};
 
#endif
