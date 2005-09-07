/*
  DbgHelp API stuff.
  Unfortunately, dbghelp.h isn't included in MinGW. So all that's needed goes here.
 */

#ifndef __CSUTIL_DEBGHELPAPI_H__
#define __CSUTIL_DEBGHELPAPI_H__

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

struct SYMBOL_INFOW
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
  WCHAR Name[1];
};
typedef SYMBOL_INFOW* PSYMBOL_INFOW;

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
  CHAR LoadedPdbName[256];  
  DWORD CVSig;  
  CHAR CVData[MAX_PATH*3];  
  DWORD PdbSig;  
  GUID PdbSig70;  
  DWORD PdbAge;  
  BOOL PdbUnmatched;  
  BOOL DbgUnmatched;  
  BOOL LineNumbers;  
  BOOL GlobalSymbols;  
  BOOL TypeInfo;
};
typedef IMAGEHLP_MODULE64* PIMAGEHLP_MODULE64;

struct IMAGEHLP_MODULEW64
{  
  DWORD SizeOfStruct;  
  uint64 BaseOfImage;  
  DWORD ImageSize;  
  DWORD TimeDateStamp;  
  DWORD CheckSum;  
  DWORD NumSyms;  
  SYM_TYPE SymType;  
  WCHAR ModuleName[32];  
  WCHAR ImageName[256];  
  WCHAR LoadedImageName[256];
  WCHAR LoadedPdbName[256];  
  DWORD CVSig;  
  WCHAR CVData[MAX_PATH*3];  
  DWORD PdbSig;  
  GUID PdbSig70;  
  DWORD PdbAge;  
  BOOL PdbUnmatched;  
  BOOL DbgUnmatched;  
  BOOL LineNumbers;  
  BOOL GlobalSymbols;  
  BOOL TypeInfo;
};
typedef IMAGEHLP_MODULEW64* PIMAGEHLP_MODULEW64;

struct IMAGEHLP_LINE64
{  
  DWORD SizeOfStruct;  
  PVOID Key;  
  DWORD LineNumber;  
  PCHAR FileName;  
  uint64 Address;
};
typedef IMAGEHLP_LINE64* PIMAGEHLP_LINE64;

struct IMAGEHLP_LINE64W
{  
  DWORD SizeOfStruct;  
  PVOID Key;  
  DWORD LineNumber;  
  PWCHAR FileName;  
  uint64 Address;
};
typedef IMAGEHLP_LINE64W* PIMAGEHLP_LINE64W;

typedef BOOL (CALLBACK* PSYM_ENUMERATESYMBOLS_CALLBACK) (PSYMBOL_INFO pSymInfo,
  ULONG SymbolSize, PVOID UserContext);
typedef BOOL (CALLBACK* PSYM_ENUMERATESYMBOLS_CALLBACKW) (PSYMBOL_INFOW pSymInfo,
  ULONG SymbolSize, PVOID UserContext);
typedef BOOL (CALLBACK* PSYM_ENUMMODULES_CALLBACK64) (PSTR ModuleName,
  uint64 BaseOfDll, PVOID UserContext);
typedef BOOL (CALLBACK* PSYM_ENUMMODULES_CALLBACKW64) (PWSTR ModuleName,
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

struct MINIDUMP_EXCEPTION_INFORMATION 
{  
  DWORD ThreadId;  
  PEXCEPTION_POINTERS ExceptionPointers;  
  BOOL ClientPointers;
};
typedef MINIDUMP_EXCEPTION_INFORMATION* PMINIDUMP_EXCEPTION_INFORMATION;

struct MINIDUMP_USER_STREAM 
{  
  ULONG32 Type;  
  ULONG BufferSize;  
  PVOID Buffer;
};
typedef MINIDUMP_USER_STREAM* PMINIDUMP_USER_STREAM;
                                                    
struct MINIDUMP_USER_STREAM_INFORMATION 
{  
  ULONG UserStreamCount;  
  PMINIDUMP_USER_STREAM UserStreamArray;
};
typedef MINIDUMP_USER_STREAM_INFORMATION* PMINIDUMP_USER_STREAM_INFORMATION;

enum MINIDUMP_CALLBACK_TYPE
{
  ModuleCallback, 
  ThreadCallback, 
  ThreadExCallback, 
  IncludeThreadCallback, 
  IncludeModuleCallback
};

struct MINIDUMP_THREAD_CALLBACK 
{  
  ULONG ThreadId;  
  HANDLE ThreadHandle;  
  CONTEXT Context;  
  ULONG SizeOfContext;  
  ULONG64 StackBase;  
  ULONG64 StackEnd;
};
typedef MINIDUMP_THREAD_CALLBACK* PMINIDUMP_THREAD_CALLBACK;

struct MINIDUMP_THREAD_EX_CALLBACK 
{  
  ULONG ThreadId;  
  HANDLE ThreadHandle;  
  CONTEXT Context;  
  ULONG SizeOfContext;  
  ULONG64 StackBase;  
  ULONG64 StackEnd;  
  ULONG64 BackingStoreBase;  
  ULONG64 BackingStoreEnd;
};
typedef MINIDUMP_THREAD_EX_CALLBACK* PMINIDUMP_THREAD_EX_CALLBACK;

#include <winver.h>
#include "sanity.inc"

struct MINIDUMP_MODULE_CALLBACK 
{  
  PWCHAR FullPath;  
  ULONG64 BaseOfImage;  
  ULONG SizeOfImage;  
  ULONG CheckSum;  
  ULONG TimeDateStamp;  
  VS_FIXEDFILEINFO VersionInfo;  
  PVOID CvRecord;  
  ULONG SizeOfCvRecord;  
  PVOID MiscRecord;  
  ULONG SizeOfMiscRecord;
};
typedef MINIDUMP_MODULE_CALLBACK* PMINIDUMP_MODULE_CALLBACK;

struct MINIDUMP_INCLUDE_THREAD_CALLBACK 
{  
  ULONG ThreadId;
};
typedef MINIDUMP_INCLUDE_THREAD_CALLBACK* PMINIDUMP_INCLUDE_THREAD_CALLBACK;

struct MINIDUMP_INCLUDE_MODULE_CALLBACK 
{  
  ULONG64 BaseOfImage;
};
typedef MINIDUMP_INCLUDE_MODULE_CALLBACK* PMINIDUMP_INCLUDE_MODULE_CALLBACK;


struct MINIDUMP_CALLBACK_INPUT	     
{  
  ULONG ProcessId;  
  HANDLE ProcessHandle;  
  ULONG CallbackType;  
  union 
  {    
    MINIDUMP_THREAD_CALLBACK Thread;    
    MINIDUMP_THREAD_EX_CALLBACK ThreadEx;    
    MINIDUMP_MODULE_CALLBACK Module;    
    MINIDUMP_INCLUDE_THREAD_CALLBACK IncludeThread;    
    MINIDUMP_INCLUDE_MODULE_CALLBACK IncludeModule;  
  };
};
typedef MINIDUMP_CALLBACK_INPUT* PMINIDUMP_CALLBACK_INPUT;

enum MODULE_WRITE_FLAGS
{
  ModuleWriteModule 			= 0x0001, 
  ModuleWriteDataSeg 			= 0x0002, 
  ModuleWriteMiscRecord 		= 0x0004, 
  ModuleWriteCvRecord 			= 0x0008, 
  ModuleReferencedByMemory 		= 0x0010
};

enum THREAD_WRITE_FLAGS
{
  ThreadWriteThread 			= 0x0001, 
  ThreadWriteStack 			= 0x0002, 
  ThreadWriteContext 			= 0x0004, 
  ThreadWriteBackingStore 		= 0x0008, 
  ThreadWriteInstructionWindow 		= 0x0010, 
  ThreadWriteThreadData 		= 0x0020
};

struct MINIDUMP_CALLBACK_OUTPUT 
{  
  union 
  {    
    ULONG ModuleWriteFlags;    
    ULONG ThreadWriteFlags;  
  };
}; 
typedef MINIDUMP_CALLBACK_OUTPUT* PMINIDUMP_CALLBACK_OUTPUT;


typedef BOOL (CALLBACK* MINIDUMP_CALLBACK_ROUTINE) (PVOID CallbackParam, 
  const PMINIDUMP_CALLBACK_INPUT CallbackInput, 
  PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);

struct MINIDUMP_CALLBACK_INFORMATION 
{  
  MINIDUMP_CALLBACK_ROUTINE CallbackRoutine;  
  PVOID CallbackParam;
};
typedef MINIDUMP_CALLBACK_INFORMATION* PMINIDUMP_CALLBACK_INFORMATION;

enum MINIDUMP_TYPE
{
  MiniDumpNormal			  = 0x0000, 
  MiniDumpWithDataSegs			  = 0x0001, 
  MiniDumpWithFullMemory		  = 0x0002, 
  MiniDumpWithHandleData		  = 0x0004, 
  MiniDumpFilterMemory			  = 0x0008, 
  MiniDumpScanMemory			  = 0x0010, 
  MiniDumpWithUnloadedModules		  = 0x0020, 
  MiniDumpWithIndirectlyReferencedMemory  = 0x0040, 
  MiniDumpFilterModulePaths		  = 0x0080, 
  MiniDumpWithProcessThreadData		  = 0x0100, 
  MiniDumpWithPrivateReadWriteMemory	  = 0x0200
};

enum MINIDUMP_STREAM_TYPE
{
  UnusedStream		   = 0, 
  ReservedStream0	   = 1, 
  ReservedStream1	   = 2, 
  ThreadListStream	   = 3, 
  ModuleListStream	   = 4, 
  MemoryListStream	   = 5, 
  ExceptionStream	   = 6, 
  SystemInfoStream	   = 7, 
  ThreadExListStream	   = 8, 
  Memory64ListStream	   = 9, 
  CommentStreamA	   = 10, 
  CommentStreamW	   = 11, 
  HandleDataStream	   = 12, 
  FunctionTableStream	   = 13, 
  UnloadedModuleListStream = 14, 
  MiscInfoStream	   = 15, 
  LastReservedStream = 0xffff
};

struct MINIDUMP_LOCATION_DESCRIPTOR 
{  
  uint32 DataSize;  
  uint32 Rva;
};

struct MINIDUMP_MEMORY_DESCRIPTOR 
{  
  uint64 StartOfMemoryRange;  
  MINIDUMP_LOCATION_DESCRIPTOR Memory;
};
typedef MINIDUMP_MEMORY_DESCRIPTOR* PMINIDUMP_MEMORY_DESCRIPTOR;

struct MINIDUMP_MEMORY_LIST 
{  
  ULONG32 NumberOfMemoryRanges;  
  /*MINIDUMP_MEMORY_DESCRIPTOR MemoryRanges[];*/
};
typedef MINIDUMP_MEMORY_LIST* PMINIDUMP_MEMORY_LIST;

struct MINIDUMP_HEADER 
{
  ULONG32 Signature;
  ULONG32 Version;
  ULONG32 NumberOfStreams;
  uint32 StreamDirectoryRva;
  ULONG32 CheckSum;
  union 
  {
    ULONG32 Reserved;
    ULONG32 TimeDateStamp;
  };
  uint64 Flags;
};
typedef MINIDUMP_HEADER* PMINIDUMP_HEADER;

struct MINIDUMP_DIRECTORY 
{
  ULONG32 StreamType;
  MINIDUMP_LOCATION_DESCRIPTOR Location;
};
typedef MINIDUMP_DIRECTORY* PMINIDUMP_DIRECTORY;

#define CS_API_NAME		DbgHelp
#define CS_API_FUNCTIONS	"csutil/win32/DbgHelpAPI.fun"

#include "APIdeclare.inc"

#endif //__CSUTIL_DEBGHELPAPI_H__
