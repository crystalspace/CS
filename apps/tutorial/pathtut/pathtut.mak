# Application description
DESCRIPTION.tutpath = Crystal Space Path Tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tutpath      Make the $(DESCRIPTION.tutpath)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tutpath tutpathclean

all apps: tutpath
tutpath:
	$(MAKE_TARGET)
tutpathclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/pathtut

TUTPATH.EXE = pathtut$(EXE)
INC.TUTPATH = $(wildcard apps/tutorial/pathtut/*.h)
SRC.TUTPATH = $(wildcard apps/tutorial/pathtut/*.cpp)
OBJ.TUTPATH = $(addprefix $(OUT),$(notdir $(SRC.TUTPATH:.cpp=$O)))
DEP.TUTPATH = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.TUTPATH = $(foreach d,$(DEP.TUTPATH),$($d.LIB))

#TO_INSTALL.EXE += $(TUTPATH.EXE)

MSVC.DSP += TUTPATH
DSP.TUTPATH.NAME = pathtut
DSP.TUTPATH.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: tutpath tutpathclean

all: $(TUTPATH.EXE)
tutpath: $(OUTDIRS) $(TUTPATH.EXE)
clean: tutpathclean

$(TUTPATH.EXE): $(DEP.EXE) $(OBJ.TUTPATH) $(LIB.TUTPATH)
	$(DO.LINK.EXE)

tutpathclean:
	-$(RM) $(TUTPATH.EXE) $(OBJ.TUTPATH)

ifdef DO_DEPEND
dep: $(OUTOS)pathtut.dep
$(OUTOS)pathtut.dep: $(SRC.TUTPATH)
	$(DO.DEP)
else
-include $(OUTOS)pathtut.dep
endif

endif # ifeq ($(MAKESECTION),targets)

