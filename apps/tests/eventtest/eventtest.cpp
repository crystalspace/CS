/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#include <stdarg.h>

#include "cssysdef.h"
#include "iengine/engine.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "cstool/initapp.h"
#include "iutil/eventq.h"
#include "csutil/cseventq.h"
#include "csutil/sysfunc.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

static bool run_pool_tests(iObjectRegistry* object_reg)
{
  csTicks begin, end;
  csRef<iEvent> ten[10], hundred[100], thousand[1000], lots[100000];

  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (!q) 
  {
    printf ("EventQueue not loaded, Zoinks!\n");
    return false;
  }
  csEventQueue *eq = CS_STATIC_CAST(csEventQueue *, (iEventQueue *)q);

  fprintf (stdout, "Event Pool count: %d\n", eq->CountPool());
  fprintf (stdout, "Allocating 10 events...  ");
  begin = csGetTicks();
  int i;
  for (i = 0; i < 10; i++)
  {
    ten[i] = q->CreateEvent(1);
  }
  end = csGetTicks();
  fprintf (stdout, "%d ms\n\n", end - begin);

  fprintf (stdout, "Event Pool count: %d\n", eq->CountPool());
  fprintf (stdout, "Allocating 100 events...  ");
  begin = csGetTicks();
  for (i = 0; i < 100; i++)
  {
    hundred[i] = q->CreateEvent(1);
  }
  end = csGetTicks();
  fprintf (stdout, "%d ms\n\n", end - begin);

  fprintf (stdout, "Event Pool count: %d\n", eq->CountPool());
  fprintf (stdout, "Allocating 1000 events...  ");
  begin = csGetTicks();
  for (i = 0; i < 1000; i += 10)
  {
    thousand[i] = q->CreateEvent(1);
    thousand[i+1] = q->CreateEvent(1);
    thousand[i+2] = q->CreateEvent(1);
    thousand[i+3] = q->CreateEvent(1);
    thousand[i+4] = q->CreateEvent(1);
    thousand[i+5] = q->CreateEvent(1);
    thousand[i+6] = q->CreateEvent(1);
    thousand[i+7] = q->CreateEvent(1);
    thousand[i+8] = q->CreateEvent(1);
    thousand[i+9] = q->CreateEvent(1);
  }
  end = csGetTicks();
  fprintf (stdout, "%d ms\n\n", end - begin);

  fprintf (stdout, "Event Pool count: %d\n", eq->CountPool());
  fprintf (stdout, "Allocating 10000 events...  ");
  begin = csGetTicks();
  // On my machine, this unrolled loop is 10x faster then doing one at a time
  for (i = 0; i < 10000; i += 10)
  {
    lots[i] = q->CreateEvent(1);
    lots[i+1] = q->CreateEvent(1);
    lots[i+2] = q->CreateEvent(1);
    lots[i+3] = q->CreateEvent(1);
    lots[i+4] = q->CreateEvent(1);
    lots[i+5] = q->CreateEvent(1);
    lots[i+6] = q->CreateEvent(1);
    lots[i+7] = q->CreateEvent(1);
    lots[i+8] = q->CreateEvent(1);
    lots[i+9] = q->CreateEvent(1);
  }
  end = csGetTicks();
  fprintf (stdout, "%d ms\n\n", end - begin);

  return true;
}

static bool compare_buffers(const char *b1, const char *b2, uint32 length)
{
  for (uint32 i = 0; i < length; i++)
  {
    if (b1[i] != b2[i])
      return false;
  } 
  return true;
}

static bool run_event_tests(iObjectRegistry* object_reg)
{
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (!q) 
  {
    fprintf (stdout, "EventQueue not loaded, Zoinks!\n");
    return false;
  }

  csRef<iEvent> e = q->CreateEvent(1);
  fprintf (stdout, "Ok, now it's time to test the events themselves:\n");
  
  fprintf (stdout, "Adding an my_int8... %s\n", e->Add("my_int8", (int8)1) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_uint8... %s\n", e->Add("my_uint8", (uint8)1) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_int16... %s\n", e->Add("my_int16", (int16)1) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_uint16... %s\n", e->Add("my_uint16", (uint16)1) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_int32... %s\n", e->Add("my_int32", (int32)1) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_uint32... %s\n", e->Add("my_uint32", (uint32)1) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_int64... %s\n", e->Add("my_int64", (int64)1) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_uint64... %s\n", e->Add("my_uint64", (uint64)1) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_float... %s\n", e->Add("my_float", (float)1.0) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_double... %s\n", e->Add("my_double", (double)1.0) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_bool... %s\n", e->Add("my_bool", (bool)true) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding an my_string... %s\n", e->Add("my_string", "Hello World!") ? "Success!" : "Failure!");
  int8 my_int8 = 0;
  fprintf (stdout, "Searching for my_int8...%s", e->Find("my_int8", my_int8) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %d)\n", my_int8);
  uint8 my_uint8 = 0;
  fprintf (stdout, "Searching for my_uint8...%s", e->Find("my_uint8", my_uint8) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %d)\n", my_uint8);
  int16 my_int16 = 0;
  fprintf (stdout, "Searching for my_int16...%s", e->Find("my_int16", my_int16) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %d)\n", my_int16);
  uint16 my_uint16 = 0;
  fprintf (stdout, "Searching for my_uint16...%s", e->Find("my_uint16", my_uint16) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %d)\n", my_uint16);
  int32 my_int32 = 0;
  fprintf (stdout, "Searching for my_int32...%s", e->Find("my_int32", my_int32) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %d)\n", my_int32);
  uint32 my_uint32 = 0;
  fprintf (stdout, "Searching for my_uint32...%s", e->Find("my_uint32", my_uint32) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %u)\n", my_uint32);
  int64 my_int64 = 0;
  fprintf (stdout, "Searching for my_int64...%s", e->Find("my_int64", my_int64) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %lld)\n", my_int64);
  uint64 my_uint64 = 0;
  fprintf (stdout, "Searching for my_uint64...%s", e->Find("my_uint64", my_uint64) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %llu)\n", my_uint64);
  float my_float= 0;
  fprintf (stdout, "Searching for my_float...%s", e->Find("my_float", my_float) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %f)\n", my_float);
  double my_double = 0;
  fprintf (stdout, "Searching for my_double...%s", e->Find("my_double", my_double) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %f)\n", my_double);
  bool my_bool = false;
  fprintf (stdout, "Searching for my_bool...%s", e->Find("my_bool", my_bool) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %s)\n", my_bool ? "true" : "false");
  const char *my_string = 0;
  fprintf (stdout, "Searching for my_string...%s", e->Find("my_string", my_string) ? "Success!" : "Failure!");
  fprintf (stdout, "  (value: %s)\n", my_string);
  char buffer[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
  fprintf (stdout, "Adding a databuffer 32 in length called my_databuffer... %s\n", e->Add("my_databuffer", (const void*)buffer, 32) ? "Success!" : "Failure!");
  const void* my_data = 0;
  uint32 l;
  bool found = false;
  fprintf (stdout, "Searching for my_databuffer...%s\n", (found = e->Find("my_databuffer", my_data, l, 0)) ? "Success!" : "Failure!");
  if (found)
    fprintf (stdout, "  Comparing buffers...%s\n", compare_buffers((const char*)my_data, (const char*)buffer, 32) ? "Perfect!" : "Not the same!");
    
  fprintf (stdout, "Adding an this event to itself... %s\n", e->Add("my_event", e) ? "Success!" : "Failure!");
  fprintf (stdout, "  That should have failed.\n");
  csRef<iEvent> f = q->CreateEvent(1);
  fprintf (stdout, "Adding a new event as my_event... %s\n", e->Add("my_event", f) ? "Success!" : "Failure!");
  fprintf (stdout, "Adding the parent event to the child event... %s\n", f->Add("my_event", e) ? "Success!" : "Failure!");
  fprintf (stdout, "  That should have failed.\n");
  uint32 size = e->FlattenSize();
  char *serialized_buffer = new char[size];
  fprintf (stdout, "Serializing the event to a buffer...%s\n", e->Flatten(serialized_buffer) ? "Success!" : "Failure");
  FILE *log = fopen("test.bin", "wb");
  fwrite(serialized_buffer,size, 1, log);
  fclose(log);
  csRef<iEvent> a = q->CreateEvent(1);
  fprintf (stdout, "Recreating event from the buffer...%s\n", a->Unflatten(serialized_buffer, size) ? "Success!" : "Failure");
  fprintf (stdout, "Printing the contents of the new event out:\n");
  a->Print();
  
  return true;
}


static bool run_tests (iObjectRegistry* object_reg)
{
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (!q) 
  {
    fprintf (stdout, "EventQueue not loaded, Zoinks!\n");
    return false;
  }
  fprintf (stdout, "\nEvent Pool test\nby Jonathan Tarbox\n\n");

  fprintf (stdout, "This will allocate 10, 100, 1000, and 10000 events.\n");
  fprintf (stdout, "As these are the initial allocations, they will be\n");
  fprintf (stdout, "allocating memory each time, thus a little slow.\n\n");

  if (!run_pool_tests(object_reg)) return false;
  
  fprintf (stdout, "\nOk, now that we've left the scope of the last test\n");
  fprintf (stdout, "all the events should have DecRef'd and should be sitting\n");
  fprintf (stdout, "in the pool.  Let's try the same test again and see how\n");
  fprintf (stdout, "much faster this is.\n\n");

  if (!run_pool_tests(object_reg)) return false;
  
  if (!run_event_tests(object_reg)) return false;

  fprintf (stdout, "\n\nTests complete.. if it hasn't crashed yet, that's a good thing!\n");
  return true;

}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (0));

  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg)
    return -1;
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_END))
  {
    csInitializer::DestroyApplication (object_reg);
    return -1;
  }

  run_tests (object_reg);

  csInitializer::DestroyApplication (object_reg);
  return 0;
}
