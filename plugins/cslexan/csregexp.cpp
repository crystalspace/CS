#include <string.h>
#include "cslex.h"

csRegExp::csRegExp():code(0), length(0) {};
csRegExp::~csRegExp() { if (code) delete [] code; }

bool 
csRegExp::GetOp(unsigned index, unsigned char &op)
{
 if (index<length)
 {
   op = code[index];
   return true;
 }
 else
 {
   return false;
 }
}

bool
csRegExp::SetOp(unsigned index, unsigned char op)
{
 if (index>=length && index < 0xfff)
 {
  unsigned char *newcode = new unsigned char[index+16];
  
  memset(newcode, 0, index+16);
  
  if (code)
  {
    memcpy(newcode, code, length);
    delete [] code;
  }
  
  code = newcode;
  length=index+16;
 }
 else if (index > 0xfff)
   return false;
 
 code[index] = op;

 return true;
}

bool 
csRegExp::Compact()
{

 for(unsigned int i=0; i<length; ++i)
 {
   if (code[i]==OP_END)
   {
    unsigned char *newcode = new unsigned char[i+1];
    
    memcpy(newcode, code, i+1);
    
    delete [] code;
    
    code = newcode;
    
    length = i+1;
    
    return true;
   }
 }
 
 // fail if we can't find the OP_END
 return false;
}

