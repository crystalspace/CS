/*
    Copyright (C) 1998 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef WATCOM_H
#define WATCOM_H

/// WATCOM/DOS version.
/**
 * This is the System driver for DOS. It implements all required functionality
 * for standard csSystemDriver class plus adds some DOS-specific command-line
 * options.
 */
class SysSystemDriver : public csSystemDriver
{
public:
  /// Initialize system-dependent data
  SysSystemDriver ();

  /**
   * System loop. This should be called last since it returns
   * only on program exit
   */
  virtual void Loop ();

  /// The system is idle: we can sleep for a while
  virtual void Sleep (int SleepTime);
};

/// WATCOM/DOS version.
class SysGraphics2D : public csGraphics2D
{
public:
  // Display depth (bits per pixel)
  static int Depth;

  SysGraphics2D(int argc, char *argv[]);
  virtual ~SysGraphics2D(void);

  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual void Print(csRect *area = NULL);

  virtual void Clear (int color);
  virtual void SetRGB(int i, int r, int g, int b);
  virtual bool SetMouseCursor (int iShape, IMipMapContainer *iBitmap);

private:
  //unsigned char *Memory;
  int *WidthAddress;
};

/// WATCOM/DOS version.
class SysKeyboardDriver : public csKeyboardDriver
{
public:
  SysKeyboardDriver();
  ~SysKeyboardDriver(void);

  virtual bool Open (csEventQueue *EvQueue);
  virtual void Close ();
  void GenerateEvents ();
};

/// WATCOM/DOS version.
class SysMouseDriver : public csMouseDriver
{
public:
  SysMouseDriver();
  ~SysMouseDriver(void);

  virtual bool Open (csEventQueue *EvQueue);
  virtual void Close ();
  void GenerateEvents ();

private:
  bool MouseExists;
};

extern void printf_Enable (bool Enable);

#endif // WATCOM_H
