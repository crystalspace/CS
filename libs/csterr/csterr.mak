# Library description
DESCRIPTION.csterr = Digital Dawn Graphics landscape engine

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csterr       Make the $(DESCRIPTION.csterr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csterr

all libs: csterr
csterr:
	$(MAKE_TARGET)
csterrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csterr/math libs/csterr/struct libs/csterr/util

CSTERR.LIB = $(OUT)$(LIB_PREFIX)csterr$(LIB_SUFFIX)
INC.CSTERR = $(wildcard libs/csterr/*/*.h)
SRC.CSTERR = $(wildcard libs/csterr/*/*.cpp)
OBJ.CSTERR = $(addprefix $(OUT),$(notdir $(SRC.CSTERR:.cpp=$O)))
CFLAGS.CSTERR = $(CFLAGS.D)__CRYSTAL_SPACE__ $(CFLAGS.I)libs/csterr

TO_INSTALL.STATIC_LIBS += $(CSTERR.LIB)

MSVC.DSP += CSTERR
DSP.CSTERR.NAME = csterr
DSP.CSTERR.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csterr csterrclean

all: $(CSTERR.LIB)
csterr: $(OUTDIRS) $(CSTERR.LIB)
clean: csterrclean

$(OUT)%$O: libs/csterr/math/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

$(OUT)%$O: libs/csterr/struct/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

$(OUT)%$O: libs/csterr/util/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

# Some (broken) versions of GNU make appear to be sensitive to the order in
# which implicit rules are seen.  Without the following rule (which is just
# a reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)%$O: %.cpp
	$(DO.COMPILE.CPP)

$(CSTERR.LIB): $(OBJ.CSTERR)
	$(DO.LIBRARY)

csterrclean:
	-$(RM) $(CSTERR.LIB) $(OBJ.CSTERR)

ifdef DO_DEPEND
dep: $(OUTOS)csterr.dep
$(OUTOS)csterr.dep: $(SRC.CSTERR)
	$(DO.DEP1) $(CFLAGS.CSTERR) $(DO.DEP2)
else
-include $(OUTOS)csterr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
