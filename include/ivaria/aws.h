#ifndef __IVARIA_AWS_H__
#define __IVARIA_AWS_H__

#include "csutil/scf.h"
#include "cssys/system.h"
#include "csgeom/csrect.h"
#include "iutil/string.h"

struct iAws;
struct iAwsPrefs;
struct iAwsSlot;
struct iAwsSigSrc;

class  awsWindow;
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

const   bool aws_debug=false;  // set to true to turn on debugging printf's
       

SCF_VERSION (iAws, 0, 0, 1);

struct iAws : public iBase
{
public:  
  /// Get a pointer to the preference manager
  virtual iAwsPrefs *GetPrefMgr()=0;

  /// Set the preference manager used by the window system
  virtual void       SetPrefMgr(iAwsPrefs *pmgr)=0;

  /// Allows a component to register itself for dynamic template instatiation via definition files.
  virtual void RegisterComponentFactory(awsComponentFactory *factory, char *name)=0;

  /// Get the top window
  virtual awsWindow *GetTopWindow()=0;

  /// Set the top window
  virtual void       SetTopWindow(awsWindow *win)=0;
  
  /// Causes the current view of the window system to be drawn to the given graphics device.
  virtual void       Print(iGraphics3D *g3d)=0;
  
  /// Redraw whatever portions of the screen need it.
  virtual void       Redraw()=0;

  /// Mark a region dirty
  virtual void       Mark(csRect &rect)=0;

  /// Mark a section of the screen clean.
  virtual void       Unmark(csRect &rect)=0;

  /// Tell the system to rebuild the update store
  virtual void       InvalidateUpdateStore()=0;

  /// Capture all mouse events until release is called, no matter where the mouse is
  virtual void       CaptureMouse()=0;

  /// Release the mouse events to go where they normally would.
  virtual void       ReleaseMouse()=0;

  /// Dispatches events to the proper components
  virtual bool HandleEvent(iEvent&)=0;
  
  /// Set the contexts however you want
  virtual void SetContext(iGraphics2D *g2d, iGraphics3D *g3d)=0;

  /// Set the context to the procedural texture
  virtual void SetDefaultContext(iEngine* engine, iTextureManager* txtmgr)=0;

  /// Get the iGraphics2D interface so that components can use it.
  virtual iGraphics2D *G2D()=0;

  /// Get the iGraphics3D interface so that components can use it.
  virtual iGraphics3D *G3D()=0; 
  
  /// Instantiates a window based on a window definition.
  virtual awsWindow *CreateWindowFrom(char *defname)=0;
  
};


SCF_VERSION (iAwsPrefs, 0, 0, 1);

struct iAwsPrefs : public iBase
{
public:

  /// Performs whatever initialization is needed
  virtual void Setup(iObjectRegistry *object_reg)=0;   

  /// Invokes the definition parser to load definition files
  virtual void Load(const char *def_file)=0;

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

SCF_VERSION (iAwsSigSrc, 0, 0, 1);

struct iAwsSigSrc : public iBase
{      
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
  /** This initializes a slot for a given component.  This slot may connect to any number of signal sources, and
   * all signals will be delivered back to this initialized slot.  Need only be intialized ONCE.
   */
  virtual void Initialize(iBase *sink, void (iBase::*Slot)(iBase &source, unsigned long signal))=0;
  
  /** Connect sets us up to receive signals from some other component.  You can connect to as many different sources 
    and signals as you'd like.  You may connect to multiple signals from the same source.
  */
  virtual void Connect(iAwsSigSrc &source, unsigned long signal)=0;
  
  /**  Disconnects us from the specified source and signal.  This may happen automatically if the signal source
   *  goes away.  You will receive disconnect notification always (even if you request the disconnection.)
   */
  virtual void Disconnect(iAwsSigSrc &source, unsigned long signal)=0;

  /** Invoked by a source to emit the signal to this slot's sink.
   */
  virtual void Emit(iBase &source, unsigned long signal)=0;
};


#endif // __IVARIA_AWS_H__
