FUNC_GROUP_BEGIN(Dwmapi)
  FUNC(HRESULT, DwmEnableBlurBehindWindow, (HWND hWnd, const DWM_BLURBEHIND* pBlurBehind))
  FUNC(HRESULT, DwmIsCompositionEnabled, (BOOL* pfEnabled))
  FUNC(HRESULT, DwmExtendFrameIntoClientArea, (HWND hWnd, const dwmMARGINS* pMarInset))
FUNC_GROUP_END

#undef FUNC_GROUP_BEGIN
#undef FUNC_GROUP_END
#undef FUNC
