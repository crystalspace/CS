/*
  DbgHelp API stuff.
  Unfortunately, dbghelp.h isn't included in MinGW. So all that's needed goes here.
 */

enum ADDRESS_MODE
{
  AddrMode1616,
  AddrMode1632,
  AddrModeReal,
  AddrModeFlat
};

struct ADDRESS64 
{
  uint64 Offset;
  WORD Segment;
  ADDRESS_MODE Mode;
};
typedef ADDRESS64* LPADDRESS64;

struct KDHELP64 
{  
  uint64 Thread;  
  DWORD ThCallbackStack;  
  DWORD ThCallbackBStore;  
  DWORD NextCallback;  
  DWORD FramePointer;  
  uint64 KiCallUserMode;  
  uint64 KeUserCallbackDispatcher;  
  uint64 SystemRangeStart;  
  uint64 Reserved[8];
};
typedef KDHELP64* PKDHELP64;

struct STACKFRAME64 
{  
  ADDRESS64 AddrPC;  
  ADDRESS64 AddrReturn;  
  ADDRESS64 AddrFrame;  
  ADDRESS64 AddrStack;  
  ADDRESS64 AddrBStore;  
  PVOID FuncTableEntry;  
  uint64 Params[4];  
  BOOL Far;  
  BOOL Virtual;  
  uint64 Reserved[3];  
  KDHELP64 KdHelp;
};
typedef STACKFRAME64* LPSTACKFRAME64;

typedef BOOL (WINAPI* PREAD_PROCESS_MEMORY_ROUTINE64) (HANDLE hProcess,
  uint64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead);
typedef PVOID (WINAPI* PFUNCTION_TABLE_ACCESS_ROUTINE64) (HANDLE hProcess,
  uint64 AddrBase);
typedef uint64 (WINAPI* PGET_MODULE_BASE_ROUTINE64) (HANDLE  hProcess, 
  uint64 Address);
typedef uint64 (WINAPI* PTRANSLATE_ADDRESS_ROUTINE64) (HANDLE hProcess,
  HANDLE hThread, LPADDRESS64 lpaddr);

struct SYMBOL_INFO 
{  
  ULONG SizeOfStruct;  
  ULONG TypeIndex;  
  uint64 Reserved[2];  
  ULONG Reserved2;  
  ULONG Size;  
  uint64 ModBase;  
  ULONG Flags;  
  uint64 Value;  
  uint64 Address;  
  ULONG Register;  
  ULONG Scope;  
  ULONG Tag;  
  ULONG NameLen;  
  ULONG MaxNameLen;  
  CHAR Name[1];
};
typedef SYMBOL_INFO* PSYMBOL_INFO;

#define SYMFLAG_PARAMETER        0x00000040

#define SYMOPT_UNDNAME                  0x00000002
#define SYMOPT_DEFERRED_LOADS           0x00000004
#define SYMOPT_LOAD_LINES               0x00000010
#define SYMOPT_FAIL_CRITICAL_ERRORS     0x00000200

enum SYM_TYPE
{
    SymNone = 0,
    SymCoff,
    SymCv,
    SymPdb,
    SymExport,
    SymDeferred,
    SymSym,
    SymDia,
    SymVirtual,
    NumSymTypes
};

struct IMAGEHLP_MODULE64 
{  
  DWORD SizeOfStruct;  
  uint64 BaseOfImage;  
  DWORD ImageSize;  
  DWORD TimeDateStamp;  
  DWORD CheckSum;  
  DWORD NumSyms;  
  SYM_TYPE SymType;  
  CHAR ModuleName[32];  
  CHAR ImageName[256];  
  CHAR LoadedImageName[256];  
  /*
    The following fields are only supported on newer versions of dbghelp.dll,
    but the versions shipped with W2k resp. WXP lack them.
   */
  /*CHAR LoadedPdbName[256];  
  DWORD CVSig;  
  CHAR CVData[MAX_PATH*3];  
  DWORD PdbSig;  
  GUID PdbSig70;  
  DWORD PdbAge;  
  BOOL PdbUnmatched;  
  BOOL DbgUnmatched;  
  BOOL LineNumbers;  
  BOOL GlobalSymbols;  
  BOOL TypeInfo;*/
};
typedef IMAGEHLP_MODULE64* PIMAGEHLP_MODULE64;

struct IMAGEHLP_LINE64
{  
  DWORD SizeOfStruct;  
  PVOID Key;  
  DWORD LineNumber;  
  PCHAR FileName;  
  uint64 Address;
};
typedef IMAGEHLP_LINE64* PIMAGEHLP_LINE64;

typedef BOOL (CALLBACK* PSYM_ENUMERATESYMBOLS_CALLBACK) (PSYMBOL_INFO pSymInfo,
  ULONG SymbolSize, PVOID UserContext);
typedef BOOL (CALLBACK* PSYM_ENUMMODULES_CALLBACK64) (PSTR ModuleName,
  uint64 BaseOfDll, PVOID UserContext);

struct IMAGEHLP_STACK_FRAME 
{  
  uint64 InstructionOffset;  
  uint64 ReturnOffset;  
  uint64 FrameOffset;  
  uint64 StackOffset;  
  uint64 BackingStoreOffset;  
  uint64 FuncTableEntry;  
  uint64 Params[4];  
  uint64 Reserved[5];  
  BOOL Virtual;  
  ULONG Reserved2;
};
typedef IMAGEHLP_STACK_FRAME* PIMAGEHLP_STACK_FRAME;
typedef void* PIMAGEHLP_CONTEXT;

#define CS_API_NAME		DbgHelp
#define CS_API_FUNCTIONS	"include/csutil/win32/DbgHelpAPI.fun"

#include "APIdeclare.inc"
