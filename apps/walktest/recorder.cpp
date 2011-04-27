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

#include "cssysdef.h"
#include "csutil/csendian.h"
#include "iutil/vfs.h"
#include "iengine/sector.h"
#include "iengine/camera.h"

#include "walktest.h"
#include "recorder.h"

extern bool CommandHandler (const char *cmd, const char *arg);

WalkTestRecorder::WalkTestRecorder (WalkTest* walktest)
  : walktest (walktest)
{
  cfg_recording = -1;
  recorded_cmd = 0;
  recorded_arg = 0;
  cfg_playrecording = -1;
}

void WalkTestRecorder::NewFrame ()
{
  if (cfg_playrecording >= 0 && recording.GetSize () > 0)
  {
    record_frame_count++;
  }
}

void WalkTestRecorder::HandleRecordedCommand ()
{
  // Emit recorded commands directly to the CommandHandler
  if (cfg_playrecording > 0 &&
	recording.GetSize () > 0)
  {
    csRecordedCamera* reccam = (csRecordedCamera*)recording[
      	cfg_playrecording];
    if (reccam->cmd)
      CommandHandler(reccam->cmd, reccam->arg);
  }
}

void WalkTestRecorder::HandleRecordedCamera (iCamera* c)
{
  if (cfg_playrecording >= 0 && recording.GetSize () > 0)
  {
    csRecordedCamera* reccam = (csRecordedCamera*)recording[cfg_playrecording];
    cfg_playrecording++;
    record_frame_count++;
    if ((size_t)cfg_playrecording >= recording.GetSize ())
    {
      csTicks t1 = record_start_time;
      csTicks t2 = csGetTicks ();
      int num = record_frame_count;
      walktest->Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "End demo: %f secs to render %d frames: %f fps",
	  (float) (t2 - t1) / 1000.,
	  num, float (num) * 1000. / (float) (t2 - t1));
      csPrintf (
	  "End demo: %f secs to render %d frames: %f fps\n",
	  (float) (t2 - t1) / 1000.,
	  num, float (num) * 1000. / (float) (t2 - t1));
      fflush (stdout);
      if (cfg_playloop)
        cfg_playrecording = 0;
      else
        cfg_playrecording = -1;

      record_start_time = csGetTicks ();
      record_frame_count = 0;
    }
    if (reccam->sector)
      c->SetSector (reccam->sector);
    c->SetMirrored (reccam->mirror);
    c->GetTransform ().SetO2T (reccam->mat);
    c->GetTransform ().SetOrigin (reccam->vec);
  }
}

void WalkTestRecorder::Clear ()
{
  recording.DeleteAll ();
  recording.SetSize (0);
}

void WalkTestRecorder::ToggleRecording ()
{
  if (cfg_recording == -1)
  {
    cfg_playrecording = -1;
    cfg_recording = 0;
    walktest->Report (CS_REPORTER_SEVERITY_NOTIFY,
      	"Start recording camera movement...");
  }
  else
  {
    cfg_recording = -1;
    walktest->Report (CS_REPORTER_SEVERITY_NOTIFY,
      	"Stop recording.");
  }
}

void WalkTestRecorder::PlayRecording (bool loop)
{
  if (cfg_playrecording == -1)
  {
    cfg_recording = -1;
    cfg_playrecording = 0;
    walktest->Report (CS_REPORTER_SEVERITY_NOTIFY,
      	"Start playing back camera movement%s...", loop ? "" : " once");
    cfg_playloop = loop;
    record_start_time = csGetTicks ();
    record_frame_count = 0;
  }
  else
  {
    cfg_playrecording = -1;
    walktest->Report (CS_REPORTER_SEVERITY_NOTIFY, "Stop playback.");
  }
}

/// Save recording
void WalkTestRecorder::SaveRecording (iVFS* vfs, const char* fName)
{
  csRef<iFile> cf;
  cf = vfs->Open (fName, VFS_FILE_WRITE);
  uint32 l = (int32)recording.GetSize ();
  l = csLittleEndian::Convert (l);
  cf->Write ((char*)&l, sizeof (l));
  size_t i;
  csRecordedCameraFile camint;
  iSector* prev_sector = 0;
  for (i = 0 ; i < recording.GetSize () ; i++)
  {
    csRecordedCamera* reccam = (csRecordedCamera*)recording[i];
    camint.m11 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m11));
    camint.m12 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m12));
    camint.m13 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m13));
    camint.m21 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m21));
    camint.m22 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m22));
    camint.m23 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m23));
    camint.m31 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m31));
    camint.m32 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m32));
    camint.m33 = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->mat.m33));
    camint.x = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->vec.x));
    camint.y = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->vec.y));
    camint.z = csLittleEndian::Convert (csIEEEfloat::FromNative (reccam->vec.z));
    camint.mirror = reccam->mirror;
    cf->Write ((char*)&camint, sizeof (camint));
    unsigned char len;
    if (prev_sector == reccam->sector)
    {
      len = 255;
      cf->Write ((char*)&len, 1);
    }
    else
    {
      size_t _len = strlen (reccam->sector->QueryObject ()->GetName ());
      len = (_len > 255) ? 255 : (unsigned char)len;
      cf->Write ((char*)&len, 1);
      cf->Write (reccam->sector->QueryObject ()->GetName (),
      	1+len);
    }
    prev_sector = reccam->sector;
    if (reccam->cmd)
    {
      size_t _len = strlen (reccam->cmd);
      len = (_len > 255) ? 255 : (unsigned char)len;
      cf->Write ((char*)&len, 1);
      cf->Write (reccam->cmd, 1+len);
    }
    else
    {
      len = 254;
      cf->Write ((char*)&len, 1);
    }
    if (reccam->arg)
    {
      size_t _len = strlen (reccam->arg);
      len = (_len > 255) ? 255 : (unsigned char)len;
      cf->Write ((char*)&len, 1);
      cf->Write (reccam->arg, 1+len);
    }
    else
    {
      len = 254;
      cf->Write ((char*)&len, 1);
    }
  }
}

/// Load recording
void WalkTestRecorder::LoadRecording (iVFS* vfs, const char* fName)
{
  csRef<iFile> cf;
  cf = vfs->Open (fName, VFS_FILE_READ);
  if (!cf) return;
  recording.DeleteAll ();
  recording.SetSize (0);
  int32 l;
  cf->Read ((char*)&l, sizeof (l));
  l = csLittleEndian::Convert (l);
  csRecordedCameraFile camint;
  iSector* prev_sector = 0;
  int i;
  for (i = 0 ; i < l ; i++)
  {
    csRecordedCamera* reccam = new csRecordedCamera ();
    cf->Read ((char*)&camint, sizeof (camint));
    reccam->mat.m11 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m11));
    reccam->mat.m12 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m12));
    reccam->mat.m13 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m13));
    reccam->mat.m21 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m21));
    reccam->mat.m22 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m22));
    reccam->mat.m23 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m23));
    reccam->mat.m31 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m31));
    reccam->mat.m32 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m32));
    reccam->mat.m33 = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.m33));
    reccam->vec.x = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.x));
    reccam->vec.y = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.y));
    reccam->vec.z = csIEEEfloat::ToNative (csLittleEndian::UInt32 (camint.z));
    reccam->mirror = (camint.mirror != 0);
    unsigned char len;
    cf->Read ((char*)&len, 1);
    iSector* s;
    if (len == 255)
    {
      s = prev_sector;
    }
    else
    {
      char* buf = new char[1+len];
      cf->Read (buf, 1+len);
      s = Sys->Engine->GetSectors ()->FindByName (buf);
      delete[] buf;
    }
    reccam->sector = s;
    prev_sector = s;

    cf->Read ((char*)&len, 1);
    if (len == 254)
    {
      reccam->cmd = 0;
    }
    else
    {
      reccam->cmd = new char[len+1];
      cf->Read (reccam->cmd, 1+len);
    }
    cf->Read ((char*)&len, 1);
    if (len == 254)
    {
      reccam->arg = 0;
    }
    else
    {
      reccam->arg = new char[len+1];
      cf->Read (reccam->arg, 1+len);
    }
    recording.Push (reccam);
  }
}

void WalkTestRecorder::RecordArgs (const char* cmd, const char* arg)
{
  if (cfg_recording >= 0)
  {
    recorded_cmd = new char[strlen(cmd)+1];
    strcpy (recorded_cmd, cmd);
    if (arg)
    {
      recorded_arg = new char[strlen(arg)+1];
      strcpy (recorded_arg, arg);
    }
  }
}

void WalkTestRecorder::RecordCommand (const char* cmd)
{
  if (cfg_recording >= 0)
  {
    recorded_cmd = new char[strlen(cmd)+1];
    strcpy (recorded_cmd, cmd);
  }
}

void WalkTestRecorder::RecordCamera (iCamera* c)
{
  if (cfg_recording >= 0)
  {
    csRecordedCamera* reccam = new csRecordedCamera ();
    const csMatrix3& m = c->GetTransform ().GetO2T ();
    const csVector3& v = c->GetTransform ().GetOrigin ();
    reccam->mat = m;
    reccam->vec = v;
    reccam->mirror = c->IsMirrored ();
    reccam->sector = c->GetSector ();
    reccam->cmd = recorded_cmd;
    reccam->arg = recorded_arg;
    recorded_cmd = recorded_arg = 0;
    recording.Push (reccam);
  }
}

