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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csgeom/csrect.h"
#include "cstool/proctex.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "igraphic/imageio.h"
#include "awscomp.h"
#include "awswin.h"

const int awsNumRectBuckets = 32;

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
   
  /** This is the maximum frame for any window, because it's the size of our
   * canvas, be it the virtual one or otherwise. 
   */
   csRect frame;

   /// The current top window
   awsWindow   *top;

   /// The 2d graphics context
   iGraphics2D *ptG2D;

   /// The 3d graphics context
   iGraphics3D *ptG3D;
   
   /// The object registry pointer, needed at odd times.
   iObjectRegistry *object_reg;
  
   /// Handle to our procedural texture, which the user can have us draw on.
   class awsCanvas : public csProcTexture
   {
     public:
  
        /// Create a new texture.
        awsCanvas ();
  
        /// Destroy this texture
        virtual ~awsCanvas () {}

        /// This is actually not used ever.  The window manager doesn't "animate", and only refreshes the canvas when needed.
        virtual void Animate (csTicks current_time);

        /// Get the iGraphics2D interface so that components can use it.
        iGraphics2D *G2D() 
        { return ptG2D; }

        /// Get the iGraphics3D interface so that components can use it.
        iGraphics3D *G3D() 
        { return ptG3D; }
	
	/// Set dimensions of texture
	void SetSize(int w, int h);

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
   
   /// Is true if the window manager is using the internal canvas, else false.
   bool UsingDefaultContext;
   
   /** Is true if the default context has already been initialized.  This is
     neccessary because csProcTex has some unusual assumptions about the
     state of the system, so I cannot initialize it until a number of conditions
     are true.  Therefore, it is initialized in SetDefaultContext.  If a user
     switches to a different context, and then switches back, we wouldn't want
     them to possibly die by re-initing the thing again.
   */
   bool DefaultContextInitialized;
   
public:
    SCF_DECLARE_IBASE;

    awsManager(iBase *p);
    virtual ~awsManager();
    
    bool Initialize(iObjectRegistry *sys);
   
    /// Get a pointer to the preference manager
    virtual iAwsPrefs *GetPrefMgr();

    /// Set the preference manager used by the window system
    virtual void       SetPrefMgr(iAwsPrefs *pmgr);

    /// Register a component factory
    virtual void       RegisterComponentFactory(awsComponentFactory *factory, char *name);
    
    /// Find a component factory
    virtual awsComponentFactory *FindComponentFactory(char *name);

    /// Get the top window
    virtual awsWindow *GetTopWindow();

    /// Set the top window
    virtual void       SetTopWindow(awsWindow *_top);

    /// Returns true if part of this window is inside the dirty zones
    virtual bool       WindowIsDirty(awsWindow *win);
    
    /// Causes the current view of the window system to be drawn to the given graphics device.
    virtual void       Print(iGraphics3D *g3d);
  
    /// Redraw whatever portions of the screen need it.
    virtual void       Redraw();

    /// Mark a section of the screen dirty
    virtual void       Mark(csRect &rect);

protected:
    /// Redraws a window only if it has areas in the dirtyarea
    void RedrawWindow(awsWindow *win, csRect &dirtyarea);
                
    /// Redraws all children recursively, but only if they have an part in dirty area
    void RecursiveDrawChildren(awsComponent *cmp, csRect &dirtyarea);
    
    /// Recursively creates child components and adds them into a parent.  Used internally.
    void CreateChildrenFromDef(iAws *wmgr, awsComponent *parent, awsComponentNode *settings);
    
public:
    /// Instantiates a window based on a window definition.
    virtual awsWindow *CreateWindowFrom(char *defname);

public:
    /// Set the contexts however you want
    virtual void SetContext(iGraphics2D *g2d, iGraphics3D *g3d);

    /// Set the context to the procedural texture
    virtual void SetDefaultContext(iEngine* engine, iTextureManager* txtmgr);

    /// Get the iGraphics2D interface so that components can use it.
    virtual iGraphics2D *G2D() 
    { return ptG2D; }

    /// Get the iGraphics3D interface so that components can use it.
    virtual iGraphics3D *G3D() 
    { return ptG3D; }

    /// Dispatches events to the proper components
    virtual bool HandleEvent(iEvent&);
    

 
  //////////////////////////////////////

  // Implement iComponent interface.
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(awsManager);
    virtual bool Initialize(iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(awsManager);
    virtual bool HandleEvent(iEvent&)   { return false; }
  } scfiEventHandler;
};
 
#endif

