/*
    Copyright (C) 1998 by Jorrit Tyberghein and K. Robert Bate.

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

/*----------------------------------------------------------------
	Written by K. Robert Bate 1998.
----------------------------------------------------------------*/

#include <InputSprocket.h>

Boolean	gInputSocketsEnabled;
Boolean	gInputSocketsRunning;
static void InitializeInputSprocket(void);

static ISpNeed gInputNeeds[] =
{
	{
		"\pAbort Game",
		256,
		0,
		kISpElementKind_Button,
		kISpElementLabel_Btn_PauseResume,
		0,
		0,
		0,
		0
	},
	{
		"\pMove To Origin",
		257,
		0,
		kISpElementKind_Button,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},
	{
		"\pX Axis Movement",
		258,
		0,
		kISpElementKind_Axis,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},
	{
		"\pY Axis Movement",
		259,
		0,
		kISpElementKind_Axis,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},
	{
		"\pZ Axis Movement",
		260,
		0,
		kISpElementKind_Axis,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},
	{
		"\pX Axis Rotation",
		261,
		0,
		kISpElementKind_Axis,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},
	{
		"\pY Axis Rotation",
		262,
		0,
		kISpElementKind_Axis,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},
	{
		"\pZ Axis Rotation",
		263,
		0,
		kISpElementKind_Axis,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	}
};

#define kNumInputElements	( sizeof( gInputNeeds ) / sizeof( ISpNeed ))
ISpElementReference	gInputElements[kNumInputElements];

static void InitializeInputSprocket()
{
	mInputVersion = ISpGetVersion();
	FailOSErr_( ISpElement_NewVirtualFromNeeds( kNumInputElements, gInputNeeds, gInputElements, 0 ));
	FailOSErr_( ISpInit( kNumInputElements, gInputNeeds, gInputElements, 'Crys', '0001', 0, 128, 0));
	ISpSuspend();
	gInputSocketsEnabled = true;
	gInputSocketsRunning = false;
}


// ---------------------------------------------------------------------------
//		¥ SpendTime
// ---------------------------------------------------------------------------
//	Idle time: Flash the insertion cursor

void C3DView::SpendTime( const EventRecord&	/* inMacEvent */ )
{
	if ( gInputSocketsEnabled ) {
		ISpTickle();

		inputControls theControls;

		theControls.pauseResume = false;

		theControls.xAxis = GetAxisValue( gInputElements[ kXAxis ] ) * kMoveDelta;
		theControls.yAxis = GetAxisValue( gInputElements[ kYAxis ] ) * kMoveDelta;
		theControls.zAxis = GetAxisValue( gInputElements[ kZAxis ] ) * kMoveDelta;

		/*	Check the rotate axis */
		theControls.xRotate = GetAxisValue( gInputElements[ kXRotate ] ) * kAngleDelta;
		theControls.yRotate = GetAxisValue( gInputElements[ kYRotate ] ) * kAngleDelta;
		theControls.zRotate = GetAxisValue( gInputElements[ kZRotate ] ) * kAngleDelta;

		/*	Check to see if the move to origin button was hit */
		theControls.moveToOrigin = WasButtonHit( gInputElements[kMoveToOrigin] );

		PerformMovementCommand( &theControls );
	}
}

//¥	--------------------	GetAxisValue

#define	kUpperSlop   0x88000000U
#define	kLowerSlop   0x78000000U

static double GetAxisValue( ISpElementReference inElement )
{
	unsigned long	input;
	double			output = 0.0;
	OSStatus		result;

	//¥	Check the movement axis
	result = ISpElement_GetSimpleState( inElement, &input );
	if ( result == noErr ) {
		if (( input < kUpperSlop ) && ( input > kLowerSlop ))
			input = kISpAxisMiddle;
		output = input;
		output /= kISpAxisMiddle;
		output -= 1.0;
	}

	return output;
}

//¥	--------------------	GetMovementValue

static void GetMovementValue( ISpElementReference inElement, float *x, float *y )
{
	ISpMovementData	input;
	OSStatus		result;

	//¥	Check the movement axis
	result = ISpElement_GetComplexState( inElement, sizeof( ISpMovementData ), &input );
	if ( result == noErr ) {
		if (( input.xAxis < kUpperSlop ) && ( input.xAxis > kLowerSlop ))
			input.xAxis = kISpAxisMiddle;
		*x = input.xAxis;
		*x /= kISpAxisMiddle;
		*x -= 1.0;
		if (( input.yAxis < kUpperSlop ) && ( input.yAxis > kLowerSlop ))
			input.yAxis = kISpAxisMiddle;
		*y = input.yAxis;
		*y /= kISpAxisMiddle;
		*y -= 1.0;
	}

	return;
}

//¥	--------------------	WasButtonHit

static Boolean WasButtonHit( ISpElementReference inElement )
{
	OSStatus	error;
	Boolean		wasEvent;
	Boolean		down = false;

	do {
		ISpElementEvent	event;

		error = ISpElement_GetNextEvent(inElement, sizeof( ISpElementEvent ), &event, &wasEvent );

		if (( ! error ) && ( wasEvent ) && ( event.data == kISpButtonDown )) {
			down = true;
			break;
		}
	} while (( wasEvent ) && ( ! error ));

	return down;
}
