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
#include "inetwork/driver2.h"
#include "inetwork/socket2.h"
#include "inetwork/sockerr.h"
#include "inetwork/netman.h"
#include "csutil/datastrm.h"

#include <stdio.h>
#include <string.h>

CS_IMPLEMENT_APPLICATION

iObjectRegistry *objreg;

char testdata [] = "Hello, world!";

class SendPacket : public iNetworkPacket
{
  char *data;

  public:
  SCF_DECLARE_IBASE;
  SendPacket ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
  }
  virtual ~SendPacket () {}

  void SetData (char d []) { data = d; }

  virtual bool Read (csDataStream &stream) { return false; }
  virtual char* Write (size_t &size)
  {
    size = strlen (data);
    return data;
  }
  virtual iNetworkPacket* New () { return new SendPacket (); }
  virtual bool FilterSocket (iNetworkSocket2 *s) { return true; }
};

SCF_IMPLEMENT_IBASE (SendPacket)
  SCF_IMPLEMENTS_INTERFACE (iNetworkPacket)
SCF_IMPLEMENT_IBASE_END

class RecvPacket : public iNetworkPacket
{
  char data [256];
  unsigned position;

  public:
  SCF_DECLARE_IBASE;
  RecvPacket ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
    position = 0;
  }
  virtual ~RecvPacket () {}

  const char* GetData () { return data; }

  virtual bool Read (csDataStream &stream)
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
  virtual char* Write (size_t &size) { return NULL; }
  virtual iNetworkPacket* New () { return new RecvPacket (); }
  virtual bool FilterSocket (iNetworkSocket2 *s) { return true; }
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

int main (int argc, char *argv[]/*, char *env[]*/)
{
  objreg = csInitializer::CreateEnvironment (argc, argv);
  if (! objreg)
  {
    fprintf (stderr, "Failed to create environment!\n");
    return 1;
  }

  bool plugins_ok = csInitializer::RequestPlugins (objreg,
    CS_REQUEST_PLUGIN ("crystalspace.network.driver.sockets2", iNetworkDriver2),
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
    csRef<iNetworkDriver2> driver = CS_QUERY_REGISTRY (objreg, iNetworkDriver2);
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
    csRef<iNetworkSocket2> listener = driver->CreateSocket (CS_NET_SOCKET_TYPE_UDP);
    listener->SetSocketBlock (false);
    listener->WaitForConnection (0, 1234, 1);
    if (listener->LastError () != CS_NET_SOCKET_NOERROR)
    {
      fprintf (stderr, "Failed to set up listening socket!\n");
      return 6;
    }
    netman->RegisterListeningSocket (listener, & recvpkt);

    SendPacket sendpkt;
    csRef<iNetworkSocket2> sender = driver->CreateSocket (CS_NET_SOCKET_TYPE_UDP);
    sender->SetSocketBlock (false);
    sender->Connect ("127.0.0.1", 1234);
    if (sender->LastError () != CS_NET_SOCKET_NOERROR)
    {
      fprintf (stderr, "Failed to connect socket!\n");
      return 7;
    }
    netman->RegisterConnectedSocket (sender, & sendpkt);

    printf ("Sending '%s' over loopback...\n", testdata);
    sendpkt.SetData (testdata);
    netman->Send (sender, & sendpkt);

    csDefaultRunLoop (objreg);
  }

  csInitializer::CloseApplication (objreg);

  csInitializer::DestroyApplication (objreg);
  return 0;
}

