#ifndef __AWS_H__
# define __AWS_H__

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
# include "iaws/aws.h"
# include "iaws/awsparm.h"
# include "iaws/awscnvs.h"
# include "iutil/eventh.h"
# include "iutil/comp.h"
# include "csgeom/csrect.h"
# include "csgeom/csrectrg.h"
# include "cstool/proctex.h"
# include "ivideo/graph2d.h"
# include "ivideo/graph3d.h"
# include "igraphic/imageio.h"
# include "awscomp.h"
# include "awswin.h"

/**
 *
 *  This is the alternate windowing system plugin.  It defines a simple, lightweight alternative to the current CSWS
 * windowing system.  It supports simple skinning via the .skn defintions, and creation of windows from .win definitions.
 *
 */
class awsManager : public iAws
{
private:
  /// Handle to the preference manager.
  csRef<iAwsPrefManager> prefmgr;

  /// Handle to the sink manager
  csRef<iAwsSinkManager> sinkmgr;

  /** This is the dirty region.  All clean/dirty code now utilizes the update
   *  region facility for non-contiguous rectangular spaces.  This buffer
   *  holds an infinite amount of optimal rectangular regions.
   */
  csRectRegion dirty;

  /** This is the erase region.  All windows will call the Erase() function if the
   *  AlwaysEraseWindows flag is set.  That function will add a rect into this region
   *  which requires erasure.  Right before final redraw, all dirty regions will be
   *  excluded from the erasure region, and the erasure region will be painted with
   *  the transparent color.
   */
  csRectRegion erase;

  /** This is the update store.  The update store contains all of the regions
   *  that actually contain anything useful, and thus the only regions that
   *  need to be thrown to the screen.  The store must be cleared and rethrown
   *  during window move operations.
   */
  csRectRegion updatestore;

  /// True if the update store needs to be cleared and updated.
  bool updatestore_dirty;

  /** This is the maximum frame for any window, because it's the size of our
   * canvas, be it the virtual one or otherwise.
   */
  csRect frame;

  /// The current top component
  iAwsComponent *top;

  /// The current component that the mouse is in.
  iAwsComponent *mouse_in;

  /// The current component that has keyboard focus.
  iAwsComponent *keyb_focus;

  /// The current component that has mouse focus locked, if there is one, NULL otherwise.
  iAwsComponent *mouse_focus;

  /// True if mouse events are locked into the top window
  bool mouse_captured;

  /// The 2d graphics context
  iGraphics2D *ptG2D;

  /// The 3d graphics context
  iGraphics3D *ptG3D;

  /// The object registry pointer, needed at odd times.
  iObjectRegistry *object_reg;

  /// canvas instantiation
  csRef<iAwsCanvas> canvas;

  /**
    * Defines the mapping between a factory and it's interned name.  Used for window template instantiation.
    */
  struct awsComponentFactoryMap
  {
    iAwsComponentFactory *factory;
    unsigned long id;
  };

  /// Contains the list of factory to ID mappings.
  csDLinkList component_factories;

  /**
   * Defines a transition
   */
  struct awsWindowTransition
  {
    /// The rect we start with
    csRect start;

    /// The rect we end with
    csRect end;

    /// Where we are
    float morph;

    /// The size of steps to take
    float morph_step;

    /// The window we're dealing with
    iAwsComponent *win;

    /// The type of transition
    unsigned transition_type;
  };

  /// Contains the list of windows in transition
  csBasicVector transitions;

  /// Mode flags for the engine
  unsigned int flags;
public:
  SCF_DECLARE_IBASE;

  awsManager (iBase *p);
  virtual ~awsManager ();

  bool Initialize (iObjectRegistry *sys);

  /// Get a pointer to the preference manager
  virtual iAwsPrefManager *GetPrefMgr ();

  /// Get a pointer to the sink manager
  virtual iAwsSinkManager *GetSinkMgr ();

  /// Set the preference manager used by the window system
  virtual void SetPrefMgr (iAwsPrefManager *pmgr);

  /// Register a component factory
  virtual void RegisterComponentFactory (
                iAwsComponentFactory *factory,
                const char *name);

  /// Find a component factory
  virtual iAwsComponentFactory *FindComponentFactory (const char *name);

  /// Create an embeddable component from a component name.
  virtual iAwsComponent *CreateEmbeddableComponentFrom (const char *name);

  /// Get the top window
  virtual iAwsComponent *GetTopComponent ();

  /// Set the top window
  virtual void SetTopComponent (iAwsComponent *_top);

  /// Returns the lowest-level visible component (if any) at the screen coordinates
  virtual iAwsComponent* ComponentAt(int x, int y);

  /// Returns true if part of this window is inside the dirty zones
  virtual bool ComponentIsDirty (iAwsComponent *win);

  /// Returns true if window is in transition
  virtual bool ComponentIsInTransition(iAwsComponent *win, bool perform_transition=false);
 
  /// Causes the current view of the window system to be drawn to the given graphics device.
  virtual void Print (iGraphics3D *g3d, uint8 Alpha = 0);

  /// Redraw whatever portions of the screen need it.
  virtual void Redraw ();

  /// Mark a section of the screen dirty
  virtual void Mark (const csRect &rect);

  /// Mark a section of the screen clean.
  virtual void Unmark (const csRect &rect);

  /// Erase a section of the screen next round (only useful if AlwaysEraseWindows flag is set)
  virtual void Erase (const csRect &rect);

  /// Mask off a section that has been marked to erase.  This part won't be erased.
  virtual void MaskEraser (const csRect &rect);

  /// Tell the system to rebuild the update store
  virtual void InvalidateUpdateStore ();

  /// Capture all mouse events until release is called, no matter where the mouse is
  virtual void CaptureMouse (iAwsComponent *comp);

  /// Release the mouse events to go where they normally would.
  virtual void ReleaseMouse ();

protected:

  /// Redraws a window only if it has areas in the dirtyarea
  void RedrawWindow (iAwsComponent *comp, csRect dirtyarea);

  /// Redraws all children recursively, but only if they have a part in the dirty area
  void RecursiveDrawChildren (iAwsComponent *cmp, csRect dirtyarea);

  /// Raises all components starting from cmp and working towards the root
  /// that have AWSF_CMP_TOP_SELECT set
  void RaiseComponents(iAwsComponent* cmp);

  /** This moves mouse focus to cmp. 
    * We assure that every parent entered and exited traversing from
    * mouse_in to cmp will receive enter/exit messages
    * This function returns true if focus reaches cmp and false if not. ( this can
    * happen when some component captures the mouse in response to losing focus )
    */
  bool ChangeMouseFocus(iAwsComponent *cmp, iEvent &Event);

  /** Dispatches MouseEnter/Exit for focus change if necessary.
    * Returns true if cmp is now focused, false if not. ( this can
    * happen when some component captures the mouse in response to
    * losing focus )
    */
  bool ChangeMouseFocusHelper (iAwsComponent *cmp, iEvent &Event);

  /// Changes keyboard focus to cmp if necessary
  void ChangeKeyboardFocus(iAwsComponent* cmp, iEvent &Event);

  /// Returns the first common parent of cmp1 and cmp2
  iAwsComponent* FindCommonParent(iAwsComponent* cmp1, iAwsComponent* cmp2);

  /// Recursively creates child components and adds them into a parent.  
  void CreateChildrenFromDef (
        iAws *wmgr,
        iAwsComponent *parent,
        iAwsComponentNode *settings);

  /// Checks the updatestore_dirty flag and refreshes the store accordingly.
  void UpdateStore ();

  /// Registers all the "known" components.
  void RegisterCommonComponents ();

  /// Performs transitioning on a window.
  bool PerformTransition(awsWindowTransition *t);

public:
  /// Instantiates a window based on a window definition.
  virtual iAwsComponent *CreateWindowFrom (const char *defname);

  /// Creates a new embeddable component
  virtual iAwsComponent *CreateEmbeddableComponent ();

  /// Creates a new parameter list
  virtual iAwsParmList *CreateParmList ();

  /// Creates and enables a transition for a window
  virtual void CreateTransition(iAwsComponent *win, unsigned transition_type, float step_size=0.1);

  /// Creates and enables a transition for a window
  virtual void CreateTransitionEx(iAwsComponent *win, unsigned transition_type, float step_size, csRect &user);

public:
  /// Set the contexts however you want
  virtual void SetCanvas (iAwsCanvas *newCanvas);

  /// Get the current context
  virtual iAwsCanvas *GetCanvas ();

  /// Create a default canvas, covering the whole screen
  virtual iAwsCanvas *CreateDefaultCanvas (
                        iEngine *engine,
                        iTextureManager *txtmgr);

  /// Create a default canvas, just a single proctex
  virtual iAwsCanvas *CreateDefaultCanvas (
                        iEngine *engine,
                        iTextureManager *txtmgr,
                        int width,
                        int height,
                        const char *name);

  /// Create a canvas that uses custom graphics devices (e.g. the screen)
  virtual iAwsCanvas *CreateCustomCanvas (iGraphics2D *g2d, iGraphics3D *g3d);

  /// Get the iGraphics2D interface so that components can use it.
  virtual iGraphics2D *G2D ();

  /// Get the iGraphics3D interface so that components can use it.
  virtual iGraphics3D *G3D ();

  /// Get the iObjectRegistry interface so that components can use it.
  virtual iObjectRegistry *GetObjectRegistry ();

  /// Dispatches events to the proper components
  virtual bool HandleEvent (iEvent &);

  /// Sets one or more flags for different operating modes
  virtual void SetFlag (unsigned int flags);

  /// Clears one or more flags for different operating modes
  virtual void ClearFlag (unsigned int flags);

  /// Returns the current flags
  virtual unsigned int GetFlags ();

  /// Returns true if all windows are presently hidden
  bool AllWindowsHidden ();

  //////////////////////////////////////

  // Implement iComponent interface.
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(awsManager);
    virtual bool Initialize (iObjectRegistry *p)
    {
      return scfParent->Initialize (p);
    }
  }
  scfiComponent;
  struct EventHandler : public iEventHandler
  {
private:
    awsManager *parent;
public:
    EventHandler(awsManager * parent)
    {
      SCF_CONSTRUCT_IBASE(NULL);
      EventHandler::parent = parent;
    }
    SCF_DECLARE_IBASE;
    virtual bool HandleEvent (iEvent &) { return false; }
  }
  *scfiEventHandler;
};

#endif

