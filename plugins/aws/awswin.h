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
#include "iaws/aws.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csgeom/csrect.h"
#include "csgeom/csrectrg.h"
#include "awscomp.h"

/***************************************************************************************************************************
*   This file details the interface of awsWindow components.  Windows, while they are just normal components, have some    *
* special properties that most other components will not have.  For example, windows are moveable, hideable, maximizeable, *
* windows can be minimized, closed, and they can sit on top of or below other windows.  Windows, while being peers, also   *
* have a depth ordering that is implicit in their hierarchy.                                                               *
*                                                                                                                          *
***************************************************************************************************************************/

class awsWindow : public iAwsWindow
{
private:
    /// Pointer to the window above this one.  Is null if there isn't one.
    iAwsWindow *above;

    /// Pointer to the window below this one.  Is null if there isn't one.
    iAwsWindow *below;

    /// Unlinks this window from the window hierarchy.
    void Unlink();

    /// Links this window in above the passed in window.  This window must be unlinked!
    void LinkAbove(iAwsWindow *win);

    /// Links this window in below the passed in window.  This window must be unlinked!
    void LinkBelow(iAwsWindow *win);

    /// Texture handles for buttons
    iTextureHandle *min_button, *max_button, *close_button, *btxt;

private:
    /** Contains the redraw tag.  This tag changes everytime we redraw the window system, but only once per frame.
      we use it to keep track of which windows have been redrawn and which haven't.
     */
    unsigned int redraw_tag;

    /// The frame style of the window
    int frame_style;

    /// Individual frame options
    int frame_options;

    /// The size of the title bar as of last draw
    int  title_bar_height;

    /// The title
    iString *title;

    /// The last values for x and y so that we can create deltas for moving
    int  last_x, last_y;

    /// True if we are currently resizing
    bool resizing_mode;

    /// True if we are currently moving
    bool moving_mode;

    /// Points for placement of controls, offset from top, right of window.
    csRect minp, maxp, closep;

    /// These are true if the control buttons that we're using are down, else false
    bool min_down, max_down, close_down;

    /// True if the window is in one of these various states
    bool is_zoomed, is_minimized;

    /// True if the child exclusion region needs to be updated
    bool todraw_dirty;

    /// The frame cache for storing frame state while the window is zoomed.
    csRect unzoomed_frame;

    /// Embedded component
    awsComponent comp;

    /// Child region excluder, optimizes drawing of windows.
    csRectRegion todraw;

private:
    void Draw3DRect(iGraphics2D *g2d, csRect &f, int hi, int lo);
    
    /// Get's this components idea of the window manager.  Should be used internally by the component ONLY.
    iAws *WindowManager()
    { return comp.WindowManager(); }

    /// Get's the window that this component resides in.
    virtual iAwsWindow *Window();
        
    /// Get's the parent component of this component;
    virtual iAwsComponent *Parent();
    
    /// Sets the window that this component resides in.
    virtual void SetWindow(iAwsWindow *win);
    
    /// Sets the parent component of this component;
    virtual void SetParent(iAwsComponent *parent);
    
    ////////////// Component declarations for embedded wrappers ///////////////////////////////

    /// Invalidation routine: allow the component to be redrawn when you call this
    virtual void Invalidate();

    /// Invalidation routine: allow component to be redrawn, but only part of it
    virtual void Invalidate(csRect area);

    /// Get this component's frame
    virtual csRect& Frame();

    /// Returns true if this window overlaps the given rect.
    virtual bool Overlaps(csRect &r);

    /// Returns the state of the hidden flag
    virtual bool isHidden();
    
    /// Hides a component
    virtual void Hide();

    /// Shows a component
    virtual void Show();

    /// Sets the flag (can handle multiple simultaneous sets)
    virtual void SetFlag(unsigned int flag);

    /// Clears the flag (can handle multiple simultaneous clears)
    virtual void ClearFlag(unsigned int flag);

    /// Returns the current state of the flags
    virtual unsigned int Flags();

    /// Get's the unique id of this component.
    virtual unsigned long GetID();
    
    /// Set's the unique id of this component. Note: only to be used by window manager.
    virtual void SetID(unsigned long _id);

    /// Recursively moves children (and all nested children) by relative amount given.
    virtual void MoveChildren(int delta_x, int delta_y);
  
    /// Adds a child
    virtual void AddChild(iAwsComponent* child, bool owner);

    /// Removes a child
    virtual void RemoveChild(iAwsComponent *child);

    /// Get's the number of children
    virtual int GetChildCount();

    /// Get's a specific child
    virtual iAwsComponent *GetChildAt(int i);
    
    /// Returns true if this component has children
    virtual bool HasChildren();

    /// Event dispatcher, demultiplexes events and sends them off to the proper event handler
    virtual bool HandleEvent(iEvent& Event);
    
public:
   static const unsigned long sWindowRaised;
   static const unsigned long sWindowLowered;
   static const unsigned long sWindowHidden;
   static const unsigned long sWindowShown;
   static const unsigned long sWindowClosed;

   /******* Frame Styles **********************/

   /// A normal frame that may have a title bar, buttons, 3d border, and a grip.
   static const int fsNormal;

   /// A frame with only a 3d border.  No controls or decorations allowed.
   static const int fsToolbar;

   /// A frame drawn with the background and overlay elements.  Nothing else is drawn.
   static const int fsBitmap;

   /******* Frame Options **********************/

   /// Should draw control box
   static const int foControl;

   /// Should draw zoom (maximize) button
   static const int foZoom;

   /// Should draw minimize button
   static const int foMin;

   /// Should draw close button
   static const int foClose;

   /// Should draw title
   static const int foTitle;

   /// Should draw grip
   static const int foGrip;

   /// Should draw round border (default)
   static const int foRoundBorder;
   
   /// Should draw beveled border
   static const int foBeveledBorder;
   
   SCF_DECLARE_IBASE;
   
   /// This is a component of type window
   virtual char *Type()
   {
      return "Window";
   }

   /// Sets the value of the redraw tag
   virtual void SetRedrawTag(unsigned int tag);

   /// Gets the value of the redraw tag
   virtual unsigned int RedrawTag()
   { return redraw_tag; }

public:
    /// Constructs window class, clear some variables to defaults
    awsWindow();

    /// empty deconstructor
    virtual ~awsWindow();

public:
    /// Registers a slot for a signal
    virtual bool RegisterSlot(iAwsSlot *slot, unsigned long signal);

    /// Unregisters a slot for a signal.
    virtual bool UnregisterSlot(iAwsSlot *slot, unsigned long signal);

    /// Broadcasts a signal to all slots that are interested.
    virtual void Broadcast(unsigned long signal);

    /// Get's component
    iAwsComponent *GetComponent();

public:
    /// Raises a window to the top.
    virtual void Raise();

    /// Lowers a window to the bottom.
    virtual void Lower();

    /// Get's the window above this one, NULL if there is none.
    virtual iAwsWindow *WindowAbove()
    { return above; }

    /// Get's the window below this one, NULL if there is none.
    virtual iAwsWindow *WindowBelow()
    { return below; }

    /// Set's the window above this one
    virtual void SetWindowAbove(iAwsWindow *win);
    
    /// Set's the window below this one
    virtual void SetWindowBelow(iAwsWindow *win);
    
    /// Does some additional setup for windows, including linking into the window hierarchy.
    virtual bool Setup(iAws *_wmgr, awsComponentNode *settings);

    /// Gets properties for this window
    bool GetProperty(char *name, void **parm);

    /// Sets properties for this window
    bool SetProperty(char *name, void *parm);

    /// Executes scriptable actions for this window
    bool Execute(char *action, iAwsParmList &parmlist);

public:
    /// Event triggered when a window is about to be raised
    virtual void OnRaise();

    /// Event triggered when a window is about to be lowered
    virtual void OnLower();

    /// Triggered when the component needs to draw
    virtual void OnDraw(csRect clip);

    /// Triggered when the user presses a mouse button down
    virtual bool OnMouseDown(int button, int x, int y);
    
    /// Triggered when the user unpresses a mouse button 
    virtual bool OnMouseUp(int button, int x, int y);
    
    /// Triggered when the user moves the mouse
    virtual bool OnMouseMove(int button, int x, int y);

    /// Triggered when the user clicks the mouse
    virtual bool OnMouseClick(int button, int x, int y);

    /// Triggered when the user double clicks the mouse
    virtual bool OnMouseDoubleClick(int button, int x, int y);

    /// Triggered when this component loses mouse focus
    virtual bool OnMouseExit();

    /// Triggered when this component gains mouse focus
    virtual bool OnMouseEnter();

    /// Triggered when the user presses a key
    virtual bool OnKeypress(int key, int modifiers);
    
    /// Triggered when the keyboard focus is lost
    virtual bool OnLostFocus();

    /// Triggered when the keyboard focus is gained
    virtual bool OnGainFocus();
};

#endif

