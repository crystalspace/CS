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

/**\file
 * Movie recorder plugin interface
 */

#include "csutil/scf.h"

/**
 * Using this interface you can communicate with the MovieRecorder plugin and programmatically
 * start, pause and stop the recorder. This plugin uses a configuration file (by default in
 * "data/config-plugin/movierecorder.cfg") to setup the various parameters of the recording sessions.
 *
 * The easiest way to use this plugin is to load it at application launch time by adding the option
 * "-plugin=movierecorder" on the command line, then the keys to start, stop and pause the recording
 * are by default "ALT-r" and "ALT-p".
 * \remarks The plugin is GPL, not LGPL.
 */
struct iMovieRecorder : public virtual iBase
{
  SCF_INTERFACE (iMovieRecorder, 0, 0, 1);

  /// Start recording using the settings in the configuration system
  virtual void Start(void) = 0;

  /// Stop recording if a recording is in progress
  virtual void Stop(void) = 0;

  /// Return whether or not a movie is currently recorded
  virtual bool IsRecording(void) const = 0;

  /// Pause an in-progress recording
  virtual void Pause(void) = 0;

  /// Resume an in-progress recording
  virtual void UnPause(void) = 0;

  /// Return whether or not a movie recording is currently paused
  virtual bool IsPaused(void) const = 0;

  /**
   * Set the VFS file that will be used to record the movie. If the file already exists
   * then it will be overwritten. Using this method also overwrite the behavior defined
   * by the filename format (see eg SetFilenameFormat()).
   */
  virtual void SetRecordingFile (const char* filename) = 0;

  /**
   * Set the format of the filename to be used to record movie. The rightmost string of
   * digits in this format will be automatically replaced with a number (eg with a format
   * "/this/crystal000.nuv" the movie files created in the current directory will be called
   * "crystal001.nuv", "crystal002.nuv" and so on). Using this method will overwrite the
   * value defined in the configuration file.
   */
  virtual void SetFilenameFormat (const char* format) = 0;
};

#endif // __CS_IVARIA_MOVIERECORDER_H__

