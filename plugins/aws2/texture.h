#ifndef __AWS2_TEXTURE_OBJECT_H__
#define __AWS2_TEXTURE_OBJECT_H__

/** 
 * Initializes and creates the builtin texture object. 
 */
void Texture_SetupAutomation ();

/** 
 * Returns true if the object is a texture. 
 */
bool IsTextureObject (JSObject *obj);

#endif
