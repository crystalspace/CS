#ifndef __AWS2_COLOR_OBJECT_H__
#define __AWS2_COLOR_OBJECT_H__

typedef struct JSObject JSObject;

/**
 * Initializes and creates the builtin color object. 
 */
void Color_SetupAutomation ();

/** 
 * Returns true if the object is a color. 
 */
bool IsColorObject (JSObject *obj);

#endif
