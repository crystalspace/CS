// test program
//#include <artsflow.h>
//#include <dispatcher.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"
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
  if (tcgetattr(0,&save)!=0) {
    fputs("Failed to init",stderr);
    exit (1);
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
  SysSystemDriver sys;
  sys.RequestPlugin ("crystalspace.kernel.vfs:VFS");
  sys.RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  sys.RequestPlugin ("crystalspace.sound.loader.multiplexer:SoundLoader");
  sys.RequestPlugin ("crystalspace.sound.loader.aiff:System.Plugins.SoundAIFF");
  sys.RequestPlugin ("crystalspace.sound.loader.au:System.Plugins.SoundAU");
  sys.RequestPlugin ("crystalspace.sound.loader.iff:System.Plugins.SoundIFF");
  sys.RequestPlugin ("crystalspace.sound.loader.wav:System.Plugins.SoundWAV");
  sys.RequestPlugin ("crystalspace.sound.render.arts:SoundRender");

  sys.RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");

  //  Arts::Dispatcher *dispatcher = new Arts::Dispatcher;

  if (!sys.Initialize (argc, args, NULL))
  {
    printf ("Error initializing system !");
    return -1;
  }

  iObjectRegistry* object_reg = sys.GetObjectRegistry ();

  // get a soundloader
  iSoundLoader *pLoader = CS_QUERY_REGISTRY (object_reg, iSoundLoader);
  if (pLoader) pLoader->IncRef ();
  // we read the soundata the CS way, that is through VFS
  iVFS *pVFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (pVFS) pVFS->IncRef ();
  // well, since we want to try our renderer, we should request it now
  iSoundRender *pSR = CS_QUERY_REGISTRY (object_reg, iSoundRender);
  if (pSR) pSR->IncRef ();
  
  // load the sound
  iDataBuffer *db = pVFS->ReadFile (args[1]);

  // let the soundloader create a sounddata object from the data
  iSoundData *sd = pLoader->LoadSound (db->GetData (), db->GetSize ());

  // retrieve a soundhandle we can play from the renderer
  iSoundHandle *sh = pSR->RegisterSound (sd);

  // ok, we want to take it to the max, so we need to operate on a soundsource
  iSoundSource *ss = sh->CreateSource (SOUND3D_RELATIVE);

  csVector3 pos (0, 0, 1);

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

  ss->DecRef ();
  sh->DecRef ();
  pSR->DecRef ();
  sd->DecRef ();
  db->DecRef ();
  pVFS->DecRef ();
  pLoader->DecRef ();

  return 1;
}
