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
#include "iutil/document.h"
#include "ivaria/reporter.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"

#include "csgeom/transfrm.h"
#include "csgeom/quaterni.h"
#include "csutil/scanstr.h"

//#define MOTION_DEBUG

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_BONE = 1,
  XMLTOKEN_FILE,
  XMLTOKEN_FRAME,
  XMLTOKEN_DURATION,
  XMLTOKEN_IDENTITY,
  XMLTOKEN_LOOP,
  XMLTOKEN_LOOPCOUNT,
  XMLTOKEN_LOOPFLIP,
  XMLTOKEN_MOTION,
  XMLTOKEN_POS,
  XMLTOKEN_ROT,
  XMLTOKEN_TIME
};

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
}

bool csMotionLoader::Initialize (iObjectRegistry* object_reg)
{
  csMotionLoader::object_reg=object_reg;
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS );
  if (!vfs)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "VFS not found!");
    return false;
  }
  motman = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.motion.manager.default", iMotionManager);
  if (!motman)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Motion manager not found!");
    return false;
  }
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Syntax services not found!");
    return false;
  }

  xmltokens.Register ("bone", XMLTOKEN_BONE);
  xmltokens.Register ("file", XMLTOKEN_FILE);
  xmltokens.Register ("frame", XMLTOKEN_FRAME);
  xmltokens.Register ("duration", XMLTOKEN_DURATION);
  xmltokens.Register ("identity", XMLTOKEN_IDENTITY);
  xmltokens.Register ("loop", XMLTOKEN_LOOP);
  xmltokens.Register ("loopcount", XMLTOKEN_LOOPCOUNT);
  xmltokens.Register ("loopflip", XMLTOKEN_LOOPFLIP);
  xmltokens.Register ("motion", XMLTOKEN_MOTION);
  xmltokens.Register ("pos", XMLTOKEN_POS);
  xmltokens.Register ("rot", XMLTOKEN_ROT);
  xmltokens.Register ("time", XMLTOKEN_TIME);
  return true;
}

//=========================================================== Load Motion

void csMotionLoader::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.motion.loader", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csMotionLoader::load_transform (iDocumentNode* node, csVector3 &v,
	csQuaternion &q, float& time)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_TIME:
        time = child->GetContentsValueAsFloat ();
	break;
      case XMLTOKEN_POS:
        if (!synldr->ParseVector (child, v))
	  return false;
        break;
      case XMLTOKEN_ROT:
        {
	  csRef<iDocumentNode> quatnode = child->GetNode ("q");
	  if (quatnode)
	  {
	    q.x = quatnode->GetAttributeValueAsFloat ("x");
	    q.y = quatnode->GetAttributeValueAsFloat ("y");
	    q.z = quatnode->GetAttributeValueAsFloat ("z");
	    q.r = quatnode->GetAttributeValueAsFloat ("r");
	  }
	  csRef<iDocumentNode> matnode = child->GetNode ("matrix");
	  if (matnode)
	  {
	    csMatrix3 m;
	    if (!synldr->ParseMatrix (matnode, m))
	      return false;
	    //q.Set (m);
	  }
	  csRef<iDocumentNode> eulernode = child->GetNode ("euler");
	  if (eulernode)
	  {
            csVector3 euler;
	    if (!synldr->ParseVector (eulernode, euler))
	      return false;
            q.SetWithEuler (euler);
	  }
	}
	break;
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }
  return true;
}

bool csMotionLoader::LoadBone (iDocumentNode* node, iMotionTemplate* mot,
	int bone)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FRAME:
        {
          float frametime = 1;
          csVector3 v (0,0,0);
          csQuaternion q (1,0,0,0);
          load_transform (child, v, q, frametime);
          mot->AddFrameBone (bone, frametime, v, q);
        }
        break;
      default:
        synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

bool csMotionLoader::LoadMotion (iDocumentNode* node, iMotionTemplate* mot)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_DURATION:
        {
	  csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
	  while (child_it->HasNext ())
	  {
	    csRef<iDocumentNode> childchild = child_it->Next ();
	    if (childchild->GetType () != CS_NODE_ELEMENT) continue;
	    const char* child_value = childchild->GetValue ();
	    csStringID child_id = xmltokens.Request (child_value);
	    switch (child_id)
	    {
	      case XMLTOKEN_TIME:
		mot->SetDuration (childchild->GetContentsValueAsFloat ());
		break;
	      case XMLTOKEN_LOOP:
                mot->SetLoopCount (-1);
                break;
	      case XMLTOKEN_LOOPFLIP:
		mot->SetLoopFlip (1);
		break;
              case XMLTOKEN_LOOPCOUNT:
	        {
		  int loopcount = childchild->GetAttributeValueAsInt ("count");
                  mot->SetLoopCount (loopcount);
                }
                break;
	      default:
	        synldr->ReportBadToken (childchild);
		return false;
	    }
          }
        }
        break;
      case XMLTOKEN_BONE:
        {
          int bone = mot->AddBone (child->GetAttributeValue ("name"));
          LoadBone (child, mot, bone);
        }
        break;
      default:
	synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}


csPtr<iBase> csMotionLoader::Parse (iDocumentNode* node,
	    iLoaderContext* /*ldr_context*/, iBase* /* context */ )
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MOTION:
        {
	  const char* motname = child->GetAttributeValue ("name");
	  iMotionTemplate* m = motman->FindMotionByName (motname);
	  if (!m)
	  {
	    m = motman->AddMotion (motname);
	    if (!LoadMotion (child, m))
	      return NULL;
	  }
	}
	break;
      default:
        synldr->ReportBadToken (child);
	return NULL;
    }
  }
  this->IncRef (); // the returned pointer wil be DecRef()ed.
  return csPtr<iBase> (this);
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

void csMotionSaver::WriteDown ( iBase* /* obj */, iFile* /* file */)
{
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  csRef<iMotionManager> motman (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.motion.manager.default", iMotionManager));
  if (!motman)
    printf("Motion Saver: Motion manager not loaded... aborting\n");
}

