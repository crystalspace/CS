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

// Fill in the mmioInfo struct by mapping in filename.
// Returns true on success, false otherwise.
inline bool MemoryMapFile(mmioInfo* info, char const* filename)
{   
  bool ok = false;
  struct stat st;
  int const fd = open(filename, O_RDONLY);
  if (fd != -1 && fstat(fd, &st) != -1)
  {
    unsigned char* p=(unsigned char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if ((int)p != -1)
    {
      info->hMappedFile = fd;
      info->data = p;
      info->file_size = st.st_size;
      ok = true;
    }
  }
  if (!ok && fd != -1)
    close(fd);
  return ok;
}

inline void UnMemoryMapFile(mmioInfo* info)
{
  if (info->data != 0)
#ifdef OS_SOLARIS  
    munmap((char *)info->data, info->file_size);
#else
    munmap(info->data, info->file_size);
#endif
  if (info->hMappedFile != -1)
    close(info->hMappedFile);
}
