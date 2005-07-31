/*
    movierecorder.h - Crystal Space plugin interface
                      for a realtime movie recorder.

    Copyright (C) 2003 by Micah Dowty <micah@navi.picogui.org>

    NOTE that this plugin is GPL, not LGPL.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
   
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __CS_IVARIA_MOVIERECORDER_H__
#define __CS_IVARIA_MOVIERECORDER_H__

#include "csutil/scf.h"

SCF_VERSION (iMovieRecorder, 0, 0, 1);

/**
 * Using this interface you can communicate with the MovieRecorder plugin.
 * This allows changing or disabling the hotkey bindings, changing the
 * video parameters, and programmatically starting and stopping the recorder.
 */
struct iMovieRecorder : public virtual iBase
{
  /// Start recording using the settings in the configuration system
  virtual void Start(void) = 0;

  /// Stop recording if a recording is in progress
  virtual void Stop(void) = 0;

  /// Are we recording?
  virtual bool IsRecording(void) const = 0;

  /// Pause an in-progress recording
  virtual void Pause(void) = 0;

  /// Resume an in-progress recording
  virtual void UnPause(void) = 0;

  /// Is the recording paused?
  virtual bool IsPaused(void) const = 0;
};

#endif // __CS_IVARIA_MOVIERECORDER_H__

