#include "cssysdef.h"
#include "awsprefs.h"
#include "awscomp.h"
#include "awsslot.h"

/**
 *  This function is normally called automatically by the window manager.  You may call it manually if you wish, but
 * there's little reason to do so.  
 **************************************************************************************************************/
bool 
awsComponent::Setup(iAws *_wmgr, awsComponentNode *settings)
{
  if (!wmgr) return false;

  wmgr = _wmgr;
  
  if (settings) 
  {
  
   iAwsPrefs *pm=WindowManager()->GetPrefMgr();
   iString *id_str=NULL;
  
   pm->GetRect(settings, "Frame", frame);
   pm->GetString(settings, "Id", id_str);

   if (id_str!=NULL) id = pm->NameToId(id_str->GetData());

   // Children are automatically filled in by the windowmanager.
   
  }

  return true;

}

void
awsComponent::Invalidate()
{
  WindowManager()->Mark(frame);
}


