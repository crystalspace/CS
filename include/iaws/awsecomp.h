/**************************************************************************
    Copyright (C) 2001 by Christopher Nelson

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

#ifndef __CS_AWS_EMBEDDED_COMPONENT_H__
#define __CS_AWS_EMBEDDED_COMPONENT_H__

#include "csutil/ref.h"
#include "iaws/aws.h"
#include "iaws/awsparm.h"
#include "iutil/event.h"
#include "iaws/awsdefs.h"
#include "csutil/event.h"

/**
 * Class used to create new AWS components.<p>
 * To create component from scratch, you need to subclass from
 * awsEmbeddedComponent.
 * After instantiating from that object, you need to call
 * Initialize() with a component gotten from
 * iAWS::CreateEmbeddableComponent().  After writing the code for that
 * component, you need to subclass from awsEmbeddedComponentFactory.  Do
 * your constant registrations, etc.  At runtime you simply need to
 * instantiate the derived factory, which handles registration and creation
 * for you. The instantiation of the component needs to happen
 * in the Factory in a function called Create().  
 */
class awsEmbeddedComponent : public iAwsComponent
{
  csRef<iAwsComponent> comp;

public:
  awsEmbeddedComponent() { }
  virtual ~awsEmbeddedComponent() { }

public:
  /// Gets the component owner for this (sources are embedded)
  virtual iAwsComponent *GetComponent ()
  { return this; }

  /// Registers a slot for a signal
  virtual bool RegisterSlot (iAwsSlot *slot, unsigned long signal)
  { return comp->RegisterSlot (slot, signal); }

  /// Unregisters a slot for a signal.
  virtual bool UnregisterSlot (iAwsSlot *slot, unsigned long signal)
  { return comp->UnregisterSlot (slot, signal); }

  /// Broadcasts a signal to all slots that are interested.
  virtual void Broadcast (unsigned long signal)
  { comp->Broadcast (signal); }

public:
  /// Sets the embedded component.  MUST BE CALLED BEFORE ANY OTHER FUNCTION!
  virtual void Initialize (iAwsComponent *component)
  { 
    comp = component; 
  }

  /**
   * This function takes care of the creation tasks required to prepare this
   * component for use. If you create a component via the window manager's
   * creation functions then you should not call this, the window manager has
   * done it for you. If you create components programatically then you are
   * encouraged to call this func to make setup easier. For component
   * developers, you should not need to override Create but rather do your
   * setup work in Setup. 
   * <p>
   * If it returns false then the component was not able to initialize
   * properly and shouldn't be used.
   * <p>
   * Reproducing the create code here is a temporary fix
   * until I can find a better solution. Currently you can
   * not properly embed the menu and popupMenu components like
   * this.
   */
  virtual bool Create (iAws *m, iAwsComponent *parent,
  	iAwsComponentNode *settings)
  {
    return comp->Create(m, parent, settings);
  }
    
  /// Sets up component
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings)
  { return comp->Setup (wmgr, settings); }
    
  /**
   * Event dispatcher, demultiplexes events and sends them off to the proper
   * event handler
   */
  virtual bool HandleEvent(iEvent& Event)
  {
    return comp->HandleEvent(Event);
  }
 
  /**
   * Gets the property specified, setting the parameter to a COPY of
   * the property's value. Returns false if there's no such property.
   */
  virtual bool GetProperty (const char *name, intptr_t *parm)
  { return comp->GetProperty (name, parm); }

#ifndef AWS_VOIDP_IS_ERROR
  /**
   * Gets a copy of the property, put it in parm.  Returns false if the
   * property does not exist.
   * \deprecated For proper 64-bit platform support, use the intptr_t version
   *   of GetProperty().
   */
  AWS_VOIDP_IS_WARNING
  virtual bool GetProperty(const char* name, void **parm)
  { return comp->GetProperty (name, (intptr_t*)parm); }
#endif

  /**
   * Sets the property specified, setting the proprty to whatever is in
   * parm. Returns false if there's no such property.
   */
  virtual bool SetProperty (const char *name, intptr_t parm)
  { return comp->SetProperty (name, parm); }

#ifndef AWS_VOIDP_IS_ERROR
  /**
   * Sets the property specified to whatever is in parm. Returns false if
   * there's no such property.
   * \deprecated For proper 64-bit platform support, use the intptr_t version
   *   of SetProperty().
   */
  AWS_VOIDP_IS_WARNING
  virtual bool SetProperty(const char* name, void *parm)
  { return comp->SetProperty (name, (intptr_t)parm); }
#endif

  /// Executes scriptable actions for this window
  virtual bool Execute (const char *action, iAwsParmList* parmlist)
  { return comp->Execute (action, parmlist); }

  /// Sets the flag (can handle multiple simultaneous sets)
  virtual void SetFlag (unsigned int flag)
  { comp->SetFlag (flag); }

  /// Clears the flag (can handle multiple simultaneous clears)
  virtual void ClearFlag (unsigned int flag)
  { comp->ClearFlag (flag); }

  /// Returns the current state of the flags
  virtual unsigned int Flags ()
  { return comp->Flags (); }

  /// Invalidation routine: allow the component to be redrawn when you call this
  virtual void Invalidate ()
  { comp->Invalidate (); }

  /// Invalidation routine: allow component to be redrawn, but only part of it
  virtual void Invalidate (csRect area)
  { comp->Invalidate (area); }

  /// Get this component's frame
  virtual csRect Frame ()
  { return comp->Frame (); }

  /// Get this component's client frame
  virtual csRect ClientFrame ()
  { return comp->ClientFrame (); }

  /**
   * Returns the named TYPE of the component, like "Radio Button", etc.
   * This should always be overridden.
   */
  virtual const char *Type ()
  { return comp->Type (); }

  /// Returns true if this window overlaps the given rect.
  virtual bool Overlaps (csRect &r)
  { return comp->Overlaps (r); }

  /// Returns the state of the hidden flag
  virtual bool isHidden ()
  { return comp->isHidden (); }

  /// Hides a component
  virtual void Hide ()
  { comp->Hide (); }

  /// Shows a component
  virtual void Show ()
  { comp->Show (); }

  /// Moves a component
  virtual void Move (int delta_x, int delta_y)
  { comp->Move (delta_x, delta_y); }

  /// Moves a component to an absolute location
  virtual void MoveTo (int x, int y)
  { comp->MoveTo (x, y); }

  /// Resizes a component
  virtual void Resize (int width, int height)
  { comp->Resize (width, height); }

  /// Resizes a component to an absolute rect
  virtual void ResizeTo (csRect newFrame)
  { comp->ResizeTo (newFrame); }

  /// Get's the unique id of this component.
  virtual unsigned long GetID ()
  { return comp->GetID (); }

  /**
   * Set's the unique id of this component. Note: only to be used by
   * window manager.
   */
  virtual void SetID (unsigned long id)
  { comp->SetID (id); }

  virtual iAwsComponent* FindChild (const char* name)
  { return comp->FindChild (name); }

  virtual iAwsComponent* DoFindChild (unsigned int id)
  { return comp->DoFindChild (id); }

  virtual bool IsMaximized()
  { return comp->IsMaximized(); }

  virtual void Maximize()
  { comp->Maximize(); }

  virtual void UnMaximize()
  { comp->UnMaximize(); }

  /// Resizes all the children of this component using the current layout
  virtual void LayoutChildren ()
  { comp->LayoutChildren (); }

  virtual void AddToLayout (iAwsComponent *cmp, iAwsComponentNode* settings)
  { comp->AddToLayout (cmp, settings); }

  /// Set component can focus
  virtual void SetFocusable (bool focusable)
  { comp->SetFocusable (focusable); }

  /// Returns component can focus
  virtual bool Focusable ()
  { return comp->Focusable (); }

  /// Return component is focused
  virtual bool isFocused ()
  { return comp->isFocused (); }

  /// Sets component focused
  virtual void SetFocus ()
  { comp->SetFocus (); }

  /// Unsets component focused
  virtual void UnsetFocus ()
  { comp->UnsetFocus (); }

  /// Add child to parent tab order
  virtual bool AddToTabOrder (iAwsComponent *cmp)
  { return comp->AddToTabOrder (cmp); }

  /// Returns next component in tab order
  virtual iAwsComponent *TabNext (iAwsComponent *cmp)
  { return comp->TabNext (cmp); }

  /// Returns previous component in tab order
  virtual iAwsComponent *TabPrev (iAwsComponent *cmp)
  { return comp->TabPrev (cmp); }

  /// Returns tab order length
  virtual int GetTabLength ()
  { return comp->GetTabLength (); }

  /// Return component by tabindex
  virtual iAwsComponent *GetTabComponent (int index)
  { return comp->GetTabComponent (index); }

  /// Returns first focusable component on this component
  virtual iAwsComponent *GetFirstFocusableChild (iAwsComponent *comp)
  { return comp->GetFirstFocusableChild (comp); }

  /// Adds a child
  virtual void AddChild(iAwsComponent* child)
  { comp->AddChild(child); }

  /// Removes a child
  virtual void RemoveChild(iAwsComponent *child)
  { comp->RemoveChild(child); }

  /// Get's the number of children
  virtual int GetChildCount()
  { return comp->GetChildCount(); }

  /// Get's a specific child
  virtual iAwsComponent *GetTopChild()
  { return comp->GetTopChild(); }

  /// Returns true if this component has children
  virtual bool HasChildren()
  { return comp->HasChildren(); }

  virtual iAwsComponent *ChildAt(int x, int y)
  { return comp->ChildAt(x,y); }

  /**
   * Get's this components idea of the window manager.
   * Should be used internally by the component ONLY,
   * or by embedding classes.
   */
  iAws *WindowManager()
  { return comp->Window()->WindowManager(); }

  /// Get's the window that this component resides in.
  iAwsComponent *Window()
  { return comp->Window(); }

  /// Get's the parent component of this component;
  iAwsComponent *Parent()
  { return comp->Parent(); }

  /// Sets the parent component of this component;
  virtual void SetParent(iAwsComponent *parent)
  { comp->SetParent(parent); }

  /// Get's the component above this one, 0 if there is none.
  virtual iAwsComponent *ComponentAbove()
  { return comp->ComponentAbove(); }

  /// Get's the component below this one, 0 if there is none.
  virtual iAwsComponent *ComponentBelow()
  { return comp->ComponentBelow(); }

  /// Set's the component above this one
  virtual void SetComponentAbove(iAwsComponent *cmp)
  { comp->SetComponentAbove(cmp); }

  /// Set's the component below this one
  virtual void SetComponentBelow(iAwsComponent *cmp)
  { comp->SetComponentBelow(cmp); }

  /// Moves this component above all its siblings
  virtual void Raise()
  { comp->Raise (); }

  /// Moves this component below all its siblings
  virtual void Lower()
  { comp->Lower (); }

  /// Sets the value of the redraw tag
  virtual void SetRedrawTag(unsigned int tag)
  { comp->SetRedrawTag (tag); }

  /// Gets the value of the redraw tag
  virtual unsigned int RedrawTag()
  { return comp->RedrawTag (); }

  /// Triggered when the component needs to draw
  virtual void OnDraw(csRect clip)
  { comp->OnDraw (clip); }

  /// Triggered when the user presses a mouse button down
  virtual bool OnMouseDown(int button, int x, int y)
  { return comp->OnMouseDown (button, x, y); }

  /// Triggered when the user unpresses a mouse button
  virtual bool OnMouseUp(int button, int x, int y)
  { return comp->OnMouseUp (button, x, y); }

  /// Triggered when the user moves the mouse
  virtual bool OnMouseMove(int button, int x, int y)
  { return comp->OnMouseMove (button, x, y); }

  /// Triggered when the user clicks the mouse
  virtual bool OnMouseClick(int button, int x, int y)
  { return comp->OnMouseClick (button, x, y); }

  /// Triggered when the user double clicks the mouse
  virtual bool OnMouseDoubleClick(int button, int x, int y)
  { return comp->OnMouseDoubleClick (button, x, y); }

  /// Triggered when this component loses mouse focus
  virtual bool OnMouseExit()
  { return comp->OnMouseExit (); }

  /// Triggered when this component gains mouse focus
  virtual bool OnMouseEnter()
  { return comp->OnMouseEnter (); }

  /// Triggered when the user presses a key
  virtual bool OnKeyboard (const csKeyEventData& eventData)
  { return comp->OnKeyboard (eventData); }

  /// Triggered when the keyboard focus is lost
  virtual bool OnLostFocus()
  { return comp->OnLostFocus (); }

  /// Triggered when the keyboard focus is gained
  virtual bool OnGainFocus()
  { return comp->OnGainFocus (); }

  /// Gets the layout manager for this component.
  virtual iAwsLayoutManager *Layout()
  { return comp->Layout ();}

  /// Set the layout manager
  virtual void SetLayout(iAwsLayoutManager *layoutMgr)
  { comp->SetLayout(layoutMgr); }

  /// get the components preferred size, used by layout manager
  virtual csRect getPreferredSize()
  { return comp->getPreferredSize (); }

  /// Set the preferred size of the component.
  virtual void setPreferredSize (const csRect& size)
  { comp->setPreferredSize(size); }

  /// Clear the manually set preferred size.
  virtual void clearPreferredSize ()
  { comp->clearPreferredSize(); }

  /// get the components minimal size, used by layout manager
  virtual csRect getMinimumSize()
  { return comp->getMinimumSize (); }

  /// get the components insets, used by layout manager
  virtual csRect getInsets()
  { return comp->getInsets (); }

  /// does the component listen to events ?
  virtual bool isDeaf()
  { return comp->isDeaf (); }

  /// let the component listen to events or not
  virtual void SetDeaf (bool isDeaf)
  { comp->SetDeaf (isDeaf); }

  /// Triggered at the start of each frame
  virtual bool OnFrame()
  { return comp->OnFrame ();}

  /// Triggered when a child is added to the parent (triggered on the child)
  virtual void OnAdded()
  { comp->OnAdded ();}

  /// Triggered when a component is resized by the layout manager.
  virtual void OnResized()
  { comp->OnResized ();}

  /// Triggered when a child component has been moved
  virtual void OnChildMoved()
  { comp->OnChildMoved(); }

  /// Triggered when the Raise function is called
  virtual void OnRaise()
  { comp->OnRaise(); }

  /// Triggered when the Lower function is called
  virtual void OnLower()
  { comp->OnLower(); }

  /// Triggered when a child becomes hidden
  virtual void OnChildHide()
  { comp->OnChildHide(); }

  /// Triggered when a child becomes shown
  virtual void OnChildShow()
  { comp->OnChildShow(); }

  /// Removes a component from the hierarchy
  virtual void Unlink()
  { comp->Unlink(); }

  /// Links a component into the hierarchy as a sibling below comp
  void LinkBelow (iAwsComponent *c)
  { comp->LinkBelow(c); }

  /// Links a component into the hierarchy as a sibling above comp
  void LinkAbove (iAwsComponent *c)
  { comp->LinkAbove(c); }

  /// Sets the top child
  virtual void SetTopChild(iAwsComponent* child)
  { comp->SetTopChild(child); }

  /// Triggered when a child becomes focused
  virtual void OnSetFocus()
  { comp->OnSetFocus(); }

  /// Triggered when a child looses focus
  virtual void OnUnsetFocus()
  { comp->OnUnsetFocus(); }
};

/**
 * Factory for custom AWS component. See also awsEmbeddedComponent.
 */
class awsEmbeddedComponentFactory : public iAwsComponentFactory
{
protected:
  iAws *wmgr;

public:
  /**
   * Calls register to register the component that it builds with the
   * window manager.
   */
  awsEmbeddedComponentFactory(iAws *_wmgr)
  {
    wmgr = _wmgr;
  }

  /// Does nothing
  virtual ~awsEmbeddedComponentFactory()
  {
  }

  /// Returns the current window manager
  iAws *WindowManager() { return wmgr; }

  /// Registers this factory with the window manager
  virtual void Register(const char *type)
  {
    wmgr->RegisterComponentFactory(this, type);
  }

  /// Registers constants for the parser so that we can construct right.
  virtual void RegisterConstant(const char *name, int value)
  {
    wmgr->GetPrefMgr()->RegisterConstant(name, value);
  }
};

#endif // __CS_AWS_EMBEDDED_COMPONENT_H__

