#ifndef __AWS2_GRADIENT_OBJECT_H__
#define __AWS2_GRADIENT_OBJECT_H__

typedef struct JSObject JSObject;

/** 
 * Initializes and creates the builtin gradient object. 
 */
void Gradient_SetupAutomation ();

/** 
 * Returns true if the object is a gradient. 
 */
bool IsGradientObject (JSObject *obj);

#endif
