DESCRIPTION.opcode = OPCODE collision detection plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make opcode       Make the $(DESCRIPTION.opcode)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: opcode opcodeclean
plugins all: opcode

opcodeclean:
	$(MAKE_CLEAN)
opcode:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/collide/opcode

ifeq ($(USE_PLUGINS),yes)
  OPCODE = $(OUTDLL)/opcode$(DLL)
  LIB.OPCODE = $(foreach d,$(DEP.OPCODE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(OPCODE)
else
  OPCODE = $(OUT)/$(LIB_PREFIX)opcode$(LIB)
  DEP.EXE += $(OPCODE)
  SCF.STATIC += opcode
  TO_INSTALL.STATIC_LIBS += $(OPCODE)
endif

INC.OPCODE = $(wildcard plugins/collide/opcode/*.h)
SRC.OPCODE = $(wildcard plugins/collide/opcode/*.cpp)
OBJ.OPCODE = $(addprefix $(OUT)/,$(notdir $(SRC.OPCODE:.cpp=$O)))
DEP.OPCODE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += OPCODE
DSP.OPCODE.NAME = opcode
DSP.OPCODE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: opcode opcodeclean
opcode: $(OUTDIRS) $(OPCODE)

$(OPCODE): $(OBJ.OPCODE) $(LIB.OPCODE)
	$(DO.PLUGIN)

clean: opcodeclean
opcodeclean:
	-$(RM) $(OPCODE) $(OBJ.OPCODE)

ifdef DO_DEPEND
dep: $(OUTOS)/opcode.dep
$(OUTOS)/opcode.dep: $(SRC.OPCODE)
	$(DO.DEP)
else
-include $(OUTOS)/opcode.dep
endif

endif # ifeq ($(MAKESECTION),targets)
