#ifndef __AWS2_TEXTURE_OBJECT_H__
#define __AWS2_TEXTURE_OBJECT_H__

/** Initializes and creates the builtin color object. */
void Texture_SetupAutomation();

/** @brief Returns true if the object is a color. */
bool IsTextureObject(JSObject *obj);

#endif
