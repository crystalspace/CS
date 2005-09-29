/*

  CodeBook.h - Codebook class for VeeQue

*/

#ifndef CODEBOOK_H
#define CODEBOOK_H

#include "common.h"
#include "Table.h"

namespace ImageLib 
{

const long CodeSize = 4;
const long CodeDWORDSize = CodeSize/4;
const long HashSize = 2048;

class Image32;
class fCodebook;
class Color;


class cbVector
{
private:
  BYTE  pData[CodeSize];

public:
  cbVector() {;}
  ~cbVector() {;}

  inline BYTE *GetPtr(void) {return pData;}

  inline BYTE &operator[](int i) {return pData[i];}
  cbVector &operator=(cbVector &Vect);
  int operator==(cbVector &Vect);

  void Min(cbVector &Test1);
  void Max(cbVector &Test1);
  void MinMax(cbVector &Min, cbVector &Max);

  long Sum(void);      // Summation of the vector componants
  long Mag(void);      // Magnitude of the vector
  long InvMag(void);    // Magnitude of the vector ^ ffffffffff...

  void Diff(cbVector &Test1, cbVector &Test2);
  // Magnitude of the difference between this and Vect (Dist ^ 2)
  long DiffMag(cbVector &Vect);

  friend class CodeBook;
  friend class ImgCodeBook;
};


typedef struct
{
  long  Origin;
  long  AntiOrigin;
} DualDist;


class CodeBook
{
private:
  Table<cbVector>    VectList;
  Table<long>      usageCount;

public:
  virtual ~CodeBook() {;}

  void AddVector(cbVector &Vect);
  long FindVectorSlow(cbVector &Vect);
  long FindVectorSlow(cbVector &Vect, long &Error);
  long ClosestError(cbVector &Vect);
  long ClosestError(long UseCount, cbVector &Vect);

  CodeBook &operator=(fCodebook &Src);
  cbVector &operator[](int i) {return VectList[i];}
  long UsageCount(int i) {return usageCount[i];}

  inline long NumCodes(void) {return VectList.Count();}
  inline void SetSize(long Size)
  {VectList.Resize(Size); usageCount.Resize(Size);}
  inline void SetCount(long Count)
  {VectList.SetCount(Count); usageCount.SetCount(Count);}

  friend class ImgCodeBook;
  friend class fCodebook;
};


class ImgCodeBook : public CodeBook
{
private:
  Table<long>      HashValues;
  Table<long>      HashList[HashSize];
  Table<DualDist>    DistList;
  Table<long>      BrightList;

  void SortCodes(void);

public:
  ImgCodeBook();
  ~ImgCodeBook();

  inline void SetSize(long Size)
  {VectList.Resize(Size); usageCount.Resize(Size); HashValues.Resize(Size);}
  void ReleaseAll(void);

  ImgCodeBook &operator=(fCodebook &Src)
  {CodeBook::operator=(Src); return *this;}

  void AddVector(cbVector &Vect);
  void AddVectorUnique(cbVector &Vect) {AddVector(Vect);}

  void FromImage(Image32 *pImg, Color* pForceColor = 0);
  void FromImageUnique(Image32 *pImg, Color* pForceColor = 0);

  void GenerateDistanceTables(void);
  long FindVector(cbVector &Vect);

  friend class fCodebook;
};

} // end of namespace ImageLib

#endif
