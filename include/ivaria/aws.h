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

  /// Mark a region dirty
  virtual void       Mark(csRect &rect)=0;
};


SCF_VERSION (iAwsPrefs, 0, 0, 1);

struct iAwsPrefs : public iBase
{
public:
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

  /// Get the an integer from a given component node
  virtual bool GetInt(awsComponentNode *node, char *name, int &val)=0;

  /// Get the a rect from a given component node
  virtual bool GetRect(awsComponentNode *node, char *name, csRect &rect)=0;

  /// Get the value of an integer from a given component node
  virtual bool GetString(awsComponentNode *node, char *name, iString *&val)=0;

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
