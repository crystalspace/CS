#####################################################################
## Uncomment the following line if you specifically                ##
## want to assembler routines (if supported by your                ##
## architecture). This will of course make Crystal                 ##
## Space run slower.                                               ##
#ASM=-DNO_ASSEMBLER
#####################################################################

#####################################################################
## The compiler/os/processor flags:                                ##
SYSFLAGS=-DCOMP_WCC -DOS_DOS -DPROC_INTEL -DDO_TGA -DDO_PNG -DDO_GIF
#####################################################################

#####################################################################
## Choose between optimizing                                       ##
##  486                                                            ##
#INTEL_OPTIMIZE=/fpi87 /fp3 /4r /omantril /s /zp4
##  Pentium, k5, 5x86                                              ##
HOW_OPTIMIZE=/fpi87 /fp5 /5r /omantril /s /zp4
##  PentiumPro, Pentium2, K6, 6x86                                 ##
#INTEL_OPTIMIZE=/fpi87 /fp6 /6r /obpmantrile /s /zp4
#####################################################################


# name the compiler
CC = wcc386

# define type of model memory
#   l = large
#   f = flat
MODEL = f

# options to use with compilator and assembler
CFLAGS = -m$(MODEL) $(HOW_OPTIMIZE) $(OPTION) -i=..\include

# name program
PROG    = video.lib

# objects
MAIN    = dpmiserv.obj scrctrl.obj vesa.obj

.EXTENSIONS:
.EXTENSIONS: .exe .obj .c

.BEFORE
        @set DOS4G=QUIET

# explicit rules

all :   install clean .symbolic
        @%null

$(PROG) : $(MAIN) .PRECIOUS
        wlib $(PROG) /c /n /b dpmiserv.obj scrctrl.obj vesa.obj

install : $(PROG) .symbolic
        copy $(PROG) ..
        @%null

clean : .SYMBOLIC
#        del *.obj


# implicit rules

.c.obj :
        $(CC) $^& $(CFLAGS)
