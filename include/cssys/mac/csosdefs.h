/*
    This header file contains all definitions needed for compatibility issues
    Most of them should be defined only if corresponding CS_SYSDEF_PROVIDE_XXX macro is
    defined (see system/cssysdef.h)
 */
#ifndef __CSOSDEFS_H__
#define __CSOSDEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

int strcasecmp (const char *str1, const char *str2);
int strncasecmp (char const *dst, char const *src, int maxLen);
char *strdup (const char *str);
#define stricmp strcasecmp

// SCF symbol export facility.
#undef SCF_EXPORT_FUNCTION
#define SCF_EXPORT_FUNCTION extern "C" __declspec(export)

#ifdef CS_SYSDEF_PROVIDE_ACCESS
# if __MWERKS__>=0x2400
# include <unistd.h>
# else
int access (const char *path, int mode);
# endif
#endif // CS_SYSDEF_PROVIDE_ACCESS

#ifdef __cplusplus
}
#endif

// The 2D graphics driver used by software renderer on this platform
#define CS_SOFTWARE_2D_DRIVER	"crystalspace.graphics2d.macintosh"
#define CS_OPENGL_2D_DRIVER	"crystalspace.graphics2d.glmac"

// Sound driver
#define CS_SOUND_DRIVER            "crystalspace.sound.driver.macintosh"

#if defined (CS_SYSDEF_PROVIDE_DIR)
#  define __NEED_GENERIC_ISDIR
#endif

// WHM CW6 fix
#if defined (CS_SYSDEF_PROVIDE_GETCWD) || defined (CS_SYSDEF_PROVIDE_UNLINK)
#if __MWERKS__>=0x2400
#include <unistd.h>
#endif
#endif

#if defined (CS_SYSDEF_PROVIDE_SELECT)
typedef unsigned long fd_set;
#undef CS_SYSDEF_PROVIDE_SELECT
#endif

#if defined (PROC_M68K) || defined (PROC_POWERPC)
#  define CS_BIG_ENDIAN
#else
#  error "Please define a suitable CS_XXX_ENDIAN macro in mac/csosdefs.h!"
#endif

#endif // __CSOSDEFS_H__
