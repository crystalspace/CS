#ifndef AWSTEST_H
#define AWSTEST_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "ivaria/aws.h"

struct iEngine;
struct iSector;
struct iView;
struct iFont;
struct iFile;
struct iImageLoader;
struct iLoaderPlugin;
struct iMeshWrapper;
struct iLoader;
struct iConsoleOutput;
class csTransform;

class awsTest : public SysSystemDriver
{
  typedef SysSystemDriver superclass;

  iEngine* engine;
  iAws *aws;
  iAwsPrefs *awsprefs;
  iSector* room;
  iView* view;
  iGraphics3D *myG3D;
  iGraphics2D *myG2D;
  iVFS *myVFS;
  iConsoleOutput *myConsole;  
  iLoader* loader;
  
  iFont* font;
  int col_red, col_blue, col_white, col_black;
  int col_yellow, col_cyan, col_green, col_gray;
  char message[255];
  csTicks message_timer;
  bool message_error;
  
  
public:
  awsTest();
  virtual ~awsTest();

  virtual bool Initialize(int argc, const char* const argv[], const char *iConfigName);

  void Report (int severity, const char* msg, ...);
    
  virtual void NextFrame();
  virtual bool HandleEvent (iEvent &Event); 
};

#endif // AWSTEST_H
