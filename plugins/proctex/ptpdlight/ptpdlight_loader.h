/*
    Copyright (C) 2003-2006 by Jorrit Tyberghein
	      (C) 2003-2007 by Frank Richter

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

#ifndef __CS_PTPDLIGHT_LOADER_H__
#define __CS_PTPDLIGHT_LOADER_H__

#include "iutil/comp.h"
#include "imap/reader.h"

#include "csutil/csstring.h"
#include "csutil/priorityqueue.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"

struct iLoader;

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

class ProctexPDLight;

class ProctexPDLightLoader :
  public scfImplementation2<ProctexPDLightLoader,
                            iLoaderPlugin, 
                            iComponent>
{
protected:
  iObjectRegistry* object_reg;

  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "plugins/proctex/ptpdlight/ptpdlight_loader.tok"
#include "cstool/tokenlist.h"

public:
  class Scheduler
  {
  public:
    struct QueueItem
    {
      uint prio;
      ProctexPDLight* tex;

      bool operator< (const QueueItem& other) const
      { return prio < other.prio; }
    };
  protected:
    CS::Utility::PriorityQueue<QueueItem> queue;
    csSet<csConstPtrKey<ProctexPDLight> > queuedPTs;

    csTicks lastFrameTime;
    uint frameNumber;

    csTicks timeBudget;
    csTicks thisFrameUsedTime;

    ProctexPDLight* currentPT;
  public:
    Scheduler () : lastFrameTime ((csTicks)~0), frameNumber (0), 
      timeBudget ((csTicks)~0), currentPT (0) {}

    void SetBudget (csTicks budget) { timeBudget = budget; }

    bool UpdatePT (ProctexPDLight* texture, csTicks time);
    void RecordUpdateTime (csTicks time)
    { thisFrameUsedTime += time; }
    void UnqueuePT (ProctexPDLight* texture);
  };
protected:
  Scheduler sched;
  bool doMMX;

  bool ParseMap (iDocumentNode* node, ProctexPDLight* pt,
                 iLoader* LevelLoader);

  void Report (int severity, iDocumentNode* node, const char* msg, ...);
  bool HexToLightID (uint8* lightID, const char* lightIDHex);
  
  ProctexPDLight* NewProctexPDLight (iImage* img);
  ProctexPDLight* NewProctexPDLight (int w, int h);
public:
  ProctexPDLightLoader (iBase *p);
  virtual ~ProctexPDLightLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource*, iLoaderContext* ldr_context,
  	iBase* context);

  virtual bool IsThreadSafe() { return true; }

  // PT update "scheduler"
  bool UpdatePT (ProctexPDLight* texture, csTicks time)
  { return sched.UpdatePT (texture, time); }
  void RecordUpdateTime (csTicks time)
  { sched.RecordUpdateTime (time); }
  void UnqueuePT (ProctexPDLight* texture)
  { sched.UnqueuePT (texture); }
};  

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)

#define NS_PTPDL    CS_PLUGIN_NAMESPACE_NAME(PTPDLight)

template<>
class csComparator<NS_PTPDL::ProctexPDLightLoader::Scheduler::QueueItem,
  NS_PTPDL::ProctexPDLight*>
{
public:
  static int Compare(
    NS_PTPDL::ProctexPDLightLoader::Scheduler::QueueItem const &a, 
    NS_PTPDL::ProctexPDLight* const& b)
  {
    if (a.tex == b) return 0;
    return csComparator<const NS_PTPDL::ProctexPDLight*,
      NS_PTPDL::ProctexPDLight*>::Compare (a.tex, b);
  }
};

#undef NS_PTPDL

#endif // __CS_PTPDLIGHT_LOADER_H__
