#include "cssysdef.h"
#include "awsprefs.h"
#include "awscomp.h"
#include "awsslot.h"

awsComponent::awsComponent()
{
}

awsComponent::~awsComponent()
{
}
    
    
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

bool 
awsComponent::HandleEvent()
{
  return false;
}

/////////////////////////////////////  awsComponentFactory ////////////////////////////////////////////////////////

/**
  *  A factory is simply a class that knows how to build your component.  Although components aren't required to have
  * a factory, they will not be able to be instantiated through the template functions and window definitions if they
  * don't.  In any case, a factory is remarkably simple to build.  All you need to do is to inherit from 
  * awsComponentFactory and call register with the window manager and the named type of the component.  That's it.
  */
awsComponentFactory::awsComponentFactory(iAws *wmgr)
{
   // This is where you call register, only you must do it in the derived factory.  Like this:
   // Register(wmgr, "Radio Button");
}

awsComponentFactory::~awsComponentFactory()
{
   // Do nothing.
}

void
awsComponentFactory::Register(iAws *wmgr, char *name)
{
    wmgr->RegisterComponentFactory(this, name);
}



