# Application description
DESCRIPTION.csarts = Crystal Space aRts sound renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make csarts       Make the $(DESCRIPTION.csarts)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csarts csartsclean csartsinstall csartslib

all apps: csarts 
#csartsinstall
csartsinstall csarts csartslib:
	$(MAKE_TARGET)
csartsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp %.cc plugins/sound/renderer/arts

ARTSTEST.EXE = artstest$(EXE)

# Unlike CS interfaces, mcop interfaces need to be precompiled and are, in the end,
# represemted by 3 classes (foo_skel, foo_stub, foo)
# Our final renderer plugin will only access foos and not their implementation, so we create
# two libraries: csarts and csart_idl, where csarts_idl only holds the interface classes
# The final renderer will be linked with csarts_idl, whereas the implementation library csarts will live
# in a path that is searched by mcop's objectmanager

# these are the files we feed the MCOP idl compiler with
IDL.CSARTS = $(wildcard plugins/sound/renderer/arts/*.idl)

# the mcop compiler will spill out the following files, we keep track of them separatly to link those
# (and only those) into the csarts_idl library
SRC.CSARTS.IDL = $(IDL.ARTSTEST:.idl=.cc)

# Next we will need the implementation sources for the interfaces.
# We introduce the convention, that the implementation sources are named *_impl.cpp.
# We keep track of them explicitly because we'll send them through libtool.
SRC.CSARTS.IMPL = $(wildcard plugins/sound/renderer/arts/*_impl.cpp) 

# what follows are the source only needed by the csarts renderer itself
SRC.CSARTS = $(filter-out $(SRC.CSARTS.IMPL) $(wildcard plugins/sound/renderer/arts/*.cpp))

# since we need the obj files for the interfaces and their implementations created through libtool
# we'll give them a unique suffix - .moo
MOOBJ.CSARTS.IDL = $(addprefix $(OUT),$(notdir $(SRC.CSARTS:.cc=.moo)))
MOOBJ.CSARTS.IMPL = $(addprefix $(OUT),$(notdir $(SRC.CSARTS.IMPL:.cpp=.moo)))

# the others use the usual suffix
OBJ.CSARTS = $(addprefix $(OUT),$(notdir $(SRC.CSARTS:.cpp=$O)))

# the lib that will be used by mcop and that ends up in a directory visible by mcop
LIB.CSARTS.IMPL = plugins/sound/renderer/arts/libcsarts.la

# the iterface lib we'll link to our csarts renderer
LIB.CSARTS.IDL  = $(OUT)$(LIB_PREFIX)csarts_idl$(DLL)

# the csarts renderer plugin
CSARTS  = $(OUTDLL)csarts$(DLL)

# common lib we need to link
LIB.CSARTS.COMMON = -lartsflow_idl -lmcop -ldl

# additional libs we need to link the csarts plugin
LIB.LINK.CSARTS =-L$(MCOP.LIBDIR) $(LIB.CSARTS.COMMON) -lsoundserver_idl -lstdc++

# additional libs we need to link the implementation library
LIB.LINK.CSARTS.IMPL=-module -rpath $(KDEDIR)/lib -L$(MCOP.LIBDIR) $(LIB.CSARTS.COMMON) -lartsmodules

DEP.CSARTS = CSGFX CSGEOM CSUTIL CSSYS CSUTIL 
LIB.CSARTS = $(foreach d,$(DEP.CSARTS),$($d.LIB))

ARTS.CXX = libtool --mode=compile g++
ARTS.LD  =libtool --mode=link g++
ARTS.CP  =libtool --mode=install cp

CSARTS.LA.SRC = $(IDLSRC.ARTSTEST) $(IMPLSRC.ARTSTEST)

CSARTS.LA.OBJ = $(addprefix plugins/sound/renderer/arts/,$(notdir $(IDLSRC.ARTSTEST:.cc=.lo))) \
$(addprefix plugins/sound/renderer/arts/,$(notdir $(IMPLSRC.ARTSTEST:.cpp=.lo)))

TO_INSTALL.EXE += $(ARTSTEST.EXE)

#MSVC.DSP += ARTSTEST
DSP.ARTSTEST.NAME = artstest
DSP.ARTSTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csarts csartsclean csartsinstall csartslib csartsidl

csarts: $(OUTDIRS) $(ARTSTEST.EXE) $(LIB.CSARTS)
csartidl: $(LIB.CSARTSIDL)
csartslib: $(LIB.CSARTS)
clean: csartsclean

$(ARTSTEST.EXE): $(OBJ.ARTSTEST) $(LIB.ARTSTEST) $(LIB.CSARTSIDL)
	$(DO.LINK.EXE) $(LIB.ARTS) $(LIB.CSARTSIDL)

$(LIB.CSARTSIDL): $(OBJ.ARTSTEST)

$(LIB.CSARTS): $(CSARTS.LA.OBJ) $(LIB.ARTSTEST)
	$(ARTS.LD) -o $(LIB.CSARTS) $(ARTS.LDFLAGS) $(CSARTS.LA.OBJ)

$(OUT)%.moo: plugins/sound/renderer/arts/%.cc
	$(DO.COMPILE.CPP) $(INC.ARTS)

plugins/sound/renderer/arts/%.lo: plugins/sound/renderer/arts/%.cc
	cd plugins/sound/renderer/arts; $(ARTS.CXX) $(INC.ARTS) -c $(<F) ; cd ../../../..

plugins/sound/renderer/arts/%.lo: plugins/sound/renderer/arts/%.cpp
	cd plugins/sound/renderer/arts; $(ARTS.CXX) $(INC.ARTS) -c $(<F) ; cd ../../../..

$(OUT)%$O: plugins/sound/renderer/arts/%.cpp
	$(DO.COMPILE.CPP) $(INC.ARTS)

plugins/sound/renderer/arts/%.cc: plugins/sound/renderer/arts/%.idl
	mcopidl $(INC.ARTS) $(<<)
	mv $(basename $(@F)).* plugins/sound/renderer/arts

plugins/sound/renderer/arts/%.h: plugins/sound/renderer/arts/%.idl
	mcopidl $(INC.ARTS) $(<<)
	mv $(basename $(@F)).* plugins/sound/renderer/arts

csartsclean: 
	-$(RM) $(ARTSTEST.EXE) $(OBJ.ARTSTEST) $(CSARTS.LA.OBJ)

csartsinstall: $(LIB.CSARTS)
	$(ARTS.CP) $(LIB.CSARTS) $(KDEDIR)/lib
	$(ARTS.CP) plugins/sound/renderer/arts/csarts.mcopclass $(KDEDIR)/lib/mcop/Arts
	
ifdef DO_DEPEND
dep: $(OUTOS)csarts.dep
$(OUTOS)csarts.dep: $(SRC.ARTSTEST)
	$(DO.DEP)
else
-include $(OUTOS)csarts.dep
endif

endif # ifeq ($(MAKESECTION),targets)
