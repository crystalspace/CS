# Library description
DESCRIPTION.cssndldr = Crystal Space sound loader library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make cssndldr     Make the $(DESCRIPTION.cssndldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cssndldr

all libs: cssndldr
cssndldr:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssndldr libs/cssndldr/common

SRC.CSSNDLDR = $(wildcard libs/cssndldr/common/*.cpp) \
  libs/cssndldr/sndload.cpp libs/cssndldr/funcs.cpp

ifeq ($(DO_AIFF),yes)
  SRC.CSSNDLDR+=libs/cssndldr/aifffile.cpp
  CFLAGS.SNDLOADER+=$(CFLAGS.D)DO_AIFF
endif
ifeq ($(DO_IFF),yes)
  SRC.CSSNDLDR+=libs/cssndldr/ifffile.cpp
  CFLAGS.SNDLOADER+=$(CFLAGS.D)DO_IFF
endif
ifeq ($(DO_WAV),yes)
  SRC.CSSNDLDR+=libs/cssndldr/wavfile.cpp
  CFLAGS.SNDLOADER+=$(CFLAGS.D)DO_WAV
endif
ifeq ($(DO_AU),yes)
  SRC.CSSNDLDR+=libs/cssndldr/aufile.cpp
  CFLAGS.SNDLOADER+=$(CFLAGS.D)DO_AU
endif

CSSNDLDR.LIB = $(OUT)$(LIB_PREFIX)cssndldr$(LIB)
OBJ.CSSNDLDR = $(addprefix $(OUT),$(notdir $(SRC.CSSNDLDR:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cssndldr cssndldrclean

all: $(CSSNDLDR.LIB)
cssndldr: $(OUTDIRS) $(CSSNDLDR.LIB)
clean: cssndldrclean

$(OUT)sndload$O: sndload.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SNDLOADER)

$(CSSNDLDR.LIB): $(OBJ.CSSNDLDR)
	$(DO.STATIC.LIBRARY)

cssndldrclean:
	-$(RM) $(CSSNDLDR.LIB)

ifdef DO_DEPEND
$(OUTOS)cssndldr.dep: $(SRC.CSSNDLDR)
	$(DO.DEP)
endif

-include $(OUTOS)cssndldr.dep

endif # ifeq ($(MAKESECTION),targets)
