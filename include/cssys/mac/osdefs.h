/*
    This header file contains all definitions needed for compatibility issues
    Most of them should be defined only if corresponding SYSDEF_XXX macro is
    defined (see system/sysdef.h)
 */
#ifndef OSDEFS_H
#define OSDEFS_H

int strcasecmp (const char *str1, const char *str2);
int strncasecmp (char const *dst, char const *src, int maxLen);
char *strdup (const char *str);
#define stricmp strcasecmp

#ifdef SYSDEF_ACCESS
int access (const char *path, int mode);
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

#define PORT_BYTESEX_BIG_ENDIAN     1


#ifdef SYSDEF_2DDRIVER_DEFS
#define kArrowCursor				128

#define kGeneralErrorDialog			1026
#define kAskForDepthChangeDialog	1027

#define kErrorStrings				1025
#define kBadDepthString				1
#define kNoDSContext				2
#define kUnableToOpenDSContext		3
#define kUnableToReserveDSContext	4
#define kFatalErrorInGlide			5
#define kFatalErrorInOpenGL2D		6
#define kFatalErrorInOpenGL3D		7
#define kFatalErrorOutOfMemory		8
#define kFatalErrorInDriver2D		9
#define kFatalErrorInRave2D			10
#define kFatalErrorInRave3D			11
#define kFatalErrorCantStartRave	12
#endif

#endif // OSDEFS_H
