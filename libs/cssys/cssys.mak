#
# This submakefile requires the following variables defined in
# system-dependent makefile:
#
# SRC.SYS_CSSYS
#   - all system-dependent source files that should be included in cssys
#     library
# SRC.SYS_CSSYS_DLL
#   - additional source files needed only for dynamic libraries
# SRC.SYS_CSSYS_EXE
#   - additional source files needed only for executable files
#

# Library description
DESCRIPTION.cssys = Crystal Space system library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make cssys        Make the $(DESCRIPTION.cssys)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cssys

all libs: cssys
cssys:
	$(MAKE_TARGET)
cssysclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssys $(sort $(dir $(SRC.SYS_CSSYS)))
vpath %.c   libs/cssys $(sort $(dir $(SRC.SYS_CSSYS)))

INC.CSSYS = $(wildcard include/cssys/*.h)
SRC.CSSYS = $(wildcard libs/cssys/*.cpp $(SRC.SYS_CSSYS))
ifeq ($(MAKE_DLL),yes)
  CSSYS.LIB = $(OUT)$(LIB_PREFIX)cssys_D$(LIB_SUFFIX)
  SRC.CSSYS += $(SRC.SYS_CSSYS_DLL)
else
  ifneq ($(OS),WIN32)
    CSSYS.LIB = $(OUT)$(LIB_PREFIX)cssys$(LIB_SUFFIX)
  else
    CSSYS.LIB = $(OUT)$(LIB_PREFIX)cssys$(LIB)
    DEP.EXE  += $(CSSYS.LIB)
    LIBS.EXE += $(LIBS.DXINPUT) $(LIBS.DXGUID)
  endif
  SRC.CSSYS += $(SRC.SYS_CSSYS_EXE)
endif
OBJ.CSSYS ?= $(addprefix $(OUT),$(notdir \
  $(subst .s,$O,$(subst .c,$O,$(SRC.CSSYS:.cpp=$O)))))

TO_INSTALL.STATIC_LIBS += $(CSSYS.LIB)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cssys cssysclean

all: $(CSSYS.LIB)
cssys: $(OUTDIRS) $(CSSYS.LIB)
clean: cssysclean

$(CSSYS.LIB): $(OBJ.CSSYS)
	$(DO.LIBRARY) 

cssysclean:
	-$(RM) $(CSSYS.LIB) $(OBJ.CSSYS) dep: $(OUTOS)cssys.dep

ifdef DO_DEPEND
dep: $(OUTOS)cssys.dep
$(OUTOS)cssys.dep: $(SRC.CSSYS)
	$(DO.DEP)
else
-include $(OUTOS)cssys.dep
endif

endif # ifeq ($(MAKESECTION),targets)
