#ifndef __IAWS_AWS_H__
#define __IAWS_AWS_H__
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

#include "csutil/scf.h"
#include "csgeom/csrect.h"
#include "csgeom/cspoint.h"
#include "iutil/string.h"

struct iAws;
struct iAwsSlot;
struct iAwsSink;
struct iAwsSource;
struct iAwsWindow;
struct iAwsParmList;
struct iAwsComponent;
struct iAwsPrefManager;
struct iAwsSinkManager;
struct iAwsCanvas;

class  awsWindow;
class  awsComponent;
class  awsComponentNode;
class  awsComponentFactory;

struct  iGraphics2D;
struct  iGraphics3D;
struct  iEngine;
struct  iTextureManager;
struct  iObjectRegistry;
struct  iTextureHandle;
struct  iFontServer;
struct  iFont;
struct  iEvent;

const   bool aws_debug=false;  // set to true to turn on debugging printf's


/** This flag makes the windowing system perform erases before drawing.  This slows
 * the engine down somewhat, but is necessary in some circumstances (such as when using
 * the single proctex mode as a surface, or to draw to the high level visible context.)
 */
const int AWSF_AlwaysEraseWindows=1;


SCF_VERSION (iAws, 0, 1, 0);

struct iAws : public iBase
{  
public:  
  /// Get a pointer to the preference manager
  virtual iAwsPrefManager *GetPrefMgr()=0;

  /// Get a pointer to the sink manager
  virtual iAwsSinkManager *GetSinkMgr()=0;

  /// Set the preference manager used by the window system
  virtual void             SetPrefMgr(iAwsPrefManager *pmgr)=0;

  /// Allows a component to register itself for dynamic template instatiation via definition files.
  virtual void RegisterComponentFactory(awsComponentFactory *factory, char *name)=0;

  /// Get the top window
  virtual iAwsWindow *GetTopWindow()=0;

  /// Set the top window
  virtual void       SetTopWindow(iAwsWindow *win)=0;
  
  /// Causes the current view of the window system to be drawn to the given graphics device.
  virtual void       Print(iGraphics3D *g3d)=0;
  
  /// Redraw whatever portions of the screen need it.
  virtual void       Redraw()=0;

  /// Mark a region dirty
  virtual void       Mark(csRect &rect)=0;

  /// Mark a section of the screen clean.
  virtual void       Unmark(csRect &rect)=0;

  /// Erase a section of the screen next round (only useful if AlwaysEraseWindows flag is set)
  virtual void       Erase(csRect &rect)=0;

  /// Mask off a section that has been marked to erase.  This part won't be erased.
  virtual void       MaskEraser(csRect &rect)=0;

  /// Tell the system to rebuild the update store
  virtual void       InvalidateUpdateStore()=0;

  /// Capture all mouse events until release is called, no matter where the mouse is
  virtual void       CaptureMouse()=0;

  /// Release the mouse events to go where they normally would.
  virtual void       ReleaseMouse()=0;

  /// Dispatches events to the proper components
  virtual bool HandleEvent(iEvent&)=0;
  
  /// Set the contexts however you want
  virtual void SetCanvas(iAwsCanvas *newCanvas)=0;

  /// Get the current context
  virtual iAwsCanvas* GetCanvas()=0;

  /// Create a default canvas, covering the whole screen
  virtual iAwsCanvas *CreateDefaultCanvas(iEngine* engine, iTextureManager* txtmgr)=0;

  /// Create a default canvas, just a single proctex
  virtual iAwsCanvas *CreateDefaultCanvas(iEngine* engine, iTextureManager* txtmgr, 
    int width, int height, const char *name)=0;

  /// Get the iGraphics2D interface so that components can use it.
  virtual iGraphics2D *G2D()=0;

  /// Get the iGraphics3D interface so that components can use it.
  virtual iGraphics3D *G3D()=0; 
  
  /// Instantiates a window based on a window definition.
  virtual iAwsWindow *CreateWindowFrom(char *defname)=0;

  /// Creates a new embeddable component
  virtual iAwsComponent *CreateEmbeddableComponent()=0;

  /// Creates a new parameter list
  virtual iAwsParmList *CreateParmList()=0;

  /// Sets one or more flags for different operating modes
  virtual void SetFlag(unsigned int flags)=0;

  /// Clears one or more flags for different operating modes
  virtual void ClearFlag(unsigned int flags)=0;

  /// Returns the current flags 
  virtual unsigned int GetFlags()=0;
};


SCF_VERSION (iAwsPrefManager, 0, 0, 1);

struct iAwsPrefManager : public iBase
{
public:
  /// Performs whatever initialization is needed
  virtual bool Setup(iObjectRegistry *object_reg)=0;   

  /// Invokes the definition parser to load definition files
  virtual bool Load(const char *def_file)=0;

  /// Maps a name to an id
  virtual unsigned long NameToId(char *name)=0;
    
  /// Select which skin is the default for components, the skin must be loaded.  True on success, false otherwise.
  virtual bool SelectDefaultSkin(char *skin_name)=0;

  /// Lookup the value of an int key by name (from the skin def)
  virtual bool LookupIntKey(char *name, int &val)=0; 

  /// Lookup the value of an int key by id (from the skin def)
  virtual bool LookupIntKey(unsigned long id, int &val)=0; 

  /// Lookup the value of a string key by name (from the skin def)
  virtual bool LookupStringKey(char *name, iString *&val)=0; 

  /// Lookup the value of a string key by id (from the skin def)
  virtual bool LookupStringKey(unsigned long id, iString *&val)=0; 

  /// Lookup the value of a rect key by name (from the skin def)
  virtual bool LookupRectKey(char *name, csRect &rect)=0; 

  /// Lookup the value of a rect key by id (from the skin def)
  virtual bool LookupRectKey(unsigned long id, csRect &rect)=0; 
  
  /// Lookup the value of an RGB key by name (from the skin def)
  virtual bool LookupRGBKey(char *name, unsigned char &red, unsigned char &green, unsigned char &blue)=0;
    
  /// Lookup the value of an RGB key by name (from the skin def)
  virtual bool LookupRGBKey(unsigned long id, unsigned char &red, unsigned char &green, unsigned char &blue)=0;

  /// Lookup the value of a point key by name (from the skin def)
  virtual bool LookupPointKey(char *name, csPoint &point)=0; 

  /// Lookup the value of a point key by id (from the skin def)
  virtual bool LookupPointKey(unsigned long id, csPoint &point)=0; 
  
  /// Get the an integer from a given component node
  virtual bool GetInt(awsComponentNode *node, char *name, int &val)=0;

  /// Get the a rect from a given component node
  virtual bool GetRect(awsComponentNode *node, char *name, csRect &rect)=0;

  /// Get the value of an integer from a given component node
  virtual bool GetString(awsComponentNode *node, char *name, iString *&val)=0;
  
  /// Find window definition and return the component node holding it, Null otherwise
  virtual awsComponentNode *FindWindowDef(char *name)=0;
  
  /// Sets the value of a color in the global AWS palette.
  virtual void SetColor(int index, int color)=0; 
    
  /// Gets the value of a color from the global AWS palette.
  virtual int  GetColor(int index)=0;

  /// Gets the current default font
  virtual iFont *GetDefaultFont()=0;

  /// Gets a font.  If it's not loaded, it will be.  Returns NULL on error.
  virtual iFont *GetFont(char *filename)=0;

  /// Gets a texture from the global AWS cache
  virtual iTextureHandle *GetTexture(char *name, char *filename=NULL)=0;

  /// Sets the texture manager that the preference manager uses
  virtual void SetTextureManager(iTextureManager *txtmgr)=0;

  /// Sets the font server that the preference manager uses
  virtual void SetFontServer(iFontServer *fntsvr)=0;
  
  /// Sets the window manager that the preference manager uses
  virtual void SetWindowMgr(iAws *wmgr)=0;
    
  /** Sets up the AWS palette so that the colors are valid reflections of
       user preferences.  Although SetColor can be used, it's recommended 
       that you do not.  Colors should always be a user preference, and 
       should be read from the window and skin definition files (as
       happens automatically normally. */
  virtual void SetupPalette()=0;

  /** Allows a component to specify it's own constant values for parsing. */
  virtual void RegisterConstant(char *name, int value)=0;

  /** Returns true if the constant has been registered, false otherwise.  */
  virtual bool ConstantExists(char *name)=0;

  /** Allows a component to retrieve the value of a constant, or the parser as well. */
  virtual int  GetConstantValue(char *name)=0;
};


SCF_VERSION (iAwsSinkManager, 0, 0, 1);

struct iAwsSinkManager : public iBase
{
  /// Registers a sink by name for lookup.
  virtual void RegisterSink(char *name, iAwsSink *sink)=0;

  /// Finds a sink by name for connection.
  virtual iAwsSink* FindSink(char *name)=0;

  /// Create a new embeddable sink, with parm as the void * passed into the triggers.
  virtual iAwsSink *CreateSink(void *parm)=0;
};


SCF_VERSION (iAwsSink, 0, 0, 1);

struct iAwsSink : public iBase
{
  /// Maps a trigger name to a trigger id
  virtual unsigned long GetTriggerID(char *name)=0;  

  /// Handles trigger events
  virtual void HandleTrigger(int trigger_id, iAwsSource *source)=0;

  /// A sink should call this to register trigger events
  virtual void RegisterTrigger(char *name, void (*Trigger)(void *, iAwsSource *))=0;
};


SCF_VERSION (iAwsSource, 0, 0, 1);

struct iAwsSource : public iBase
{      
  /// Gets the component owner for this (sources are embedded)
  virtual iAwsComponent *GetComponent()=0;

  /// Registers a slot for any one of the signals defined by a source.  Each sources's signals exist in it's own namespace
  virtual bool RegisterSlot(iAwsSlot *slot, unsigned long signal)=0;

  /// Unregisters a slot for a signal.
  virtual bool UnregisterSlot(iAwsSlot *slot, unsigned long signal)=0;

  /// Broadcasts a signal to all slots that are interested.
  virtual void Broadcast(unsigned long signal)=0;
};


SCF_VERSION (iAwsSlot, 0, 0, 1);

struct iAwsSlot : public iBase
{
  /** Connect sets us up to receive signals from some other component.  You can connect to as many different sources 
   * and signals as you'd like.  You may connect to multiple signals from the same source.
   */
  virtual void Connect(iAwsSource *source, unsigned long signal, iAwsSink *sink, unsigned long trigger)=0;
  
  /**  Disconnects us from the specified source and signal.  This may happen automatically if the signal source
   *  goes away.  You will receive disconnect notification always (even if you request the disconnection.)
   */
  virtual void Disconnect(iAwsSource *source, unsigned long signal, iAwsSink *sink, unsigned long trigger)=0;

  /** Invoked by a source to emit the signal to this slot's sink.
   */
  virtual void Emit(iAwsSource &source, unsigned long signal)=0;
};


SCF_VERSION (iAwsComponent, 0, 0, 1);

struct iAwsComponent : public iAwsSource
{
  /// Sets up a component.
  virtual bool Setup(iAws *wmgr, awsComponentNode *settings)=0;

  /// Event dispatcher, demultiplexes events and sends them off to the proper event handler
  virtual bool HandleEvent(iEvent& Event)=0;

  /// Gets a copy of the property, put it in parm.  Returns false if the property does not exist.
  virtual bool GetProperty(char *name, void **parm)=0;

  /// Sets the property specified to whatever is in parm. Returns false if there's no such property.
  virtual bool SetProperty(char *name, void *parm)=0;

  /// Executes a scriptable action
  virtual bool Execute(char *action, iAwsParmList &parmlist)=0;

  /// Invalidation routine: allow the component to be redrawn when you call this
  virtual void Invalidate()=0;

  /// Invalidation routine: allow component to be redrawn, but only part of it
  virtual void Invalidate(csRect area)=0;

  /// Get this component's frame
  virtual csRect& Frame()=0;

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual char *Type()=0;

  /// Sets the flag (can handle multiple simultaneous sets)
  virtual void SetFlag(unsigned int flag)=0;

  /// Clears the flag (can handle multiple simultaneous clears)
  virtual void ClearFlag(unsigned int flag)=0;

  /// Returns the current state of the flags
  virtual unsigned int Flags()=0;

  /// Gets the window that this component resides in.
  virtual iAwsWindow *Window()=0;

  /// Gets the parent component of this component;
  virtual iAwsComponent *Parent()=0;

  /// Sets the window that this component resides in.
  virtual void SetWindow(iAwsWindow *win)=0;

  /// Sets the parent component of this component;
  virtual void SetParent(iAwsComponent *parent)=0;

  /// Returns true if this window overlaps the given rect.
  virtual bool Overlaps(csRect &r)=0;

  /// Returns the state of the hidden flag
  virtual bool isHidden()=0;

  /// Hides a component
  virtual void Hide()=0;

  /// Shows a component
  virtual void Show()=0;

  /// Get's the unique id of this component.
  virtual unsigned long GetID()=0;

  /// Set's the unique id of this component. Note: only to be used by window manager.
  virtual void SetID(unsigned long _id)=0;

  /// Recursively moves children (and all nested children) by relative amount given.
  virtual void MoveChildren(int delta_x, int delta_y)=0;

  /// Adds a child into this component.   
  virtual void AddChild(iAwsComponent* child, bool owner=true)=0;

  /// Removes a child from this component. 
  virtual void RemoveChild(iAwsComponent *child)=0;

  /// Get's the number of children
  virtual int GetChildCount()=0;

  /// Get's a specific child
  virtual iAwsComponent *GetChildAt(int i)=0;
    
  /// Returns true if this component has children
  virtual bool HasChildren()=0;

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
  virtual bool OnKeypress(int key, int modifiers)=0;
    
  /// Triggered when the keyboard focus is lost
  virtual bool OnLostFocus()=0;

  /// Triggered when the keyboard focus is gained
  virtual bool OnGainFocus()=0;
};


SCF_VERSION (iAwsWindow, 0, 0, 1);

struct iAwsWindow : public iAwsComponent
{
  /// Sets the value of the redraw tag
  virtual void SetRedrawTag(unsigned int tag)=0;

  /// Gets the value of the redraw tag
  virtual unsigned int RedrawTag()=0;
  
  /// Raises a window to the top.
  virtual void Raise()=0;

  /// Lowers a window to the bottom.
  virtual void Lower()=0;

  /// Get's the window above this one, NULL if there is none.
  virtual iAwsWindow *WindowAbove()=0;

  /// Get's the window below this one, NULL if there is none.
  virtual iAwsWindow *WindowBelow()=0;

  /// Set's the window above this one
  virtual void SetWindowAbove(iAwsWindow *win)=0;
    
  /// Set's the window below this one
  virtual void SetWindowBelow(iAwsWindow *win)=0;

  /// Does some additional setup for windows, including linking into the window hierarchy.
  virtual bool Setup(iAws *_wmgr, awsComponentNode *settings)=0;

  /// Event triggered when a window is about to be raised
  virtual void OnRaise()=0;

  /// Event triggered when a window is about to be lowered
  virtual void OnLower()=0;
};


SCF_VERSION (iAwsComponentFactory, 0, 0, 1);

struct iAwsComponentFactory : public iBase
{
  /// Returns a newly created component of the type this factory handles. 
  virtual iAwsComponent *Create()=0;

  /// Registers this factory with the window manager
  virtual void Register(char *type)=0;

  /// Registers constants for the parser so that we can construct right.
  virtual void RegisterConstant(char *name, int value)=0;
};

#endif // __IAWS_AWS_H__
