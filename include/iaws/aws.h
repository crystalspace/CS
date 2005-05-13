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

#ifndef __CS_IAWS_AWS_H__
#define __CS_IAWS_AWS_H__

/**\file 
 * Advanced Windowing System
 */

#include "csutil/scf.h"
#include "csutil/refarr.h"
#include "csutil/stringarray.h"
#include "csgeom/csrect.h"
#include "csgeom/cspoint.h"
#include "iutil/event.h"
#include "iutil/string.h"

struct iAws;
struct iAwsCanvas;
struct iAwsComponent;
struct iAwsComponentFactory;
struct iAwsConnectionNodeFactory;
struct iAwsKeyFactory;
struct iAwsParmList;
struct iAwsPrefManager;
struct iAwsSink;
struct iAwsSinkManager;
struct iAwsSlot;
struct iAwsSource;

typedef iAwsComponent iAwsWindow;

class awsWindow;
class awsComponent;
class awsComponentNode;
class awsConnectionNode;
class awsComponentFactory;
class awsLayoutManager;

struct iEngine;
struct iEvent;
struct iFont;
struct iFontServer;
struct iGraphics2D;
struct iGraphics3D;
struct iObjectRegistry;
struct iStringSet;
struct iTextureHandle;
struct iTextureManager;
struct iView;

const bool aws_debug = false;  // set to true to turn on debugging printf()'s

// Use of (void*) to store opaque data is now discouraged in order to properly
// support 64-bit platforms. For backward compatibility with existing client
// software, we can still provide (void*) API in addition to the new (intptr_t)
// API. Ideally, clients should convert away from (void*) usage, but doing so
// may be a large task, so we enable support for the deprecated API for now.
// In order to aid clients in the conversion, we provide two controls. If
// AWS_OBSOLETE_VOIDP is defined by the client, then all methods dealing with
// (void*) will be removed completely from the API. If AWS_DEPRECATE_VOIDP is
// defined, then invocations of methods dealing with (void*) will emit warnings
// (from compilers which support deprecation warnings).
#ifdef AWS_OBSOLETE_VOIDP
#define AWS_VOIDP_IS_ERROR
#endif
#ifdef AWS_DEPRECATE_VOIDP
#define AWS_VOIDP_IS_WARNING CS_DEPRECATED_METHOD
#else
#define AWS_VOIDP_IS_WARNING
#endif

/**
 * \addtogroup aws_sys_flags
 * @{ */

/**
 * This flag makes the windowing system perform erases before drawing.
 * This slows the engine down somewhat, but is necessary in some circumstances
 * (such as when using the single proctex mode as a surface, or to draw to the
 * high level visible context.)
 */
const int AWSF_AlwaysEraseWindows = 1;

/**
 * This flag makes the windowing system redraw every time, which is necessary
 * when drawing to the screen context since this gets erased every frame by
 * the engine. Note that this flag is NOT necessary if the engine will not be
 * drawing to the background with AWS.  In other words, if AWS has complete
 * control of the screen context.
 */
const int AWSF_AlwaysRedrawWindows = 2;

/**
 * This flag makes windows come to the front solely by moving the mouse over
 * them. If it is disabled then windows will only come to the front by having
 * the mouse clicked in them.
*/
const int AWSF_RaiseOnMouseOver = 4;

/**
 * This flag allows focusing controls using TAB (CTR + TAB) key, calling
 * Click events using ENTER key and some other features that are not finished
 * yet.
 */
const int AWSF_KeyboardControl = 8;

/** @} */

SCF_VERSION(iAwsKey, 0, 0, 1);

/// Document me!@@@
struct iAwsKey : public iBase
{
  /// returns the type of key
  virtual uint8 Type () const = 0;

  /// Accessor function gets name of key
  virtual unsigned long Name () const = 0;
};

SCF_VERSION(iAwsIntKey, 0, 0, 1);

/// Document me!@@@
struct iAwsIntKey : public iAwsKey
{
  /// Gets the value of this key as an integer
  virtual int Value () const = 0;
};

SCF_VERSION(iAwsFloatKey, 0, 0, 1);

/// Document me!@@@
struct iAwsFloatKey : public iAwsKey
{
  /// Gets the value of this key as a float
  virtual float Value () const = 0;
};

SCF_VERSION(iAwsStringKey, 0, 0, 1);

/// Document me!@@@
struct iAwsStringKey : public iAwsKey
{
  /// Gets the value of this key as an iString
  virtual iString* Value () const = 0;
};

SCF_VERSION(iAwsRectKey, 0, 0, 1);

/// Document me!@@@
struct iAwsRectKey : public iAwsKey
{
  /// Gets the value of this key as a rectangle
  virtual csRect Value () const = 0;
};

SCF_VERSION(iAwsRGBKey, 0, 0, 1);

/// Document me!@@@
struct iAwsRGBKey : public iAwsKey
{
  struct RGB
  { 
    unsigned char red, green, blue;
  };
  
  /// Gets the value of this key as an rgb
  virtual const iAwsRGBKey::RGB &Value() const = 0;
};

SCF_VERSION(iAwsPointKey, 0, 0, 1);

/// Document me!@@@
struct iAwsPointKey : public iAwsKey
{
  /// Gets the value of this key as a point
  virtual csPoint Value () const = 0;
};

SCF_VERSION(iAwsConnectionKey, 0, 0, 1);

/// Document me!@@@
struct iAwsConnectionKey : public iAwsKey
{
  /// Gets the sink for this key
  virtual iAwsSink *Sink () const = 0;

  /// Gets the trigger for this key
  virtual unsigned long Trigger () const = 0;

  /// Gets the signal for this key
  virtual unsigned long Signal () const = 0;
};

SCF_VERSION(iAwsKeyContainer, 0, 0, 1);

/// Document me!@@@
struct iAwsKeyContainer : public iAwsKey
{
  /// Looks up a key based on it's name.
  virtual iAwsKey* Find (iString* name) const = 0;

  /// Looks up a key based on it's name.
  virtual iAwsKey* Find (const char* name) const = 0;

  /// Looks up a key based on it's ID.
  virtual iAwsKey *Find (unsigned long id) const = 0;

  virtual const csRefArray<iAwsKey> &Children () const = 0;

  /// Adds an item to the container
  virtual void Add (iAwsKey *key) = 0;

  /// returns children number i
  virtual iAwsKey* GetAt (int i) const = 0;

  /// returns number of childrens
  virtual int Length () const = 0;
    
  /// Removes an item from the container
  virtual void Remove (iString *name) = 0;
  /// Removes an item from the container
  virtual void Remove (const char* name) = 0;
  /// Removes a specific item from the container
  virtual void Remove (iAwsKey *key) = 0;
  /// Removes all items from the container
  virtual void RemoveAll () = 0;

  /**
   * Consumes an entire list by moving all of it's member's to this one,
   * and removing them from it.
   */
  virtual void Consume (iAwsKeyContainer *c) = 0;
};

SCF_VERSION(iAwsComponentNode, 0, 0, 1);

/// Document me!@@@
struct iAwsComponentNode : public iAwsKeyContainer
{
  /// So that we can find out what sort of component type this should be
  virtual iString *ComponentTypeName () const = 0;
};


/**
 * \addtogroup aws
 * @{ */

SCF_VERSION (iAws, 0, 2, 4);

/// Interface for the window manager.
struct iAws : public iBase
{
public:
  /// Get a pointer to the preference manager
  virtual iAwsPrefManager *GetPrefMgr()=0;

  /// Get a pointer to the sink manager
  virtual iAwsSinkManager *GetSinkMgr()=0;

  /// Set the preference manager used by the window system
  virtual void SetPrefMgr(iAwsPrefManager *pmgr)=0;

  /// Get the shared string table.
  virtual iStringSet* GetStringTable()=0;

  /**
   * Allows a component to register itself for dynamic template instatiation
   * via definition files.
   */
  virtual void RegisterComponentFactory(iAwsComponentFactory *factory,
    const char* name)=0;

  /// Find a component factory
  virtual iAwsComponentFactory *FindComponentFactory (const char* name)=0;

  /// Get the top component
  virtual iAwsComponent *GetTopComponent()=0;

  /// Set the top component
  virtual void SetTopComponent(iAwsComponent *win)=0;

  /// Get the focused component
  virtual iAwsComponent *GetFocusedComponent()=0;

  /// Set the focused component
  virtual void SetFocusedComponent(iAwsComponent * _focused)=0;

  /// Get the component with the keyboard focus
  virtual iAwsComponent *GetKeyboardFocusedComponent()=0;

  /// Finds the smallest visible component which contains the point (x,y)
  virtual iAwsComponent* ComponentAt(int x, int y)=0;

  /// Returns true if the mouse is inside any of the top-level components.
  virtual bool MouseInComponent(int x, int y)=0;

  /**
   * Causes the current view of the window system to be drawn to the
   * given graphics device.
   */
  virtual void Print(iGraphics3D *g3d, uint8 Alpha=0)=0;

  /// Redraw whatever portions of the screen need it.
  virtual void Redraw()=0;

  /// Mark a region dirty
  virtual void Mark(const csRect &rect)=0;

  /// Mark a section of the screen clean.
  virtual void Unmark(const csRect &rect)=0;

  /**
   * Erase a section of the screen next round (only useful if
   * AlwaysEraseWindows flag is set)
   */
  virtual void Erase(const csRect &rect)=0;

  /**
   * Mask off a section that has been marked to erase.  This part won't be
   * erased.
   */
  virtual void MaskEraser(const csRect &rect)=0;

  /// Tell the system to rebuild the update store
  virtual void InvalidateUpdateStore()=0;

  /**
   * Capture all mouse events until release is called, no matter where the
   * mouse is
   */
  virtual void CaptureMouse(iAwsComponent *comp)=0;

  /// Release the mouse events to go where they normally would.
  virtual void ReleaseMouse()=0;

  /// Set this compoment to be a modal dialog.
  virtual void SetModal(iAwsComponent *comp)=0;

  /// Set no active modal dialog.
  virtual void UnSetModal()=0;

  /// Dispatches events to the proper components
  virtual bool HandleEvent(iEvent&)=0;

  /**
   * Sets up the canvas to draw on.
   * @param newCanvas The canvas to draw on.  If this parameter is 0, then
   *        g2d and g3d MUST be present.  AWS will use them to create a
   *        default, direct to screen canvas. 
   * @param g2d A pointer to a valid iGraphics2D instance. (If newCanvas is
   *        NOT null, this param may be ommitted.)
   * @param g3d A pointer to a valid iGraphics3D instance. (If newCanvas is
   *        NOT null, this param may be ommitted.)
   * @return True if everything works, False otherwise.  If this function
   *        returns False AWS will NOT work.
   */
  virtual bool SetupCanvas (iAwsCanvas *newCanvas, iGraphics2D *g2d=0,
  	iGraphics3D *g3d=0)=0;

  /// Get the current context
  virtual iAwsCanvas* GetCanvas()=0;

  /// Create a canvas that uses custom graphics devices
  //virtual iAwsCanvas *CreateCustomCanvas(iGraphics2D*, iGraphics3D*)=0;

  /// Get the iGraphics2D interface so that components can use it.
  virtual iGraphics2D *G2D()=0;

  /// Get the iGraphics3D interface so that components can use it.
  virtual iGraphics3D *G3D()=0;

  /// Instantiates a window based on a window definition.
  virtual iAwsComponent *CreateWindowFrom(const char* defname)=0;

  /// Creates a new embeddable component
  virtual iAwsComponent *CreateEmbeddableComponent(iAwsComponent *covercomp)=0;

  /// Creates a new parameter list
  virtual iAwsParmList *CreateParmList()=0;

  /**
   * Creates and enables a transition for a window.
   * <code>transition_type</code> is one of AWS_TRANSITION_*.
   * \sa \ref aws_window_trans
   */
  virtual void CreateTransition(iAwsComponent *win, unsigned transition_type,
  	csTicks duration=250)=0;

  /**
   * Creates and enables a transition for a window, using a user specified
   * start or finish (transition type defines which).
   * <code>transition_type</code> is one of AWS_TRANSITION_*.
   * \sa \ref aws_window_trans
   */
  virtual void CreateTransitionEx(iAwsComponent *win,
  	unsigned transition_type, csTicks duration, csRect &user)=0;

  /**
   * Sets one or more flags for different operating modes. 
   * <code>flags</code> is a combination of AWSF_*.
   * \sa \ref aws_sys_flags
   */
  virtual void SetFlag(unsigned int flags)=0;

  /**
   * Clears one or more flags for different operating modes.
   * <code>flags</code> is a combination of AWSF_*.
   * \sa \ref aws_sys_flags
   */
  virtual void ClearFlag(unsigned int flags)=0;

  /**
   * Returns the current flags
   * <code>flags</code> is a combination of AWSF_*.
   * \sa \ref aws_sys_flags
   */
  virtual unsigned int GetFlags()=0;
  
  /// Return object registry
  virtual iObjectRegistry *GetObjectRegistry ()=0;

  /// Returns true if all windows are presently hidden
  virtual bool AllWindowsHidden()=0;
  	
  /// Checks if the specified component is currently going through a transition
  virtual bool ComponentIsInTransition(iAwsComponent *win)=0;

  /// Notify the manager about component destruction.
  virtual void ComponentDestroyed(iAwsComponent *comp)=0;

  /// Call this if you want to delete marked components immediately.
  virtual void DeleteMarkedComponents()=0;

  /// Mark the component and its sub-components to be deleted.
  virtual void MarkToDeleteRecursively(iAwsComponent *comp)=0;
};

SCF_VERSION (iAwsPrefManager, 0, 0, 4);

/// Interface for the preferences manager (window manager needs one.)
struct iAwsPrefManager : public iBase
{
public:
  /// Performs whatever initialization is needed
  virtual bool Setup(iObjectRegistry *object_reg)=0;

  /// Invokes the definition parser to load definition files
  virtual bool Load(const char *def_file)=0;

  /// Maps a name to an id
  virtual unsigned long NameToId (const char*name)=0;

  /**
   * Select which skin is the default for components, the skin must be
   * loaded.  True on success, false otherwise.
   */
  virtual bool SelectDefaultSkin (const char* skin_name)=0;

  /// Lookup the value of an int key by name (from the skin def)
  virtual bool LookupIntKey (const char* name, int &val)=0;

  /// Lookup the value of an int key by id (from the skin def)
  virtual bool LookupIntKey(unsigned long id, int &val)=0;

  /// Lookup the value of a string key by name (from the skin def)
  virtual bool LookupStringKey(const char* name, iString *&val)=0;

  /// Lookup the value of a string key by id (from the skin def)
  virtual bool LookupStringKey(unsigned long id, iString *&val)=0;

  /// Lookup the value of a rect key by name (from the skin def)
  virtual bool LookupRectKey(const char* name, csRect &rect)=0;

  /// Lookup the value of a rect key by id (from the skin def)
  virtual bool LookupRectKey(unsigned long id, csRect &rect)=0;

  /// Lookup the value of an RGB key by name (from the skin def)
  virtual bool LookupRGBKey(const char* name, unsigned char &red,
  	unsigned char &green, unsigned char &blue)=0;

  /// Lookup the value of an RGB key by name (from the skin def)
  virtual bool LookupRGBKey(unsigned long id, unsigned char &red,
  	unsigned char &green, unsigned char &blue)=0;

  /// Lookup the value of a point key by name (from the skin def)
  virtual bool LookupPointKey(const char* name, csPoint &point)=0;

  /// Lookup the value of a point key by id (from the skin def)
  virtual bool LookupPointKey(unsigned long id, csPoint &point)=0;

  /// Get the an integer from a given component node
  virtual bool GetInt(iAwsComponentNode *node, const char* name, int &val)=0;

  /// Get the float value from a given component node
  virtual bool GetFloat(iAwsComponentNode *, const char* name, float &val)=0;

  /// Get the a rect from a given component node
  virtual bool GetRect(iAwsComponentNode *node, const char* name,
  	csRect &rect)=0;

  /**
   * Get the value of a string from a given component node.
   * Replaces the contents of \a val.
   */
  virtual bool GetString(iAwsComponentNode *node, const char *name,
	iString* val)=0;


  /// Get the a color from a given component node
  virtual bool GetRGB(iAwsComponentNode *node, const char* name,
  	unsigned char& r, unsigned char& g, unsigned char& b)=0;

  /**
   * Find window definition and return the component node holding it,
   * Null otherwise
   */
  virtual iAwsComponentNode *FindWindowDef(const char* name)=0;

  /// Find skin def and return key container, Null if not found
  virtual iAwsKeyContainer *FindSkinDef(const char* name)=0;

  /// Completely remove a window definition (return false if not found)
  virtual bool RemoveWindowDef (const char *name)=0;

  /// Remove all window definitions
  virtual void RemoveAllWindowDefs ()=0;

  /// Completely remove a skin definition (return false if not found)
  virtual bool RemoveSkinDef (const char *name)=0;

  /// Remove all skin definitions
  virtual void RemoveAllSkinDefs ()=0;

  /// Sets the value of a color in the global AWS palette.
  virtual void SetColor(int index, int color)=0;

  /// Gets the value of a color from the global AWS palette.
  virtual int  GetColor(int index)=0;

  /// Finds the closest matching color
  virtual int FindColor(unsigned char r, unsigned char g, unsigned char b)=0;

  /// Gets the current default font
  virtual iFont *GetDefaultFont()=0;

  /// Gets a font.  If it's not loaded, it will be.  Returns 0 on error.
  virtual iFont *GetFont(const char* filename)=0;

  /// Gets a texture from the global AWS cache
  virtual iTextureHandle *GetTexture(const char* name,
  	const char* filename=0)=0;

  /**
   * Gets a texture from the global AWS cache, if its loaded for the first
   * time then the keycolor (key_r,key_g,key_b) is set.
   */
  virtual iTextureHandle *GetTexture (const char* name, const char* filename, 
                                      unsigned char key_r,
                                      unsigned char key_g,
                                      unsigned char key_b)=0;

  /// Sets the texture manager that the preference manager uses
  virtual void SetTextureManager(iTextureManager *txtmgr)=0;

  /// Sets the font server that the preference manager uses
  virtual void SetFontServer(iFontServer *fntsvr)=0;

  /// Set the default font that the preference manager uses
  virtual void SetDefaultFont(iFont* font)=0;

  /// Sets the window manager that the preference manager uses
  virtual void SetWindowMgr(iAws *wmgr)=0;

  /**
   * Sets up the AWS palette so that the colors are valid reflections of
   * user preferences.  Although SetColor can be used, it's recommended
   * that you do not.  Colors should always be a user preference, and
   * should be read from the window and skin definition files (as
   * happens automatically normally.
   */
  virtual void SetupPalette()=0;

  /// Allows a component to specify it's own constant values for parsing.
  virtual void RegisterConstant(const char* name, int value)=0;

  /// Returns true if the constant has been registered, false otherwise.
  virtual bool ConstantExists(const char* name)=0;

  /**
   * Allows a component to retrieve the value of a constant, or the parser
   * as well.
   */
  virtual int  GetConstantValue(const char* name)=0;

  /// Creates a new key factory
  virtual iAwsKeyFactory *CreateKeyFactory()=0;

  /// Creates a new connection node factory
  virtual iAwsConnectionNodeFactory *CreateConnectionNodeFactory()=0;

  /// Add custom string property to be read for all components.
  virtual void AddCustomStringProperty (const char *prop)=0;

  /// Get all the custom string properties in the manager.
  virtual const csStringArray &GetCustomStringProperties()=0;
};


SCF_VERSION (iAwsSinkManager, 0, 1, 0);

/// Interface for the sink manager
struct iAwsSinkManager : public iBase
{
  /// Performs whatever initialization is needed.
  virtual bool Setup(iObjectRegistry *object_reg)=0;

  /// Registers a sink by name for lookup.
  virtual void RegisterSink(const char *name, iAwsSink *sink)=0;

  /// Remove the indicated sink.
  virtual bool RemoveSink (iAwsSink* sink)=0;

  /// Finds a sink by name for connection.
  virtual iAwsSink* FindSink(const char *name)=0;

  /**
   * Create a new embeddable sink, with parm as the intptr_t passed into the
   * triggers.
   */
  virtual iAwsSink *CreateSink(intptr_t parm)=0;

#ifndef AWS_VOIDP_IS_ERROR
  /**
   * Create a new embeddable sink, with parm as the void* passed into the
   * triggers.
   * \deprecated For proper 64-bit platform support, use the intptr_t version
   *   of CreateSink().
   */
  AWS_VOIDP_IS_WARNING
  virtual iAwsSink *CreateSink(void *parm)=0;
#endif

  /// Create a new embeddable slot
  virtual iAwsSlot *CreateSlot ()=0;
};


SCF_VERSION (iAwsSink, 0, 1, 0);

/// Interface for sinks
struct iAwsSink : public iBase
{
  /// Maps a trigger name to a trigger id
  virtual unsigned long GetTriggerID(const char *name)=0;

  /// Handles trigger events
  virtual void HandleTrigger(int trigger_id, iAwsSource *source)=0;

  /// A sink should call this to register trigger events
  virtual void RegisterTrigger(const char *name,
  	void (*Trigger)(intptr_t, iAwsSource *))=0;

#ifndef AWS_VOIDP_IS_ERROR
  /**
   * A sink should call this to register trigger events
   * \deprecated For proper 64-bit platform support, use the intptr_t version
   *   of RegisterTrigger().
   */
  AWS_VOIDP_IS_WARNING
  virtual void RegisterTrigger(const char *name,
  	void (*Trigger)(void *, iAwsSource *))=0;
#endif

  /**
   * Returns the last error code set.  This code is good until the next
   * call to this sink.
   * Return value is one of AWS_ERR_SINK_*.
   * \sa \ref aws_sink_errors
   */
  virtual unsigned int GetError()=0;
};


SCF_VERSION (iAwsSource, 0, 0, 1);

/// Interface for signal sources
struct iAwsSource : public iBase
{
  /// Gets the component owner for this (sources are embedded)
  virtual iAwsComponent *GetComponent()=0;

  /**
   * Registers a slot for any one of the signals defined by a source.
   * Each sources's signals exist in it's own namespace
   */
  virtual bool RegisterSlot(iAwsSlot *slot, unsigned long signal)=0;

  /// Unregisters a slot for a signal.
  virtual bool UnregisterSlot(iAwsSlot *slot, unsigned long signal)=0;

  /// Broadcasts a signal to all slots that are interested.
  virtual void Broadcast(unsigned long signal)=0;
};


SCF_VERSION (iAwsSlot, 0, 0, 1);

/// Interface for signal slots (conduits)
struct iAwsSlot : public iBase
{
  /**
   * Connect sets us up to receive signals from some other component.
   * You can connect to as many different sources and signals as you'd like.
   * You may connect to multiple signals from the same source.
   */
  virtual void Connect(iAwsSource *source, unsigned long signal,
  	iAwsSink *sink, unsigned long trigger)=0;

  /**
   * Disconnects us from the specified source and signal.  This may happen
   * automatically if the signal source goes away.  You will receive
   * disconnect notification always (even if you request the disconnection.)
   */
  virtual void Disconnect(iAwsSource *source, unsigned long signal,
  	iAwsSink *sink, unsigned long trigger)=0;

  /**
   * Invoked by a source to emit the signal to this slot's sink.
   */
  virtual void Emit(iAwsSource &source, unsigned long signal)=0;
};

SCF_VERSION(iAwsLayoutManager, 0, 0, 1);

/// Document me!@@@
struct iAwsLayoutManager : public iBase
{
  /**
   * Sets the owner.  Normally the owner should never change, but in some rare
   * cases (like in the Window class) the owner is set improperly by the setup
   * code and must be fixed by the embedder.  This should ALWAYS be used by
   * widgets which embed the component and use delegate wrappers (i.e.
   * awsecomponent)
   */
  virtual void SetOwner (iAwsComponent *_owner) = 0;

  /**
   * Adds a component to the layout, returning it's actual rect. 
   */
  virtual csRect AddComponent (iAwsComponent *cmp,
  	iAwsComponentNode* settings) = 0;

  /// Removes a component from the layout
  virtual void RemoveComponent(iAwsComponent* ) = 0;

  /// Lays out components properly
  virtual void LayoutComponents () = 0;
};

SCF_VERSION (iAwsComponent, 0, 2, 0);

/// Interface that is the base of ALL components.
struct iAwsComponent : public iAwsSource
{
  
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
   */
  virtual bool Create(iAws* mgr, iAwsComponent* parent,
  	iAwsComponentNode* settings)=0;

  /// Sets up a component.
  virtual bool Setup(iAws *wmgr, iAwsComponentNode *settings)=0;

  /**
   * Event dispatcher, demultiplexes events and sends them off to the proper
   * event handler.
   */
  virtual bool HandleEvent(iEvent& Event)=0;

  /**
   * Gets a copy of the property, put it in parm.  Returns false if the
   * property does not exist.
   */
  virtual bool GetProperty(const char* name, intptr_t *parm)=0;

#ifndef AWS_VOIDP_IS_ERROR
  /**
   * Gets a copy of the property, put it in parm.  Returns false if the
   * property does not exist.
   * \deprecated For proper 64-bit platform support, use the intptr_t version
   *   of GetProperty().
   */
  AWS_VOIDP_IS_WARNING
  virtual bool GetProperty(const char* name, void **parm)=0;
#endif

  /**
   * Sets the property specified to whatever is in parm. Returns false if
   * there's no such property.
   */
  virtual bool SetProperty(const char* name, intptr_t parm)=0;

#ifndef AWS_VOIDP_IS_ERROR
  /**
   * Sets the property specified to whatever is in parm. Returns false if
   * there's no such property.
   * \deprecated For proper 64-bit platform support, use the intptr_t version
   *   of SetProperty().
   */
  AWS_VOIDP_IS_WARNING
  virtual bool SetProperty(const char* name, void *parm)=0;
#endif

  /// Executes a scriptable action
  virtual bool Execute(const char* action, iAwsParmList* parmlist = 0) = 0;

  /**
   * Invalidation routine: allow the component to be redrawn when you call
   * this.
   */
  virtual void Invalidate()=0;

  /// Invalidation routine: allow component to be redrawn, but only part of it
  virtual void Invalidate(csRect area)=0;

  /// Get this component's frame
  virtual csRect Frame()=0;

  /// Get this component's client area
  virtual csRect ClientFrame()=0;

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type()=0;

  /**
   * Sets the flag (can handle multiple simultaneous sets). 
   * <code>flag</code> is one of AWSF_CMP_*.
   * \sa \ref aws_comp_flags
   */
  virtual void SetFlag(unsigned int flag)=0;

  /**
   * Clears the flag (can handle multiple simultaneous clears).
   * <code>flag</code> is one of AWSF_CMP_*.
   * \sa \ref aws_comp_flags
   */
  virtual void ClearFlag(unsigned int flag)=0;

  /**
   * Returns the current state of the flags.
   * <code>flag</code> is one of AWSF_CMP_*.
   * \sa \ref aws_comp_flags
   */
  virtual unsigned int Flags()=0;

  /** Should be used ONLY by this component, or an embedding object. */
  virtual iAws *WindowManager ()=0;

  /// Gets the parent component of this component.
  virtual iAwsComponent *Parent()=0;

  /// Gets the window this component is in
  virtual iAwsComponent *Window()=0;

  /// Gets the layout manager for this component.
  virtual iAwsLayoutManager *Layout()=0;

  /// Sets the parent component of this component.
  virtual void SetParent(iAwsComponent *parent)=0;

  /// Sets the layout manager for this component.
  virtual void SetLayout(iAwsLayoutManager *layout)=0;

  /// Adds a component to this component's layout
  virtual void AddToLayout(iAwsComponent *cmp, iAwsComponentNode *settings)=0;
  
  /// Gets the preferred size of the component
  virtual csRect getPreferredSize()=0;

  /// Set the preferred size of the component.
  virtual void setPreferredSize (const csRect& size) = 0;

  /// Clear the manually set preferred size.
  virtual void clearPreferredSize () = 0;

  /// Gets the minimum size that the component can be
  virtual csRect getMinimumSize()=0;

  /// Gets the inset amounts that are needed to fit components properly.
  virtual csRect getInsets()=0;

  /// Returns true if this window overlaps the given rect.
  virtual bool Overlaps(csRect &r)=0;

  /// Returns the state of the hidden flag
  virtual bool isHidden()=0;

  /// Sets focusable flag
  virtual void SetFocusable(bool _focusable)=0;

  /// Returns focusable flag
  virtual bool Focusable()=0;

  /// Returns the state of the focused flag
  virtual bool isFocused()=0;

  /// Returns true if the component is maximized
  virtual bool IsMaximized()=0;

  /// Hides a component
  virtual void Hide()=0;

  /// Shows a component
  virtual void Show()=0;

  /// Focus a component
  virtual void SetFocus()=0;

  /// Unfocus a component
  virtual void UnsetFocus()=0;

  /// Moves a component
  virtual void Move(int delta_x, int delta_y)=0;

  /// Moves a component to an absolute location
  virtual void MoveTo(int x, int y)=0;

  /// Resizes a component
  virtual void Resize(int width, int height)=0;

  /// Resizes a component to an absolute rect
  virtual void ResizeTo(csRect newFrame)=0;

  /// Maximizes this component
  virtual void Maximize()=0;

  /// Returns the component to its unmaximized size
  virtual void UnMaximize()=0;

  /// Resizes all the children of this component using the current layout
  virtual void LayoutChildren()=0;

  /// Returns the state of the DEAF flag
  virtual bool isDeaf()=0;

  /// set deaf/not deaf
  virtual void SetDeaf (bool bDeaf)=0;

  /// Get's the unique id of this component.
  virtual unsigned long GetID()=0;

  /**
   * Set's the unique id of this component. Note: only to be used by window
   * manager.
   */
  virtual void SetID(unsigned long _id)=0;

  /// Gets a child component by name, returns 0 on failure.
  virtual iAwsComponent *FindChild(const char *name)=0;

  /// Gets a child component by id, returns 0 on failure
  virtual iAwsComponent *DoFindChild(unsigned id)=0;

  /// Returns the highest child (if any) whose frame contains (x,y)
  virtual iAwsComponent* ChildAt(int x, int y)=0;

  /// Adds a child into this component.
  virtual void AddChild(iAwsComponent* child)=0;

  /// Removes a child from this component.
  virtual void RemoveChild(iAwsComponent *child)=0;

  /// Get's the number of children
  virtual int GetChildCount()=0;

  /// Get's a specific child
  virtual iAwsComponent *GetTopChild()=0;

  /// Get's the component above this one, 0 if there is none.
  virtual iAwsComponent *ComponentAbove()=0;

  /// Get's the component below this one, 0 if there is none.
  virtual iAwsComponent *ComponentBelow()=0;

  /// Set's the component above this one
  virtual void SetComponentAbove(iAwsComponent *comp)=0;

  /// Set's the component below this one
  virtual void SetComponentBelow(iAwsComponent *comp)=0;

  /**
   * Add child to TabOrder
   * Actually at this moment TabOrder is an array that contains all children
   * of component ordered by their creation. 
   */ 
  virtual bool AddToTabOrder(iAwsComponent *child)=0;

  /**
   * Get's next child component in parent TabOrder, 
   * First if there is none, 0, if child not belongs to this component
   */
  virtual iAwsComponent *TabNext(iAwsComponent *child)=0;

  /**
   * Get's previous child component in parent TabOrder,
   * Last if there is none, 0, if child not belongs to this component
   */
  virtual iAwsComponent *TabPrev(iAwsComponent *child)=0;

  /// Returns TabOrder length
  virtual int GetTabLength()=0;

  /// Returns component from TabOrder, 0 if there is none or index is invalid
  virtual iAwsComponent *GetTabComponent(int index)=0;

  /// Returns first focusable component on this window
  virtual iAwsComponent *GetFirstFocusableChild(iAwsComponent *comp)=0;

  /// Moves this component above all its siblings
  virtual void Raise()=0;

  /// Moves this component below all its siblings
  virtual void Lower()=0;

  /// Returns true if this component has children
  virtual bool HasChildren()=0;

  /// Sets the value of the redraw tag
  virtual void SetRedrawTag(unsigned int tag)=0;

  /// Gets the value of the redraw tag
  virtual unsigned int RedrawTag()=0;

  /// Triggered when the component needs to draw
  virtual void OnDraw(csRect clip)=0;

  /// Triggered when the user presses a mouse button down
  virtual bool OnMouseDown(int button, int x, int y)=0;

  /// Triggered when the user unpresses a mouse button
  virtual bool OnMouseUp(int button, int x, int y)=0;

  /// Triggered when the user moves the mouse
  virtual bool OnMouseMove(int button, int x, int y)=0;

  /// Triggered when the user clicks the mouse
  virtual bool OnMouseClick(int button, int x, int y)=0;

  /// Triggered when the user double clicks the mouse
  virtual bool OnMouseDoubleClick(int button, int x, int y)=0;

  /// Triggered when this component loses mouse focus
  virtual bool OnMouseExit()=0;

  /// Triggered when this component gains mouse focus
  virtual bool OnMouseEnter()=0;

  /// Triggered when the user presses a key
  virtual bool OnKeyboard (const csKeyEventData& eventData) = 0;

  /// Triggered when the keyboard focus is lost
  virtual bool OnLostFocus()=0;

  /// Triggered when the keyboard focus is gained
  virtual bool OnGainFocus()=0;

  /// Triggered at the start of each frame
  virtual bool OnFrame()=0;

  /// Triggered when a child is added to the parent (triggered on the child)
  virtual void OnAdded()=0;

  /// Triggered when a component is resized by the layout manager.
  virtual void OnResized()=0;

  /// Triggered when a child component has been moved
  virtual void OnChildMoved() = 0;

  /// Triggered when the Raise function is called
  virtual void OnRaise()=0;

  /// Triggered when the Lower function is called
  virtual void OnLower()=0;

  /// Triggered when a child becomes hidden
  virtual void OnChildHide()=0;

  /// Triggered when a child becomes shown
  virtual void OnChildShow()=0;

  /// Triggered when a child becomes focused
  virtual void OnSetFocus()=0;

  /// Triggered when a child becomes unfocused
  virtual void OnUnsetFocus()=0;

  /* Only awsComponent should make use of the funcs below. Nothing else =) */

  /// Removes a component from the hierarchy
  virtual void Unlink()=0;

  /// Links a component into the hierarchy as a sibling above comp
  virtual void LinkAbove(iAwsComponent* comp)=0;

  /// Links a component into the hierarchy as a sibling below comp
  virtual void LinkBelow(iAwsComponent* comp)=0;

  /// Sets the top child
  virtual void SetTopChild(iAwsComponent* child)=0;

  /// Mark the component to be deleted in PreProcess phase of the next frame.
  virtual void MarkToDelete()=0;

  /// Return the delete mark.
  virtual bool GetMarkToDelete()=0;
};


SCF_VERSION (iAwsComponentFactory, 0, 0, 2);

/// Interface for component factories
struct iAwsComponentFactory : public iBase
{
  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create()=0;

  /// Registers this factory with the window manager
  virtual void Register(const char *type)=0;

  /// Registers constants for the parser so that we can construct right.
  virtual void RegisterConstant(const char *name, int value)=0;
};


SCF_VERSION (iAwsKeyFactory, 0, 0, 4);

/// Interface for key factories.
struct iAwsKeyFactory : public iBase
{
   /**
    * Initializes the factory , name is the name of this component,
    * component type is it's type.
    */
   virtual void Initialize(const char* name, const char* component_type)=0;
   /// Adds this factory's base to the window manager IF the base is a window
   virtual void AddToWindowList(iAwsPrefManager *pm)=0;
   /// Adds the given factory's base in as a child of this factory.
   virtual void AddFactory(iAwsKeyFactory *factory)=0;
   /// Add an integer key
   virtual void AddIntKey (const char* name, int v)=0;
   /// Add a string key
   virtual void AddStringKey (const char* name, const char* v)=0;
   /// Add a rect key
   virtual void AddRectKey (const char* name, csRect v)=0;
   /// Add an RGB key
   virtual void AddRGBKey (const char* name, unsigned char r,
   	unsigned char g, unsigned char b)=0;
   /// Add a point key
   virtual void AddPointKey (const char* name, csPoint v)=0;
   /// Add a connection key
   virtual void AddConnectionKey (const char* name, iAwsSink *s,
   	unsigned long t, unsigned long sig)=0;
   /// Add a connection node (from a factory)
   virtual void AddConnectionNode (iAwsConnectionNodeFactory *node)=0;
   
   virtual iAwsComponentNode* GetThisNode () = 0;
};

SCF_VERSION (iAwsConnectionNodeFactory, 0, 0, 1);

/// Interface for connection node factories.
struct iAwsConnectionNodeFactory : public iBase
{
   /// Initializes the factory
   virtual void Initialize ()=0;
   /// Add a connection key
   virtual void AddConnectionKey (
                 const char* name,
                 iAwsSink *s,
                 unsigned long t,
                 unsigned long sig)=0;

   /// Get the base node
   virtual awsConnectionNode* GetThisNode () = 0;
};

/* @} */

#endif // __CS_IAWS_AWS_H__
