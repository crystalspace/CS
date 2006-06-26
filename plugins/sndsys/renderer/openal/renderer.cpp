/*
	Copyright (C) 2006 by Søren Bøg

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

#include "cssysdef.h"

#include "isndsys/ss_structs.h"

#include "csutil/sysfunc.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/csendian.h"
#include "csutil/scopedmutexlock.h"

#include "iutil/plugin.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "isndsys/ss_data.h"
#include "isndsys/ss_stream.h"
#include "isndsys/ss_listener.h"
#include "isndsys/ss_renderer.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "iutil/cmdline.h"
#include "ivaria/reporter.h"

#include "AL/alc.h"

#include "listener.h"
#include "source.h"
#include "renderer.h"
#include "scopedlock.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csSndSysRendererOpenAL)

/*
 * Constructor / Destructor
 */
csSndSysRendererOpenAL::csSndSysRendererOpenAL(iBase* pParent) :
  scfImplementationType(this, pParent),
  m_Device( 0 ), m_Context( 0 )
{
  m_ContextLock = csMutex::Create( true );
}

csSndSysRendererOpenAL::~csSndSysRendererOpenAL()
{
}

/*
 * iComponent interface
 */
bool csSndSysRendererOpenAL::Initialize (iObjectRegistry *obj_reg)
{
  // Save the object registry for later use
  m_ObjectRegistry = obj_reg;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Initializing OpenAL sound system");

  // Read the config file
  m_Config.AddConfig (obj_reg, "/config/sound.cfg");

  // Get a list of OpenAL devices:
  // The spec says it's an ALCchar, but my headers include no such type.
  // The format is a series of strings separatrd by nulls, teminated by a
  // double null.
  Report (CS_REPORTER_SEVERITY_DEBUG, "Retriving available devices.");
  const ALCchar *devices = alcGetString (0, ALC_DEVICE_SPECIFIER);
  // Loop while there is no second null
  while (*devices != 0) {
    Report (CS_REPORTER_SEVERITY_DEBUG, "Available OpenAL device: %s", devices);
    // Seek until the null
    while (*devices != 0)
      devices++;
    // Skip the null
    devices++;
  }

  // Report the default device.
  Report (CS_REPORTER_SEVERITY_DEBUG, "Default OpenAL device: %s", alcGetString (0, ALC_DEFAULT_DEVICE_SPECIFIER));

  // Check if a specific device is specified
  const ALCchar *device = m_Config->GetStr ("SndSys.OpenALDevice", 0);
  if (device == 0) {
    // The config did not contain a device to use
    Report (CS_REPORTER_SEVERITY_DEBUG, "No device spcified");
  } else {
    // Attempt to open the spcified device
    m_Device = alcOpenDevice (device);
    if (m_Device == 0) {
      // Failed to open the device
      Report (CS_REPORTER_SEVERITY_WARNING, "Unable to open device %s", device);
    }
  }

  // If we still don't have a device, try the default:
  if (m_Device == 0) {
    Report (CS_REPORTER_SEVERITY_DEBUG, "Falling back on default device");
    m_Device = alcOpenDevice (0);
    if (m_Device == 0) {
      // Even that failed, give up.
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to open device");
      return false;
    }
  }

  // Register for event callbacks.
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(m_ObjectRegistry, iEventQueue));
  evSystemOpen = csevSystemOpen(m_ObjectRegistry);
  evSystemClose = csevSystemClose(m_ObjectRegistry);
  evFrame = csevFrame(m_ObjectRegistry);
  if (q != 0) {
    csEventID subEvents[] = { evSystemOpen, evSystemClose, evFrame, CS_EVENTLIST_END };
    q->RegisterListener(this, subEvents);
  }

  return true;
}

/*
 * iEventHandler interface
 */
bool csSndSysRendererOpenAL::HandleEvent (iEvent &e)
{
  if (e.Name == evFrame) 
  {
    // Process any changes
    Update();
  }
  else if (e.Name == evSystemOpen) 
  {
    // Open the renderer
    Open();
  }
  else if (e.Name == evSystemClose) 
  {
    // Close the renderer
    Close();
  }
  return false;
}

/*
 * iSndSysRenderer interface
 */
void csSndSysRendererOpenAL::SetVolume (float vol)
{
  m_Listener->SetVolume (vol);
}

float csSndSysRendererOpenAL::GetVolume ()
{
  return m_Listener->GetVolume ();
}

csPtr<iSndSysStream> csSndSysRendererOpenAL::CreateStream(iSndSysData* data,
      int mode3d)
{
  // Copy over format information
  csSndSysSoundFormat format = *data->GetFormat ();

  // Make sure 3D streams are in mono
  if (mode3d != CS_SND3D_DISABLE)
    format.Channels = 1;

  // Create the stream
  iSndSysStream *stream = data->CreateStream (&format, mode3d);

  // Add it the our list of streams:
  m_Streams.Push (stream);

  // TODO: Callback notification

  return stream;
}

csPtr<iSndSysSource> csSndSysRendererOpenAL::CreateSource(iSndSysStream* stream)
{
  iSndSysSourceOpenAL *source = 0;

  // Get a lock on the context
  ScopedRendererLock lock (*this);

  // Create the correct type of source.
  if (stream->Get3dMode() == CS_SND3D_DISABLE)
    source = new SndSysSourceOpenAL2D( stream, this );
  else
    // TODO: Create a proper 3D source
    source = new SndSysSourceOpenAL2D( stream, this );

  // Add it the our list of sources:
  m_Sources.Push (source);

  // TODO: Callback notification

  return scfQueryInterface<iSndSysSource>( source );
}

bool csSndSysRendererOpenAL::RemoveStream(iSndSysStream* stream)
{
  // I have got to be overlooking something here.
   m_Streams.Delete (stream);

  // TODO: Callback notification

  return true;
}

bool csSndSysRendererOpenAL::RemoveSource(iSndSysSource* source)
{
  // Get a lock on the context
  ScopedRendererLock lock (*this);

  // I have got to be overlooking something here.
  m_Sources.Delete (dynamic_cast<iSndSysSourceOpenAL*> (source));

  // TODO: Callback notification

  return true;
}

csRef<iSndSysListener> csSndSysRendererOpenAL::GetListener ()
{
  return m_Listener;
}

bool csSndSysRendererOpenAL::RegisterCallback(iSndSysRendererCallback *pCallback)
{
  // TODO: Callback notification
}

bool csSndSysRendererOpenAL::UnregisterCallback(iSndSysRendererCallback *pCallback)
{
  // TODO: Callback notification
}

/*
 * iSndSysRendererOpenAL interface
 */
bool csSndSysRendererOpenAL::LockWait()
{
  m_ContextLock->LockWait();

  // Make sure the context is actually valid, and not already current.
  if (m_Context != 0 && m_Context != alcGetCurrentContext())
    alcMakeContextCurrent( m_Context );
  else
    return false;
  return true;
}

void csSndSysRendererOpenAL::Release()
{
  //alcMakeContextCurrent( 0 );
  m_ContextLock->Release();
}

/*
 * csSndSysRendererOpenAL implementation
 */
void csSndSysRendererOpenAL::Report(int severity, const char* msg, ...)
{
  va_list arg;

  // TODO: Send notification to alternate destinations

  // Send notification to the reporter
  va_start (arg, msg);
  csReportV (m_ObjectRegistry, severity,
    "crystalspace.sndsys.renderer.openal", msg, arg);
  va_end (arg);
}

void csSndSysRendererOpenAL::Update()
{
  // Get exclusive access to the OpenAL context.
  ScopedRendererLock lock (*this);

  // Make sure the context, not really necessary as we requested a async
  // context, but documentation recommends it, in case async contexts are not
  // supported.
  //alcProcessContext (m_Context);

  // Update the listeners state.
  m_Listener->Update();

  // Update the sources
  size_t iMax = m_Sources.GetSize();
  for (size_t i=0;i<iMax;i++)
    m_Sources[i]->PerformUpdate();

  // Check for any errors
  ALCenum err = alcGetError (m_Device);
  if (err != ALC_NO_ERROR)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "An OpenAL error occured: %s", alcGetString (m_Device, err));
    CS_ASSERT (err == ALC_NO_ERROR)
  }

}

void csSndSysRendererOpenAL::Open()
{
  ScopedRendererLock lock (*this);

  // First assume we have both a config and a device, but no context.
  CS_ASSERT (m_Config != 0);
  CS_ASSERT (m_Device != 0);
  CS_ASSERT (m_Context == 0);

  Report (CS_REPORTER_SEVERITY_DEBUG, "Opening OpenAL sound system");

  // Clear any error condition
  alcGetError (m_Device);

  // Setup the attribute list for the OpenAL context
  const ALCint attr[] =
  {
    ALC_REFRESH,   m_Config->GetInt ("SndSys.OpenALRefresh", 10),    // How often do we update the mixahead buffer (hz).
    ALC_SYNC,      AL_FALSE,                                         // We want an asynchronous context.
    0
  };
  // Note: If the sound is choppy, it may be because your OpenAL
  //       implementation does not implement async (threaded) contexts and
  //       your framerate is below SndSys.OpenALRefresh. If this is the case,
  //       please try to decrease SndSys.OpenALRefresh to below your
  //       framerate. This however will increase sound latency. Alternatively
  //       you may attempt to implement the async operation in CS.

  // Get an OpenAL context
  m_Context = alcCreateContext (m_Device, attr);
  if (m_Context == 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Unable to get OpenAL context");
    CS_ASSERT (m_Context != 0);
  }

  // Make our new context current
  alcMakeContextCurrent (m_Context);

  // Set the context processing
  alcProcessContext (m_Context);

  // Check for any errors
  ALCenum err = alcGetError (m_Device);
  if (err != ALC_NO_ERROR)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "An OpenAL error occured: %s", alcGetString (m_Device, err));
    CS_ASSERT (err == ALC_NO_ERROR)
  }

  // Create a listener
  m_Listener = new SndSysListenerOpenAL();
}

void csSndSysRendererOpenAL::Close()
{
  ScopedRendererLock lock (*this);

  Report (CS_REPORTER_SEVERITY_DEBUG, "Closing OpenAL sound system");

  // Do we have a context?
  if (m_Context != 0) {
    // TODO: Remove sources and streams

    // Finally, destroy the context.
    Report (CS_REPORTER_SEVERITY_DEBUG, "Destroying context");
    alcDestroyContext (m_Context);
    m_Context = 0;
  }
}
