// Needed for Memory-Mapped IO functions below.
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
MemoryMapFile(mmioInfo *platform, const char *filename)
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
UnMemoryMapFile(mmioInfo *platform)
{
  if (platform->data != 0)
    munmap(platform->data, platform->file_size);

  if (platform->hMappedFile != -1)
    close(platform->hMappedFile);
}
