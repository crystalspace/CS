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
#include "cssys/sysfunc.h"
#include "motldr.h"
#include "iengine/motion.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#include "csgeom/transfrm.h"
#include "csgeom/quaterni.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"

//#define MOTION_DEBUG

CS_IMPLEMENT_PLUGIN

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

SCF_IMPLEMENT_IBASE (csMotionLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMotionLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csMotionSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMotionSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csMotionLoader)
SCF_IMPLEMENT_FACTORY (csMotionSaver)

SCF_EXPORT_CLASS_TABLE (motldr)
  SCF_EXPORT_CLASS (csMotionLoader, "crystalspace.motion.loader.default",
    "Skeletal Motion Manager Loader for Crystal Space")
  SCF_EXPORT_CLASS (csMotionSaver, "crystalspace.motion.saver.default",
    "Skeletal Motion Manager Saver for Crystal Space")
SCF_EXPORT_CLASS_TABLE_END

csMotionLoader::csMotionLoader(iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  object_reg=NULL;
}

csMotionLoader::~csMotionLoader()
{
  vfs->DecRef();
  motman->DecRef();
}

bool csMotionLoader::Initialize (iObjectRegistry* object_reg)
{
  csMotionLoader::object_reg=object_reg;
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS );
  if (!vfs)
  {
	printf("Motion Loader: Virtual file system not loaded.. aborting\n");
	return false;
  }
  motman = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.motion.manager.default", iMotionManager);
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
        csScanStr (params, "%f", &angle);
        m *= csXRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Y:
        csScanStr (params, "%f", &angle);
        m *= csYRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Z:
        csScanStr (params, "%f", &angle);
        m *= csZRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT:
        csScanStr (params, "%F", list, &num);
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
        csScanStr (params, "%f", &scaler);
        m *= csXScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Y:
        csScanStr (params, "%f", &scaler);
        m *= csYScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Z:
        csScanStr (params, "%f", &scaler);
        m *= csZScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE:
        csScanStr (params, "%F", list, &num);
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
    csScanStr (buf, "%F", list, &num);
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
  csScanStr (buf, "%f,%f,%f", &v.x, &v.y, &v.z);
  return true;
}

static bool load_quaternion (char* buf, csQuaternion &q)
{
  csScanStr (buf, "%f,%f,%f,%f", &q.x, &q.y, &q.z, &q.r);
  return true;
}


//=========================================================== Load Motion

void csMotionLoader::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.motion.loader", msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

iMotion* csMotionLoader::LoadMotion (const char* fname )
{
  iDataBuffer *databuff = vfs->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    Report (CS_REPORTER_SEVERITY_ERROR,
    	"Could not open motion file \"%s\" on VFS!", fname);
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
      Report (CS_REPORTER_SEVERITY_ERROR, "Expected parameters instead of '%s'!", buf);
      exit (1);
    }

    if (!motman)
      Report (CS_REPORTER_SEVERITY_ERROR, "No motion manager loaded!");
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
      Report (CS_REPORTER_SEVERITY_ERROR, "Expected parameters instead of '%s'!", buf);
      exit (1);
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
	      Report (CS_REPORTER_SEVERITY_ERROR, "Expected MATRIX, Q, or V instead of '%s'!", buf);
	      exit (1);
	  }     
        }
        break;
	  case CS_TOKEN_ACTIONSET:
	{
	  csScanStr( params, "%s", &buffer );
	  mot->AddFrameSet(buffer);	   
	}
	break;
      case CS_TOKEN_FRAME:
	{
	  int frametime,link;
	  csScanStr(name, "%d", &frametime);
	  int index=mot->AddFrame(frametime);
	  while((cmd = csGetObject (&params, tok_frame, &name, &params2))>0)
	  {
		switch (cmd)
		{
		  case CS_TOKEN_QLINK:
			{
	  		  csScanStr(params2, "%d", &link);
	  		  mot->AddFrameQLink(index, name, link);
			}
			break;
		  case CS_TOKEN_MLINK:
			{
			  csScanStr( params2, "%d", &link);
			  mot->AddFrameMLink(index, name, link);
			}
			break;
		  case CS_TOKEN_VLINK:
			{
			  csScanStr( params2, "%d", &link);
			  mot->AddFrameVLink(index, name, link);
			}
			break;
		  default:
	    	Report (CS_REPORTER_SEVERITY_ERROR, "Expected LINK instead of '%s'!", buf);
	    	exit (1);
		}
	  }
    } // case CS_TOKEN_FRAME
	
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Token '%s' not found while parsing the a sprite template!",
        csGetLastOffender ());
    exit (1);
  }
  return true;
}


iBase* csMotionLoader::Parse ( const char *string, iMaterialList*,
	iMeshFactoryList*, iBase* /* context */ )
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
	  exit (1);
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
	  Report(CS_REPORTER_SEVERITY_ERROR,
		  "Token '%s' not found while parsing the iMotionLoader plugin",
			csGetLastOffender());
	  exit (1);
	}
	return this;
}

//=============================================================== Motion Saver

csMotionSaver::csMotionSaver( iBase* base )
{
  SCF_CONSTRUCT_IBASE (base);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csMotionSaver::~csMotionSaver()
{
}

void csMotionSaver::WriteDown ( iBase* /* obj */, iStrVector* /* string */)
{
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iMotionManager *motman = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.motion.manager.default", iMotionManager);
  if (motman)
  {
	motman->DecRef();
  }
  else	
  printf("Motion Saver: Motion manager not loaded... aborting\n");
  plugin_mgr->DecRef ();
}
