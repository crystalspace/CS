/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef __AWS_MANAGER_H__
#define __AWS_MANAGER_H__

#include "iaws/aws2.h"
#include "preferences.h"

#include "csgeom/csrectrg.h"

#include "iutil/comp.h"
#include "iutil/eventh.h"

#include "ivideo/fontserv.h"

#include "widget.h"

/**
 * The management object for AWS: creates windows, destroys windows, keeps
 * track of all the windows, etc.
 */
class awsManager2 : public iAws
{
  // Event ID's used
  csEventID KeyboardDown;	
 
  /** Store the object registry so we can get at it later. */
  iObjectRegistry *object_reg;

  /** The 2D graphics context. */
  csRef<iGraphics2D> g2d;

  /** The 3D graphics context. */
  csRef<iGraphics3D> g3d;

  /** Store a reference to the default font so that it's quick and easy. */  
  csRef<iFont> default_font;
  
  /** Preferences... this is actually not really preferences anymore. */
  aws::preferences prefs;
  
  /** The last widget to have the mouse focus. */
  aws::widget *mouse_focus;
  
  /** The last widget to have keyboard focus. */
  aws::widget *keyboard_focus;
  
  /** Set if the mouse is captured. */
  bool mouse_captured;

public:
  /////////////////////// Accessors //////////////////////////////

  /// Get the iObjectRegistry interface so that components can use it.
  virtual iObjectRegistry *GetObjectRegistry () { return object_reg; }

public:
  SCF_DECLARE_IBASE;

  awsManager2(iBase *the_base);
  virtual ~awsManager2();

  /** Initializes the manager. Must be called before anything else. */
  virtual bool Initialize (iObjectRegistry *_object_reg);

  /** Setup the drawing targets. */
  virtual void SetDrawTarget(iGraphics2D *_g2d, iGraphics3D *_g3d);
  
  /** Get the 2D graphics context. */
  csRef<iGraphics2D> G2D() { return g2d; }
  
  /** Get the 3D graphics context. */
  csRef<iGraphics3D> G3D() { return g3d; } 
 
public:
  //////////////////////// Definition Files ////////////////////////

  virtual bool Load(const scfString &_filename);
  
public:
  //////////////////////// Definition Files ////////////////////////
  
  virtual iAwsScriptObject *CreateScriptObject(const char *name);

public:
  //////////////////////// Event Handling ////////////////////////

  /// Dispatches events to the proper components.
  virtual bool HandleEvent (iEvent &);

  /// Redraws all the windows into the current graphics contexts.
  virtual void Redraw();

  /// Captures the mouse.
  void CaptureMouse(aws::widget *w);
  
  /// Releases the mouse from capture.
  void ReleaseMouse();
  
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(awsManager2);
    virtual bool Initialize (iObjectRegistry *p)
    {
      return scfParent->Initialize (p);
    }
  } scfiComponent;

  struct EventHandler : public iEventHandler
  {
  private:
    awsManager2 *parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (awsManager2 * parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    virtual bool HandleEvent (iEvent &) { return false; }
  } *scfiEventHandler;


};

extern awsManager2 *AwsMgr();

#endif
