#
# This submakefile requires the following variables defined in
# system-dependent makefile:
#
# SRC.SYS_CSSYS
#   - to contain all system-dependent source files that should be
#     included into cssys library
# SRC.SYS_CSSYS_DLL
#   - additional source files needed only for dynamic libraries
# SRC.SYS_CSSYS_EXE
#   - additional source files needed only for executable files
#

# Library description
DESCRIPTION.cssys = Crystal Space system library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make cssys        Make the $(DESCRIPTION.cssys)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cssys

all libs: cssys
cssys:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssys/common $(sort $(dir $(SRC.SYS_CSSYS)))

ifneq ($(MEM),)
  SRC.SYS_CSSYS_EXE+=memory.cpp
  SRC.SYS_CSSYS_DLL+=memory.cpp
endif

SRC.CSSYS = $(wildcard libs/cssys/common/*.cpp $(SRC.SYS_CSSYS))
ifeq ($(MAKE_DLL),yes)
  CSSYS.LIB = $(OUT)$(LIB_PREFIX)cssys_D$(LIB)
  SRC.CSSYS += $(SRC.SYS_CSSYS_DLL)
else
  CSSYS.LIB = $(OUT)$(LIB_PREFIX)cssys$(LIB)
  SRC.CSSYS += $(SRC.SYS_CSSYS_EXE)
endif
OBJ.CSSYS = $(addprefix $(OUT),$(notdir $(subst .s,$O,$(subst .c,$O,$(SRC.CSSYS:.cpp=$O)))))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cssys cssysclean

all: $(CSSYS.LIB)
cssys: $(OUTDIRS) $(CSSYS.LIB)
clean: cssysclean

$(CSSYS.LIB): $(OBJ.CSSYS)
	$(DO.STATIC.LIBRARY)

cssysclean:
	-$(RM) $(CSSYS.LIB)

ifdef DO_DEPEND
depend: $(OUTOS)cssys.dep
$(OUTOS)cssys.dep: $(SRC.CSSYS)
	$(DO.DEP)
else
-include $(OUTOS)cssys.dep
endif

endif # ifeq ($(MAKESECTION),targets)
