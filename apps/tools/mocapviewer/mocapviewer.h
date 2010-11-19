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

#ifndef __MOCAPVIEWER_H__
#define __MOCAPVIEWER_H__

#include "cstool/csdemoapplication.h"
#include "cstool/mocapparser.h"
#include "imesh/animnode/skeleton2anim.h"

struct iMovieRecorder;

namespace CS {
namespace Animation {

struct iBodyManager;
struct iSkeletonDebugNodeManager;
struct iSkeletonDebugNode;

} // namespace Animation
} // namespace CS

class MocapViewer : public CS::Demo::DemoApplication,
  public scfImplementation1<MocapViewer, CS::Animation::iSkeletonAnimCallback>
{
 private:
  bool CreateAvatar ();

  csRef<iMovieRecorder> movieRecorder;
  csRef<CS::Animation::iBodyManager> bodyManager;
  csRef<CS::Animation::iSkeletonDebugNodeManager> debugNodeManager;
  csRef<CS::Animation::iSkeletonDebugNode> debugNode;
  csRef<CS::Animation::iSkeletonAnimNode> animNode;
  csRef<iMeshWrapper> meshWrapper;
  CS::Animation::MocapParserResult parsingResult;

  csPixmap* debugImage;
  bool printInfo;

 public:
  MocapViewer ();
  ~MocapViewer ();

  //-- CS::Demo::DemoApplication
  void Frame ();

  //-- csApplicationFramework
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();

  //-- CS::Animation::iSkeletonAnimCallback
  void AnimationFinished (CS::Animation::iSkeletonAnimNode* node);
  void AnimationCycled (CS::Animation::iSkeletonAnimNode* node) {}
  void PlayStateChanged (CS::Animation::iSkeletonAnimNode* node, bool isPlaying) {}
  void DurationChanged (CS::Animation::iSkeletonAnimNode* node) {}
};

#endif // __MOCAPVIEWER_H__
