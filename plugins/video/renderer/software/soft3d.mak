# This is a subinclude file used to define the rules needed
# to build the 3D software rendering driver -- soft

# Driver description
DESCRIPTION.soft = Crystal Space software renderer

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make soft         Make the $(DESCRIPTION.soft)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: soft

ifeq ($(USE_DLL),yes)
all drivers drivers3d: soft
endif

soft:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(DO_MMX),yes)
  CFLAGS.SOFT3D += $(CFLAGS.D)DO_MMX
endif

ifeq ($(USE_DLL),yes)
  SOFT3D=$(OUTDLL)softrndr$(DLL)
  DEP.SOFT3D=$(CSCOM.LIB) $(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  SOFT3D=$(OUT)$(LIB_PREFIX)soft$(LIB)
  DEP.EXE+=$(SOFT3D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_SOFT3D
endif
DESCRIPTION.$(SOFT3D) = $(DESCRIPTION.soft)
SRC.SOFT3D = $(wildcard libs/cs3d/software/*.cpp) \
  libs/cs3d/common/txtmgr.cpp libs/cs3d/common/memheap.cpp \
  libs/cs3d/common/inv_cmap.cpp libs/cs3d/common/imgtools.cpp
OBJ.SOFT3D = $(addprefix $(OUT),$(notdir $(SRC.SOFT3D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: soft softclean softcleanlib

# Chain rules
all: $(SOFT3D)
clean: softclean
cleanlib: softcleanlib

soft: $(OUTDIRS) $(SOFT3D)

$(OUT)%$O: libs/cs3d/software/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)
 
$(SOFT3D): $(OBJ.SOFT3D) $(DEP.SOFT3D)
	$(DO.LIBRARY)

softclean:
	$(RM) $(SOFT3D)

softcleanlib:
	$(RM) $(OBJ.SOFT3D) $(SOFT3D)

ifdef DO_DEPEND
$(OUTOS)soft3d.dep: $(SRC.SOFT3D)
	$(DO.DEP) $(OUTOS)soft3d.dep
endif

-include $(OUTOS)soft3d.dep

endif # ifeq ($(MAKESECTION),targets)
