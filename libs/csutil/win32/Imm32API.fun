FUNC_GROUP_BEGIN(Imm)
  FUNC(HIMC, ImmGetContext, (HWND wnd))
  FUNC(BOOL, ImmReleaseContext, (HWND wnd, HIMC imc))
  FUNC(DWORD, ImmGetCompositionStringW, (HIMC imc, DWORD dwIndex, 
    LPVOID lpBuf, DWORD dwBufLen))
  FUNC(DWORD, ImmGetCompositionStringA, (HIMC imc, DWORD dwIndex, 
    LPVOID lpBuf, DWORD dwBufLen))
FUNC_GROUP_END

#undef FUNC_GROUP_BEGIN
#undef FUNC_GROUP_END
#undef FUNC
