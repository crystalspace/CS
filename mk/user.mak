# This is an include file for all the makefiles which describes user settings
# global to all Crystal Space programs and libraries. Edit to suit your taste.
# Also have a look at system-dependent makefile for system settings.

# Should we build drivers/plugins as loadable modules?
ifndef USE_DLL
  USE_DLL=yes
endif

# Default build mode
ifndef MODE
  MODE=optimize
endif

# Should we use NASM for assembly?
ifndef USE_NASM
  USE_NASM=no
endif

# If 'yes' include assembly optimizations in Crystal Space. On systems
# that don't support this, setting this option to 'yes' does nothing.
DO_ASM=yes

# If 'yes' include support for PNG graphic files (required for MazeD).
DO_PNG=yes

# If 'yes' include support for GIF graphic files (required for all current
# data files).
DO_GIF=yes

# If 'yes' include support for BMP graphic files (not required currently).
DO_BMP=yes

# If 'yes' include support for TGA graphic files (not required currently).
DO_TGA=yes

# If 'yes' include support for JPG graphic files (not required currently).
DO_JPG=yes

# If 'yes' include sound support.
DO_SOUND=yes

# If "yes" include AIFF support
DO_AIFF=yes

# If "yes" include IFF support
DO_IFF=yes

# If "yes" include AU support
DO_AU=yes

# If "yes" include WAV support
DO_WAV=yes

# If "yes" include MMX support in software renderer
DO_MMX=yes

# Uncomment the following line(s) if you want to
# enable the memory debugger that is built in.
# This is only recommended for debugging purposes
# as it makes Crystal Space a lot slower.
# If you want more extensive memory checking
# then you should also enable MEM_CHECK_EXTENSIVE.
# In this mode CS also keeps all allocated memory
# even after it has been freed and fills it with
# some special value. That way you can detect
# if memory is still being used after it has been
# freed. Define MEM_CHECK_FILL symbol so that
# newly allocated memory will be filled with
# garbage to detect memory areas that are used
# uninitialized (since most OSes fills allocated
# memory with zeros).
#MEM=-DMEM_CHECK -DMEM_CHECK_FILL
#MEM=-DMEM_CHECK -DMEM_CHECK_EXTENSIVE
MEM=

# Set to 1 to use Mesa instead of "real" OpenGL
# You can define MESA_PATH variable in environment to point to MesaGL base path
USE_MESA=1

# Set this flag to 'yes' if you have a buggy egcs compiler (versions 1.1.2 and 1.1.3)
# 1.0.3a is the latest pre 1.1.0 egcs which does not have this bug. Versions after 1.1.3
# may also be safe.
BUGGY_EGCS_COMPILER=no
