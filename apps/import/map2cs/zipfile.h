#ifndef __ZIPFILE_H__
#define __ZIPFILE_H__

#include "csutil/archive.h"

class CBinaryData;

/**
  * This class will add the ability to add external files to a
  * zip file, to the original Archive class from Crystal Space.
  */
class CZipFile : protected csArchive
{
public:
  /// The constructor
  CZipFile(const char* filename) : csArchive(filename) {}

  /// The destructo
  ~CZipFile(){}

  /// Adds the given file to the zip archive. using the new name
  bool AddFile(const char* originalname, const char* zipname);

  /// Adds the given Data to the zip archive. using the new name
  bool AddData(const char* Data, int Size, const char* zipname);

  /// Adds the given Data to the zip archive. using the new name
  bool AddData(CBinaryData* pData, const char* zipname);

  /// Extracts a File from a Zip Archive
  bool ExtractData(CBinaryData* pData, const char* zipname);

  /// Writes to Archive to disk.
  bool WriteArchive() {return Flush();}
};

#endif // __ZIPFILE_H__

