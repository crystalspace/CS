/*
    This header file contains all definitions needed for compatibility issues
    Most of them should be defined only if corresponding SYSDEF_XXX macro is
    defined (see system/sysdef.h)
 */
#ifndef OSDEFS_H
#define OSDEFS_H

#ifdef __cplusplus
	extern "C" {
#endif

int strcasecmp (const char *str1, const char *str2);
int strncasecmp (char const *dst, char const *src, int maxLen);
char *strdup (const char *str);
#define stricmp strcasecmp

#ifdef SYSDEF_ACCESS
int access (const char *path, int mode);
#endif

#ifdef __cplusplus
	}
#endif

// The 2D graphics driver used by software renderer on this platform
#define SOFTWARE_2D_DRIVER	"crystalspace.graphics2d.mac"
#define OPENGL_2D_DRIVER	"crystalspace.graphics2d.defaultgl"
#define GLIDE_2D_DRIVER		"crystalspace.graphics3d.glide.2x"
#define RAVE_2D_DRIVER		"crystalspace.graphics3d.rave"

// Sound driver
#define SOUND_DRIVER            "crystalspace.sound.driver.mac"

#if defined (SYSDEF_DIR)
#  define __NEED_GENERIC_ISDIR
#endif

#endif // OSDEFS_H
