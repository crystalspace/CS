# Library description
DESCRIPTION.cstool = Crystal Space tool Library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP+=$(NEWLINE)echo $"  make cstool       Make the $(DESCRIPTION.cstool)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cstool

all libs: cstool
cstool:
	$(MAKE_TARGET)
cstoolclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cstool libs/cstool/impexp

CSTOOL.LIB = $(OUT)$(LIB_PREFIX)cstool$(LIB_SUFFIX)
INC.CSTOOL = $(wildcard include/cstool/*.h)
SRC.CSTOOL = $(wildcard libs/cstool/*.cpp libs/cstool/impexp/*.cpp)
OBJ.CSTOOL = $(addprefix $(OUT),$(notdir $(SRC.CSTOOL:.cpp=$O)))

TO_INSTALL.STATIC_LIBS += $(CSTOOL.LIB)

MSVC.DSP += CSTOOL
DSP.CSTOOL.NAME = cstool
DSP.CSTOOL.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cstool cstoolclean

all: $(CSTOOL.LIB)
cstool: $(OUTDIRS) $(CSTOOL.LIB)
clean: cstoolclean

$(CSTOOL.LIB): $(OBJ.CSTOOL)
	$(DO.LIBRARY)

cstoolclean:
	-$(RM) $(CSTOOL.LIB) $(OBJ.CSTOOL)

ifdef DO_DEPEND
dep: $(OUTOS)cstool.dep
$(OUTOS)cstool.dep: $(SRC.CSTOOL)
	$(DO.DEP)
else
-include $(OUTOS)cstool.dep
endif

endif # ifeq ($(MAKESECTION),targets)
