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
#include "mocapviewer.h"
#include "cstool/cspixmap.h"
#include "imesh/animesh.h"
#include "imesh/bodymesh.h"
#include "imesh/animnode/debug.h"
#include "iutil/cfgmgr.h"

#include "ivaria/movierecorder.h"

//#include "mocapparser.h"

MocapViewer::MocapViewer ()
  : DemoApplication ("CrystalSpace.MocapViewer", "csmocapviewer", "csmocapviewer [OPTIONS] [filename]"
		     "\n\nUsage examples:\n"
		     "\tcsmocapviewer idle01.bvh\n"
		     "\tcsmocapviewer /lib/krystal/mocap/idle01.bvh\n"
		     "\tcsmocapviewer C:\\CS\\data\\krystal\\mocap\\idle01.bvh\n"
		     "\tcsmocapviewer -start=20 -end=60 -scale=0.1 idle01.bvh\n"
		     "\tcsmocapviewer -rootmask=Hips idle01.bvh\n"
		     "\tcsmocapviewer -rootmask=Hips -childmask=Head -childmask=LeftHand idle01.bvh\n"
		     "\tcsmocapviewer -rootmask=Hips -childall idle01.bvh\n"
		     "\tcsmocapviewer -pld -record -recordfile=mocap.nuv idle01.bvh\n"
		     "\tcsmocapviewer -rotcamera=-90 idle01.bvh\n",
		     "Crystal Space's viewer for motion captured data. This viewer supports currently"
		     " only the Biovision Hierarchical data file format (BVH).\n\n"
		     "A bone chain can be set as a mask, then only the bones from this chain will be displayed."
		     " A bone chain is defined by a root bone, then the user can add either all children and"
		     " sub-children of the root bone, either all bones on the way to a given child bone.\n\n"
		     "This viewer can also be used to display Point Light Displays and record automatically"
		     " videos with these data (e.g. for a psychology study on motion perception).\n\n"
		     "Finally, this application uses a configuration file. See \"/config/csmocapviewer.cfg\""
		     " for more information"),
    scfImplementationType (this), debugImage (nullptr)
{
  // Configure the options for DemoApplication
  // Set the camera mode
  cameraHelper.SetCameraMode (CS::Demo::CSDEMO_CAMERA_MOVE_FREE);

  // Command line options
  commandLineHelper.AddCommandLineOption
    ("start=<int>", "Set the index of the start frame");
  commandLineHelper.AddCommandLineOption
    ("end=<int>", "Set the index of the end frame");
  commandLineHelper.AddCommandLineOption
    ("scale=<float>", "Set the global scale to apply to the distances (default is 0.01)");
  commandLineHelper.AddCommandLineOption
    ("pld", "Set the display mode as 'Point Light Display'");
  commandLineHelper.AddCommandLineOption
    ("rootmask=<string>", "Set the bone name of the root of the bone chain that will be used as a mask");
  commandLineHelper.AddCommandLineOption
    ("childmask=<string>", "Add a child to the bone chain that will be used as a mask");
  commandLineHelper.AddCommandLineOption
    ("childall", "Add all sub-children of the root bone to the bone chain that will be used as a mask");
  commandLineHelper.AddCommandLineOption
    ("info", "Parse the file, print out the mocap data information, then exit");
  commandLineHelper.AddCommandLineOption
    ("record", "Record the session in a video file, then exit");
  commandLineHelper.AddCommandLineOption
    ("recordfile=<string>", "Force the name of the video file to be created");
  commandLineHelper.AddCommandLineOption
    ("rotcamera=<float>", "Rotate the camera of a given angle around the Y axis, in degree");
  // TODO: animesh target & animation save
}

MocapViewer::~MocapViewer ()
{
  delete debugImage;
}

void MocapViewer::Frame ()
{
  // Update manually the animation of the animesh since it is not in a sector
  csVector3 position (0.0f);
  meshWrapper->GetMeshObject ()->NextFrame (vc->GetCurrentTicks (), position, 0);

  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  // Ask the debug node to display the data
  debugNode->Draw (view->GetCamera (), csColor (0.0f, 8.0f, 0.0f));

  // Update the HUD
  hudHelper.stateDescriptions.DeleteIndex (0);
  csString txt;
  txt.Format ("Frame: %i on %u",
	      (int) (animNode->GetPlaybackPosition () / parsingResult.frameDuration),
	      parsingResult.frameCount);
  hudHelper.stateDescriptions.Insert (0, txt);
}

bool MocapViewer::OnInitialize (int argc, char* argv[])
{
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.utilities.movierecorder", iMovieRecorder),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  // Default behavior from DemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  // Load the needed plugins
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.body",
		       CS::Animation::iBodyManager),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.debug",
		       CS::Animation::iSkeletonDebugNodeManager),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  return true;
}

bool MocapViewer::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  // Find a reference to the video recorder plugin
  movieRecorder = csQueryRegistry<iMovieRecorder> (GetObjectRegistry ());
  if (!movieRecorder) 
    return ReportError("Failed to locate the movie recorder plugin!");

  // Find a reference to the bodymesh plugin
  bodyManager = csQueryRegistry<CS::Animation::iBodyManager> (GetObjectRegistry ());
  if (!bodyManager) 
    return ReportError("Failed to locate CS::Animation::iBodyManager plugin!");

  // Find references to the plugins of the animation nodes
  debugNodeManager =
    csQueryRegistry<CS::Animation::iSkeletonDebugNodeManager> (GetObjectRegistry ());
  if (!debugNodeManager)
    return ReportError("Failed to locate CS::Animation::iSkeletonDebugNodeManager plugin!");

  // Default behavior from DemoApplication for the creation of the scene
  if (!CreateRoom ())
    return false;

  // Create the avatar
  if (!CreateAvatar ())
    return false;

  // Run the application
  if (!printInfo)
    Run();

  return true;
}

bool MocapViewer::CreateAvatar ()
{
  // TODO: catch errors

  // Load the configuration file
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  cfg->AddDomain ("/config/csmocapviewer.cfg", vfs, iConfigManager::ConfigPriorityPlugin);
  csString ressourcePath = cfg->GetStr ("MocapViewer.Settings.RessourcePath", "");
  csString videoFormat = cfg->GetStr ("MocapViewer.Settings.VideoFormat", "");
  csString pldImage = cfg->GetStr ("MocapViewer.Settings.PLDImage", "");
  csString pldMode = cfg->GetStr ("MocapViewer.Settings.Display", "");

  // Read the command line options
  // Read the file name
  csRef<iCommandLineParser> clp =
    csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());
  csString mocapFilename = clp->GetName (0);
  if (mocapFilename == "")
  {
    ReportError ("No BVH file provided");
    commandLineHelper.WriteHelp (GetObjectRegistry ());
    return false;
  }

  // Check if we need to add the default mocap data path to the filename
  if (!vfs->Exists (mocapFilename.GetData ()) 
      && ressourcePath != "")
  {
    // Check if there is a slash in the mocapFilename
    size_t index = mocapFilename.FindLast ('\\');
    if (index == (size_t) -1)
      index = mocapFilename.FindLast ('/');

    if (index == (size_t) -1)
      mocapFilename = ressourcePath + mocapFilename;
  }

  // Read the start and end frames
  csString txt = clp->GetOption ("start", 0);
  int startFrame = 0;
  int frame;
  if (txt && sscanf (txt.GetData (), "%i", &frame) == 1)
    startFrame = frame;

  txt = clp->GetOption ("end", 0);
  int endFrame = 0;
  if (txt && sscanf (txt.GetData (), "%i", &frame) == 1)
    endFrame = frame;

  // Read the global scale
  txt = clp->GetOption ("scale", 0);
  float globalScale = 0.01f;
  float scale;
  if (txt && sscanf (txt.GetData (), "%f", &scale) == 1)
    globalScale = scale;

  // Parse the BVH file
  CS::Animation::BVHMocapParser mocapParser (GetObjectRegistry (), vfs);
  if (!mocapParser.SetRessourceFile (mocapFilename.GetData ()))
    return false;

  if (startFrame > 0)
    mocapParser.SetStartFrame (startFrame);
  if (endFrame > 0)
    mocapParser.SetEndFrame (endFrame);
  mocapParser.SetGlobalScale (globalScale);

  parsingResult = mocapParser.ParseData ();
  if (!parsingResult.result)
    return false;

  // Check if we simply need to print the mocap information then exit
  printInfo = clp->GetBoolOption ("info", false);
  if (printInfo)
  {
    // Create a body chain for easier analyzis of the skeleton structure
    CS::Animation::iBodySkeleton* bodySkeleton =
      bodyManager->CreateBodySkeleton ("mocap_body", parsingResult.skeletonFactory);
    CS::Animation::iBodyChain* bodyChain = bodySkeleton->CreateBodyChain ("body_chain", 0);
    bodyChain->AddAllSubChains ();

    // Print 
    printf ("=================================================\n");
    printf ("=== Mocap file: %s ===\n", mocapFilename.GetData ());
    printf ("=================================================\n");
    printf ("=== Frame count: %u ===\n", parsingResult.frameCount);
    printf ("=== Frames per second: %.4f ===\n", 1.0f / parsingResult.frameDuration);
    printf ("=== Total duration: %.4f seconds ===\n",
	    parsingResult.frameCount * parsingResult.frameDuration);
    printf ("=================================================\n");
    printf ("=== Skeleton structure: ===\n");
    printf ("=================================================\n");
    bodyChain->DebugPrint ();
    printf ("=================================================\n");

    return true;
  }

  // Read if we are in PLD display mode
  bool pld = clp->GetBoolOption ("pld", false)
    || pldMode == "PLD";

  // Read if we need to record a video session
  bool recordVideo = clp->GetBoolOption ("record", false);
  csString recordFile = clp->GetOption ("recordfile", 0);

  // Load the animesh plugin
  csRef<iMeshObjectType> meshType = csLoadPluginCheck<iMeshObjectType>
    (GetObjectRegistry (), "crystalspace.mesh.object.animesh", false);

  if (!meshType)
    return ReportError ("Could not load the animesh object plugin!");

  // Create a new animesh factory
  csRef<iMeshObjectFactory> meshFactory = meshType->NewFactory ();
  csRef<CS::Mesh::iAnimatedMeshFactory> animeshFactory = 
    scfQueryInterfaceSafe<CS::Mesh::iAnimatedMeshFactory> (meshFactory);

  if (!animeshFactory)
    return ReportError ("Could not create an animesh factory!");

  animeshFactory->SetSkeletonFactory (parsingResult.skeletonFactory);

  // Load the iSkeletonManager plugin
  csRef<iPluginManager> plugmgr = 
    csQueryRegistry<iPluginManager> (object_reg);
  csRef<CS::Animation::iSkeletonManager> skeletonManager =
    csLoadPlugin<CS::Animation::iSkeletonManager> (plugmgr,
						   "crystalspace.skeletalanimation");
  if (!skeletonManager)
    return ReportError ("Could not load the skeleton plugin");

  // Create a new animation tree. The structure of the tree is:
  //   + Debug node
  //     + Animation node with the mocap data
  csRef<CS::Animation::iSkeletonAnimPacketFactory> animPacketFactory =
    parsingResult.skeletonFactory->GetAnimationPacket ();

  // Create the 'debug' animation node
  csRef<CS::Animation::iSkeletonDebugNodeFactory> debugNodeFactory =
    debugNodeManager->CreateAnimNodeFactory ("debug");
  debugNodeFactory->SetDebugModes
    (pld ? CS::Animation::DEBUG_SQUARES
     : (CS::Animation::SkeletonDebugMode)
     (CS::Animation::DEBUG_2DLINES | CS::Animation::DEBUG_SQUARES));
  debugNodeFactory->SetLeafBonesDisplayed (false);
  animPacketFactory->SetAnimationRoot (debugNodeFactory);

  // Setup the bone chain mask
  txt = clp->GetOption ("rootmask", 0);
  if (txt)
  {
    CS::Animation::BoneID boneID = parsingResult.skeletonFactory->FindBone (txt);
    if (boneID == CS::Animation::InvalidBoneID)
      ReportError ("Could not find root bone %s!", txt.GetData ());

    else
    {
      // Create the body chain
      CS::Animation::iBodySkeleton* bodySkeleton =
	bodyManager->CreateBodySkeleton ("mocap_body", parsingResult.skeletonFactory);
      CS::Animation::iBodyChain* bodyChain = bodySkeleton->CreateBodyChain ("body_chain", boneID);

      // Check if we need to add all children
      if (clp->GetBoolOption ("childall", false))
	printf ("add all children\n");
      if (clp->GetBoolOption ("childall", false))
	bodyChain->AddAllSubChains ();

      // Add all user defined sub-chains
      size_t index = 0;
      txt = clp->GetOption ("childmask", index);
      while (txt)
      {
	boneID = parsingResult.skeletonFactory->FindBone (txt);
	if (boneID == CS::Animation::InvalidBoneID)
	  ReportError ("Could not find child bone %s!", txt.GetData ());

	else
	  bodyChain->AddSubChain (boneID);

	index++;
	txt = clp->GetOption ("childmask", index);
      }

      debugNodeFactory->AddChainMask (bodyChain);
    }
  }

  // Load the debug image
  if (pld && pldImage != "")
  {
    csRef<iTextureWrapper> texture = loader->LoadTexture
      ("pld_image", pldImage.GetData (), CS_TEXTURE_2D, 0, true, true, true);
    if (!texture.IsValid ())
      ReportWarning ("Failed to load PLD image %s!\n", pldImage.GetData ());

    else
    {
      // Create a 2D sprite for the logo
      iTextureHandle* textureHandle = texture->GetTextureHandle ();
      if (textureHandle)
      {
	debugImage = new csSimplePixmap (textureHandle);
	debugNodeFactory->SetDebugImage (debugImage);
	debugNodeFactory->SetDebugModes (CS::Animation::DEBUG_IMAGES);
      }
    }
  }

  // Create the 'mocap' animation node
  csRef<CS::Animation::iSkeletonAnimationNodeFactory> mocapNodeFactory =
    animPacketFactory->CreateAnimationNode ("mocap");
  mocapNodeFactory->SetAnimation (parsingResult.animPacketFactory->GetAnimation (0));
  mocapNodeFactory->SetCyclic (!recordVideo);
  debugNodeFactory->SetChildNode (mocapNodeFactory);

  // Create the animated mesh
  csRef<iMeshFactoryWrapper> meshFactoryWrapper =
    engine->CreateMeshFactory (meshFactory, "mocap_meshfact");
  meshWrapper = engine->CreateMeshWrapper (meshFactoryWrapper, "mocap_mesh");
  csRef<CS::Mesh::iAnimatedMesh> animesh =
    scfQueryInterface<CS::Mesh::iAnimatedMesh> (meshWrapper->GetMeshObject ());

  // When the animated mesh is created, the animation nodes are created too.
  // We can therefore set them up now.
  CS::Animation::iSkeletonAnimNode* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();

  // Find a reference to the animation nodes
  debugNode = scfQueryInterface<CS::Animation::iSkeletonDebugNode> (rootNode->FindNode ("debug"));
  animNode = rootNode->FindNode ("mocap");

  // Register a callback for the changes of the state of the playback of the animation
  if (recordVideo)
    animNode->AddAnimationCallback (this);

  // Initialize the HUD
  hudHelper.stateDescriptions.Push ("Frame:");
  csString hudTxt;
  hudTxt.Format ("Mocap FPS: %.2f", 1.0f / parsingResult.frameDuration);
  hudHelper.stateDescriptions.Push (hudTxt);
  hudTxt.Format ("Total length: %.2f seconds", animNode->GetDuration ());
  hudHelper.stateDescriptions.Push (hudTxt);

  // Initialize the position of the camera
  const csArray<CS::Animation::BoneID>& boneList = parsingResult.skeletonFactory->GetBoneOrderList ();
  if (boneList.GetSize ())
  {
    // Compute the bounding box of the skeleton
    csBox3 bbox;
    csQuaternion rotation;
    csVector3 offset;
    float time;
     CS::Animation::iSkeletonAnimation* animation = parsingResult.animPacketFactory->GetAnimation (0);

    for (size_t i = 0; i < parsingResult.skeletonFactory->GetTopBoneID (); i++)
      if (parsingResult.skeletonFactory->HasBone (i))
      {
	parsingResult.skeletonFactory->GetTransformAbsSpace (i, rotation, offset);
	bbox.AddBoundingVertex (offset);
      }

    // Find the initial position of the root of the skeleton
    CS::Animation::BoneID rootBone = boneList[0];
    animation->GetKeyFrame (animation->FindChannel (rootBone), 0, rootBone, time, rotation, offset);
    csVector3 cameraTarget = offset + bbox.GetCenter ();
    csVector3 cameraOffset = csVector3 (0.0f, 0.0f, -300.0f) * globalScale;

    // Check if the user has provided an angle for the camera
    csString txt = clp->GetOption ("rotcamera", 0);
    float angle;
    if (txt && sscanf (txt.GetData (), "%f", &angle) == 1)
      // TODO: the csYRotMatrix3 is defined in right-handed coordinate system!
      cameraOffset = csYRotMatrix3 (-angle * 3.1415927 / 180.0f) * cameraOffset;

    // Update the position of the camera
    view->GetCamera ()->GetTransform ().SetOrigin (cameraTarget + cameraOffset);
    view->GetCamera ()->GetTransform ().LookAt (-cameraOffset, csVector3 (0.0f, 1.0f, 0.0f));
  }

  // Display the origin
  if (!pld)
  {
    csTransform transform;
    CS::Debug::VisualDebuggerHelper::DebugTransform (GetObjectRegistry (), transform, true);
  }

  // Start the video recording
  if (recordVideo)
  {
    // Check if a video file has been provided
    if (recordFile != "")
    {
      // Check if there is a slash in the filename
      size_t index = recordFile.FindLast ('\\');
      if (index == (size_t) -1)
	index = recordFile.FindLast ('/');

      if (index == (size_t) -1)
	recordFile = "/this/" + recordFile;

      movieRecorder->SetRecordingFile (recordFile.GetData ());
    }

    // Else check if a default video file format has been provided
    else if (videoFormat != "")
      movieRecorder->SetFilenameFormat (videoFormat.GetData ());

    // Disable the display of the HUD
    SetHUDDisplayed (false);

    // Disable the movements of the camera
    cameraHelper.SetCameraMode (CS::Demo::CSDEMO_CAMERA_NONE);

    // Start the movie recording
    movieRecorder->Start ();
  }

  return true;
}

void MocapViewer::AnimationFinished (CS::Animation::iSkeletonAnimNode* node)
{
  // This is the end of the animation, exit the application in order to end the recording
  // of the video
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (GetObjectRegistry ()));
  if (q) q->GetEventOutlet()->Broadcast (csevQuit (GetObjectRegistry ()));
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return MocapViewer ().Main (argc, argv);
}
