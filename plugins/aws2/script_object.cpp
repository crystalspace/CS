#include "cssysdef.h"
#include "script_object.h"

/***** Script Object Callback ******/
static JSBool
so_callback(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)		 
{
	// Get the script object.
	scriptObject *so = (scriptObject *)JS_GetPrivate(cx, obj);
	
	// Notify the owner.
	so->Notify(cx, obj, argc, argv, &rval);
	
	return JS_TRUE;	
}
/***********************************/


JSClass client_object_class = {
    "Aws2Event", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
    JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub 
};

scriptObject::scriptObject(const char *name):func(0)
{
	// Create the object for this scriptObject.
	so = ScriptMgr()->CreateObject(name, &client_object_class);
	cx = ScriptMgr()->GetContext();
	
	// Create the onEvent function that will be called.
	js_fun = JS_DefineFunction(cx, so, "onEvent", so_callback, 2, 0);
	
	//  Set the private data so that when the callback hits, we know what
	// object to reference. 
	if (so) JS_SetPrivate(cx, so, (void *)this);	
}

scriptObject::~scriptObject()
{
		
}

// newline
