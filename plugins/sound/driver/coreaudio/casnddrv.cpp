// casnddrv.cpp
// CoreAudio (MacOS/X) Sound Driver for Crystal Space
//
// Created by mreda on Sun Nov 11 2001.
// Copyright (c) 2001 Matt Reda. All rights reserved.


#include "cssysdef.h"
#include "csver.h"
#include "csutil/sysfunc.h"
#include "csutil/scf.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "isound/renderer.h"
#include "casnddrv.h"


#define SAMPLES_PER_BUFFER (8 * 1024)


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csSoundDriverCoreAudio)


SCF_IMPLEMENT_IBASE(csSoundDriverCoreAudio)
    SCF_IMPLEMENTS_INTERFACE(iSoundDriver)
    SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(csSoundDriverCoreAudio::eiComponent)
    SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


// CoreAudio IO proc
static OSStatus AudioProc(AudioDeviceID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData,
                            const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData,
                            const AudioTimeStamp *inOutputTime, void *inClientData);


// Constructor
csSoundDriverCoreAudio::csSoundDriverCoreAudio(iBase *base)
{
    SCF_CONSTRUCT_IBASE(base);
    SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
    reg = 0;
    soundRender = 0;
    memory = 0;
    memorySize = 0;
    frequency = 0;
    is16Bit = false;
    isStereo = false;
    isPlaying = false;
}


// Destructor
csSoundDriverCoreAudio::~csSoundDriverCoreAudio()
{
    if (isPlaying)
        Close();
    SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
    SCF_DESTRUCT_IBASE();
}


// Open
// Open the driver and begin playing
bool csSoundDriverCoreAudio::Open(iSoundRender *render, int freq, bool bit16, bool stereo)
{
    // Report driver information
    csReport(reg, CS_REPORTER_SEVERITY_NOTIFY, CS_SOUND_DRIVER,
            CS_PLATFORM_NAME " CoreAudio sound driver for Crystal Space "
            CS_VERSION_NUMBER "\nWritten by Matt Reda <mreda@mac.com>");

    OSStatus status;
    UInt32 propertySize, bufferSize;		// bufferSize is in bytes
    AudioStreamBasicDescription outStreamDesc;

    // Get output device
    propertySize = sizeof(audioDevice);
    status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propertySize, &audioDevice);
    if ((status != 0) || (audioDevice == kAudioDeviceUnknown))
        return false;

    // Set buffer size
    propertySize = sizeof(bufferSize);
    bufferSize = SAMPLES_PER_BUFFER * sizeof(float);
    status = AudioDeviceSetProperty(audioDevice, 0, 0, false,
                                        kAudioDevicePropertyBufferSize, propertySize, &bufferSize);
    if (status != 0)
        return false;

    // Get stream information
    propertySize = sizeof(outStreamDesc);
    status = AudioDeviceGetProperty(audioDevice, 0, false, kAudioDevicePropertyStreamFormat,
                                        &propertySize, &outStreamDesc);
    if (status != 0)
        return false;

    // Creation went ok, copy to local variables
    soundRender = render;
    isStereo = (outStreamDesc.mChannelsPerFrame == 2);
    is16Bit = true;
    frequency = (int) outStreamDesc.mSampleRate;

    // Allocate memory
    memorySize = SAMPLES_PER_BUFFER * sizeof(short);
    memory = (short *) malloc(memorySize);

    // Add callback
    status = AudioDeviceAddIOProc(audioDevice, AudioProc, this);
    if (status != 0)
        return false;

    // Begin playback
    status = AudioDeviceStart(audioDevice, AudioProc);
    if (status != 0)
        return false;

    // Indicate that Initialization has completed
    isPlaying = true;

    return true;
}


// Close
// Stop playback and clean up
void csSoundDriverCoreAudio::Close()
{
    if (isPlaying)
    {
        OSStatus status;
        status = AudioDeviceStop(audioDevice, AudioProc);
        if (status != 0)
            return;

        status = AudioDeviceRemoveIOProc(audioDevice, AudioProc);
        if (status != 0)
            return;

        free(memory);
        memorySize = 0;

        isPlaying = false;
    };
}


// LockMemory
// Return memor buffer and size
void csSoundDriverCoreAudio::LockMemory(void **mem, int *memsize)
{
    *mem = memory;
    *memsize = memorySize;
}


// UnlockMemory
void csSoundDriverCoreAudio::UnlockMemory()
{
    // Do nothing
}


// IsBackground
// Return true to indicate driver can play in background
bool csSoundDriverCoreAudio::IsBackground()
{
    return true;
}

// Is16Bits
// Return whether or not driver is set up for 16 bit playback
bool csSoundDriverCoreAudio::Is16Bits()
{
    return is16Bit;
}

// IsStereo
// Indicate whether the driver is set up for stereo playback
bool csSoundDriverCoreAudio::IsStereo()
{
    return isStereo;
}

// GetFrequency
// Return playback frequency
int csSoundDriverCoreAudio::GetFrequency()
{
    return frequency;
}


// IsHandleVoidSound
// Return false to indicate driver needs input to create silence
bool csSoundDriverCoreAudio::IsHandleVoidSound()
{
    return false;
}


// CreateSamples
// Ask the renderer for samples, and then scale them
void csSoundDriverCoreAudio::CreateSamples(float *buffer)
{
    // Create new samples
    soundRender->MixingFunction();

    // Copy and scale Crystal Space samples to the floats that CoreAudio can use
    float scaleFactor = 1.0f / SHRT_MAX;
    for (int i = 0; i < SAMPLES_PER_BUFFER; i++)
        buffer[i] = memory[i] * scaleFactor;
}



// AudioProc
// Create samples in output buffer - that will be played automatically
static OSStatus AudioProc(AudioDeviceID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData,
                            const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData,
                            const AudioTimeStamp *inOutputTime, void *inClientData)
{
    csSoundDriverCoreAudio *driver = (csSoundDriverCoreAudio *) inClientData;
    float *buffer = (float *) outOutputData->mBuffers[0].mData;

    driver->CreateSamples(buffer);

    return 0;
}
