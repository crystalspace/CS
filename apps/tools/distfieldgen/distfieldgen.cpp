/*
    Copyright (C) 2008 by Frank Richter

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

#include "distfieldgen.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

DistFieldGen::DistFieldGen (iObjectRegistry* object_reg) : object_reg (object_reg)
{
}

bool DistFieldGen::Initialize ()
{
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.distfieldgen",
	"Can't initialize plugins!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    PrintHelp ();
    return false;
  }

  return true;
}

class ComputationProgress
{
  CS::Threading::Mutex mutex;
  csTextProgressMeter progress;
public:
  ComputationProgress (int total) : progress (0, total) {}

  void Step()
  {
    CS::Threading::MutexScopedLock l (mutex);
    progress.Step ();
  }
};

struct ImageWrapper
{
  int width;
  int height;
  csRGBpixel* data;

  ImageWrapper (iImage* image)
  {
    width = image->GetWidth();
    height = image->GetHeight();
    data = (csRGBpixel*)image->GetImageData();
  }
};

class DistFieldComputer : public CS::Threading::Runnable
{
  ImageWrapper loResImage;
  ImageWrapper hiResImage;

  int y1, y2;

  ComputationProgress& progress;
  CS::Threading::Barrier* barrier;
public:
  bool runStatus;

  DistFieldComputer (ImageWrapper loResImage, ImageWrapper hiResImage,
		     int y1, int y2, ComputationProgress& progress,
		     CS::Threading::Barrier* barrier = 0)
   : loResImage (loResImage), hiResImage (hiResImage), y1 (y1), y2 (y2),
     progress (progress), barrier (barrier), runStatus (false) {}

  bool Compute()
  {
    int lwidth = loResImage.width;
    int lheight = loResImage.height;
    int hwidth = hiResImage.width;
    int hheight = hiResImage.height;
    if ((lwidth > hwidth) && (lheight > hheight))
      return false;
    float scaleXToHiRes = (float)hwidth/(float)lwidth;
    float scaleYToHiRes = (float)hheight/(float)lheight;
    /* Radius around a pixel to search for closest pixel of the opposite state
     * ("spread factor" in the paper) */
    int searchRadius = (int)ceilf (csMax (scaleXToHiRes, scaleYToHiRes) * 0.707106781);

    const csRGBpixel* hiData = hiResImage.data;
    csRGBpixel* loData = loResImage.data;

    for (int y = y1; y < y2; y++)
    {
      for (int x = 0; x < lwidth; x++)
      {
	int hiResX = (int)((x + 0.5f) * scaleXToHiRes);
	int hiResY = (int)((y + 0.5f) * scaleYToHiRes);
	bool state = hiData[hiResY * hwidth + hiResX].alpha > 0.5f;
	float minRadius = searchRadius;
	int search_x1 = hiResX-searchRadius;
	int search_x2 = hiResX+searchRadius;
	int search_y1 = hiResY-searchRadius;
	int search_y2 = hiResY+searchRadius;

	for (int sy = search_y1; sy <= search_y2; sy++)
	{
	  int real_y = sy;
	  if (real_y < 0) real_y += hheight;
	  else if (real_y >= hheight) real_y -= hheight;
	  for (int sx = search_x1; sx <= search_x2; sx++)
	  {
	    int real_x = sx;
	    if (real_x < 0) real_x += hwidth;
	    else if (real_x >= hwidth) real_x -= hwidth;
	    bool search_state = hiData[real_y * hwidth + real_x].alpha > 0.5f;
	    if (search_state != state)
	    {
	      int dx = sx - hiResX;
	      int dy = sy - hiResY;
	      minRadius = csMin (minRadius, sqrtf (dx*dx + dy*dy));
	    }
	  }
	}

	uint8 distNorm;
	if (state)
	  distNorm = int (127.5f + 127.5f*(minRadius/(float)searchRadius));
	else
	  distNorm = int (127.5f - 127.5f*(minRadius/(float)searchRadius));
	loData[y * lwidth + x].alpha = distNorm;
      }
      progress.Step ();
    }
    return true;
  }

  void Run()
  {
    runStatus = Compute();
    barrier->Wait ();
  }
};

bool DistFieldGen::Run ()
{
  csRef<iImageIO> imageio = csQueryRegistry<iImageIO> (object_reg);
  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (object_reg);

  const char* lowResImageFN = cmdline->GetName (0);
  const char* hiResImageFN = cmdline->GetName (1);
  if ((lowResImageFN == 0) || (hiResImageFN == 0))
  {
    PrintHelp ();
    return false;
  }

  const char* outFile = cmdline->GetOption ("output");
  if (outFile == 0) outFile = lowResImageFN;

  const char* mimeType = cmdline->GetOption ("format");
  if (mimeType == 0) mimeType = "image/png";

  // Read images
  csRef<iImage> loResImage;
  {
    csPhysicalFile file (lowResImageFN, "rb");
    if (file.GetStatus() != VFS_STATUS_OK) return false;
    csRef<iDataBuffer> buf = file.GetAllData();
    if (!buf.IsValid()) return false;
    loResImage = imageio->Load (buf, CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
    if (!loResImage.IsValid()) return false;
  }
  csRef<iImage> hiResImage;
  {
    csPhysicalFile file (hiResImageFN, "rb");
    if (file.GetStatus() != VFS_STATUS_OK) return false;
    csRef<iDataBuffer> buf = file.GetAllData();
    if (!buf.IsValid()) return false;
    hiResImage = imageio->Load (buf, CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
    if (!hiResImage.IsValid()) return false;
  }

  // Actual computation
  {
    ImageWrapper loImage (loResImage);
    ImageWrapper hiImage (hiResImage);

    int lheight = loImage.height;
    ComputationProgress progress (lheight);

    uint numThreads = CS::Platform::GetProcessorCount();
    if (numThreads > 1)
    {
      if (numThreads > (uint)lheight) numThreads = (uint)lheight;

      int linesPerThread = (lheight + numThreads - 1) / numThreads;
      int y1 = 0;
      csPDelArray<DistFieldComputer> computers;
      csPDelArray<CS::Threading::Thread> threads;
      CS::Threading::Barrier barrier (numThreads+1);
      for (uint t = 0; t <numThreads; t++)
      {
	DistFieldComputer* computer = new DistFieldComputer (loImage, hiImage, 
	  y1, csMin (y1 + linesPerThread, lheight), 
	  progress, &barrier);
	computers.Push (computer);
	threads.Push (new CS::Threading::Thread (computer, true));
	y1 += linesPerThread;
      }
      barrier.Wait ();

      bool status = true;
      for (uint t = 0; t <numThreads; t++)
	status &= computers[t]->runStatus;
      if (!status) return false;
    }
    else
    {
      DistFieldComputer computer (loImage, hiImage, 0, lheight, progress);
      if (!computer.Compute ()) return false;
    }
  }

  // Write output
  {
    csRef<iDataBuffer> outData = imageio->Save (loResImage, mimeType);
    if (!outData.IsValid()) return false;
    csPhysicalFile file (outFile, "wb");
    if (file.GetStatus() != VFS_STATUS_OK) return false;
    if (file.Write (outData->GetData(), outData->GetSize())
	!= outData->GetSize())
      return false;
  }

  return true;
}

void DistFieldGen::PrintHelp ()
{
  const char* appname = "distfieldgen";

  csPrintf ("Usage: %s [options] <low-res image> <high-res image>\n", appname);
  csPrintf ("\n");
  csPrintf ("Maps the alpha channel of <high-res image> to a binary alpha value,\n");
  csPrintf ("computes a distance field in the resolution of <low-res image>\n");
  csPrintf ("and stores the result in the alpha channel of <low-res image>.\n");
  csPrintf ("\n");
  csPrintf ("For details on the nature of this distance field and how it is used see\n");
  csPrintf ("http://www.valvesoftware.com/publications/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf\n");
  csPrintf ("\n");
  csPrintf ("Options:\n");
  csPrintf ("  -output=<file>    Write result to the given file. Default: <low-res image>\n");
  csPrintf ("  -format=<file>    Write result in the given format. Default: image/png\n");
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  int res = 1;
  {
    CS::Utility::ScopedDelete<DistFieldGen> app (new DistFieldGen (object_reg));

    if (app->Initialize ())
      res = app->Run () ? 0 : 1;
  }

  csInitializer::DestroyApplication (object_reg);
  return res;
}
