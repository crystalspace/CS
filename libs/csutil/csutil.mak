# Library description
DESCRIPTION.csutil = Crystal Space utility library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csutil       Make the $(DESCRIPTION.csutil)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csutil

all libs: csutil
csutil:
	$(MAKE_TARGET)
csutilclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csutil
vpath %.c libs/csutil

CSUTIL.LIB = $(OUT)$(LIB_PREFIX)csutil$(LIB_SUFFIX)
INC.CSUTIL = $(wildcard include/csutil/*.h)
SRC.CSUTIL = $(wildcard libs/csutil/*.cpp libs/csutil/*.c)
OBJ.CSUTIL = \
  $(addprefix $(OUT),$(notdir $(patsubst %.c,%$O,$(SRC.CSUTIL:.cpp=$O))))
CFG.CSUTIL = scf.cfg data/config/mouse.cfg

TO_INSTALL.ROOT += $(CFG.CSUTIL)
TO_INSTALL.STATIC_LIBS += $(CSUTIL.LIB)

MSVC.DSP += CSUTIL
DSP.CSUTIL.NAME = csutil
DSP.CSUTIL.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csutil csutilclean

all: $(CSUTIL.LIB)
csutil: $(OUTDIRS) $(CSUTIL.LIB)
clean: csutilclean

$(CSUTIL.LIB): $(OBJ.CSUTIL)
	$(DO.LIBRARY)

csutilclean:
	-$(RM) $(CSUTIL.LIB) $(OBJ.CSUTIL)

ifdef DO_DEPEND
dep: $(OUTOS)csutil.dep
$(OUTOS)csutil.dep: $(SRC.CSUTIL)
	$(DO.DEP)
else
-include $(OUTOS)csutil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
