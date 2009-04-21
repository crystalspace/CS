/*
    Copyright (C) 2008 by Jorrit Tyberghein

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

#ifndef __WALKTEST_RECORDER_H__
#define __WALKTEST_RECORDER_H__

#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"
#include "csutil/parray.h"

class WalkTest;
struct iSector;
struct iVFS;
struct iCamera;

/**
 * An entry for the record function.
 */
struct csRecordedCamera
{
  csMatrix3 mat;
  csVector3 vec;
  bool mirror;
  iSector* sector;
  char *cmd;
  char *arg;
  ~csRecordedCamera ()
  { delete [] cmd; delete [] arg; }
};

/**
 * A recorded entry saved in a file.
 */
struct csRecordedCameraFile
{
  int32 m11, m12, m13;
  int32 m21, m22, m23;
  int32 m31, m32, m33;
  int32 x, y, z;
  unsigned char mirror;
};

/**
 * A vector which holds the recorded items and cleans them up if needed.
 */
typedef csPDelArray<csRecordedCamera> csRecordVector;

/**
 * Recording facility for movement and camera.
 */
class WalkTestRecorder
{
private:
  WalkTest* walktest;

  /// Vector with recorded camera transformations and commands.
  csRecordVector recording;
  /// This frames current recorded cmd and arg
  char *recorded_cmd;
  char *recorded_arg;
  /// Time when we started playing back the recording.
  csTicks record_start_time;
  /// Number of frames that have passed since we started playing back recording.
  int record_frame_count;
  /// If >= 0 then we're recording. The value is the current frame entry.
  int cfg_recording;
  /// If >= 0 then we're playing a recording.
  int cfg_playrecording;
  /// If true the demo recording loops.
  bool cfg_playloop;

public:
  WalkTestRecorder (WalkTest* walktest);

  void NewFrame ();
  void Clear ();
  void ToggleRecording ();
  void PlayRecording (bool loop);
  void HandleRecordedCommand ();
  void HandleRecordedCamera (iCamera* camera);

  void SaveRecording (iVFS* vfs, const char* fName);
  void LoadRecording (iVFS* vfs, const char* fName);

  void RecordArgs (const char* cmd, const char* arg);
  void RecordCommand (const char* cmd);
  void RecordCamera (iCamera* camera);
};

#endif // __WALKTEST_RECORDER_H__

