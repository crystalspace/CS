#ifndef __AWS2_GRADIENT_OBJECT_H__
#define __AWS2_GRADIENT_OBJECT_H__



/** Initializes and creates the builtin gradient object. */
void Gradient_SetupAutomation();

/** @brief Returns true if the object is a gradient. */
bool IsGradientObject(JSObject *obj);

#endif
