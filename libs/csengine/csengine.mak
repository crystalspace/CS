# Library description
DESCRIPTION.csengine = Crystal Space 3D engine

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csengine     Make the $(DESCRIPTION.csengine)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csengine

all libs: csengine
csengine:
	$(MAKE_TARGET)
csengineclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csengine libs/csengine/2d libs/csengine/basic \
  libs/csengine/colldet libs/csengine/light libs/csengine/objects \
  libs/csengine/polygon libs/csengine/polytree libs/csengine/culler

CSENGINE.LIB = $(OUT)$(LIB_PREFIX)csengine$(LIB_SUFFIX)
INC.CSENGINE = $(wildcard include/csengine/*.h)
SRC.CSENGINE = $(wildcard libs/csengine/*.cpp libs/csengine/*/*.cpp)
OBJ.CSENGINE = $(addprefix $(OUT),$(notdir $(SRC.CSENGINE:.cpp=$O)))

TO_INSTALL.DATA += data/standard.zip
TO_INSTALL.STATIC_LIBS += $(CSENGINE.LIB)

MSVC.DSP += CSENGINE
DSP.CSENGINE.NAME = csengine
DSP.CSENGINE.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csengine csengineclean

all: $(CSENGINE.LIB)
csengine: $(OUTDIRS) $(CSENGINE.LIB)
clean: csengineclean

$(CSENGINE.LIB): $(OBJ.CSENGINE)
	$(DO.LIBRARY)

csengineclean:
	-$(RM) $(CSENGINE.LIB) $(OBJ.CSENGINE)

ifdef DO_DEPEND
dep: $(OUTOS)csengine.dep
$(OUTOS)csengine.dep: $(SRC.CSENGINE)
	$(DO.DEP)
else
-include $(OUTOS)csengine.dep
endif

endif # ifeq ($(MAKESECTION),targets)
