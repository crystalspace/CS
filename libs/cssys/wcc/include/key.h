#ifndef KEY_H
#define KEY_H

// Scan codes for keys
#define SCAN_LSHIFT    0x2a
#define SCAN_RSHIFT    0x36
#define SCAN_CTRL      0x1d
#define SCAN_ALT       0x38

// Bit flags in KeyMap
#define KEYMAP_CHANGED 0x80
#define KEYMAP_SHIFT   0x40
#define KEYMAP_CTRL    0x20
#define KEYMAP_ALT     0x10
#define KEYMAP_DOWN    0x01

#ifdef __cplusplus
extern "C" {
#endif

extern char KeyMap[128];
extern int  KEY_Initialized;

// prototype
extern void keyMain();
extern void keyExit();

#ifdef __cplusplus
}
#endif

#endif
