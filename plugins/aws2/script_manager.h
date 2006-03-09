#ifndef __AWS2_SCRIPT_H__
#define __AWS2_SCRIPT_H__

#include "javascript.h"

#include "ivaria/reporter.h"
#include "iutil/vfs.h"

/** @file The JavaScript engine interface
 *
 *  This object mediates interaction between the JavaScript engine, and the outside world.  
 * AWS external clients will require a certain amount of callback ability.  There is a
 * proxy that handles that mediation, which is also defined in this file. */


class scriptManager
{
	/** A runtime per process is needed. */
	JSRuntime *rt;
	
	/** A context per thread is needed. */
    JSContext *cx;
    
    /** The global object. */
    JSObject  *global;
    
    /** File object so that we can work with files. */
    csRef<iVFS> vfs;
    
    /** Set to true if the script manager initialized properly. */
    bool valid;

public:
	scriptManager();
	
	~scriptManager();
	
	/** @brief Initializes the script manager. */
	void Initialize(iObjectRegistry *obj_reg);
	
	/** Returns true if the script manager was able to initialize
	 * the JavaScript engine correctly. */
	bool isInitialized() { return valid; }
	
	/** Gets the context for this session. */
	JSContext *GetContext() { return cx; }
	
	/** Gets the global object. */
	JSObject *GetGlobalObject() { return global; }
	
	/** Creates a new Javascript object. */
	JSObject *CreateObject(const char *name, JSClass *clasp);
	
	/** Loads and executes a script. */
	void Exec(const csString &filename);
	
	
};

/** Returns the current script manager. */
extern scriptManager *ScriptMgr();

/** Creates the one and only script manager for this plugin. */
void CreateScriptManager();

/** Destroys the one and only script manager. */
void DestroyScriptManager();

#endif
