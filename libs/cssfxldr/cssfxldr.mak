# Library description
DESCRIPTION.cssfxldr = Crystal Space sound loader library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make cssfxldr     Make the $(DESCRIPTION.cssfxldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cssfxldr

all libs: cssfxldr
cssfxldr:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssfxldr libs/cssfxldr/common

SRC.CSSFXLDR = $(wildcard libs/cssfxldr/common/*.cpp) \
  libs/cssfxldr/sndload.cpp libs/cssfxldr/funcs.cpp

ifeq ($(DO_AIFF),yes)
  SRC.CSSFXLDR+=libs/cssfxldr/aifffile.cpp
endif
ifeq ($(DO_IFF),yes)
  SRC.CSSFXLDR+=libs/cssfxldr/ifffile.cpp
endif
ifeq ($(DO_WAV),yes)
  SRC.CSSFXLDR+=libs/cssfxldr/wavfile.cpp
endif
ifeq ($(DO_AU),yes)
  SRC.CSSFXLDR+=libs/cssfxldr/aufile.cpp
endif

CSSFXLDR.LIB = $(OUT)$(LIB_PREFIX)cssfxldr$(LIB_SUFFIX)
OBJ.CSSFXLDR = $(addprefix $(OUT),$(notdir $(SRC.CSSFXLDR:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cssfxldr cssfxldrclean

all: $(CSSFXLDR.LIB)
cssfxldr: $(OUTDIRS) $(CSSFXLDR.LIB)
clean: cssfxldrclean

$(CSSFXLDR.LIB): $(OBJ.CSSFXLDR)
	$(DO.LIBRARY)

cssfxldrclean:
	-$(RM) $(CSSFXLDR.LIB)

ifdef DO_DEPEND
depend: $(OUTOS)cssfxldr.dep
$(OUTOS)cssfxldr.dep: $(SRC.CSSFXLDR)
	$(DO.DEP)
else
-include $(OUTOS)cssfxldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)

#------------------------------------------------------------------- config ---#
ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)

ifeq ($(DO_AIFF),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_AIFF$">>volatile.tmp
endif
ifeq ($(DO_IFF),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_IFF$">>volatile.tmp
endif
ifeq ($(DO_WAV),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_WAV$">>volatile.tmp
endif
ifeq ($(DO_AU),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_AU$">>volatile.tmp
endif

endif # ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)
