 #ifndef __AWS_COMPONENT_H__
 #define __AWS_COMPONENT_H__
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
#include "iaws/iaws.h"
#include "iaws/iawsdefs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csgeom/csrect.h"
#include "awsprefs.h"
#include "awsslot.h"

class awsCanvas;

SCF_VERSION (awsComponent, 0, 0, 1);

/**************************************************************************************************************************
*   The general idea for a component's initialization stage is like this:                                                 *
*       1. construction - any internal structures should be created and intialized.                                       *
*       2. setup - the window manager calls Setup() on the component being created when it is added into the window       *
*       manager's hierarchy.  During setup the component should get a handle to the preference manager (via GetPrefMgr()) *
*       and ask the preference manager about things like it's default texture, colors, etc; using the Lookup* functions.  *
*       The component can then get texture handles from the preference manager using the values of the keys that it has   *
*       looked up.
*           e.g. 
*            
*            iAwsPrefManager *pm = wmgr->GetPrefMgr();
*            
*            pm->LookupStringKey("Texture", texturename);
*            
*            SetTexture(pm->GetPixmapFor(texturename));
*                                                                                                                   
*                                                                              
*                                                                                                                         *
**************************************************************************************************************************/
class awsComponent : public iAwsComponent
{
   /// The stored handle to the window manager, in case a component needs it. 
   iAws  *wmgr;

   /// The stored handle to the window 
   iAwsWindow *win;

   /// The stored handle to the parent
   iAwsComponent *parent;

   /// The rectangle marking the frame of this component
   csRect frame;

   /// This points to a vector if this component has children
   csBasicVector *children;

   /// Every component will have a name, which is translated to an id
   unsigned long id;

   /// 32 bits worth of flags, like hidden, transparent, etc.
   unsigned int flags;

   /// Embedded awsSource
   awsSource signalsrc;
   
public:
    SCF_DECLARE_IBASE;

    awsComponent();
    virtual ~awsComponent();

public:
    /// Registers a slot for a signal
    virtual bool RegisterSlot(iAwsSlot *slot, unsigned long signal);

    /// Unregisters a slot for a signal.
    virtual bool UnregisterSlot(iAwsSlot *slot, unsigned long signal);

    /// Broadcasts a signal to all slots that are interested.
    virtual void Broadcast(unsigned long signal);

    /// Returns component that this belongs to.
    virtual iAwsComponent *GetComponent();
         
public:
    /**
     *  This is the function that components use to set themselves up.  All components MUST implement this function.  
     *  You should also call awsComponent::Setup() so that it can perform some default initialization work.
     */
    virtual bool Setup(iAws *wmgr, awsComponentNode *settings);
   
    /// Event dispatcher, demultiplexes events and sends them off to the proper event handler
    virtual bool HandleEvent(iEvent& Event);

    /// Gets the property specified, setting the parameter to a COPY of the property's value. Returns false if there's no such property.
    virtual bool GetProperty(char *name, void **parm);

    /// Sets the property specified, setting the proprty to whatever is in parm. Returns false if there's no such property.
    virtual bool SetProperty(char *name, void *parm);

    /// Executes a scriptable action
    virtual bool Execute(char *action, awsParmList &parmlist);

    /// Invalidation routine: allow the component to be redrawn when you call this
    virtual void Invalidate();

    /// Invalidation routine: allow component to be redrawn, but only part of it
    virtual void Invalidate(csRect area);

    /// Get this component's frame
    virtual csRect& Frame();

    /// Returns the named TYPE of the component, like "Radio Button", etc.
    virtual char *Type();

    /// Sets the flag (can handle multiple simultaneous sets)
    virtual void SetFlag(unsigned int flag);

    /// Clears the flag (can handle multiple simultaneous clears)
    virtual void ClearFlag(unsigned int flag);

    /// Returns the current state of the flags
    virtual unsigned int Flags();

    /// Returns true if this window overlaps the given rect.
    virtual bool Overlaps(csRect &r);

    /// Returns the state of the hidden flag
    virtual bool isHidden();
    
    /// Hides a component
    virtual void Hide();

    /// Shows a component
    virtual void Show();

    /// Get's the unique id of this component.
    virtual unsigned long GetID();
    
    /// Set's the unique id of this component. Note: only to be used by window manager.
    virtual void SetID(unsigned long _id);

    /// Recursively moves children (and all nested children) by relative amount given.
    virtual void MoveChildren(int delta_x, int delta_y);

public:
    /** Adds a child into this component.  It's frame should be respective this component, not absolute.
     * This component can grab a reference to the child and dispose of it when it destructs, unless you
     * call RemoveChild() beforehand.  If you set owner to false, this component will call IncRef, otherwise
     * it will assume control of the child and not IncRef.  The difference is that, if owner is false the
     * child component will NOT be destroyed on destruction of this component, or on call of RemoveChild().
     */
    virtual void AddChild(iAwsComponent* child, bool owner=true);

    /** Removes a child from this component.  Important!! The child will be destroyed automatically if owner
     *  was true when you called AddChild().
     */
    virtual void RemoveChild(iAwsComponent *child);

    /// Get's the number of children
    virtual int GetChildCount();

    /// Get's a specific child
    virtual iAwsComponent *GetChildAt(int i);
    
    /// Returns true if this component has children
    virtual bool HasChildren();

    /** Get's this components idea of the window manager.  
      * Should be used internally by the component ONLY,
      * or by embedding classes. */
    iAws *WindowManager();

    /// Get's the window that this component resides in.
    virtual iAwsWindow *Window();

    /// Get's the parent component of this component;
    virtual iAwsComponent *Parent();
    
    /// Sets the window that this component resides in.
    virtual void SetWindow(iAwsWindow *win);

    /// Sets the parent component of this component;
    virtual void SetParent(iAwsComponent *parent);
    
        
public:
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

class awsComponentFactory : public iAwsComponentFactory
{
  iAws *wmgr;
public:
    /// Calls register to register the component that it builds with the window manager
    awsComponentFactory(iAws *_wmgr);

    /// Does nothing
    virtual ~awsComponentFactory();

    /// Returns the current window manager
    iAws *WindowManager() { return wmgr; }

    /// Returns a newly created component of the type this factory handles. 
    virtual iAwsComponent *Create()=0;

    /// Registers this factory with the window manager
    void Register(char *type);

    /// Registers constants for the parser so that we can construct right.
    void RegisterConstant(char *name, int value);
};

#endif

