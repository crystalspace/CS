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

#ifdef CS_SYSDEF_PROVIDE_HARDWARE_MMIO

// Needed for Memory-Mapped IO functions below.
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Defines that this platform supports hardware memory-mapped i/o
#define CS_HAS_MEMORY_MAPPED_IO 1

// Unix specific memory mapped I/O platform dependent stuff
struct mmioInfo
{          
    /// Handle to the mapped file 
    int hMappedFile;

    /// Base pointer to the data
    unsigned char *data;

    /// File size
    unsigned int file_size;
};

// Fills in the mmioInfo struct by mapping in filename.  Returns true on success, false otherwise.
inline 
bool
MemoryMapFile(mmioInfo *platform, char *filename)
{   
  struct stat statInfo;
  
  // Have 'nix map this file in for use
  if (
      (platform->hMappedFile = open(filename, O_RDONLY)) == -1   ||
      (fstat(platform->hMappedFile, &statInfo )) == -1           ||
      (int)(platform->data = (unsigned char *)mmap(0, statInfo.st_size, PROT_READ, 0, platform->hMappedFile, 0)) == -1
     )
  {
    return false;
  }
  else
  {
    platform->file_size=statInfo.st_size;
    return true;
  }
}

inline 
void
UnMemoryMapFile(mmioInfo *platform, char *filename)
{
  if (platform->data != -1)
    munmap(platform->data, file_size);

  if (platform->hMappedFile != -1)
    close(platform->hMappedFile);
}

#endif // memory-mapped I/O


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
