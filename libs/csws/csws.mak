# Library description
DESCRIPTION.csws = Crystal Space Windowing System library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csws         Make the $(DESCRIPTION.csws)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csws

all libs: csws
csws:
	$(MAKE_TARGET)
cswsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csws libs/csws/skins/default

CSWS.LIB = $(OUT)$(LIB_PREFIX)csws$(LIB_SUFFIX)
INC.CSWS = $(wildcard include/csws/*.h)
SRC.CSWS = $(wildcard libs/csws/*.cpp libs/csws/skins/*/*.cpp)
OBJ.CSWS = $(addprefix $(OUT),$(notdir $(SRC.CSWS:.cpp=$O)))
CFG.CSWS = data/config/csws.cfg

TO_INSTALL.STATIC_LIBS += $(CSWS.LIB)
TO_INSTALL.DATA += data/csws.zip
TO_INSTALL.CONFIG += $(CFG.CSWS)

MSVC.DSP += CSWS
DSP.CSWS.NAME = csws
DSP.CSWS.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csws cswsclean

all: $(CSWS.LIB)
csws: $(OUTDIRS) $(CSWS.LIB)
clean: cswsclean

$(CSWS.LIB): $(OBJ.CSWS)
	$(DO.LIBRARY)

cswsclean:
	-$(RM) $(CSWS.LIB) $(OBJ.CSWS)

ifdef DO_DEPEND
dep: $(OUTOS)csws.dep
$(OUTOS)csws.dep: $(SRC.CSWS)
	$(DO.DEP)
else
-include $(OUTOS)csws.dep
endif

endif # ifeq ($(MAKESECTION),targets)
