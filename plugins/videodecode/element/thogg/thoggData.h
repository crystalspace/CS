#ifndef __THOGGDATA_H__
#define __THOGGDATA_H__

#include <iutil/comp.h>
#include <videodecode/vpl_data.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>

#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.element.thogg"

struct iObjectRegistry;
struct csVPLvideoFormat;

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class thoggData : public scfImplementation2<thoggData,iVPLData,iComponent>
{
private:
  const char * pDescription;
  csVPLvideoFormat *format;
  iObjectRegistry* object_reg;

public:
  thoggData (iBase* parent);
  virtual ~thoggData ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  /// Get the format of the sound data.
  virtual const csVPLvideoFormat *GetFormat();

  virtual void SetFormat(csVPLvideoFormat *vplFormat);

  /// Get size of this sound in frames.
  virtual size_t GetFrameCount();

  /**
   * Return the size of the data stored in bytes.  This is informational only
   * and is not guaranteed to be a number usable for sound calculations.
   * For example, an audio file compressed with variable rate compression may
   * result in a situation where FILE_SIZE is not equal to
   * FRAME_COUNT * FRAME_SIZE since FRAME_SIZE may vary throughout the
   * audio data.
   */
  virtual size_t GetDataSize();

  /// Set an optional description to be associated with this sound data
  //   A filename isn't a bad idea!
  virtual void SetDescription(const char *pDescription);

  /// Retrieve the description associated with this sound data
  //   This may return 0 if no description is set.
  virtual const char *GetDescription();

  virtual void getNextFrame(vidFrameData &data);
};

#endif // __THOGGLOADER_H__