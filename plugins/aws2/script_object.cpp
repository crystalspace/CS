#include "cssysdef.h"
#include "script_object.h"

/***** Script Object Callback ******/
static JSBool so_callback (JSContext *cx, JSObject *obj, uintN argc, 
                           jsval *argv, jsval *rval)		 
{
  // Get the script object.
  scriptObject *so = (scriptObject *)JS_GetPrivate (cx, obj);

  // Notify the owner.
  so->Notify (cx, obj, argc, argv, &rval);

  return JS_TRUE;	
}
/***********************************/


JSClass client_object_class = {
  "Aws2Event", 
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_EnumerateStub,
  JS_ResolveStub,
  JS_ConvertStub,
  JS_FinalizeStub 
};

scriptObject::scriptObject (const char *name)
  : func (0)
{
  // Create the object for this scriptObject.
  so = ScriptMgr ()->CreateObject (name, &client_object_class);
  cx = ScriptMgr ()->GetContext ();

  // Create the onEvent function that will be called.
  js_fun = JS_DefineFunction (cx, so, "onEvent", so_callback, 2, 0);

  //  Set the private data so that when the callback hits, we know what
  // object to reference. 
  if (so) JS_SetPrivate (cx, so, (void *)this);	
}

scriptObject::~scriptObject ()
{

}

int32 scriptObject::GetIntArg(uint arg)
{
	int32 val;
	
	JS_ValueToInt32 (ar.cx, ar.argv[arg], &val);	
	
	return val;
}
	 
double scriptObject::GetDoubleArg(uint arg)
{
  jsdouble val=0.0;

  JS_ValueToNumber (ar.cx,  ar.argv[arg], &val);
  
  return (double)val;
}
	 
scfString scriptObject::GetStringArg(uint arg)
{
	JSString *val = JS_ValueToString (ar.cx, ar.argv[arg]);

    return scfString(JS_GetStringBytes (val));	
}


int32 scriptObject::GetIntProp(const scfString &name)
{	
	jsval sval;
	
	if (JS_GetProperty (ar.cx, ar.obj, name.GetData(), &sval)==JS_TRUE && sval!=JSVAL_VOID)
	{
		int32 val;
	
		JS_ValueToInt32 (ar.cx, sval, &val);	
	
		return val;		
	}	
	
	return 0;
}	

double scriptObject::GetDoubleProp(const scfString &name)		
{	
	jsval sval;
	
	if (JS_GetProperty (ar.cx, ar.obj, name.GetData(), &sval)==JS_TRUE && sval!=JSVAL_VOID)
	{
		 jsdouble val=0.0;

  		JS_ValueToNumber (ar.cx, sval, &val);
  
  		return (double)val;
	}	
	
	return 0.0f;
}	

scfString scriptObject::GetStringProp(const scfString &name)
{	
	jsval sval;
	
	if (JS_GetProperty (ar.cx, ar.obj, name.GetData(), &sval)==JS_TRUE && sval!=JSVAL_VOID)
	{
		JSString *val = JS_ValueToString (ar.cx, sval);

    	return scfString(JS_GetStringBytes (val));	
	}	
	
	return scfString();	
}


void scriptObject::SetIntProp(const scfString &name, int32 val)
{
	jsval sval = INT_TO_JSVAL(val);
	
	JS_SetProperty (ar.cx, ar.obj, name.GetData(), &sval);
}	

void scriptObject::SetDoubleProp(const scfString &name, double val)
{
	jsdouble tmp=(jsdouble)val;
	jsval sval = DOUBLE_TO_JSVAL(tmp);
	
	JS_SetProperty (ar.cx, ar.obj, name.GetData(), &sval);	
}	

void scriptObject::SetStringProp(const scfString &name, const scfString val)
{
	JSString *str = JS_NewString(ar.cx, (char *)name.GetData(), name.Length());
	jsval sval = STRING_TO_JSVAL(str);
			
	JS_SetProperty (ar.cx, ar.obj, name.GetData(), &sval);	
}


// newline
