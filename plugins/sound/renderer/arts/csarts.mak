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

.PHONY: csarts csartsclean csartsinstall csartstest

all apps: csarts csartstest

csartsinstall csarts csartstest:
	$(MAKE_TARGET) MAKE_DLL=yes
csartsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp %.cc plugins/sound/renderer/arts

CSARTS.DIR = plugins/sound/renderer/arts
MCOP.LIBDIR = $(KDEDIR)/lib
MCOP.INCDIR = $(KDEDIR)/include/arts

# a few variables for the testapp
ARTSTEST.EXE = artstest$(EXE)
SRC.ARTSTEST = $(CSARTS.DIR)/tt.cpp
OBJ.ARTSTEST = $(addprefix $(OUT),$(notdir $(SRC.ARTSTEST:.cpp=$O)))

# Unlike CS interfaces, mcop interfaces need to be precompiled and are, in the end,
# represemted by 3 classes (foo_skel, foo_stub, foo)
# Our final renderer plugin will only access foos and not their implementation, so we create
# two libraries: csarts and csart_idl, where csarts_idl only holds the interface classes
# The final renderer will be linked with csarts_idl, whereas the implementation library csarts will live
# in a path that is searched by mcop's objectmanager

# these are the files we feed the MCOP idl compiler with
IDL.CSARTS = $(wildcard $(CSARTS.DIR)/*.idl)

# the mcop compiler will spill out the following files, we keep track of them separatly to link those
# (and only those) into the csarts_idl library
SRC.CSARTS.IDL = $(IDL.CSARTS:.idl=.cc)

# Next we will need the implementation sources for the interfaces.
# We introduce the convention, that the implementation sources are named *_impl.cpp.
# We keep track of them explicitly because we'll send them through libtool.
SRC.CSARTS.IMPL = $(wildcard $(CSARTS.DIR)/*_impl.cpp) 

# what follows are the source only needed by the csarts renderer itself
SRC.CSARTS = $(filter-out $(SRC.CSARTS.IMPL) $(SRC.ARTSTEST), $(wildcard $(CSARTS.DIR)/*.cpp))

OBJ.CSARTS.IDL = $(addprefix $(OUT),$(notdir $(SRC.CSARTS.IDL:.cc=$O)))
OBJ.CSARTS.IMPL = $(addprefix $(OUT),$(notdir $(SRC.CSARTS.IMPL:.cpp=$O)))

# the others use the usual suffix
OBJ.CSARTS = $(addprefix $(OUT),$(notdir $(SRC.CSARTS:.cpp=$O)))

# the lib that will be used by mcop and that ends up in a directory visible by mcop
CSARTS.IMPL.LIB = $(OUT)$(LIB_PREFIX)csarts.la

# the iterface lib we'll link to our csarts renderer
CSARTS.IDL.LIB  = $(OUT)$(LIB_PREFIX)csarts_idl$(LIB_SUFFIX)

# the csarts renderer plugin
CSARTS  = $(OUTDLL)csarts$(DLL)

# common lib we need to link
LIB.CSARTS.COMMON = -lartsflow -lartsflow_idl -lmcop

# additional libs we need to link the csarts plugin
LIB.LINK.CSARTS =-L$(MCOP.LIBDIR) $(LIB.CSARTS.COMMON) -lsoundserver_idl -lstdc++

DEP.CSARTS = CSGEOM CSUTIL CSSYS CSUTIL CSARTS.IDL
LIB.CSARTS = $(foreach d,$(DEP.CSARTS),$($d.LIB))

ARTS.CXX = libtool --mode=compile g++
ARTS.LD  = libtool --mode=link g++
ARTS.CP  = libtool --mode=install cp
ARTS.RM  = libtool --mode=uninstall $(RM)
ARTS.LDFLAGS= -module -rpath $(MCOP.LIBDIR) -L$(MCOP.LIBDIR) $(LIB.CSARTS.COMMON) -ldl -lartsmodules

CSARTS.LA.OBJ = $(OBJ.CSARTS.IDL:$O=.lo) $(OBJ.CSARTS.IMPL:$O=.lo)

TO_INSTALL.EXE += $(ARTSTEST.EXE)

#MSVC.DSP += ARTSTEST
#DSP.ARTSTEST.NAME = artstest
#DSP.ARTSTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csarts csartsclean csartsinstall csartstest

csarts: $(OUTDIRS) $(CSARTS) $(CSARTS.IMPL.LIB)
csartstest: csarts $(ARTSTEST.EXE)

clean: csartsclean

$(ARTSTEST.EXE): $(OBJ.ARTSTEST) $(LIB.CSARTS) 
	$(DO.LINK.CONSOLE.EXE) -L$(MCOP.LIBDIR)

$(CSARTS): $(OBJ.CSARTS) $(LIB.CSARTS)
	$(DO.PLUGIN) $(LIB.LINK.CSARTS)

$(CSARTS.IDL.LIB): $(OBJ.CSARTS.IDL)
	$(DO.LIBRARY)

$(CSARTS.IMPL.LIB): $(CSARTS.LA.OBJ)
	$(ARTS.LD) -o $(CSARTS.IMPL.LIB) $(ARTS.LDFLAGS) $(CSARTS.LA.OBJ)

$(OUT)%.lo: $(CSARTS.DIR)/%.cc
	$(ARTS.CXX) -I$(MCOP.INCDIR) -c $^ -o $@

$(OUT)%.lo: $(CSARTS.DIR)/%.cpp
	$(ARTS.CXX) -I$(MCOP.INCDIR) -c $^ -o $@

$(OUT)%$O: $(CSARTS.DIR)/%.cpp
	$(DO.COMPILE.CPP) -I$(MCOP.INCDIR)

$(OUT)%$O: $(CSARTS.DIR)/%.cc
	$(DO.COMPILE.CPP) -I$(MCOP.INCDIR)

$(CSARTS.DIR)/%.h $(CSARTS.DIR)/%.cc: $(CSARTS.DIR)/%.idl
	mcopidl -I$(MCOP.INCDIR) $(<<)
	mv $(basename $(@F)).* $(CSARTS.DIR)

csartsclean: 
	$(ARTS.RM) $(CSARTS.IMPL.LIB)
	$(ARTS.RM) $(CSARTS.LA.OBJ)
	-$(RM) $(CSARTS.IDL.LIB) $(CSARTS) $(OBJ.CSARTS) $(OBJ.CSARTS.IDL) \
	$(SRC.CSARTS.IDL) $(ARTSTEST.EXE) $(OBJ.ARTSTEST)

csartsinstall: $(CSARTS.IMPL.LIB)
	$(ARTS.CP) $(CSARTS.IMPL.LIB) $(MCOP.LIBDIR)
	$(ARTS.CP) $(CSARTS.DIR)/csarts.mcopclass $(MCOP.LIBDIR)/mcop/Arts
	
ifdef DO_DEPEND
dep: $(OUTOS)csarts.dep
$(OUTOS)csarts.dep: $(SRC.ARTSTEST)
	$(DO.DEP)
else
-include $(OUTOS)csarts.dep
endif

endif # ifeq ($(MAKESECTION),targets)
