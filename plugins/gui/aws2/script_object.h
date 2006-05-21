#ifndef __AWS2_SCRIPT_OBJECT__
#define __AWS2_SCRIPT_OBJECT__

#include "iaws/aws2.h"
#include "script_manager.h"
#include "csutil/scfstr.h"

/** 
 * This class is a proxy between the JavaScript shtuff and the windowing system.  
 * It's used to allow external clients to interact with the scripting system 
 * as event sinks.  
 */
class scriptObject : public iAws2ScriptObject
{
  /// The script object.
  JSObject  *so;

  /// The context.
  JSContext *cx;

  /// The function "onEvent" for this script object.
  JSFunction *js_fun;

  /// The native callback function for this object.
  iAws2ScriptEvent *func;       

  /** 
   * The activation record keeps track of all the important stuff that happens 
   * during a function call.  The information is valid ONLY _during_ a 
   * notification event. 
   */
  struct activation_record
  {
    JSContext *cx;
    JSObject *obj;
    uintN argc;
    jsval *argv;
    jsval *rval;
  } ar;

public:
  /// Creates a new script object named "name"
  scriptObject (const char *name);

  /// Tears down the script object.
  virtual ~scriptObject ();

  /// Sets the notification function.
  virtual void SetNotification (iAws2ScriptEvent *_func) { func=_func; }

  /// Returns the number of arguments sent.
  virtual uint32 NumArgs () { return ar.argc; }
  
  /** Returns the argument in position arg as an integer. */
  virtual int32 GetIntArg(uint arg);		
	
  /** Returns the argument in position arg as a double. */
  virtual double GetDoubleArg(uint arg);		
	
  /** Returns the argument in position arg as a string. */
  virtual csRef<iString> GetStringArg(uint arg);	
  
  /** Gets the named property as an integer, or zero if it doesn't exist. Returns true if it existed, false if it didn't. */
  virtual bool GetProp(const char *name, int32 &val);		
	
 /** Gets the named property as a double, or zero if it doesn't exist.  Returns true if it existed, false if it didn't. */
  virtual bool GetProp(const char *name, double &val);		
	
  /** Gets the named property as a string, or an empty string if it doesn't exist.  Returns true if it existed, false if it didn't. */
  virtual bool GetProp(const char *name, iString *val);	
  
  /** Sets the named property to the integer val. */
  virtual void SetProp(const char *name, int32 val);		
	
  /** Sets the named property to the double val. */
  virtual void SetProp(const char *name, double val);		
	
  /** Sets the named property to the string val. */
  virtual void SetProp(const char *name, const char *val);
    
  /** Executes the given code. */
  virtual void Exec(const char *code);					
  
  /** 
   * Sets up the activation record for the notification, and then
   * notifies the requestor that an event has occured. 
   */
  void Notify (JSContext *_cx, JSObject *_obj, uintN argc, jsval *argv, jsval **rval)
  {
    // Setup the activation record.
    ar.cx   = _cx;
    ar.obj  = _obj;
    ar.argc = argc;
    ar.argv = argv;

    (*func) (this);

    // Return the value.
    *rval = ar.rval;
  }


};

#endif
