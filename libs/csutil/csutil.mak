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

vpath %.cpp $(SRCDIR)/libs/csutil
vpath %.c $(SRCDIR)/libs/csutil

CSUTIL.LIB = $(OUT)/$(LIB_PREFIX)csutil$(LIB_SUFFIX)
INC.CSUTIL = $(INC.CSSYS) \
  $(wildcard $(addprefix $(SRCDIR)/,include/csutil/*.h))
SRC.CSUTIL.LOCAL = \
  $(wildcard $(addprefix $(SRCDIR)/,libs/csutil/*.cpp libs/csutil/*.c))
SRC.CSUTIL = $(SRC.CSSYS) $(SRC.CSUTIL.LOCAL)
OBJ.CSUTIL = $(OBJ.CSSYS) $(addprefix $(OUT)/, \
  $(notdir $(patsubst %.c,%$O,$(SRC.CSUTIL.LOCAL:.cpp=$O))))
CFG.CSUTIL = $(SRCDIR)/data/config/mouse.cfg

TO_INSTALL.CONFIG += $(CFG.CSUTIL)
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

$(OUT)/archive$O: $(SRCDIR)/libs/csutil/archive.cpp
	$(DO.COMPILE.CPP) $(ZLIB.CFLAGS)

$(CSUTIL.LIB): $(OBJ.CSUTIL)
	$(DO.LIBRARY)

csutilclean:
	-$(RM) $(CSUTIL.LIB) $(OBJ.CSUTIL)

ifdef DO_DEPEND
dep: $(OUTOS)/csutil.dep
$(OUTOS)/csutil.dep: $(SRC.CSUTIL)
	$(DO.DEP1) $(ZLIB.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/csutil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
