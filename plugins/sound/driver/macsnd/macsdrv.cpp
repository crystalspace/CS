/*
	Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
	Copyright (C) 1998, 1999 by Jorrit Tyberghein
	Written by Nathaniel 'NooTe' Saint Martin

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free
	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdarg.h>

#include "sysdef.h"
#include "cscom/com.h"
#include "cssnddrv/macsnd/macsdrv.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isndsrc.h"

static pascal void SoundDoubleBackProc(
	SndChannelPtr		channel,
	SndDoubleBufferPtr	doubleBuffer );

#define kDoubleBufferSize	4096L

IMPLEMENT_UNKNOWN_NODELETE (csSoundDriverMac)

BEGIN_INTERFACE_TABLE(csSoundDriverMac)
  IMPLEMENTS_INTERFACE(ISoundDriver)
END_INTERFACE_TABLE()

csSoundDriverMac::csSoundDriverMac(ISystem* piSystem)
{
	m_piSystem = piSystem;
	mStopPlayback = false;
	mSoundDBHeader.dbhDoubleBack = NULL;
	mSoundDBHeader.dbhBufferPtr[0] = NULL;
	mSoundDBHeader.dbhBufferPtr[1] = NULL;
	mSoundChannel = NULL;
	mFramesPerBuffer = 0L;
	mBuffersFilled = 0L;
}

csSoundDriverMac::~csSoundDriverMac()
{
}

STDMETHODIMP csSoundDriverMac::Open(ISoundRender *render, int frequency, bool bit16, bool stereo)
{
	SysPrintf (MSG_INITIALIZATION, "\nSoundDriver Mac selected\n");

	m_piSoundRender = render;
	OSErr	theError;
	short	outputChannels;

	m_bStereo = stereo;
	m_b16Bits = bit16;
	m_nFrequency = frequency;
	MemorySize = kDoubleBufferSize;

 	if ( stereo ) {
 		outputChannels = initStereo;
 		mSoundDBHeader.dbhNumChannels = 2;
 	} else {
  		outputChannels = initMono;
 		mSoundDBHeader.dbhNumChannels = 1;
  	}

	theError = SndNewChannel( &mSoundChannel, sampledSynth, outputChannels, NULL );
	if ( theError != noErr ) {
  		SysPrintf( MSG_FATAL_ERROR, "Unable to open a sound channel.");
		return false;
	}

	/*
	 *	Fill in the double buffer header
	 */

	if ( bit16 ) {
 		mSoundDBHeader.dbhSampleSize = 16;
	} else {
 		mSoundDBHeader.dbhSampleSize = 8;
	}

 	mSoundDBHeader.dbhCompressionID = 0;
 	mSoundDBHeader.dbhPacketSize = 0;
 	mSoundDBHeader.dbhSampleRate = frequency << 16;
	mSoundDBHeader.dbhDoubleBack = NewSndDoubleBackProc( SoundDoubleBackProc );

	/*
	 *	Calculate the number of frames per buffer.
	 *	1 frame is = to 2 samples if stereo or 1 sample if mono.
	 */
	mFramesPerBuffer = kDoubleBufferSize;
	if ( m_bStereo )
		mFramesPerBuffer /= 2L;
	if ( m_b16Bits )
		mFramesPerBuffer /= 2L;

	/*
	 *	Get the space for the first buffer.
	 */
	mSoundDBHeader.dbhBufferPtr[0] = (SndDoubleBufferPtr)NewPtr( sizeof( SndDoubleBuffer ) + kDoubleBufferSize );
	if ( mSoundDBHeader.dbhBufferPtr[0] == NULL ) {
		SndDisposeChannel( mSoundChannel, TRUE );
		mSoundChannel = NULL;
		DisposeRoutineDescriptor( mSoundDBHeader.dbhDoubleBack );
		mSoundDBHeader.dbhDoubleBack = NULL;
  		SysPrintf( MSG_FATAL_ERROR, "Unable to get the space for the sound buffer.");
		return false;
	}
	mSoundDBHeader.dbhBufferPtr[0]->dbNumFrames = 0L;
	mSoundDBHeader.dbhBufferPtr[0]->dbFlags = 0;
	mSoundDBHeader.dbhBufferPtr[0]->dbUserInfo[0] = (long)this;

	/*
	 *	Fill in this buffer with sounds.
	 */
	SndDoubleBackProc( mSoundChannel, mSoundDBHeader.dbhBufferPtr[0] );

	/*
	 *	Get the space for the second buffer.
	 */
	mSoundDBHeader.dbhBufferPtr[1] = (SndDoubleBufferPtr)NewPtr( sizeof( SndDoubleBuffer ) + kDoubleBufferSize );
	if ( mSoundDBHeader.dbhBufferPtr[1] == NULL ) {
		SndDisposeChannel( mSoundChannel, TRUE );
		mSoundChannel = NULL;
		DisposeRoutineDescriptor( mSoundDBHeader.dbhDoubleBack );
		mSoundDBHeader.dbhDoubleBack = NULL;
		DisposePtr( (Ptr)mSoundDBHeader.dbhBufferPtr[0] );
		mSoundDBHeader.dbhBufferPtr[0] = NULL;
  		SysPrintf( MSG_FATAL_ERROR, "Unable to get the space for the sound buffer.");
		return false;
	}
	mSoundDBHeader.dbhBufferPtr[1]->dbNumFrames = 0L;
	mSoundDBHeader.dbhBufferPtr[1]->dbFlags = 0;
	mSoundDBHeader.dbhBufferPtr[1]->dbUserInfo[0] = (long)this;

	/*
	 *	Fill in this buffer with sounds.
	 */
	SndDoubleBackProc( mSoundChannel, mSoundDBHeader.dbhBufferPtr[1] );

	/*
	 *	Start the sounds playing.
	 */
	theError = SndPlayDoubleBuffer( mSoundChannel, &mSoundDBHeader );
	if ( theError != noErr ) {
		SndDisposeChannel( mSoundChannel, TRUE );
		mSoundChannel = NULL;
		DisposeRoutineDescriptor( mSoundDBHeader.dbhDoubleBack );
		mSoundDBHeader.dbhDoubleBack = NULL;
		DisposePtr( (Ptr)mSoundDBHeader.dbhBufferPtr[0] );
		mSoundDBHeader.dbhBufferPtr[0] = NULL;
		DisposePtr( (Ptr)mSoundDBHeader.dbhBufferPtr[1] );
		mSoundDBHeader.dbhBufferPtr[1] = NULL;
  		SysPrintf( MSG_FATAL_ERROR, "Unable to start the sound playing.");
		return false;
	}

	GetDefaultOutputVolume( &mOutputVolume );

  return S_OK;
}

STDMETHODIMP csSoundDriverMac::Close()
{
	SCStatus	theStatus;

	/*
	 *	Tell the double buffer callback that this is the end.
	 */
	mStopPlayback = true;

	if ( mSoundChannel ) {
		/*
		 *	Wait until the sound channel is done.
		 */
		do {
			SndChannelStatus( mSoundChannel, sizeof( SCStatus ), &theStatus );
		} while ( theStatus.scChannelBusy );

		/*
		 *	Get rid of the sound channel.
		 */
		SndDisposeChannel( mSoundChannel, TRUE );
	}

	if ( mSoundDBHeader.dbhDoubleBack )
		DisposeRoutineDescriptor( mSoundDBHeader.dbhDoubleBack );

	/*
	 *	Get rid of the memory used for the double buffers
	 */
	if ( mSoundDBHeader.dbhBufferPtr[0] )
		DisposePtr( (Ptr)mSoundDBHeader.dbhBufferPtr[0] );
	if ( mSoundDBHeader.dbhBufferPtr[1] )
		DisposePtr( (Ptr)mSoundDBHeader.dbhBufferPtr[1] );

	/*
	 *	Make sure our superclass does not also go a dispose of the memory
	 */
	Memory = NULL;

	SetDefaultOutputVolume( mOutputVolume );


  return S_OK;
}

STDMETHODIMP csSoundDriverMac::SetVolume(float newVolume)
{
	long  theVolume;

	/*
	 *	Make sure the volume is between 0 and 1.
	 */
	if (( newVolume < 0 ) || ( newVolume > 1 ))
		return S_FALSE;

	/*
	 *	The mac volume is between 0 and 256 so scale up the passed in volume.
	 */
	theVolume = newVolume * 256.0;
	SetDefaultOutputVolume( theVolume );

	return S_OK;
}

STDMETHODIMP csSoundDriverMac::GetVolume(float *vol)
{
	*vol = volume;

	return S_OK;
}

STDMETHODIMP csSoundDriverMac::LockMemory(void **mem, int *memsize)
{
	*mem = Memory;
	*memsize = MemorySize;

	return S_OK;
}

STDMETHODIMP csSoundDriverMac::UnlockMemory()
{
	return S_OK;
}

STDMETHODIMP csSoundDriverMac::IsBackground(bool *back)
{
	*back = true;

	return S_OK;
}

STDMETHODIMP csSoundDriverMac::Is16Bits(bool *bit)
{
	*bit = m_b16Bits;

	return S_OK;
}

STDMETHODIMP csSoundDriverMac::IsStereo(bool *stereo)
{
	*stereo = m_bStereo;

	return S_OK;
}

STDMETHODIMP csSoundDriverMac::GetFrequency(int *freq)
{
	*freq = m_nFrequency;

	return S_OK;
}

STDMETHODIMP csSoundDriverMac::IsHandleVoidSound(bool *handle)
{
	*handle = false;

	return S_OK;
}

void csSoundDriverMac::SysPrintf(int mode, char* szMsg, ...)
{
	char buf[1024];
	va_list arg;
  
	va_start (arg, szMsg);
	vsprintf (buf, szMsg, arg);
	va_end (arg);
  
	m_piSystem->Print(mode, buf);
}

void csSoundDriverMac::SndDoubleBackProc(
	SndChannelPtr		channel,
	SndDoubleBufferPtr	doubleBuffer )
{
	/*
	 *	Set the location where the sounds will be written into by the
	 *	mixing function to the currently empty buffer.
	 */
	Memory = &(doubleBuffer->dbSoundData[0]);

	/*
	 *	Get more sound samples from the mixing function.
	 */
	m_piSoundRender->MixingFunction();

	/*
	 *	Mark the buffer as ready to be emptied.
	 *	If this object is being closed mark the buffer
	 *	as the last one to be processed.
	 */
	doubleBuffer->dbNumFrames = mFramesPerBuffer;
	doubleBuffer->dbFlags |= dbBufferReady;
	if ( mStopPlayback )
		doubleBuffer->dbFlags |= dbLastBuffer;

	++mBuffersFilled;
}

static pascal void SoundDoubleBackProc(
	SndChannelPtr		channel,
	SndDoubleBufferPtr	doubleBuffer )
{
	csSoundDriverMac *me;

	/*
	 *	Get the address to this object from the double buffer user info.
	 *	It was placed there earlier.
	 */
	me = (csSoundDriverMac *)(doubleBuffer->dbUserInfo[0]);

	/*
	 *	Set the location where the sounds will be written into by the
	 *	mixing function to the currently empty buffer.
	 */
	me->SndDoubleBackProc( channel, doubleBuffer );
}
