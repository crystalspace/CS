#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_Reset              0x00
#define MOUSE_Show               0x01
#define MOUSE_Hide               0x02
#define MOUSE_ButtPos            0x03
#define MOUSE_MotionRel          0x0B
#define MOUSE_SetSensitivity     0x1A
#define MOUSE_Int                0x33
#define MOUSE_Movement           0x101
#define MOUSE_Relative           0x102
#define MOUSE_RelativeMove       0x103

#define MOUSE_LeftButton         0x01
#define MOUSE_RightButton        0x02
#define MOUSE_CenterButton       0x04

#ifdef __cplusplus
extern "C" {
#endif

int mouseCommand(int command, short *x, short *y, short *buttons);
int mouseDetect();

#ifdef __cplusplus
}
#endif

#endif
