#
# This submakefile requires the following variables defined in
# system-dependent makefile:
#
# SRC.SYS_CSSYS
#   - to contain all system-dependent source files that should be
#     included into cssys library
# SRC.SYS_CSSYS_DLL
#   - additional source files needed only for dynamic libraries
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

CSSYS.LIB = $(OUT)$(LIB_PREFIX)cssys$(LIB)
SRC.CSSYS = libs/cssys/common/csendian.cpp libs/cssys/common/system.cpp \
  $(SRC.SYS_CSSYS)
ifeq ($(MAKE_DLL),yes)
  SRC.CSSYS += $(SRC.SYS_CSSYS_DLL)
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
$(OUTOS)cssys.dep: $(SRC.CSSYS)
	$(DO.DEP) $(OUTOS)cssys.dep
endif

-include $(OUTOS)cssys.dep

endif # ifeq ($(MAKESECTION),targets)
