/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "motldr.h"
#include "iengine/motion.h"
#include "isys/system.h"
#include "isys/vfs.h"
#include "iutil/databuff.h"

#include "csgeom/transfrm.h"
#include "csgeom/quaterni.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"

//#define MOTION_DEBUG

CS_TOKEN_DEF_START
  CS_TOKEN_DEF(ANIM)
  CS_TOKEN_DEF(EULER)
  CS_TOKEN_DEF(FILE)
  CS_TOKEN_DEF(FRAME)
  CS_TOKEN_DEF(ACTIONSET)
  CS_TOKEN_DEF(QLINK)
  CS_TOKEN_DEF(MLINK)
  CS_TOKEN_DEF(VLINK)
  CS_TOKEN_DEF(IDENTITY)
  CS_TOKEN_DEF(MATRIX)
  CS_TOKEN_DEF(MOTION)
  CS_TOKEN_DEF(Q)
  CS_TOKEN_DEF(ROT_X)
  CS_TOKEN_DEF(ROT_Y)
  CS_TOKEN_DEF(ROT_Z)
  CS_TOKEN_DEF(ROT)
  CS_TOKEN_DEF(SCALE_X)
  CS_TOKEN_DEF(SCALE_Y)
  CS_TOKEN_DEF(SCALE_Z)
  CS_TOKEN_DEF(SCALE)
  CS_TOKEN_DEF(V)
CS_TOKEN_DEF_END


IMPLEMENT_IBASE (csMotionLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csMotionSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END


IMPLEMENT_FACTORY (csMotionLoader)
IMPLEMENT_FACTORY (csMotionSaver)

EXPORT_CLASS_TABLE (motldr)
  EXPORT_CLASS (csMotionLoader, "crystalspace.motion.loader.default",
    "Skeletal Motion Manager Loader for Crystal Space")
  EXPORT_CLASS (csMotionSaver, "crystalspace.motion.saver.default",
    "Skeletal Motion Manager Saver for Crystal Space")
EXPORT_CLASS_TABLE_END

csMotionLoader::csMotionLoader(iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
  sys=NULL;
}

csMotionLoader::~csMotionLoader()
{
  vfs->DecRef();
  motman->DecRef();
}

bool csMotionLoader::Initialize (iSystem* Sys)
{
  sys=Sys;
  vfs = QUERY_PLUGIN_ID( sys, CS_FUNCID_VFS, iVFS );
  if (!vfs)
  {
	printf("Motion Loader: Virtual file system not loaded.. aborting\n");
	return false;
  }
  motman = QUERY_PLUGIN_CLASS( sys, "crystalspace.motion.manager.default", "MotionManager" , iMotionManager );
  if (!motman)
  {
	printf("Motion Loader: Motion manager not loaded... aborting\n");
	return false;
  }
  return true;
}

static bool load_matrix (char* buf, csMatrix3 &m)
{
  CS_TOKEN_TABLE_START(commands)
	CS_TOKEN_TABLE(IDENTITY)
	CS_TOKEN_TABLE(ROT_X)
	CS_TOKEN_TABLE(ROT_Y)
	CS_TOKEN_TABLE(ROT_Z)
	CS_TOKEN_TABLE(ROT)
	CS_TOKEN_TABLE(SCALE_X)
	CS_TOKEN_TABLE(SCALE_Y)
	CS_TOKEN_TABLE(SCALE_Z)
	CS_TOKEN_TABLE(SCALE)
  CS_TOKEN_TABLE_END

  char* params;
  int cmd, num;
  float angle;
  float scaler;
  float list[30];
  const csMatrix3 identity;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_IDENTITY:
        m = identity;
        break;
      case CS_TOKEN_ROT_X:
        ScanStr (params, "%f", &angle);
        m *= csXRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Y:
        ScanStr (params, "%f", &angle);
        m *= csYRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Z:
        ScanStr (params, "%f", &angle);
        m *= csZRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT:
        ScanStr (params, "%F", list, &num);
        if (num == 3)
        {
          m *= csXRotMatrix3 (list[0]);
          m *= csZRotMatrix3 (list[2]);
          m *= csYRotMatrix3 (list[1]);
        }
        else
	  printf ("Badly formed rotation: '%s'\n", params);
        break;
      case CS_TOKEN_SCALE_X:
        ScanStr (params, "%f", &scaler);
        m *= csXScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Y:
        ScanStr (params, "%f", &scaler);
        m *= csYScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Z:
        ScanStr (params, "%f", &scaler);
        m *= csZScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE:
        ScanStr (params, "%F", list, &num);
        if (num == 1)      // One scaler; applied to entire matrix.
	  m *= list[0];
        else if (num == 3) // Three scalers; applied to X, Y, Z individually.
	  m *= csMatrix3 (list[0],0,0,0,list[1],0,0,0,list[2]);
        else
	  printf ("Badly formed scale: '%s'\n", params);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    // Neither SCALE, ROT, nor IDENTITY, so matrix may contain a single scaler
    // or the nine values of a 3x3 matrix.
    ScanStr (buf, "%F", list, &num);
    if (num == 1)
      m = csMatrix3 () * list[0];
    else if (num == 9)
      m = csMatrix3 (
        list[0], list[1], list[2],
        list[3], list[4], list[5],
        list[6], list[7], list[8]);
    else
      printf ("Badly formed matrix '%s'\n", buf);
  }
  return true;
}

static bool load_vector (char* buf, csVector3 &v)
{
  ScanStr (buf, "%f,%f,%f", &v.x, &v.y, &v.z);
  return true;
}

static bool load_quaternion (char* buf, csQuaternion &q)
{
  ScanStr (buf, "%f,%f,%f,%f", &q.x, &q.y, &q.z, &q.r);
  return true;
}


//=========================================================== Load Motion

iMotion* csMotionLoader::LoadMotion (const char* fname )
{
  iDataBuffer *databuff = vfs->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    sys->Printf (MSG_FATAL_ERROR, "Could not open motion file \"%s\" on VFS!\n", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
  	CS_TOKEN_TABLE (MOTION)
  CS_TOKEN_TABLE_END

  char *name, *data;
  char *buf = **databuff;
  long cmd;
  

  if ((cmd=csGetObject (&buf, tokens, &name, &data)) > 0)
  {
    if (!data)
    {
      sys->Printf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }

    if (!motman)
      sys->Printf (MSG_FATAL_ERROR, "No motion manager loaded!\n");
    else
    {
      iMotion* m = motman->FindByName (name);
      if (!m)
      {
	m = motman->AddMotion (name);
	if (LoadMotion (m, data))
	{
	  databuff->DecRef ();
	  return m;
	}
	else
	{
	  m->DecRef ();
	  databuff->DecRef ();
	  return NULL;
	}
      }
    }
  }
  databuff->DecRef ();
  return NULL;
}

bool csMotionLoader::LoadMotion (iMotion* mot, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ANIM)
    CS_TOKEN_TABLE (FRAME)
	CS_TOKEN_TABLE (ACTIONSET)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_anim)
    CS_TOKEN_TABLE (EULER)
    CS_TOKEN_TABLE (Q)
    CS_TOKEN_TABLE (MATRIX)
	CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_frame)
    CS_TOKEN_TABLE (QLINK)
    CS_TOKEN_TABLE (MLINK)
    CS_TOKEN_TABLE (VLINK)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char* params2;
  char buffer[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      sys->Printf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_ANIM:
        {
	  cmd = csGetObject (&params, tok_anim, &name, &params2);
	  switch (cmd) 
	  {
	    case CS_TOKEN_EULER:
	      {
	        csVector3 e;
	        csQuaternion quat;
	        load_vector(params2, e);
	        quat.SetWithEuler(e);
	        mot->AddAnim(quat);
	      }
	      break;
	    case CS_TOKEN_Q:
	      {
	        csQuaternion quat;
	        load_quaternion(params2, quat);
	        mot->AddAnim(quat);
	      }
	      break;
	    case CS_TOKEN_MATRIX:
	      {
	        csMatrix3 mat;
	        load_matrix(params2, mat);
	        mot->AddAnim(mat);
	      }
	      break;
		case CS_TOKEN_V:
		  {
			csVector3 vec;
			load_vector(params2, vec);
			mot->AddAnim(vec);
		  }
		  break;
	    default:
	      sys->Printf (MSG_FATAL_ERROR, "Expected MATRIX, Q, or V instead of '%s'!\n", buf);
	      fatal_exit (0, false);
	  }     
        }
        break;
	  case CS_TOKEN_ACTIONSET:
	{
	  ScanStr( params, "%s", &buffer );
	  mot->AddFrameSet(buffer);	   
	}
	break;
      case CS_TOKEN_FRAME:
	{
	  int frametime,link;
	  ScanStr(name, "%d", &frametime);
	  int index=mot->AddFrame(frametime);
	  while((cmd = csGetObject (&params, tok_frame, &name, &params2))>0)
	  {
		switch (cmd)
		{
		  case CS_TOKEN_QLINK:
			{
	  		  ScanStr(params2, "%d", &link);
	  		  mot->AddFrameQLink(index, name, link);
			}
			break;
		  case CS_TOKEN_MLINK:
			{
			  ScanStr( params2, "%d", &link);
			  mot->AddFrameMLink(index, name, link);
			}
			break;
		  case CS_TOKEN_VLINK:
			{
			  ScanStr( params2, "%d", &link);
			  mot->AddFrameVLink(index, name, link);
			}
			break;
		  default:
	    	sys->Printf (MSG_FATAL_ERROR, "Expected LINK instead of '%s'!\n", buf);
	    	fatal_exit (0, false);
		}
	  }
    } // case CS_TOKEN_FRAME
	
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    sys->Printf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a sprite template!\n",
        csGetLastOffender ());
    fatal_exit (0, false);
  }
  return true;
}


iBase* csMotionLoader::Parse ( const char *string, iEngine* /* engine */, iBase* /* context */ )
{
CS_TOKEN_TABLE_START(commands)
  CS_TOKEN_TABLE(FILE)
  CS_TOKEN_TABLE(MOTION)
CS_TOKEN_TABLE_END

  char *name;
  long cmd;
  char *params;
  char str[255];
  char *buf = (char *) string;
  str[0] = '\0';
  iMotion *m;

  while (( cmd = csGetObject ( &buf, commands, &name, &params )) > 0)
  {
	if (!params)
	{
	  printf("Expected parameters instead of '%s'\n", string);
	  fatal_exit( 0, false );
	}
	switch (cmd)
	{
      case CS_TOKEN_MOTION:
	  {
	    m = motman->FindByName (name);
	    if (!m)
	    {
		  m = motman->AddMotion (name);
		  LoadMotion (m, params);
	    }
	  }
	  break;
	  case CS_TOKEN_FILE:
	  {
		m = LoadMotion(params);
		if (!m)
		{
		  printf("Failed to load motion file '%s' from VFS\n",params);
		}
	  }
	  break;
	}
  }
	if (cmd == CS_PARSERR_TOKENNOTFOUND)
	{
	  sys->Printf( MSG_FATAL_ERROR,
		  "Token '%s' not found while parsing the iMotionLoader plugin\n",
			csGetLastOffender());
	  fatal_exit(0,false);
	}
	return this;
}

//=============================================================== Motion Saver

csMotionSaver::csMotionSaver( iBase* base )
{
  CONSTRUCT_IBASE (base);
}

csMotionSaver::~csMotionSaver()
{
}

bool csMotionSaver::Initialize( iSystem* system )
{
  sys = system;
  return true;
}

void csMotionSaver::WriteDown ( iBase* /* obj */, iStrVector* /* string */, iEngine* /* engine */)
{
  iMotionManager *motman = QUERY_PLUGIN_CLASS( sys, "crystalspace.motion.manager.default", "MotionManager" , iMotionManager );
  if (motman)
  {
	motman->DecRef();
  }
  else	
  printf("Motion Saver: Motion manager not loaded... aborting\n");
}
