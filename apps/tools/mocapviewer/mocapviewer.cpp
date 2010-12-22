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
#include "csutil/floatrand.h"
#include "csutil/randomgen.h"
#include "cstool/cspixmap.h"
#include "imesh/animesh.h"
#include "imesh/bodymesh.h"
#include "imesh/animnode/debug.h"
#include "iutil/cfgmgr.h"
#include "ivaria/movierecorder.h"

MocapViewer::MocapViewer ()
  : DemoApplication ("CrystalSpace.MocapViewer", "csmocapviewer",
		     "csmocapviewer [OPTIONS] [filename]"
		     "\n\nUsage examples:\n"
		     "\tcsmocapviewer idle01.bvh\n"
		     "\tcsmocapviewer /lib/krystal/mocap/idle01.bvh\n"
		     "\tcsmocapviewer C:\\CS\\data\\krystal\\mocap\\idle01.bvh\n"
		     "\tcsmocapviewer -start=20 -end=60 -scale=0.1 idle01.bvh\n"
		     "\tcsmocapviewer -rootmask=Hips idle01.bvh\n"
		     "\tcsmocapviewer -rootmask=Hips -childmask=Head -childmask=LeftHand idle01.bvh\n"
		     "\tcsmocapviewer -rootmask=Hips -childall idle01.bvh\n"
		     "\tcsmocapviewer -pld -record -recordfile=mocap.nuv idle01.bvh\n"
		     "\tcsmocapviewer -rotcamera=-90 idle01.bvh\n"
		     "\tcsmocapviewer idle01.bvh -pld -ncount=200 -nfrequency=0.4 -poscamera=0.7\n"
		     "\tcsmocapviewer -targetfile=data/krystal/krystal.xml -targetname=krystal -nobone idle01.bvh\n",
		     csString().Format (
		     "Crystal Space's viewer for motion captured data. This viewer supports currently"
		     " only the Biovision Hierarchical data file format (BVH).\n\n"
		     "The animation can be retargeted automatically to an animesh. Use either the -targetfile and"
		     " -targetname options for that. The results will depend on the actual similitudes between the"
		     " two skeletons.\n\n"
		     "A mask can be defined to select the bones that are displayed. A simple way to populate the "
		     "bone mask is by defining a bone chain. A bone chain is defined by a root bone (option "
		     "-rootmask), then the user can add either all children and sub-children of the root bone (option"
		     " -childall), either all bones on the way to a given child bone (option -childmask). Only one bone"
		     " chain can be defined, but the -childmask option can be used additively. Some"
		     " specific bones can be added and removed by using the -bone and -nobone options. If the only bone"
		     " option provided is the empty -nobone option, then no bones at all will be displayed.\n\n"
		     "This viewer can also be used as a Point Light Display system, and is able to record automatically"
		     " videos with these data. This system has been designed as a base tool for a psychology study on "
		     "the human perception of the motion.\n\n"
		     "The Point Light Display can be perturbated by adding noise points animated by a Perlin"
		     " noise. The behavior of the motion of these noise points can be tweaked by several"
		     " parameters. See http://libnoise.sourceforge.net/docs/classnoise_1_1module_1_1Perlin.html"
		     " for more information on these parameters.\n\n"
		     "Finally, this application uses a configuration file. See %s"
		     " for more information", CS::Quote::Double ("/config/csmocapviewer.cfg"))),
    scfImplementationType (this), debugImage (nullptr), noiseScale (0.5f)
{
  // Configure the options for DemoApplication
  // Set the camera mode
  cameraHelper.SetCameraMode (CS::Demo::CSDEMO_CAMERA_MOVE_FREE);

  // Command line options
  commandLineHelper.AddCommandLineOption
    ("info", "Parse the file, print out the mocap data information, then exit");
  commandLineHelper.AddCommandLineOption
    ("noanim", "Don't play the animation, only display the skeleton in rest pose");
  commandLineHelper.AddCommandLineOption
    ("start=<int>", "Set the index of the start frame");
  commandLineHelper.AddCommandLineOption
    ("end=<int>", "Set the index of the end frame");
  commandLineHelper.AddCommandLineOption
    ("scale=<float>", "Set the global scale to apply to the distances (default is 0.01)");
  commandLineHelper.AddCommandLineOption
    ("speed=<float>", "Set the speed to play the animation (default is 1.0)");
  commandLineHelper.AddCommandLineOption
    ("targetfile=<string>", "Set the file of the animesh to retarget the animation to");
  commandLineHelper.AddCommandLineOption
    ("targetname=<string>", "Set the name of the animesh factory to retarget the animation to");
  commandLineHelper.AddCommandLineOption
    ("pld", csString().Format ("Set the display mode as %s",
			       CS::Quote::Single ("Point Light Display")));
  commandLineHelper.AddCommandLineOption
    ("rotcamera=<float>", "Rotate the camera of a given angle around the Y axis, in degree");
  commandLineHelper.AddCommandLineOption
    ("poscamera=<float>", "Scale the distance between the camera and the target. Default value is 1.0");
  commandLineHelper.AddCommandLineOption
    ("rootmask=<string>", "Set the bone name of the root of the bone chain that will be used as a mask");
  commandLineHelper.AddCommandLineOption
    ("childmask=<string>", "Add a child to the bone chain that will be used as a mask");
  commandLineHelper.AddCommandLineOption
    ("childall", "Add all sub-children of the root bone to the bone chain that will be used as a mask");
  commandLineHelper.AddCommandLineOption
    ("bone=<string>", "Add a bone to be displayed by its name");
  commandLineHelper.AddCommandLineOption
    ("nobone", "Don't display any bone at all");
  commandLineHelper.AddCommandLineOption
    ("nobone=<string>", "Remove a bone to be displayed by its name");
  commandLineHelper.AddCommandLineOption
    ("record", "Record the session in a video file, then exit");
  commandLineHelper.AddCommandLineOption
    ("recordfile=<string>", "Force the name of the video file to be created");
  commandLineHelper.AddCommandLineOption
    ("ncount=<int>", "Set the number of noise points added. Default value is 0");
  commandLineHelper.AddCommandLineOption
    ("nscale=<float>", "Scale to apply on the position of the noise points. Default value is 0.5");
  commandLineHelper.AddCommandLineOption
    ("noctaves=<int>", "Set the number of octaves of the noise. Value must be between 1 and 30. Default value is 6");
  commandLineHelper.AddCommandLineOption
    ("nfrequency=<float>", "Set the frequency of the noise. Value must be positive. Default value is 1.0");
  commandLineHelper.AddCommandLineOption
    ("nlacunarity=<float>", "Set the lacunarity of the noise. Value is suggested to be between 1.5 and 3.5. Default value is 2.0");
  commandLineHelper.AddCommandLineOption
    ("npersistence=<float>", "Set the persistence of the noise. Value is suggested to be between 0.0 and 1.0. Default value is 0.5");
}

MocapViewer::~MocapViewer ()
{
  delete debugImage;
}

void MocapViewer::Frame ()
{
  // Update manually the animation of the animesh since it is not in a sector
  // TODO: use engine flag ALWAYS_ANIMATE
  csVector3 position (0.0f);
  meshWrapper->GetMeshObject ()->NextFrame (vc->GetCurrentTicks (), position, 0);

  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  // Ask the debug node to display the data
  csColor color (0.0f, 8.0f, 0.0f);
  debugNode->Draw (view->GetCamera (), color);

  // Display the noise points
  int colorI = g2d->FindRGB (255.0f * color[0],
			     255.0f * color[1],
			     255.0f * color[2]);
  float seed0 = ((float) vc->GetCurrentTicks ()) / 10000.0f;
  for (csArray<csVector3>::Iterator it = noisePoints.GetIterator (); it.HasNext (); )
  {
    csVector3& point = it.Next ();

    float px = noiseX.GetValue (seed0 + point[0], point[1], point[2])
      * noiseScale * ((float) g2d->GetWidth ()) + ((float) g2d->GetWidth ()) * 0.5f;
    float py = noiseY.GetValue (seed0 + point[0], point[1], point[2])
      * noiseScale * ((float) g2d->GetHeight ()) + ((float) g2d->GetHeight ()) * 0.5f;

    // Display the debug image if available
    if (debugImage)
      debugImage->Draw (g3d, px - debugImage->Width () / 2,
			py - debugImage->Height () / 2);

    // Else display a square
    else
    {
      size_t size = 5;
      for (size_t i = 0; i < size; i++)
	for (size_t j = 0; j < size; j++)
	  g2d->DrawPixel (((int) px) - size / 2 + i,
			  ((int) py) - size / 2 + j,
			  colorI);
    }
  }

  // Update the HUD
  hudHelper.stateDescriptions.DeleteIndex (0);
  csString txt;
  txt.Format ("Frame: %i on %u",
	      (int) (animNode->GetPlaybackPosition () / parsingResult.frameDuration),
	      (unsigned int) parsingResult.frameCount);
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
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.retarget",
		       CS::Animation::iSkeletonRetargetNodeManager),
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

  retargetNodeManager =
    csQueryRegistry<CS::Animation::iSkeletonRetargetNodeManager> (GetObjectRegistry ());
  if (!retargetNodeManager)
    return ReportError("Failed to locate CS::Animation::iSkeletonRetargetNodeManager plugin!");

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
  // Load the configuration file
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  cfg->AddDomain ("/config/csmocapviewer.cfg", vfs, iConfigManager::ConfigPriorityPlugin);
  csString ressourcePath = cfg->GetStr ("MocapViewer.Settings.RessourcePath", "");
  csString videoFormat = cfg->GetStr ("MocapViewer.Settings.VideoFormat", "");
  csString pldImage = cfg->GetStr ("MocapViewer.Settings.PLDImage", "");
  csString pldMode = cfg->GetStr ("MocapViewer.Settings.Display", "");
  csString targetFile = cfg->GetStr ("MocapViewer.Settings.TargetFile", "");
  csString targetName = cfg->GetStr ("MocapViewer.Settings.TargetName", "");

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
  float fvalue;
  if (txt && sscanf (txt.GetData (), "%f", &fvalue) == 1)
    globalScale = fvalue;

  // Parse the BVH file
  CS::Animation::BVHMocapParser mocapParser (GetObjectRegistry ());
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
    printf ("=================================================\n");
    printf ("=== Mocap file: %s ===\n", mocapFilename.GetData ());
    printf ("=================================================\n");
    printf ("=== Frame count: %u ===\n", (unsigned int) parsingResult.frameCount);
    printf ("=== Frames per second: %.4f ===\n", 1.0f / parsingResult.frameDuration);
    printf ("=== Total duration: %.4f seconds ===\n",
	    parsingResult.frameCount * parsingResult.frameDuration);
    printf ("=================================================\n");
    printf ("=== Skeleton structure: ===\n");
    printf ("=================================================\n");
    printf ("%s", parsingResult.skeletonFactory->Description ().GetData ());
    printf ("=================================================\n");

    return true;
  }

  // Read if we are in PLD display mode
  bool pld = clp->GetBoolOption ("pld", false)
    || pldMode == "PLD";

  // Read the remaining configuration data
  bool recordVideo = clp->GetBoolOption ("record", false);
  csString recordFile = clp->GetOption ("recordfile", 0);
  bool noAnimation = clp->GetBoolOption ("noanim", false);
  float playbackSpeed = 1.0f;
  txt = clp->GetOption ("speed", 0);
  if (txt && sscanf (txt.GetData (), "%f", &fvalue) == 1)
    playbackSpeed = fvalue;

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

    // Check if the automatic animation has to be disabled
  if (noAnimation)
    parsingResult.skeletonFactory->SetAutoStart (false);

  // Load the iSkeletonManager plugin
  csRef<iPluginManager> plugmgr = csQueryRegistry<iPluginManager> (GetObjectRegistry ());
  csRef<CS::Animation::iSkeletonManager> skeletonManager =
    csLoadPlugin<CS::Animation::iSkeletonManager> (plugmgr, "crystalspace.skeletalanimation");
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

  // Setup the bone chain mask to select which bones are displayed
  bool hasMask = clp->GetOption ("rootmask", 0) || clp->GetOption ("bone", 0) || clp->GetOption ("nobone", 0);
  csBitArray boneMask;
  boneMask.SetSize (parsingResult.skeletonFactory->GetTopBoneID () + 1);

  // Check if there is only one empty "-nobone" option provided
  txt = clp->GetOption ("nobone", 0);
  if (txt && txt == ""
      && !clp->GetOption ("rootmask", 0) && !clp->GetOption ("bone", 0))
    boneMask.Clear ();

  else
    boneMask.SetAll ();

  // Check for a definition of a bone chain
  txt = clp->GetOption ("rootmask", 0);
  if (txt)
  {
    boneMask.Clear ();

    CS::Animation::BoneID boneID = parsingResult.skeletonFactory->FindBone (txt);
    if (boneID == CS::Animation::InvalidBoneID)
      ReportWarning ("Could not find root bone %s!", txt.GetData ());

    else
    {
      // Create the body chain
      CS::Animation::iBodySkeleton* bodySkeleton =
	bodyManager->CreateBodySkeleton ("mocap_body", parsingResult.skeletonFactory);
      CS::Animation::iBodyChain* bodyChain = bodySkeleton->CreateBodyChain ("body_chain", boneID);

      // Check if we need to add all children
      if (clp->GetBoolOption ("childall", false))
	bodyChain->AddAllSubChains ();

      // Add all user defined sub-chains
      size_t index = 0;
      txt = clp->GetOption ("childmask", index);
      while (txt)
      {
	boneID = parsingResult.skeletonFactory->FindBone (txt);
	if (boneID == CS::Animation::InvalidBoneID)
	  ReportWarning ("Could not find child bone %s!", txt.GetData ());

	else
	  bodyChain->AddSubChain (boneID);

	index++;
	txt = clp->GetOption ("childmask", index);
      }

      bodyChain->PopulateBoneMask (boneMask);
    }
  }

  // Setup the mask for the bones that are explicitely given on command line
  size_t index = 0;
  txt = clp->GetOption ("bone", index);
  while (txt)
  {
    CS::Animation::BoneID boneID = parsingResult.skeletonFactory->FindBone (txt);
    if (boneID == CS::Animation::InvalidBoneID)
      ReportWarning ("Could not find user specified bone %s!", txt.GetData ());

    else boneMask.SetBit (boneID);

    txt = clp->GetOption ("bone", ++index);
  }

  index = 0;
  txt = clp->GetOption ("nobone", index);
  while (txt)
  {
    if (txt != "")
    {
      CS::Animation::BoneID boneID = parsingResult.skeletonFactory->FindBone (txt);
      if (boneID == CS::Animation::InvalidBoneID)
	ReportWarning ("Could not find user specified bone %s!", txt.GetData ());

      else boneMask.ClearBit (boneID);
    }

    txt = clp->GetOption ("nobone", ++index);
  }

  // Setup the bone mask
  if (hasMask)
    debugNodeFactory->SetBoneMask (boneMask);

  // Load the debug image
  if (pld && pldImage != "")
  {
    csRef<iTextureWrapper> texture = loader->LoadTexture
      ("pld_image", pldImage.GetData (), CS_TEXTURE_2D, 0, true, true, true);
    if (!texture.IsValid ())
      ReportWarning ("Failed to load PLD image %s!\n", pldImage.GetData ());

    else
    {
      // Create the 2D sprite
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
  mocapNodeFactory->SetPlaybackSpeed (playbackSpeed);
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

  // Setup the noise points
  txt = clp->GetOption ("ncount", 0);
  int noiseCount;
  if (txt && sscanf (txt.GetData (), "%i", &noiseCount) == 1)
  {
    // Initialize the perlin noise modules
    csRandomGen irandomGenerator (406321958);
    csRandomGen irandomGenerator2 (18974329);
    csRandomFloatGen frandomGenerator (50963095);
    noiseX.SetSeed (irandomGenerator.Get (~0));
    noiseY.SetSeed (irandomGenerator2.Get (~0));

    // Read the command line parameters of the noise modules
    txt = clp->GetOption ("noctaves", 0);
    int ivalue;
    if (txt && sscanf (txt.GetData (), "%i", &ivalue) == 1)
    {
      if (ivalue < 1)
	ivalue = 1;
      if (ivalue > 30)
	ivalue = 30;
      noiseX.SetOctaveCount (ivalue);
      noiseY.SetOctaveCount (ivalue);
    }

    txt = clp->GetOption ("nscale", 0);
    float fvalue;
    if (txt && sscanf (txt.GetData (), "%f", &fvalue) == 1)
      noiseScale = fvalue;

    txt = clp->GetOption ("nfrequency", 0);
    if (txt && sscanf (txt.GetData (), "%f", &fvalue) == 1)
    {
      noiseX.SetFrequency (fvalue);
      noiseY.SetFrequency (fvalue);
    }

    txt = clp->GetOption ("nlacunarity", 0);
    if (txt && sscanf (txt.GetData (), "%f", &fvalue) == 1)
    {
      noiseX.SetLacunarity (fvalue);
      noiseY.SetLacunarity (fvalue);
    }

    txt = clp->GetOption ("npersistence", 0);
    if (txt && sscanf (txt.GetData (), "%f", &fvalue) == 1)
    {
      noiseX.SetPersistence (fvalue);
      noiseY.SetPersistence (fvalue);
    }

    // Create the noise points
    for (int i = 0; i < noiseCount; i++)
    {
      csVector3 point;
      point[0] = frandomGenerator.Get (10.0f);
      point[1] = frandomGenerator.Get (10.0f);
      point[2] = frandomGenerator.Get (10.0f);
      noisePoints.Push (point);
    }
  }

  // Check if we have to retarget the animation to an animesh
  txt = clp->GetOption ("targetfile", 0);
  if (txt) targetFile = txt;

  txt = clp->GetOption ("targetname", 0);
  if (txt) targetName = txt;

  CS::Animation::iSkeletonFactory* retargetSkeletonFactory = nullptr;
  if (targetFile != "" && targetName != "")
  {
    // Load the animesh factory
    csLoadResult rc = loader->Load (targetFile.GetData ());
    if (!rc.success)
      return ReportError ("Can't load target library file %s!", targetFile.GetData ());

    csRef<iMeshFactoryWrapper> meshfact = engine->FindMeshFactory (targetName.GetData ());
    if (!meshfact)
      return ReportError ("Can't find target mesh factory %s!", targetName.GetData ());

    animeshFactory = scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
      (meshfact->GetMeshObjectFactory ());
    if (!animeshFactory)
      return ReportError ("Can't find the animesh interface for the animesh target!");
    retargetSkeletonFactory = animeshFactory->GetSkeletonFactory ();

    // Check if the automatic animation has to be disabled
    if (noAnimation)
      animeshFactory->GetSkeletonFactory ()->SetAutoStart (false);

    // Create a new animation tree. The structure of the tree is:
    //   + Retarget node
    //     + Animation node with the mocap data
    csRef<CS::Animation::iSkeletonAnimPacketFactory> animPacketFactory =
      animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ();

    // Create the 'retarget' animation node
    csRef<CS::Animation::iSkeletonRetargetNodeFactory> retargetNodeFactory =
      retargetNodeManager->CreateAnimNodeFactory ("mocap_retarget");
    animPacketFactory->SetAnimationRoot (retargetNodeFactory);
    retargetNodeFactory->SetSourceSkeleton (parsingResult.skeletonFactory);

    // This mapping is for the motion capture data with the same name of the bones
    CS::Animation::BoneMapping skeletonMapping;
    CS::Animation::NameBoneMappingHelper::GenerateMapping
      (skeletonMapping, parsingResult.skeletonFactory, animeshFactory->GetSkeletonFactory ());
    retargetNodeFactory->SetBoneMapping (skeletonMapping);

    // If the animesh target is Krystal, then we load some hardcoded information to setup the retarget mode
    if (targetName == "krystal")
    {
      // Create the bone mapping between the source and the target skeletons
      /*
      // This mapping is for the motion capture data of the Carnegie Mellon University
      CS::Animation::BoneMapping skeletonMapping;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("hip"), animeshFactory->GetSkeletonFactory ()->FindBone ("Hips"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("abdomen"), animeshFactory->GetSkeletonFactory ()->FindBone ("ToSpine"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("chest"), animeshFactory->GetSkeletonFactory ()->FindBone ("Spine"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("neck"), animeshFactory->GetSkeletonFactory ()->FindBone ("Neck"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("head"), animeshFactory->GetSkeletonFactory ()->FindBone ("Head"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("rCollar"), animeshFactory->GetSkeletonFactory ()->FindBone ("RightShoulder"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("rShldr"), animeshFactory->GetSkeletonFactory ()->FindBone ("RightArm"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("rForeArm"), animeshFactory->GetSkeletonFactory ()->FindBone ("RightForeArm"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("rHand"), animeshFactory->GetSkeletonFactory ()->FindBone ("RightHand"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("lCollar"), animeshFactory->GetSkeletonFactory ()->FindBone ("LeftShoulder"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("lShldr"), animeshFactory->GetSkeletonFactory ()->FindBone ("LeftArm"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("lForeArm"), animeshFactory->GetSkeletonFactory ()->FindBone ("LeftForeArm"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("lHand"), animeshFactory->GetSkeletonFactory ()->FindBone ("LeftHand"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("rThigh"), animeshFactory->GetSkeletonFactory ()->FindBone ("RightUpLeg"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("rShin"), animeshFactory->GetSkeletonFactory ()->FindBone ("RightLeg"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("rFoot"), animeshFactory->GetSkeletonFactory ()->FindBone ("RightFoot"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("lThigh"), animeshFactory->GetSkeletonFactory ()->FindBone ("LeftUpLeg"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("lShin"), animeshFactory->GetSkeletonFactory ()->FindBone ("LeftLeg"));;
      skeletonMapping.AddMapping (parsingResult.skeletonFactory->FindBone ("lFoot"), animeshFactory->GetSkeletonFactory ()->FindBone ("LeftFoot"));;
      retargetNodeFactory->SetBoneMapping (skeletonMapping);
      */
      // Create the body chains used for retargeting
      CS::Animation::iBodySkeleton* bodySkeleton =
	bodyManager->CreateBodySkeleton ("target_body", animeshFactory->GetSkeletonFactory ());

      CS::Animation::iBodyChain* bodyChain = bodySkeleton->CreateBodyChain
	// (the "Hips" bone is positioned too differently from the mocap data, we therefore skip this bone)
	//("torso", animeshFactory->GetSkeletonFactory ()->FindBone ("Hips"));
	("torso", animeshFactory->GetSkeletonFactory ()->FindBone ("ToSpine"));
      bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("Head"));
      retargetNodeFactory->AddBodyChain (bodyChain);

      bodyChain = bodySkeleton->CreateBodyChain
	("left_arm", animeshFactory->GetSkeletonFactory ()->FindBone ("LeftShoulder"));
      bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("LeftHand"));
      retargetNodeFactory->AddBodyChain (bodyChain);

      bodyChain = bodySkeleton->CreateBodyChain
	("right_arm", animeshFactory->GetSkeletonFactory ()->FindBone ("RightShoulder"));
      bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("RightHand"));
      retargetNodeFactory->AddBodyChain (bodyChain);

      bodyChain = bodySkeleton->CreateBodyChain
	("left_leg", animeshFactory->GetSkeletonFactory ()->FindBone ("LeftUpLeg"));
      bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("LeftToeBase"));
      retargetNodeFactory->AddBodyChain (bodyChain);

      bodyChain = bodySkeleton->CreateBodyChain
	("right_leg", animeshFactory->GetSkeletonFactory ()->FindBone ("RightUpLeg"));
      bodyChain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("RightToeBase"));
      retargetNodeFactory->AddBodyChain (bodyChain);
    }

    // Create the playback animation node of the motion captured data
    csRef<CS::Animation::iSkeletonAnimationNodeFactory> mocapNodeFactory =
      animPacketFactory->CreateAnimationNode ("mocap_retarget");
    mocapNodeFactory->SetAnimation
      (parsingResult.animPacketFactory->GetAnimation (0));
    mocapNodeFactory->SetCyclic (true);
    mocapNodeFactory->SetPlaybackSpeed (playbackSpeed);
    retargetNodeFactory->SetChildNode (mocapNodeFactory);

    // Create the animated mesh
    csRef<iMeshWrapper> retargetMesh =
      engine->CreateMeshWrapper (meshfact, "retarget", room, csVector3 (0.0f));
  }

  // Compute the position of the target of the camera
  csVector3 cameraTarget (0.0f);

  // Compute the bounding box of the bones of the skeleton that are visible
  const csArray<CS::Animation::BoneID>& boneList = parsingResult.skeletonFactory->GetBoneOrderList ();
  if (boneList.GetSize ())
  {
    csBox3 bbox;
    csQuaternion rotation;
    csVector3 offset;
    CS::Animation::ChannelID rootChannel = CS::Animation::InvalidChannelID;
    CS::Animation::iSkeletonAnimation* animation = parsingResult.animPacketFactory->GetAnimation (0);
    for (size_t i = 0; i < boneList.GetSize (); i++)
      if (!hasMask || boneMask.IsBitSet (i))
      {
	if (rootChannel == CS::Animation::InvalidChannelID
	    && animation->FindChannel (i))
	  rootChannel = animation->FindChannel (i);

	parsingResult.skeletonFactory->GetTransformAbsSpace (i, rotation, offset);
	bbox.AddBoundingVertex (offset);
      }
    cameraTarget = bbox.GetCenter ();

    // Find the initial position of the root of the skeleton
    if (!noAnimation
	&& rootChannel != CS::Animation::InvalidChannelID)
    {
      float time;
      CS::Animation::BoneID rootBone;
      animation->GetKeyFrame (rootChannel, 0, rootBone, time, rotation, offset);
      cameraTarget += offset;
    }
  }

  // If there are no bones then use the position of the root of the target mesh
  if (hasMask && boneMask.AllBitsFalse ()
      && retargetSkeletonFactory
      && retargetSkeletonFactory->GetBoneOrderList ().GetSize ())
  {
    csQuaternion rotation;
    csVector3 offset;
    retargetSkeletonFactory->GetTransformAbsSpace
      (retargetSkeletonFactory->GetBoneOrderList ().Get (0), rotation, offset);
    cameraTarget += offset;
  }

  // Compute the position of the camera
  txt = clp->GetOption ("poscamera", 0);
  float scale = 1.0f;
  float value;
  if (txt && sscanf (txt.GetData (), "%f", &value) == 1)
    scale = value;
  csVector3 cameraOffset = csVector3 (0.0f, 0.0f, -300.0f * scale) * globalScale;

  // Check if the user has provided an angle for the camera
  txt = clp->GetOption ("rotcamera", 0);
  float angle;
  if (txt && sscanf (txt.GetData (), "%f", &angle) == 1)
    // TODO: the csYRotMatrix3 is defined in right-handed coordinate system!
    cameraOffset = csYRotMatrix3 (-angle * 3.1415927 / 180.0f) * cameraOffset;

  // Update the position of the camera
  view->GetCamera ()->GetTransform ().SetOrigin (cameraTarget + cameraOffset);
  view->GetCamera ()->GetTransform ().LookAt (-cameraOffset, csVector3 (0.0f, 1.0f, 0.0f));

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
