FUNC_GROUP_BEGIN(SymSupport)
  FUNC(BOOL, StackWalk64, (DWORD MachineType, HANDLE hProcess, 
    HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord,  
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,  
    PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,  
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,  
    PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress))
  FUNC(BOOL, SymInitialize, (HANDLE hProcess, PSTR UserSearchPath,
    BOOL fInvadeProcess))
  FUNC(BOOL, SymCleanup, (HANDLE hProcess))
  FUNC(DWORD, SymSetOptions, (DWORD SymOptions))
  FUNC(BOOL, SymFromAddr, (HANDLE hProcess, uint64 Address, 
    uint64* Displacement, PSYMBOL_INFO Symbol))
  FUNC(PVOID, SymFunctionTableAccess64, (HANDLE hProcess, 
    uint64 AddrBase))
  FUNC(uint64, SymGetModuleBase64, (HANDLE hProcess, uint64 dwAddr))
  FUNC(BOOL, SymGetModuleInfo64, (HANDLE hProcess, uint64 dwAddr,
    PIMAGEHLP_MODULE64 ModuleInfo))
  FUNC(BOOL, SymGetModuleInfoW64, (HANDLE hProcess, uint64 dwAddr,
    PIMAGEHLP_MODULEW64 ModuleInfo))
  FUNC(BOOL, SymGetLineFromAddr64, (HANDLE hProcess, uint64 dwAddr,
    PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line))
  FUNC_OPT(BOOL, SymGetLineFromAddrW64, (HANDLE hProcess, uint64 dwAddr,
    PDWORD pdwDisplacement, PIMAGEHLP_LINEW64 Line))
  FUNC(BOOL, SymEnumSymbols, (HANDLE hProcess, uint64 BaseOfDll,  
    PCSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, 
    PVOID UserContext))
  FUNC(BOOL, SymSetContext, (HANDLE hProcess, PIMAGEHLP_STACK_FRAME StackFrame,
    PIMAGEHLP_CONTEXT Context))
  FUNC(uint64, SymLoadModule64, (HANDLE hProcess, HANDLE hFile, 
    PSTR ImageName, PSTR ModuleName, uint64 BaseOfDll, DWORD SizeOfDll))
FUNC_GROUP_END

FUNC_GROUP_BEGIN(Minidump)
  FUNC(BOOL, MiniDumpWriteDump, (HANDLE hProcess, DWORD ProcessId, 
    HANDLE hFile, int/*MINIDUMP_TYPE*/ DumpType, 
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, 
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam))
FUNC_GROUP_END
