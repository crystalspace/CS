# This is a subinclude file used to define the rules needed
# to build the image 2D driver -- memory

# Driver description
DESCRIPTION.memory = Crystal Space memory 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make memory         Make the $(DESCRIPTION.memory)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: memory memoryclean
all plugins drivers drivers2d: memory

memory:
	$(MAKE_TARGET) MAKE_DLL=yes
memoryclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/memory

# The 2D Memorylib driver
ifeq ($(USE_PLUGINS),yes)
  MEMORY = $(OUTDLL)memory$(DLL)
  LIB.MEMORY = $(foreach d,$(DEP.MEMORY),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(MEMORY)
else
  MEMORY = memory.a
  DEP.EXE += $(MEMORY)
  SCF.STATIC += memory
  TO_INSTALL.STATIC_LIBS += $(MEMORY)
endif

INC.MEMORY = $(wildcard plugins/video/canvas/memory/*.h   $(INC.COMMON.DRV2D))
SRC.MEMORY = $(wildcard plugins/video/canvas/memory/*.cpp $(SRC.COMMON.DRV2D))
OBJ.MEMORY = $(addprefix $(OUT),$(notdir $(SRC.MEMORY:.cpp=$O)))
DEP.MEMORY = CSUTIL CSSYS



#MSVC.DSP += MEMORY
#DSP.MEMORY.NAME = memory
#DSP.MEMORY.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/video/canvas/memory

.PHONY: memory memoryclean

# Chain rules
clean: memoryclean

memory: $(OUTDIRS) $(MEMORY)

$(MEMORY): $(OBJ.MEMORY) $(LIB.MEMORY)
	$(DO.PLUGIN) $(LIB.MEMORY.SYSTEM)

memoryclean:
	$(RM) $(MEMORY) $(OBJ.MEMORY) $(OUTOS)memory.dep

ifdef DO_DEPEND
dep: $(OUTOS)memory.dep
$(OUTOS)memory.dep: $(SRC.MEMORY)
	$(DO.DEP)
else
-include $(OUT)memory.dep
endif

endif # ifeq ($(MAKESECTION),targets)
