# Library description
DESCRIPTION.csfx = Crystal Space Special Effects

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP+=$(NEWLINE)echo $"  make csfx         Make the $(DESCRIPTION.csfx)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csfx

all libs: csfx
csfx:
	$(MAKE_TARGET)
csfxclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csfx

CSFX.LIB = $(OUT)$(LIB_PREFIX)csfx$(LIB_SUFFIX)
INC.CSFX = $(wildcard include/csfx/*.h)
SRC.CSFX = $(wildcard libs/csfx/*.cpp)
OBJ.CSFX = $(addprefix $(OUT),$(notdir $(SRC.CSFX:.cpp=$O)))

TO_INSTALL.STATIC_LIBS += $(CSFX.LIB)

MSVC.DSP += CSFX
DSP.CSFX.NAME = csfx
DSP.CSFX.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csfx csfxclean

all: $(CSFX.LIB)
csfx: $(OUTDIRS) $(CSFX.LIB)
clean: csfxclean

$(CSFX.LIB): $(OBJ.CSFX)
	$(DO.LIBRARY)

csfxclean:
	-$(RM) $(CSFX.LIB) $(OBJ.CSFX)

ifdef DO_DEPEND
dep: $(OUTOS)csfx.dep
$(OUTOS)csfx.dep: $(SRC.CSFX)
	$(DO.DEP)
else
-include $(OUTOS)csfx.dep
endif

endif # ifeq ($(MAKESECTION),targets)
