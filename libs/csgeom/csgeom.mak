# Library description
DESCRIPTION.csgeom = Crystal Space geometry library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP+=$(NEWLINE)echo $"  make csgeom       Make the $(DESCRIPTION.csgeom)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csgeom

all libs: csgeom
csgeom:
	$(MAKE_TARGET)
csgeomclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csgeom

CSGEOM.LIB = $(OUT)$(LIB_PREFIX)csgeom$(LIB_SUFFIX)
INC.CSGEOM = $(wildcard include/csgeom/*.h)
SRC.CSGEOM = $(wildcard libs/csgeom/*.cpp)
OBJ.CSGEOM = $(addprefix $(OUT),$(notdir $(SRC.CSGEOM:.cpp=$O)))

TO_INSTALL.STATIC_LIBS += $(CSGEOM.LIB)

MSVC.DSP += CSGEOM
DSP.CSGEOM.NAME = csgeom
DSP.CSGEOM.TYPE = library
DSP.CSGEOM.RESOURCES = $(wildcard libs/csgeom/*.inc)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csgeom csgeomclean

all: $(CSGEOM.LIB)
csgeom: $(OUTDIRS) $(CSGEOM.LIB)
clean: csgeomclean

$(CSGEOM.LIB): $(OBJ.CSGEOM)
	$(DO.LIBRARY)

csgeomclean:
	-$(RM) $(CSGEOM.LIB) $(OBJ.CSGEOM)

ifdef DO_DEPEND
dep: $(OUTOS)csgeom.dep
$(OUTOS)csgeom.dep: $(SRC.CSGEOM)
	$(DO.DEP)
else
-include $(OUTOS)csgeom.dep
endif

endif # ifeq ($(MAKESECTION),targets)
