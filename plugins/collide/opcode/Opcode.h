///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for Opcode.dll.
 *	\file		Opcode.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __CS_OPCODE_H__
#define __CS_OPCODE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation messages
//#if defined(OPCODE_EXPORTS)
//	#pragma message("Compiling OPCODE")
//#elif !defined(OPCODE_EXPORTS)
//	#pragma message("Using OPCODE")
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Automatic linking
//	#ifndef BAN_OPCODE_AUTOLINK
//		#ifdef _DEBUG
//			#pragma comment(lib, "Opcode_D.lib")
//		#else
//			#pragma comment(lib, "Opcode.lib")
//		#endif
//	#endif
//#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor
//#ifdef OPCODE_EXPORTS
//	#define OPCODE_API __declspec(dllexport)
//#else
//	#define OPCODE_API __declspec(dllimport)
//#endif

#ifndef __ICECORE_H__
//	#ifdef WIN32
//	#include <windows.h>
//	#include <windowsx.h>
//	#endif // WIN32

	#include <stdio.h>
	#include <stdlib.h>
	#include <assert.h>
	#include <float.h>
        #include <string.h>

	#ifndef ASSERT
		#define	ASSERT	assert
	#endif

        #define _asm   				__asm__
        #define __asm  				__asm__


	#define	Log
	#define	SetIceError		false
	#define	EC_OUTOFMEMORY	"Out of memory"
	#define	Alignment

	#include "OPC_Preprocessor.h"
//	#undef ICECORE_API
//	#define ICECORE_API	__declspec(dllimport)
	#define ICECORE_API	OPCODE_API

	#include "OPC_Types.h"
	#include "OPC_FPU.h"
	#include "OPC_MemoryMacros.h"
	namespace IceCore
	{
		#include "OPC_Container.h"
	}
	using namespace IceCore;
#endif

#ifndef __ICEMATHS_H__
	#include <math.h>
	#define ICEMATHS_API	OPCODE_API
	namespace IceMaths
	{
		#include "OPC_Point.h"
		#include "OPC_Matrix3x3.h"
		#include "OPC_Matrix4x4.h"
		#include "OPC_Plane.h"
		#include "OPC_Ray.h"
	}
	using namespace IceMaths;
#endif

#ifndef __MESHMERIZER_H__
	#define MESHMERIZER_API	OPCODE_API
	namespace Meshmerizer
	{
		#include "OPC_Triangle.h"
		#include "OPC_AABB.h"
		#include "OPC_OBB.h"
		#include "OPC_BoundingSphere.h"
	}
	using namespace Meshmerizer;
#endif

	namespace Opcode
	{
		// Bulk-of-the-work
		#include "OPC_Settings.h"
		#include "OPC_Common.h"
		#include "OPC_TreeBuilders.h"
		#include "OPC_AABBTree.h"
		#include "OPC_OptimizedTree.h"
		#include "OPC_Model.h"
		#include "OPC_BVTCache.h"
		#include "OPC_Collider.h"
		#include "OPC_VolumeCollider.h"
		#include "OPC_TreeCollider.h"
		#include "OPC_RayCollider.h"
		#include "OPC_SphereCollider.h"
		#include "OPC_OBBCollider.h"
		#include "OPC_AABBCollider.h"
		#include "OPC_PlanesCollider.h"
	}

#endif // __CS_OPCODE_H__
