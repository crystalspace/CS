FUNC_GROUP_BEGIN(MultiMon)
  FUNC(HMONITOR, MonitorFromWindow, (HWND hWnd, DWORD dwFlags))
  FUNC(BOOL, GetMonitorInfoA, (HMONITOR monitor, void* lpmi))
FUNC_GROUP_END

#undef FUNC_GROUP_BEGIN
#undef FUNC_GROUP_END
#undef FUNC
