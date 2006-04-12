#ifndef __AWS2_FONT_OBJECT_H__
#define __AWS2_FONT_OBJECT_H__

typedef struct JSObject JSObject;

/** 
 * Initializes and creates the builtin color object. 
 */
void Font_SetupAutomation ();

/** 
 * Returns true if the object is a color. 
 */
bool IsFontObject (JSObject *obj);

#endif
