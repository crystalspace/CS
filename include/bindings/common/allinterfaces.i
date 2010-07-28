/*
    Copyright (C) 2003 Rene Jager <renej_frog@users.sourceforge.net>

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

// The following list holds all the interfaces that are handled correctly.
// If you have problems, first check if the interface in question is in this
// list. Please keep the list sorted alphabetically.

%define IENGINE_APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iCamera)
  INTERFACE_APPLY(iCameraPosition)
  INTERFACE_APPLY(iCollection)
  INTERFACE_APPLY(iCollectionArray)
  INTERFACE_APPLY(iEngine)
  INTERFACE_APPLY(iLight)
  INTERFACE_APPLY(iLightIterator)
  INTERFACE_APPLY(iLightList)
  INTERFACE_APPLY(iMaterialEngine)
  INTERFACE_APPLY(iMaterialWrapper)
  INTERFACE_APPLY(iMeshFactoryWrapper)
  INTERFACE_APPLY(iMeshWrapper)
  INTERFACE_APPLY(iMeshWrapperIterator)
  INTERFACE_APPLY(iMovable)
  INTERFACE_APPLY(iMovableListener)
  INTERFACE_APPLY(iPortal)
  INTERFACE_APPLY(iPortalContainer)
  INTERFACE_APPLY(iProcTexture)
  INTERFACE_APPLY(iRenderLoop)
  INTERFACE_APPLY(iRenderLoopManager)
  INTERFACE_APPLY(iRenderManager)
  INTERFACE_APPLY(iRenderManagerPostEffects)
  INTERFACE_APPLY(iRenderManagerTargets)
  INTERFACE_APPLY(iSceneNode)
  INTERFACE_APPLY(iSceneNodeArray)
  INTERFACE_APPLY(iSector)
  INTERFACE_APPLY(iSectorIterator)
  INTERFACE_APPLY(iSectorList)
  INTERFACE_APPLY(iTextureList)
  INTERFACE_APPLY(iTextureWrapper)
  INTERFACE_APPLY(iVisibilityCuller)
%enddef

%define IMAP_APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iBinaryLoaderPlugin)
  INTERFACE_APPLY(iLoader)
  INTERFACE_APPLY(iLoaderPlugin)
  INTERFACE_APPLY(iMissingLoaderData)
%enddef

%define IMESH_APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(CS::Mesh::iAnimatedMeshFactory)
  INTERFACE_APPLY(CS::Mesh::iAnimatedMeshSubMeshFactory)
  INTERFACE_APPLY(CS::Mesh::iAnimatedMesh)
  INTERFACE_APPLY(CS::Mesh::iAnimatedMeshSubMesh)
  INTERFACE_APPLY(CS::Mesh::iAnimatedMeshMorphTarget)
  INTERFACE_APPLY(CS::Mesh::iAnimatedMeshSocket)
  INTERFACE_APPLY(CS::Mesh::iAnimatedMeshSocketFactory)
  INTERFACE_APPLY(iGeneralFactoryState)
  INTERFACE_APPLY(iGeneralMeshState)
  INTERFACE_APPLY(iParticleEmitter)
  INTERFACE_APPLY(iParticleEffector)
  INTERFACE_APPLY(iParticleSystemBase)
  INTERFACE_APPLY(iParticleSystemFactory)
  INTERFACE_APPLY(iParticleSystem)
  INTERFACE_APPLY(iParticleBuiltinEmitterBase)
  INTERFACE_APPLY(iParticleBuiltinEmitterSphere)
  INTERFACE_APPLY(iParticleBuiltinEmitterCone)
  INTERFACE_APPLY(iParticleBuiltinEmitterBox)
  INTERFACE_APPLY(iParticleBuiltinEmitterCylinder)
  INTERFACE_APPLY(iParticleBuiltinEmitterFactory)
  INTERFACE_APPLY(iParticleBuiltinEffectorForce)
  INTERFACE_APPLY(iParticleBuiltinEffectorLinColor)
  INTERFACE_APPLY(iParticleBuiltinEffectorLinear)
  INTERFACE_APPLY(iParticleBuiltinEffectorFactory)
  INTERFACE_APPLY(iMaterialArray)
  INTERFACE_APPLY(iMeshObject)
  INTERFACE_APPLY(iMeshObjectFactory)
  INTERFACE_APPLY(iMeshObjectType)
  INTERFACE_APPLY(iGenMeshSkeletonControlState)
  INTERFACE_APPLY(iSkeleton)
  INTERFACE_APPLY(iSkeletonBone)
  INTERFACE_APPLY(iSkeletonBoneFactory)
  INTERFACE_APPLY(iSkeletonBoneRagdollInfo)
  INTERFACE_APPLY(iSkeletonFactory)
  INTERFACE_APPLY(iSkeletonGraveyard)
  INTERFACE_APPLY(iSkeletonSocket)
  INTERFACE_APPLY(iSkeletonSocketFactory)
  INTERFACE_APPLY(iSkeletonManager2)
  INTERFACE_APPLY(iSkeletonFactory2)
  INTERFACE_APPLY(iSkeletonFSMNode2)
  INTERFACE_APPLY(iSkeletonFSMNodeFactory2)
  INTERFACE_APPLY(iSkeleton2)
  INTERFACE_APPLY(iSkeletonAnimPacketFactory2)
  INTERFACE_APPLY(iSkeletonAnimPacket2)
  INTERFACE_APPLY(iSkeletonAnimNodeFactory2)
  INTERFACE_APPLY(iSkeletonAnimNode2)
  INTERFACE_APPLY(iSkeletonAnimation2)
  INTERFACE_APPLY(iSkeletonAnimationNode2)
  INTERFACE_APPLY(iSkeletonAnimationNodeFactory2)
  INTERFACE_APPLY(iSkeletonPriorityNode2)
  INTERFACE_APPLY(iSkeletonPriorityNodeFactory2)
  INTERFACE_APPLY(iSkeletonBlendNodeFactory2)
  INTERFACE_APPLY(iSkeletonBlendNode2)
  INTERFACE_APPLY(iSprite2DState)
  INTERFACE_APPLY(iSprite3DState)
  INTERFACE_APPLY(iSpriteCal3DState)
  INTERFACE_APPLY(iTerrainCellCollisionProperties)
  INTERFACE_APPLY(iTerrainCellRenderProperties)
  INTERFACE_APPLY(iTerrainCellFeederProperties)
  INTERFACE_APPLY(iTerrainDataFeeder)
  INTERFACE_APPLY(iTerrainCollider)
  INTERFACE_APPLY(iTerrainRenderer)
  INTERFACE_APPLY(iTerrainCellHeightDataCallback)
  INTERFACE_APPLY(iTerrainCellLoadCallback)
  INTERFACE_APPLY(iTerrainSystem)
  INTERFACE_APPLY(iTerrainCell)
  INTERFACE_APPLY(iTerrainFactoryCell)
  INTERFACE_APPLY(iTerrainFactory)
  INTERFACE_APPLY(iTerrainFactoryState)
  INTERFACE_APPLY(iTerrainObjectState)
  INTERFACE_APPLY(iObjectModel)
  INTERFACE_APPLY(iObjectModelListener)
  INTERFACE_APPLY(iTriangleMeshIterator)
%enddef

%define ISNDSYS_APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iSndSysData)
  INTERFACE_APPLY(iSndSysManager)
  INTERFACE_APPLY(iSndSysSoftwareDriver)
  INTERFACE_APPLY(iSndSysSoftwareFilter3D)
  INTERFACE_APPLY(iSndSysListener)
  INTERFACE_APPLY(iSndSysLoader)
  INTERFACE_APPLY(iSndSysSource)
  INTERFACE_APPLY(iSndSysSource3D)
  INTERFACE_APPLY(iSndSysStream)
  INTERFACE_APPLY(iSndSysRenderer)
  INTERFACE_APPLY(iSndSysWrapper)
%enddef

%define IVARIA_APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iBodyGroup)
  INTERFACE_APPLY(iBugPlug)
  INTERFACE_APPLY(iDynamics)
  INTERFACE_APPLY(iDynamicSystem)
  INTERFACE_APPLY(iJoint)
  INTERFACE_APPLY(iScript)
  INTERFACE_APPLY(iScriptObject)
  INTERFACE_APPLY(iCollider)
  INTERFACE_APPLY(iCollideSystem)
  INTERFACE_APPLY(iConsoleInput)
  INTERFACE_APPLY(iConsoleOutput)
  INTERFACE_APPLY(iConsoleExecCallback)
  INTERFACE_APPLY(iODEDynamicState)
  INTERFACE_APPLY(iODEDynamicSystemState)
  INTERFACE_APPLY(iODEJointState)
  INTERFACE_APPLY(iODESliderJoint)
  INTERFACE_APPLY(iODEUniversalJoint)
  INTERFACE_APPLY(iODEAMotorJoint)
  INTERFACE_APPLY(iODEHingeJoint)
  INTERFACE_APPLY(iODEHinge2Joint)
  INTERFACE_APPLY(iODEBallJoint)
  INTERFACE_APPLY(iMapNode)
  INTERFACE_APPLY(iSequence)
  INTERFACE_APPLY(iSequenceCondition)
  INTERFACE_APPLY(iSequenceManager)
  INTERFACE_APPLY(iSequenceOperation)
  INTERFACE_APPLY(iSimpleFormerState)
  INTERFACE_APPLY(iStandardReporterListener)
  INTERFACE_APPLY(iEngineSequenceManager)
  INTERFACE_APPLY(iTerraFormer)
  INTERFACE_APPLY(iTerraSampler)
  INTERFACE_APPLY(iDecalManager)
  INTERFACE_APPLY(iDecalTemplate)
  INTERFACE_APPLY(iMovieRecorder)
  INTERFACE_APPLY(iView)
  INTERFACE_APPLY(iTranslator)
%enddef

%define IVIDEO_APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iFont)
  INTERFACE_APPLY(iFontServer)
  INTERFACE_APPLY(iGraphics3D)
  INTERFACE_APPLY(iGraphics2D)
  INTERFACE_APPLY(iHalo)
  INTERFACE_APPLY(iMaterial)
  INTERFACE_APPLY(iShader)
  INTERFACE_APPLY(iShaderManager)
  INTERFACE_APPLY(iShaderVariableContext)
  INTERFACE_APPLY(iTextureHandle)
  INTERFACE_APPLY(iTextureManager)
%enddef

%define CORE_APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iAnimatedImage)
  INTERFACE_APPLY(iBase)
  INTERFACE_APPLY(iCacheManager)
  INTERFACE_APPLY(iCommandLineParser)
  INTERFACE_APPLY(iComponent)
  INTERFACE_APPLY(iConfigFile)
  INTERFACE_APPLY(iConfigIterator)
  INTERFACE_APPLY(iConfigManager)
  INTERFACE_APPLY(iDataBuffer)
  INTERFACE_APPLY(iDebugHelper)
  INTERFACE_APPLY(iDocument)
  INTERFACE_APPLY(iDocumentAttribute)
  INTERFACE_APPLY(iDocumentAttributeIterator)
  INTERFACE_APPLY(iDocumentNode)
  INTERFACE_APPLY(iDocumentNodeIterator)
  INTERFACE_APPLY(iDocumentSystem)
  INTERFACE_APPLY(iEvent)
  INTERFACE_APPLY(iEventAttributeIterator)
  INTERFACE_APPLY(iEventHandler)
  INTERFACE_APPLY(iEventPlug)
  INTERFACE_APPLY(iEventQueue)
  INTERFACE_APPLY(iEventNameRegistry)
  INTERFACE_APPLY(iEventOutlet)
  INTERFACE_APPLY(iFactory)
  INTERFACE_APPLY(iFile)
  INTERFACE_APPLY(iJoystickDriver)
  INTERFACE_APPLY(iKeyboardDriver)
  INTERFACE_APPLY(iMouseDriver)
  INTERFACE_APPLY(iObject)
  INTERFACE_APPLY(iObjectIterator)
  INTERFACE_APPLY(iObjectRegistry)
  INTERFACE_APPLY(iPath)
  INTERFACE_APPLY(iPluginIterator)
  INTERFACE_APPLY(iPluginManager)
  INTERFACE_APPLY(iReporter)
  INTERFACE_APPLY(iReporterIterator)
  INTERFACE_APPLY(iReporterListener)
  INTERFACE_APPLY(iSCF)
  INTERFACE_APPLY(iString)
  INTERFACE_APPLY(iStringArray)
  INTERFACE_APPLY(iStringSet)
  INTERFACE_APPLY(iTriangleMesh)
  INTERFACE_APPLY(iVFS)
  INTERFACE_APPLY(iVirtualClock)
%enddef

#define CSTOOL_APPLY_FOR_EACH_INTERFACE

%define CSGFX_APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iImage)
  INTERFACE_APPLY(iImageIO)
  INTERFACE_APPLY(iShaderVarStringSet)
%enddef


%define APPLY_FOR_ALL_INTERFACES
  IENGINE_APPLY_FOR_EACH_INTERFACE
  IVARIA_APPLY_FOR_EACH_INTERFACE
  IVIDEO_APPLY_FOR_EACH_INTERFACE
  ISNDSYS_APPLY_FOR_EACH_INTERFACE
  IMAP_APPLY_FOR_EACH_INTERFACE
  IMESH_APPLY_FOR_EACH_INTERFACE
  CSTOOL_APPLY_FOR_EACH_INTERFACE
  CSGFX_APPLY_FOR_EACH_INTERFACE
  CORE_APPLY_FOR_EACH_INTERFACE
%enddef

/*
%define INTERFACE_PRE(T)
  %nodefault T;
  TYPEMAP_OUT_csRef(T)
  TYPEMAP_OUT_csPtr(T)
  TYPEMAP_OUT_csRefArray(T)
%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_PRE(x)
APPLY_FOR_ALL_INTERFACES
*/

