
#ifndef ISNDLDR_H
#define ISNDLDR_H

#include "iplugin.h"

struct iSoundData;
struct csSoundFormat;

/**
 * The sound loader plugin. This loads sound files and creates iSoundData
 * objects from it, which you can use to play the sound.
 */
SCF_VERSION (iSoundLoader, 0, 0, 1);

struct iSoundLoader : public iPlugIn
{
public:
  /// Initialize the Sound Loader.
  virtual bool Initialize (iSystem *sys) = 0;

  /**
   * Load a sound from the VFS. The fields of the format descriptor may be
   * set to a negative number to let the sound loader choose a format.
   * Set 'Streamed' to true to keep the sound data's original representation
   * as streamed data. This is currently useless but should in the future be
   * helpful to implement music. You should leave it false currently.
   */
  virtual iSoundData *LoadSound(UByte *Buffer, ULong Size,
    const csSoundFormat *RequestFormat, bool Streamed=false) = 0;
};

#endif
