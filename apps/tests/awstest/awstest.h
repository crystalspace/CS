#ifndef AWSTEST_H
#define AWSTEST_H

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "iaws/aws.h"
#include "iaws/awscnvs.h"

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
struct iObjectRegistry;
struct iVirtualClock;
struct iEvent;
struct iGraphics3D;
struct iGraphics2D;
struct iVFS;
class csTransform;

class awsTest
{
public:
  iObjectRegistry* object_reg;

private:
  iEngine* engine;
  iAws *aws;
  iAwsPrefManager *awsprefs;
  iAwsCanvas *awsCanvas;
  iSector* room;
  iView* view, *wview;
  iGraphics3D *myG3D;
  iGraphics2D *myG2D;
  iVFS *myVFS;
  iConsoleOutput *myConsole;
  iLoader* loader;
  iVirtualClock* vc;

  iFont* font;
  int col_red, col_blue, col_white, col_black;
  int col_yellow, col_cyan, col_green, col_gray;
  char message[255];
  csTicks message_timer;
  bool message_error;


public:
  awsTest();
  virtual ~awsTest();

  bool Initialize(int argc, const char* const argv[], const char *iConfigName);

  void Report (int severity, const char* msg, ...);

  void SetupFrame();
  void FinishFrame();
  bool HandleEvent (iEvent &Event);
};

#endif // AWSTEST_H
