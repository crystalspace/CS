#include "cssysdef.h"
#include "mapstd.h"
#include "zipfile.h"
#include "bindata.h"

bool CZipFile::AddFile(const char* originalname, const char* zipname)
{
  FILE* fd = fopen(originalname, "rb");
  if (!fd)
  {
    printf("Can't find file '%s'!\n", originalname);
    return false;
  }

  //Check the size of the file.
  fseek(fd, 0, SEEK_END); //Seek to end of file
  int size = ftell(fd);   //Check current position
  fseek(fd, 0, SEEK_SET); //Seek to beginning of file

  void* file = NewFile(zipname, size);
  if (!file)
  {
    printf("Can't add new file '%s' to zip archive\n", originalname);
    return false;
  }

  char Buffer[1000];

  int totalread = 0;
  int numread   = 0;
  do
  {
    numread = fread(Buffer, 1, 1000, fd);
    if (!Write(file, Buffer, numread))
    {
      printf("Can't add data of file '%s' to zip archive\n", originalname);
      return false;
    }
    totalread += numread;
  }
  while(numread>0);

  assert(totalread==size);

  return true;
}

bool CZipFile::AddData(const char* Data, int Size, const char* zipname)
{
  assert(Data);
  assert(zipname);

  void* file = NewFile(zipname, Size);
  if (!file)
  {
    printf("Can't add new file '%s' to zip archive\n", zipname);
    return false;
  }

  if (!Write(file, Data, Size))
  {
    printf("Can't add data (%s) to zip archive\n", zipname);
    return false;
  }

  return true;
}

bool CZipFile::AddData(CBinaryData* pData, const char* zipname)
{
  assert(pData);
  assert(zipname);

  return AddData(pData->GetData(), pData->GetSize(), zipname);
}

bool CZipFile::ExtractData(CBinaryData* pData, const char* zipname)
{
  assert(pData);
  assert(zipname);

  size_t filesize = 0;
  if (FileExists (zipname, &filesize))
  {
    char* pZipData = Read (zipname);
    assert(pZipData);

    pData->SetData(pZipData, filesize);
    delete[] pZipData;

    return true;
  }

  return false;
}

