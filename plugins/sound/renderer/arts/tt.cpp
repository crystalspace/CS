// test program
#include <artsflow.h>
#include <connect.h>
#include <soundserver.h>
#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"
#include "isys/vfs.h"
#include "isound/data.h"
#include "isound/loader.h"
#include "csutil/databuf.h"
#include "csarts.h"
#include "csarts_impl.h"
#include <termios.h>
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
  init ();
  SysSystemDriver sys;
  sys.RequestPlugin ("crystalspace.kernel.vfs:VFS");
  sys.RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  sys.RequestPlugin ("crystalspace.sound.loader.multiplexer:SoundLoader");
  sys.RequestPlugin ("crystalspace.sound.loader.aiff:System.PlugIns.SoundAIFF");
  sys.RequestPlugin ("crystalspace.sound.loader.au:System.PlugIns.SoundAU");
  sys.RequestPlugin ("crystalspace.sound.loader.iff:System.PlugIns.SoundIFF");
  sys.RequestPlugin ("crystalspace.sound.loader.wav:System.PlugIns.SoundWAV");

  sys.RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");

  if (!sys.Initialize (argc, args, NULL))
  {
    sys.Printf (MSG_FATAL_ERROR, "Error initializing system !");
    return -1;
  }

  // get a soundloader
  iSoundLoader *pLoader = QUERY_PLUGIN_ID (&sys, "SoundLoader", iSoundLoader);
  // we read the soundata the CS way, that is through VFS
  iVFS *pVFS = QUERY_PLUGIN (&sys, iVFS);
  
  // load the sound
  iDataBuffer *db = pVFS->ReadFile (args[1]);

  // let the soundloader create a sounddata object from the data
  iSoundData *sd = pLoader->LoadSound (db->GetData (), db->GetSize ());

  // make a soundformat we like
  csSoundFormat format;
  format.Freq = 44100;
  format.Bits = 16;
  format.Channels = 2;

  sd->Initialize (&format);

  // now create the arts soundplay control
  Arts::Dispatcher dispatcher;
  Arts::SimpleSoundServer server;

  server = Arts::Reference ("global:Arts_SimpleSoundServer");
  if (server.isNull ())
  {
    printf ("server null\n");
    return 0;
  }

  Arts::csSoundModule sm = Arts::DynamicCast (server.createObject ("Arts::csSoundModule"));
  assert (!sm.isNull());

  std::vector<float> vData (sd->GetStaticNumSamples ()*2);
  short *data = (short*)sd->GetStaticData ();
  for (long i=0; i < sd->GetStaticNumSamples ()*2; i++)
    vData[i] = (float)data[i];
  sm.SetData (vData);

  sm.Set3DType (Arts::SOUND3D_RELATIVE);
  sm.start ();

  csVector3 pos (0, 0, 1);

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
	sm.SetSoundPosition (pos.x, pos.y, pos.z);
    }
  }

  sd->DecRef ();
  db->DecRef ();
  pVFS->DecRef ();
  pLoader->DecRef ();

  tcsetattr(0,TCSANOW,&save);
  return 1;
}
