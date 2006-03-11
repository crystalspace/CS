#include "cssysdef.h"
#include "script_manager.h"
#include "script_console.h"
#include "manager.h"
#include "timer.h"

#include <stdlib.h>

scriptManager *__script_manager__=0;

void
CreateScriptManager()
{
	__script_manager__ = new scriptManager();	
}

void
DestroyScriptManager()
{
	delete __script_manager__;
	
	__script_manager__ = 0;
}

scriptManager *ScriptMgr()
{
	return __script_manager__;
}


/**************************
 * The error reporter will forward error messages
 * to the script console. */
 
static void
aws_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
	
	ScriptCon()->Message("");
	
	if (report)
	{
		csString err, tmp;
		
		if (report->filename)
		{
			err+=report->filename;
			err+=" : ";
		}
				
		err+="line ";
		tmp.Format("%d", report->lineno);
		err+=tmp;
		err+=" : ";
		
		if (report->linebuf)
		{
			err+=report->linebuf;
		}
		
		ScriptCon()->Message(err);
		
		if (report->tokenptr)
		{
			err = "  near: ";
			err+= report->tokenptr;
			ScriptCon()->Message(err);
		}		
	}
	
	ScriptCon()->Message(message);
	
	ScriptCon()->Message("");
	
}

/****************************************************/

JSClass global_class = {
    "global", JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,   JS_FinalizeStub
};

/**************************************************
 *  The script manager object also owns a class called
 * "Automation", which contains some functions that
 * call into the script manager. */
 
/** @brief Loads and executes a script. */
static JSBool
Exec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	
	JSString *str = JS_ValueToString(cx, argv[0]);
	
	ScriptMgr()->Exec(csString(JS_GetStringBytes(str)));
		
	return JS_TRUE;
} 

/** @brief Calls the garbage collector. */
static JSBool
GarbageCollect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	
	JS_GC(cx);
		
	return JS_TRUE;
} 

/** @brief Calls the garbage collector, but only collects garbage if memory is at 75% of the threshold. */
static JSBool
GarbageCollectSmart(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	
	JS_MaybeGC(cx);
		
	return JS_TRUE;
} 

/** @brief Calls the garbage collector, but only collects garbage if memory is at 75% of the threshold. */
static JSBool
Print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	csString msg;
	
	for(uintN i=0; i<argc; ++i)	
	{
		JSString *str = JS_ValueToString(cx, argv[i]);
		csString tmp(JS_GetStringBytes(str));
		
		msg+=tmp;	
	}	
	
	ScriptCon()->Message(msg);
	
	return JS_TRUE;
} 

/** @brief Creates timer events. */
static JSBool
CreateTimer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	int32 ticks;
		
	aws::Timer *t = new aws::Timer(obj, argc, argv);
	JS_ValueToInt32(cx, argv[0], &ticks);
	
	AwsMgr()->GetTimer()->AddTimerEvent(t, (csTicks)ticks);		
	
	return JS_TRUE;
} 

/** @brief Get the rendering buffer width. */
static JSBool
GetWidth(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{			
	*rval = INT_TO_JSVAL(AwsMgr()->G2D()->GetWidth());
	
	return JS_TRUE;
} 

/** @brief Get the rendering buffer height. */
static JSBool
GetHeight(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	*rval = INT_TO_JSVAL(AwsMgr()->G2D()->GetHeight());
	
	return JS_TRUE;
}
 
JSClass autom_class = {
    "Sys", 0,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,   JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
}; 
 

static JSFunctionSpec autom_static_methods[] = {
    {"Exec",		Exec,		1, 0, 0},    
    {"Load",		Exec,		1, 0, 0},   
    {"GarbageCollect", GarbageCollect, 0, 0, 0}, 
    {"GarbageCollectSmart", GarbageCollectSmart, 0, 0, 0},
    {"Print",				Print, 				 0, 0, 0}, 
    {"CreateTimer",			CreateTimer, 		 2, 0, 0},
    {"GetWidth",			GetWidth, 		 	 0, 0, 0},
    {"GetHeight",			GetHeight, 		 	 0, 0, 0},    
    {0,0,0,0,0}
};

/***************************************************/

scriptManager::scriptManager():valid(false)
{	
	
}
	
scriptManager::~scriptManager()
{
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();		
}

#define CHECK(objname, check) if (!(check)) { \
	    msg.Format(objname ": failed on " #check " in\n   %s:%s:%d", __FILE__, __FUNCTION__, __LINE__); \
		ScriptCon()->Message(msg); \
	}

void 
scriptManager::Initialize(iObjectRegistry *obj_reg)
{
	csString msg;
	
	CHECK("Init", rt = JS_NewRuntime(64L * 1024L * 1024L));
	
	if (!rt)
	{
		
		
		valid=false;
    	return;
	}
	
	CHECK("Init", cx = JS_NewContext(rt, 8192));

	if (!cx)
	{						
    	valid=false;
    	return;
	}
	
	CHECK("Init", global = JS_NewObject(cx, &global_class, NULL, NULL));

	if (!global)
	{		
    	valid=false;
    	return;	    	
	}
	
	valid=true;
	
	/* Setup lazy global objects. */
	JS_InitStandardClasses(cx, global);
	
	/* Setup the error reporter. */
	JS_SetErrorReporter(cx, aws_ErrorReporter);
	
	/* Setup the automation class. */
	{		
		JSObject *autom;
		
		CHECK("Sys", autom = JS_DefineObject(cx, global, "Sys", &autom_class, NULL, 0));
		
		if (autom)
		{
			CHECK("Sys", JS_DefineFunctions(cx, autom, autom_static_methods)==JS_TRUE);			
		}
	}
	
	// Get the VFS object.
	vfs = CS_QUERY_REGISTRY (obj_reg, iVFS);
	
	ScriptCon()->Message("AWS2 automation initialized.");
		
}

JSObject *
scriptManager::CreateObject(const char *name, JSClass *clasp)
{
	if (valid==false) return NULL;
	
	return 	JS_DefineObject(cx, global, name, clasp, NULL, 0);
}

void 
scriptManager::Exec(const csString &filename)
{
	if (valid==false) return;
	
	csString msg;
	
	csRef<iFile> input = vfs->Open (filename.GetData(), VFS_FILE_READ);
	
	if (!input)
	{
		msg.Format("error: Unable to open file: \"%s\"", filename.GetData());
		return;	
	}
	
	msg.Format("Loading \"%s\"...", filename.GetData());
	ScriptCon()->Message(msg);
	
	// Read the whole file.
	csRef<iDataBuffer> buf = input->GetAllData();
	
	{
	  jsval tmp;	  
	  JSScript *sc;	 
	  
	  if((sc=JS_CompileScript(cx, global,
						buf->GetData(), buf->GetSize(),
						filename.GetData(), 1))!=0)
		{
			if (JS_ExecuteScript(ScriptMgr()->GetContext(), ScriptMgr()->GetGlobalObject(), sc, &tmp)==JS_FALSE)
			{						  					
				
				ScriptCon()->Message("error: Execution failed.");
			}	
		    									 
			JS_DestroyScript(ScriptMgr()->GetContext(), sc);
		}
		else
		{
			ScriptCon()->Message("error: Compiling failed.");			
		}	  
	}
		
	
}
	
// newline
