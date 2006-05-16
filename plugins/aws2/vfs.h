#ifndef __AWS2_VFS_OBJECT_H__
#define __AWS2_VFS_OBJECT_H__

/** 
 * Initializes and creates the builtin VFS object. 
 */
void Vfs_SetupAutomation ();

/** 
 * Returns true if the object is a VFS object. 
 */
bool IsVfsObject (JSObject *obj);

#endif
