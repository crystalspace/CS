/*
    Copyright (C) 2002 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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
#include "csutil/ref.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "cssys/sysfunc.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "inetwork/driver.h"
#include "inetwork/netman.h"
#include "csutil/datastrm.h"

#include <stdio.h>
#include <string.h>

#define PORT "1234"
#define PROTO "udp"

CS_IMPLEMENT_APPLICATION

iObjectRegistry *objreg;

char testdata [] = "Hello, world!";

class SendPacket : public iNetworkPacket
{
  char *data;

  public:
  SCF_DECLARE_IBASE;
  SendPacket ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~SendPacket ()
  { SCF_DESTRUCT_IBASE (); }

  void SetData (char d []) { data = d; }

  virtual bool Read (csDataStream &st, iNetworkConnection *so) { return false; }
  virtual char* Write (size_t &size)
  {
    size = strlen (data);
    return data;
  }
  virtual csPtr<iNetworkPacket> New ()
    { return csPtr<iNetworkPacket> (new SendPacket ()); }
  virtual bool FilterSocket (iNetworkConnection *s) { return true; }
};

SCF_IMPLEMENT_IBASE (SendPacket)
  SCF_IMPLEMENTS_INTERFACE (iNetworkPacket)
SCF_IMPLEMENT_IBASE_END

class RecvPacket : public iNetworkPacket
{
  int8 data [256];
  unsigned position;

  public:
  SCF_DECLARE_IBASE;
  RecvPacket ()
  {
    SCF_CONSTRUCT_IBASE (0);
    position = 0;
  }
  virtual ~RecvPacket ()
  { SCF_DESTRUCT_IBASE(); }

  const int8* GetData () { return data; }

  virtual bool Read (csDataStream &stream, iNetworkConnection *sock)
  {
    while (stream.ReadInt8 (data [position])) position++;
    if (position >= strlen (testdata))
    {
      data [position] = 0;
      return true;
    }
    else
      return false;
  }
  virtual char* Write (size_t &size) { return 0; }
  virtual csPtr<iNetworkPacket> New ()
    { return csPtr<iNetworkPacket> (new RecvPacket ()); }
  virtual bool FilterSocket (iNetworkConnection *s) { return true; }
};

SCF_IMPLEMENT_IBASE (RecvPacket)
  SCF_IMPLEMENTS_INTERFACE (iNetworkPacket)
SCF_IMPLEMENT_IBASE_END

bool HandleEvent (iEvent &ev)
{
  printf ("...received '%s'. :-)\n",
    ((RecvPacket *) ev.Network.Data)->GetData ());

  csRef<iEventQueue> eventq = CS_QUERY_REGISTRY (objreg, iEventQueue);
  eventq->GetEventOutlet ()->Broadcast (cscmdQuit);

  return true;
}

int main (int argc, char *argv[])
{
  objreg = csInitializer::CreateEnvironment (argc, argv);
  if (! objreg)
  {
    fprintf (stderr, "Failed to create environment!\n");
    return 1;
  }

  bool plugins_ok = csInitializer::RequestPlugins (objreg,
    CS_REQUEST_PLUGIN ("crystalspace.network.driver.sockets", iNetworkDriver),
    CS_REQUEST_PLUGIN ("crystalspace.network.manager.standard", iNetworkManager),
    CS_REQUEST_END);
  if (! plugins_ok)
  {
    fprintf (stderr, "Failed to load plugins!\n");
    return 2;
  }

  if (csCommandLineHelper::CheckHelp (objreg))
  {
    csCommandLineHelper::Help (objreg);
    return 0;
  }

  bool ev_ok = csInitializer::SetupEventHandler (objreg, HandleEvent, CSMASK_Network);
  if (! ev_ok)
  {
    fprintf (stderr, "Failed to set up event handler!\n");
    return 3;
  }

  {
    csRef<iNetworkDriver> driver = CS_QUERY_REGISTRY (objreg, iNetworkDriver);
    if (! driver)
    {
      fprintf (stderr, "Failed to find network driver plugin!\n");
      return 4;
    }

    csRef<iNetworkManager> netman = CS_QUERY_REGISTRY (objreg, iNetworkManager);
    if (! netman)
    {
      fprintf (stderr, "Failed to find network manager plugin!\n");
      return 5;
    }

    csInitializer::OpenApplication (objreg);

    RecvPacket recvpkt;
    csRef<iNetworkListener> listener = driver->NewListener (PORT "/" PROTO);
    if (! listener)
    {
      fprintf (stderr, "Failed to set up listening socket!\n");
      return 6;
    }
    netman->RegisterListener (listener, & recvpkt);

    SendPacket sendpkt;
    csRef<iNetworkConnection> sender = driver->NewConnection ("localhost:" PORT "/" PROTO);
    if (! sender)
    {
      fprintf (stderr, "Failed to connect socket!\n");
      return 7;
    }
    netman->RegisterConnection (sender, & sendpkt);

    printf ("Sending '%s' over loopback...\n", testdata);
    sendpkt.SetData (testdata);
    netman->Send (sender, & sendpkt);
    netman->UnregisterEndPoint (sender);

    csDefaultRunLoop (objreg);
  }

  csInitializer::CloseApplication (objreg);

  csInitializer::DestroyApplication (objreg);
  return 0;
}

