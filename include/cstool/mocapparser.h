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
#ifndef __CS_CSTOOL_MOCAPREADER_H__
#define __CS_CSTOOL_MOCAPREADER_H__

/**\file
 * Parsing of motion capture data files
 */

#include "csutil/scf_interface.h"
#include "csutil/ref.h"
#include "csutil/csstring.h"
#include "imesh/skeleton2.h"

struct iVFS;

namespace CS {
namespace Animation {

struct iSkeletonFactory;
struct iSkeletonAnimation;

/**
 * Return structure for CS::Animation::MocapParser::ParseData()
 */
struct MocapParserResult
{
  /// Whether the parsing has been successful or not
  bool result;

  /**
   * The newly created animation packet factory, if the parsing is successful, nullptr otherwise.
   * This animation packet will contain the animations imported from the parsing.
   */
  csRef<CS::Animation::iSkeletonAnimPacketFactory> animPacketFactory;

  /// The newly created skeleton factory, if the parsing is successful, nullptr otherwise
  csRef<CS::Animation::iSkeletonFactory> skeletonFactory;

  /// The count of animation frames that have been imported
  size_t frameCount;

  /// The duration of each frames, in second
  float frameDuration;
};

/**
 * Tool for parsing motion capture ressource files and importing them into an animation
 * data suitable for the CS::Mesh::iAnimatedMesh.
 *
 * You should be able to parse successively more than one ressource with the same parser.
 */
class CS_CRYSTALSPACE_EXPORT MocapParser
{
 public:
  /**
   * Set the VFS path of the ressource file containing the motion capture data. You may have
   * to add more than one ressource depending on the motion capture file format.
   */
  virtual bool SetRessourceFile (const char* filename) = 0;

  /**
   * Set the frame where to start importing the animation data. The default value is 0.
   */
  virtual void SetStartFrame (size_t frame) = 0;

  /**
   * Set the frame where to stop importing the animation data. A value of 0 means that
   * all the animations until the end of the ressource file have to be imported. The
   * default value is 0.
   */
  virtual void SetEndFrame (size_t frame) = 0;

  /**
   * Set the global scale to be applied on all dimensions. The default value is 0.01.
   */
  virtual void SetGlobalScale (float scale) = 0;

  /**
   * Parse the ressource files containing the motion capture data
   */
  virtual MocapParserResult ParseData () = 0;
};

/**
 * Tool for parsing BVH (BioVision Hierarchical data) motion capture ressource files.
 */
class CS_CRYSTALSPACE_EXPORT BVHMocapParser : public MocapParser
{
 public:
  /// Constructor
  BVHMocapParser (iObjectRegistry* object_reg, iVFS* vfs);

  /**
   * Set the VFS path of the ressource file containing the motion capture data. For the
   * BVH file format there may be only one ressource file at a time.
   */
  virtual bool SetRessourceFile (const char* filename);

  /**
   * Set the frame where to start importing the animation data. The default value is 0.
   */
  virtual void SetStartFrame (size_t frame);

  /**
   * Set the frame where to stop importing the animation data. A value of 0 means that
   * all the animations until the end of the ressource file have to be imported. The
   * default value is 0.
   */
  virtual void SetEndFrame (size_t frame);

  /**
   * Set the global scale to be applied on all dimensions. The default value is 0.01.
   */
  virtual void SetGlobalScale (float scale);

  /**
   * Parse the file containing the motion capture data
   */
  virtual MocapParserResult ParseData ();

  virtual void SetEndSitesAdded (bool added);

 private:
  bool ParseSkeletonBone (iFile* file, CS::Animation::BoneID parentBone,
			 const char* previousLine = nullptr);
  bool ParseVector (const char* txt, csVector3& vector);
  bool ParseChannels (const char* txt, CS::Animation::BoneID bone, csVector3 offset);
  bool ParseAnimationFrame (iFile* file);
  bool Report (int severity, const char* msg, ...) const;

  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csString filename;
  csString filenameVFS;
  csString mountname;
  csRef<CS::Animation::iSkeletonAnimation> animation;

  CS::Animation::MocapParserResult result;

  int frameCount;
  float frameDuration;
  int currentFrame;
  size_t startFrame, endFrame;
  float globalScale;
  bool endSitesAdded;
  int endSitesCount;
  CS::Animation::BoneID sampleBone;

  enum DOFChannel
  {
    XROT = 0,
    YROT,
    ZROT,
    XPOS,
    YPOS,
    ZPOS,
    XSCA,
    YSCA,
    ZSCA
  };

  struct ChannelData
  {
    CS::Animation::BoneID boneID;
    csVector3 offset;
    csArray<DOFChannel> dofs;
  };

  csArray<ChannelData> channels;
  size_t totalChannelCount;
};

} // namespace Animation
} // namespace CS


#endif // __CS_CSTOOL_MOCAPREADER_H__
