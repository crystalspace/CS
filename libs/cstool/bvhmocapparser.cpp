/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csgeom/matrix3.h"
#include "csutil/plugmgr.h"
#include "imesh/animnode/skeleton2anim.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "cstool/mocapparser.h"

bool ParseLine (iFile* file, char* buf, int nbytes)
{
  if (!file)
    return false;

  char c = '\n';
  while (c == '\n' || c == '\r')
    if (!file->Read (&c, 1))
      break;

  if (file->AtEOF())
    return false;

  char* p = buf;
  const char* plim = p + nbytes - 1;
  while (p < plim)
  {
    if (c == '\n' || c == '\r')
      break;
    *p++ = c;
    if (!file->Read (&c, 1))
      break;
  }
  *p = '\0';
  return true;
}

bool ParseWord (const char* txt, char* buf, int& start)
{
  int index = start;
  while (txt[index] != ' '
	 && txt[index] != '\0')
  {
    buf[index - start] = txt[index];
    index++;
  }
  buf[index - start] = '\0';
  bool found = index - start != 0;
  start = index;
  return found;
}

CS::Animation::BVHMocapParser::BVHMocapParser (iObjectRegistry* object_reg, iVFS* vfs)
  : object_reg (object_reg), vfs (vfs), startFrame (0), endFrame (0), globalScale (0.01f),
    endSitesAdded (true)
{
}

bool CS::Animation::BVHMocapParser::SetRessourceFile (const char* filename)
{
  this->filename = filename;

  if (!vfs->Exists (this->filename.GetData ()))
  {
    size_t index = this->filename.FindLast ('\\');
    if (index == (size_t) -1)
      index = this->filename.FindLast ('/');

    if (index != (size_t) -1)
    {
      csString vfspath = this->filename.Slice (0, index + 1);
      filenameVFS = "/mocapviewer_datapath/";

      if (!vfs->Mount (filenameVFS.GetData (), vfspath.GetData ()))
	return Report (CS_REPORTER_SEVERITY_ERROR, "Mount failed on path %s", vfspath.GetData ());

      filenameVFS += this->filename.Slice (index + 1);
    }
    else
      filenameVFS = this->filename;
  }
  else
    filenameVFS = this->filename;

  if (!vfs->Exists (filenameVFS.GetData ()))
    return Report (CS_REPORTER_SEVERITY_ERROR, "File %s does not exist", filename);

  return true;
}

void CS::Animation::BVHMocapParser::SetStartFrame (size_t frame)
{
  this->startFrame = frame;
}

void CS::Animation::BVHMocapParser::SetEndFrame (size_t frame)
{
  this->endFrame = frame;
}

void CS::Animation::BVHMocapParser::SetGlobalScale (float scale)
{
  globalScale = scale;
}

void CS::Animation::BVHMocapParser::SetEndSitesAdded (bool added)
{
  endSitesAdded = added;
}

CS::Animation::MocapParserResult CS::Animation::BVHMocapParser::ParseData ()
{
  char buf[256];
  csString buffer;
  size_t textSize;
  csRef<iPluginManager> plugmgr;
  csRef<CS::Animation::iSkeletonManager> skeletonManager;
  endSitesCount = 0;
  sampleBone = CS::Animation::InvalidBoneID;

  // Open the ressource file
  csRef<iFile> file = vfs->Open (filenameVFS.GetData (), VFS_FILE_READ);
  if (!file)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not open file %s",
	    filename.GetData ());
    goto parsing_failed;
  }

  // Parse 'HIERARCHY' tag
  if (!ParseLine (file, buf, 255)
      || strcmp (buf, "HIERARCHY") != 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Malformed BVH file: no 'HIERARCHY' tag");
    goto parsing_failed;
  }

  // Load the iSkeletonManager plugin
  plugmgr = csQueryRegistry<iPluginManager> (object_reg);
  skeletonManager = csLoadPlugin<CS::Animation::iSkeletonManager>
    (plugmgr, "crystalspace.skeletalanimation");
  if (!skeletonManager)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
	    "Could not load CS::Animation::iSkeletonManager plugin");
    goto parsing_failed;
  }

  // Create the animation packet, the skeleton factory and the animation
  result.animPacketFactory =
    skeletonManager->CreateAnimPacketFactory ((filename + "_packet").GetData ());
  result.skeletonFactory = 
    skeletonManager->CreateSkeletonFactory ((filename + "_skelfact").GetData ());
  result.skeletonFactory->SetAnimationPacket (result.animPacketFactory);
  animation = 
    result.animPacketFactory->CreateAnimation ((filename + "_anim").GetData ());

  // Parse the skeleton
  totalChannelCount = 0;
  if (!ParseSkeletonBone (file, CS::Animation::InvalidBoneID))
    goto parsing_failed;

  // Parse the 'MOTION' tag
  if (!ParseLine (file, buf, 255)
      || strcmp (buf, "MOTION") != 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Malformed BVH file: no 'MOTION' tag");
    goto parsing_failed;
  }

  // Parse the 'Frames' tag
  textSize = strlen ("Frames:");
  if (!ParseLine (file, buf, 255)
      || strncmp (buf, "Frames:", textSize) != 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Malformed BVH file: no 'Frames' tag");
    goto parsing_failed;
  }

  buffer = buf;
  buffer = buffer.Slice (textSize);
  buffer.Collapse ();

  if (sscanf (buffer.GetData (), "%i", &frameCount) != 1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Malformed BVH file: could not parse frame count");
    goto parsing_failed;
  }

  // Parse the 'Frame Time' tag
  textSize = strlen ("Frame Time:");
  if (!ParseLine (file, buf, 255)
      || strncmp (buf, "Frame Time:", textSize) != 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Malformed BVH file: no 'Frame Time' tag");
    goto parsing_failed;
  }

  buffer = buf;
  buffer = buffer.Slice (textSize);
  buffer.Collapse ();

  if (sscanf (buffer.GetData (), "%f", &frameDuration) != 1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Malformed BVH file: could not parse frame duration");
    goto parsing_failed;
  }

  // Parse the animation data
  currentFrame = 0;
  while (!file->AtEOF ())
    if (!ParseAnimationFrame (file))
      goto parsing_failed;

  if (currentFrame != frameCount)
    Report (CS_REPORTER_SEVERITY_WARNING,
	    "Malformed BVH file: not as many frames as announced");

  // Validate the results
  result.result = true;
  result.frameCount = sampleBone == CS::Animation::InvalidBoneID ? 0
    : animation->GetKeyFrameCount (animation->FindChannel (sampleBone));
  result.frameDuration = frameDuration;
  return result;

 parsing_failed:
  // Invalidate the results
  result.result = false;
  result.animPacketFactory = nullptr;
  result.skeletonFactory = nullptr;
  result.frameCount = 0;
  result.frameDuration = 0.0f;
  return result;
}

bool CS::Animation::BVHMocapParser::ParseSkeletonBone
(iFile* file, CS::Animation::BoneID parentBone, const char* previousLine)
{
  char buf[512];
  csString buffer;

  // Parse a first line
  if (previousLine)
    buffer = previousLine;
  else
  {
    if (!ParseLine (file, buf, 511))
      return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: no root tag");
    buffer = buf;
    buffer.Collapse ();
  }

  // Check if this bone definition is an end site

  if (buffer == "End Site")
  {
    if (!ParseLine (file, buf, 511))
      return false;

    // Parse the offset
    if (!ParseLine (file, buf, 511))
      return Report (CS_REPORTER_SEVERITY_ERROR,
		     "Malformed BVH file: no 'OFFSET' tag");

    else if (endSitesAdded)
    {
      // Parse effectively the offset
      buffer = buf;
      buffer.Collapse ();

      if (!buffer.StartsWith ("OFFSET "))
	return Report (CS_REPORTER_SEVERITY_ERROR,
		       "Malformed BVH file: no 'OFFSET' tag");

      csVector3 offset;
      if (!ParseVector (buffer.Slice (strlen ("OFFSET ")), offset))
	return Report (CS_REPORTER_SEVERITY_ERROR,
		       "Malformed BVH file: could not parse 'OFFSET' value");

      // The BVH file format is in a right-handed coordinate system, while CS is left-handed.
      // Therefore, convert the transform:
      offset[2] = -offset[2];

      // Create the bone entry
      CS::Animation::BoneID boneID = result.skeletonFactory->CreateBone (parentBone);
      csString txt;
      txt.Format ("EndSite%i", endSitesCount++);
      result.skeletonFactory->SetBoneName (boneID, txt.GetData());
      offset *= globalScale;
      csQuaternion rotation;
      result.skeletonFactory->SetTransformBoneSpace (boneID, rotation, offset);
    }

    if (!ParseLine (file, buf, 511))
      return false;

    return true;
  }

  // Parse the bone name
  csString boneName;
  if (parentBone == CS::Animation::InvalidBoneID)
  {
    if (!buffer.StartsWith ("ROOT "))
      return Report (CS_REPORTER_SEVERITY_ERROR,
		     "Malformed BVH file: no 'ROOT' tag");
    else
      buffer.SubString (boneName, strlen ("ROOT "));
  }

  else
  {
    if (!buffer.StartsWith ("JOINT "))
      return Report (CS_REPORTER_SEVERITY_ERROR,
		     "Malformed BVH file: no 'JOINT' tag");
    else
      buffer.SubString (boneName, strlen ("JOINT "));
  }

  // Parse the start parenthesis
  if (!ParseLine (file, buf, 511))
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: no '{' tag");
  buffer = buf;
  buffer.Collapse ();

  if (buffer != "{")
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: no '{' tag");

  // Parse the offset
  if (!ParseLine (file, buf, 511))
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: no 'OFFSET' tag");
  buffer = buf;
  buffer.Collapse ();

  if (!buffer.StartsWith ("OFFSET "))
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: no 'OFFSET' tag");

  csVector3 offset;
  if (!ParseVector (buffer.Slice (strlen ("OFFSET ")), offset))
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: could not parse 'OFFSET' value");
  offset[2] = -offset[2];

  // Parse the channels
  if (!ParseLine (file, buf, 511))
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: no 'CHANNELS' tag");
  buffer = buf;
  buffer.Collapse ();

  if (!buffer.StartsWith ("CHANNELS "))
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: no 'CHANNELS' tag");

  // Create the bone entry
  CS::Animation::BoneID boneID = result.skeletonFactory->CreateBone (parentBone);
  result.skeletonFactory->SetBoneName (boneID, boneName.GetData());
  offset *= globalScale;
  csQuaternion rotation;
  result.skeletonFactory->SetTransformBoneSpace (boneID, rotation, offset);

  // Find a sample bone
  if (sampleBone == CS::Animation::InvalidBoneID)
    sampleBone = boneID;

  // Create an animation channel for this bone
  animation->AddChannel (boneID);

  // Parse the channels
  if (!ParseChannels (buffer.Slice (strlen ("CHANNELS ")), boneID, offset))
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: could not parse 'CHANNELS' values");

  // Parse the child bones
  while (1)
  {
    // Parse the end parenthesis
    if (!ParseLine (file, buf, 511))
      return Report (CS_REPORTER_SEVERITY_ERROR,
		     "Malformed BVH file: no '}' tag");
    buffer = buf;
    buffer.Collapse ();

    if (buffer == "}")
      break;

    // Parse a child
    if (!ParseSkeletonBone (file, boneID, buffer.GetData ()))
      return false;
  }

  return true;
}

bool CS::Animation::BVHMocapParser::ParseVector (const char* txt, csVector3& vector)
{
  if (sscanf (txt, "%f %f %f", &vector[0], &vector[1], &vector[2]) == 3)
    return true;
  return false;
}

bool CS::Animation::BVHMocapParser::ParseChannels (const char* txt,
						  CS::Animation::BoneID bone,
						  csVector3 offset)
{
  int channelCount;

  char buf[256];
  int index = 0;

  if (!ParseWord (txt, buf, index)
      || sscanf (txt, "%i", &channelCount) != 1)
    return Report (CS_REPORTER_SEVERITY_ERROR,
		   "Malformed BVH file: could not parse number of channels");

  // Skip one " "
  index++;

  // Create the channel data
  ChannelData channel;
  channel.boneID = bone;
  channel.offset = offset;

  for (int i = 0; i < channelCount; i++)
  {
    if (!ParseWord (txt, buf, index))
      return Report (CS_REPORTER_SEVERITY_ERROR,
		     "Malformed BVH file: could not parse %ith channel", i);

    // Skip one " "
    index++;

    if (strcmp (buf, "Xrotation") == 0)
      channel.dofs.Push (XROT);
    else if (strcmp (buf, "Yrotation") == 0)
      channel.dofs.Push (YROT);
    else if (strcmp (buf, "Zrotation") == 0)
      channel.dofs.Push (ZROT);
    else if (strcmp (buf, "Xposition") == 0)
      channel.dofs.Push (XPOS);
    else if (strcmp (buf, "Yposition") == 0)
      channel.dofs.Push (YPOS);
    else if (strcmp (buf, "Zposition") == 0)
      channel.dofs.Push (ZPOS);
    else if (strcmp (buf, "Xscale") == 0)
      channel.dofs.Push (XSCA);
    else if (strcmp (buf, "Yscale") == 0)
      channel.dofs.Push (YSCA);
    else if (strcmp (buf, "Zscale") == 0)
      channel.dofs.Push (ZSCA);
    else return Report (CS_REPORTER_SEVERITY_ERROR,
			"Malformed BVH file: invalid %ith channel", i);
  }

  if (channel.dofs.GetSize ())
    channels.Push (channel);

  totalChannelCount += channel.dofs.GetSize ();

  return true;
}

bool CS::Animation::BVHMocapParser::ParseAnimationFrame (iFile* file)
{
  const float degree2radian = 3.1415927f / 180.0f;

  size_t bufSize = totalChannelCount * 20;
  CS_ALLOC_STACK_ARRAY (char, buf, bufSize);
  csString buffer;

  if (!ParseLine (file, buf, (int)bufSize - 1))
    return true;

  // Check if we have to add this frame
  buffer = buf;
  buffer.Collapse ();

  if (buffer == "")
    return true;

  currentFrame++;
  if ((startFrame && currentFrame <= (int) startFrame)
      || (endFrame && currentFrame > ((int) endFrame) + 1))
    return true;

  int index = 0;
  char word[256];

  // Compute the frame time
  float frameTime = ((float) currentFrame - 1) * frameDuration;
  if (startFrame)
    frameTime -= ((float) startFrame) * frameDuration;

  // Parse the value of each channel
  csVector3 position;
  csMatrix3 rotation;
  int channelCount = 0;
  for (size_t i = 0; i < channels.GetSize (); i++)
  {
    ChannelData& channel = channels[i];

    position.Set (0.0f);
    rotation.Identity ();

    for (size_t j = 0; j < channel.dofs.GetSize (); j++)
    {
      channelCount++;
      float dofValue;
      if (!ParseWord (buffer.GetData (), word, index)
	  || sscanf (word, "%f", &dofValue) != 1)
	return Report (CS_REPORTER_SEVERITY_ERROR,
		       "Malformed BVH file: could not parse the value of the %ith channel",
		       channelCount);

      switch (channel.dofs[j])
      {
      case XROT:
	rotation = rotation * csXRotMatrix3 (dofValue * degree2radian);
	break;
      case YROT:
	// TODO: the csYRotMatrix3 is defined in right-handed coordinate system!
	rotation = rotation * csYRotMatrix3 (-dofValue * degree2radian);
	break;
      case ZROT:
	rotation = rotation * csZRotMatrix3 (dofValue * degree2radian);
	break;
      case XPOS:
	position[0] = dofValue * globalScale;
	break;
      case YPOS:
	position[1] = dofValue * globalScale;
	break;
      case ZPOS:
	// Convert from right- to left-handed coordinate system
	position[2] = -dofValue * globalScale;
	break;
      default:
	break;
      }

      // Skip one " "
      index++;
    }

    // Convert from right- to left-handed coordinate system
    csVector3 row1 = rotation.Row1 ();
    csVector3 row2 = rotation.Row2 ();
    csVector3 row3 = rotation.Row3 ();
    rotation.Set (row1.x, row1.y, -row1.z, row2.x, row2.y, -row2.z, -row3.x, -row3.y, row3.z);

    // Create an animation keyframe
    CS::Animation::ChannelID channelID = animation->FindChannel (channel.boneID);
    position = position + channel.offset;
    csQuaternion quaternion;
    quaternion.SetMatrix (rotation);
    animation->AddKeyFrame (channelID, frameTime, quaternion, position);
  }

  return true;
}

bool CS::Animation::BVHMocapParser::Report (int severity, const char* msg, ...) const
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
  if (rep)
    rep->ReportV (severity,
		  "crystalspace.libs.cstool.mocapparser.bvh",
		  msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
  return false;
}
