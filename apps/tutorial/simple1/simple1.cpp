#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csutil/hashmap.h"

#include <stdio.h>
#include <iostream>
#include <fstream>

#include <string>
#include <map>
#include <vector>
//#include <hash_map>

CS_IMPLEMENT_APPLICATION

int func (int a)
{
  return a;
}

int main (int argc, char* argv[])
{

#define TOTAL_ELEMENTS 1000000
#define RANDOM_ELEMENTS 100000

  //construct a list of 1000 randoms in range 1..90000
  int i = 0;
  srand ((unsigned) time (0));
  int numberlist [RANDOM_ELEMENTS+1];
  for (i = 0; i < RANDOM_ELEMENTS; ++i)
  {
    numberlist [i] = (int) TOTAL_ELEMENTS * rand() / (RAND_MAX + 1.0);
  }


  csHashKey key = csHashCompute ("abigkeyusedforhashing");
  csTicks start, end;
  int testdata;

//---------------------------------------------------------------------------
  std::cout << "Testing csArray\n";
  csArray<int> csArr;
  start = csGetTicks ();
  for (i = 0; i < TOTAL_ELEMENTS; ++i)
  {
    csArr.Push (i);
    //func (i);
  }
  end = csGetTicks ();
  std::cout << "Time to add " << i << " elements to csArray was " << end-start << " ms\n";
printf ("%d\n", csArr.Length ());

  std::cout << "Testing vector\n";
  std::vector<int> stlArr;
  start = csGetTicks ();
  for (i = 0; i < TOTAL_ELEMENTS; ++i)
  {
    stlArr.push_back (i);
  }
  end = csGetTicks ();
  std::cout << "Time to add " << i << " elements to vector was " << end-start << " ms\n";
//---------------------------------------------------------------------------

  std::cout << "Testing CS-hashmap\n";
  csHashMap csHash;
  start = csGetTicks ();
  for (i = 0; i < TOTAL_ELEMENTS; ++i)
  {
    csHash.Put ((key+i)%key, (void*)i);
  }
  end = csGetTicks ();
  std::cout << "Time to add " << i << " elements to hash was " << end-start << " ms\n";

  //iterate all elements
  start = csGetTicks ();
  csGlobalHashIterator cI (&csHash);
  while (cI.HasNext ())
  {
    testdata = (int) cI.Next ();
    func (testdata);
  }
  end = csGetTicks ();
  std::cout << "Time to traverse all elements was " << end-start << " ms\n";

  //get a non-sequensial access to 1000 elements
  start = csGetTicks ();
  for (i = 0; i < RANDOM_ELEMENTS; ++i)
  {
    testdata = (int) csHash.Get ( (key+numberlist[i])%key );
    func (testdata);
  }
  end = csGetTicks ();
  std::cout << "Time to get " << i << " random elements was " << end-start << " ms\n\n";

  //clean up
  csHash.DeleteAll ();

  std::cout << "Testing std::map of int,int\n";
  std::map< int, int> stdmap;
  start = csGetTicks ();
  for (i = 0; i < TOTAL_ELEMENTS; ++i)
  {
    stdmap[ (key+i)%key] = i;
  }
  end = csGetTicks ();
  std::cout << "Time to add " << i << " elements to hash was " << end-start << " ms\n";

  start = csGetTicks ();
  std::map<int, int>::const_iterator stdit;
  stdit = stdmap.begin ();
  for (; stdit != stdmap.end (); ++stdit)
  {
    testdata = stdit->second;
    func (testdata);
  }
  end = csGetTicks ();
  std::cout << "Time to traverse all elements was " << end-start << " ms\n";

  start = csGetTicks ();
  for (i = 0; i < RANDOM_ELEMENTS; ++i)
  {
    testdata = stdmap.find ( (key+numberlist[i])%key )->second;
    func (testdata);
  }
  end = csGetTicks ();
  std::cout << "Time to get " << i << " random elements was " << end-start <<  " ms\n";
  

#if 0
  std::cout << "Testing std::hash_map of int,int\n";
  std::hash_map<int, int> stdhash;

  start = csGetTicks ();
  for (i = 0; i < TOTAL_ELEMENTS; ++i)
  {
    stdhash[ (key+i)%key] = i;
  }
  end = csGetTicks ();
  std::cout << "Time to add " << i << " elements to hash was " << end-start << " ms\n";

  start = csGetTicks ();
  std::hash_map<int, int>::const_iterator stdit2;
  stdit2 = stdhash.begin ();
  for (; stdit2 != stdhash.end (); ++stdit2)
  {
    testdata = stdit2->second;
    mystream << testdata;
  }
  mystream << "\n";
  end = csGetTicks ();
  std::cout << "Time to traverse all elements was " << end-start << " ms\n";

  start = csGetTicks ();
  for (i = 0; i < RANDOM_ELEMENTS; ++i)
  {
    testdata = stdhash.find ( (key+numberlist[i])%key )->second;
    mystream << testdata;
  }
  mystream << "\n";
  end = csGetTicks ();
  std::cout << "Time to get " << i << " random elements was " << end-start <<  " ms\n\n";
  std::cout << "bucketcount: " << stdhash.bucket_size;
#endif

  return 0;
}
