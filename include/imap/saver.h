#ifndef __IMAP_SAVER_H__
#define __IMAP_SAVER_H__

#include "csutil/scf.h"

struct iString;
struct iMetaManager;

SCF_VERSION (iSaver, 0, 0, 2);

  /**
   * This interface is used to serialize the engine
   * contents.
   */ 
struct iSaver : public iBase {
  /**
   * Save the current engine contents to the filename.
   */ 
  virtual bool SaveMapFile(const char *filename)=0;
  /**
   * Return the current engine contents as a string.
   */ 
  virtual iString* SaveMapFile()=0;
};

#endif

