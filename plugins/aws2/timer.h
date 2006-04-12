#ifndef __AWS2_TIMER_H__
#define __AWS2_TIMER_H__

#include "iutil/timer.h"
#include "script_manager.h"

namespace aws
{

  class Timer : public iTimerEvent
  {
    /// the object to use.
    JSObject *obj;

    /// the function to call.
    jsval func;

  public:	
    SCF_DECLARE_IBASE;

    Timer (JSObject *_obj, uintN argc, jsval *argv)
      : obj (_obj)
    {
      // Store the function.
      func = argv[1];
    }

    // Fire the timer.
    virtual bool Perform (iTimerEvent *ev)
    {	
      jsval rv;

      JS_CallFunctionValue (ScriptMgr ()->GetContext (), obj, func, 0, NULL, &rv);

      return (rv==JSVAL_TRUE);
    }	
  };


} // end namespace

#endif
