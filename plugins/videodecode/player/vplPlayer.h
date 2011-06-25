#ifndef __VPLPLAYER_H__
#define __VPLPLAYER_H__

#include <iutil/comp.h>
#include <videodecode/mediaplayer.h>
#include <videodecode/mediacontainer.h>
#include <videodecode/media.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>

struct iObjectRegistry;

#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.player"

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class vplPlayer : public scfImplementation2<vplPlayer,iMediaPlayer,iComponent>
{
private:
	iObjectRegistry* object_reg;

	csRef<iMediaContainer> _mediaFile;
	csRef<iTextureHandle> _target;

	bool playing;

public:
	vplPlayer (iBase* parent);
	virtual ~vplPlayer ();

	// From iComponent.
	virtual bool Initialize (iObjectRegistry*);

	/// Initialize the video player
	virtual void InitializePlayer (csRef<iMediaContainer> media) ;

	/// Activates a stream from inside the iMediaContainer
	virtual void SetActiveStream (int index) ;

	/// Deactivates a stream from inside the iMediaContainer
	virtual void RemoveActiveAudioStream (int index) ;

	/// Set the target texture
	virtual void SetTargetTexture (csRef<iTextureHandle> target) ;

	/// Called continuously to update the player
	virtual void Update ();

	/// Starts playing the media
	virtual void Play () ;

	/// Pauses the media
	virtual void Pause() ;

	/// Stops the media and seeks to the beginning
	virtual void Stop () ;

	/// Seeks the media
	virtual void Seek (float time) ;

	/// Get the position of the media
	virtual csTicks GetPosition () ;

	/// Returns if the media is playing or not
	virtual bool IsPlaying () ;
};

#endif // __VPLPLAYER_H__