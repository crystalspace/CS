// test program
//#include <artsflow.h>
//#include <dispatcher.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"
#include "cstool/initapp.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "isound/data.h"
#include "isound/loader.h"
#include "isound/renderer.h"
#include "isound/source.h"
#include "isound/handle.h"
#include "csutil/databuf.h"
#include <termios.h>
#include <sys/time.h>
#include "csgeom/vector3.h"

struct termios neu;
struct termios save;

CS_IMPLEMENT_APPLICATION

int kbhit(void)
{
    fd_set set;
    struct timeval tv;
    int tmp;
    int result;

    tv.tv_sec=tv.tv_usec=0;    /* select will not wait even a microsecond */

    FD_ZERO(&set);    /* Must be done to clear the file descriptor set */
    FD_SET(0,&set);    /* Set watch on stdin (file descriptor 0) */

    /* Find out if characters are available in stdin */
    tmp=select(1,&set,NULL,NULL,&tv);
    if (-1==tmp) {
        /* An error occurred with select */
        result=-1;
    } else {
        /* Are there characters available? */
        if (FD_ISSET(0,&set)) {
            /* Yes */
            result=1;
        } else {
            /* Nope */
            result=0;
        }
    }
    return result;
}

void init ()
{
  /* Get the current terminal settings */
  if (tcgetattr(0,&save)!=0)
  {
    fputs("Failed to init",stderr);
    return;	// @@@ Use reporter and failure value!
  }

  /* We'll use save as a copy to restore the original
     * settings later
     */
  neu=save;

  /* Set up terminal so that characters are not
     * echoed and turn off canonical mode.
     */
  neu.c_cc[VMIN]=1;
  neu.c_lflag&=~ICANON;
  neu.c_lflag&=~ECHO;
  /* Send the new settings to the terminal */
  tcsetattr(0,TCSANOW,&neu);

}

int main(int argc, const char* const args[])
{
  iObjectRegistry *oreg = csInitializer::CreateEnvironment (argc, args);
  csInitializer::RequestPlugins (
				 oreg,
				 CS_REQUEST_VFS,
				 CS_REQUEST_PLUGIN ("crystalspace.sound.loader.multiplexer", iSoundLoader),
				 CS_REQUEST_PLUGIN ("crystalspace.sound.render.arts", iSoundRender),
				 CS_REQUEST_END
				 );


  // get a soundloader
  csRef<iSoundLoader> pLoader (CS_QUERY_REGISTRY (oreg, iSoundLoader));
  // we read the soundata the CS way, that is through VFS
  csRef<iVFS> pVFS (CS_QUERY_REGISTRY (oreg, iVFS));
  // well, since we want to try our renderer, we should request it now
  csRef<iSoundRender> pSR (CS_QUERY_REGISTRY (oreg, iSoundRender));

  // load the sound
  csRef<iDataBuffer> db (pVFS->ReadFile (args[1]));

  // let the soundloader create a sounddata object from the data
  csRef<iSoundData> sd (pLoader->LoadSound (db->GetData (), db->GetSize ()));

  // retrieve a soundhandle we can play from the renderer
  csRef<iSoundHandle> sh (pSR->RegisterSound (sd));

  // ok, we want to take it to the max, so we need to operate on a soundsource
  //  csRef<iSoundSource> ss (sh->CreateSource (SOUND3D_RELATIVE));
  csRef<iSoundSource> ss (sh->Play (true));

  csVector3 pos (0, 0, 1);
  ss->Play (true);
  init ();
  while (1)
  {
    if (kbhit())
    {
      bool bSet = true;
      int c = getchar ();
      if (c == 'q')
	break;
      switch (c)
      {
      case 'a':
	pos.z+=0.1;
	break;
      case 'y':
	pos.z-=0.1;
	break;
      case 'x':
	pos.x-=0.1;
	break;
      case 'c':
	pos.x+=0.1;
	break;
      default:
	bSet = false;
      }
      if (bSet)
	ss->SetPosition (pos);
    }
  }
  tcsetattr(0,TCSANOW,&save);

  csInitializer::DestroyApplication (oreg);
  return 0;
}

