# Application description
DESCRIPTION.phyz = Crystal Space phyztest example

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make phyz         Make the $(DESCRIPTION.phyz)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: phyz

all apps: phyz
phyz:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/phyztest apps/support

PHYZTEST.EXE=phyztest$(EXE)
SRC.PHYZTEST = $(wildcard apps/phyztest/*.cpp) \
  apps/support/static.cpp
OBJ.PHYZTEST = $(addprefix $(OUT),$(notdir $(SRC.PHYZTEST:.cpp=$O)))
DESCRIPTION.$(PHYZTEST.EXE) = $(DESCRIPTION.phyz)

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: phyz phyzclean

all: $(PHYZTEST.EXE)
phyz: $(OUTDIRS) $(PHYZTEST.EXE)
clean: phyzclean

$(PHYZTEST.EXE): $(DEP.EXE) $(OBJ.PHYZTEST) \
  $(CSPARSER.LIB) $(CSENGINE.LIB) $(CSTERR.LIB) $(CSSCRIPT.LIB) $(CSPHYZIK.LIB) \
  $(CSSFXLDR.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB) $(CSGEOM.LIB) $(CSINPUT.LIB) \
  $(CSOBJECT.LIB) $(CSUTIL.LIB)
	$(DO.LINK.EXE)

phyzclean:
	-$(RM) $(PHYZTEST.EXE)

ifdef DO_DEPEND
depend: $(OUTOS)phyztest.dep
$(OUTOS)phyztest.dep: $(SRC.PHYZTEST)
	$(DO.DEP)
else
-include $(OUTOS)phyztest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
