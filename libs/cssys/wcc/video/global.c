  int FlagFlipVMode;
  int OffsetVPage;
  int WidthPageFactor;
  int VBE_DetectFlag;
  int XModeFlag;
  int numXmode;
  int VesaFlag;
  int ClipMinX;
  int ClipMinY;
  int ClipMaxX;
  int ClipMaxY;
  int SizeScreenX;
  int SizeScreenY;

  char StringHorRaster[1024];
  char ColumnHorRaster[768];
  char *WidthAdressTable[768];

  union REGS regs;
  struct SREGS sregs;
