# This is a subinclude file used to define the rules needed
# to build the MacOS/X Server, OpenStep, or NextStep 2d driver -- next2d

# Driver description
DESCRIPTION.next2d = Crystal Space NeXT 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make next2d       Make the $(DESCRIPTION.next2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: next2d

all drivers drivers2d: next2d

next2d:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

NEXT.SOURCE_2D_PATHS=$(addprefix libs/cs2d/next/,$(NEXT.SEARCH_PATH))
CFLAGS.INCLUDE+=$(addprefix $(CFLAGS.I),$(NEXT.SOURCE_2D_PATHS))

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# The NeXT 2D driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  NEXT2D=$(OUTDLL)next2d$(DLL)
else
  NEXT2D=$(OUT)$(LIB_PREFIX)next2d$(LIB)
  DEP.EXE+=$(NEXT2D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_NEXT2D
endif
DESCRIPTION.$(NEXT2D) = $(DESCRIPTION.next2d)
SRC.NEXT2D = $(wildcard $(addsuffix /*.cpp,$(NEXT.SOURCE_2D_PATHS)) \
  $(SRC.COMMON.DRV2D))
OBJ.NEXT2D = $(addprefix $(OUT),$(notdir $(SRC.NEXT2D:.cpp=$O)))

vpath %.cpp $(sort $(dir $(SRC.NEXT2D)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: next2d next2dclean next2dcleanlib

# Chain rules
clean: next2dclean
cleanlib: next2dcleanlib

next2d: $(OUTDIRS) $(NEXT2D)

$(NEXT2D): $(OBJ.NEXT2D)
	$(DO.PLUGIN)

next2dclean:
	$(RM) $(NEXT2D)

next2dcleanlib:
	$(RM) $(OBJ.NEXT2D) $(NEXT2D)

ifdef DO_DEPEND
depend: $(OUTOS)next2d.dep
$(OUTOS)next2d.dep: $(SRC.NEXT2D)
	$(DO.DEP)
else
-include $(OUTOS)next2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
