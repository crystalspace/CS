#------------------------------------------------------------------------------
# Map File Parser plugin makefile
#------------------------------------------------------------------------------
DESCRIPTION.xml = Crystal Space XML document system (using libxml2)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make xml     Make the $(DESCRIPTION.xml)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: xml xmlclean
all plugins: xml

xml:
	$(MAKE_TARGET) MAKE_DLL=yes
xmlclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/documentsystem/xml

ifeq ($(USE_PLUGINS),yes)
  XML = $(OUTDLL)/xml$(DLL)
  LIB.XML = $(foreach d,$(DEP.XML),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(XML)
else
  XML = $(OUT)/$(LIB_PREFIX)xml$(LIB)
  DEP.EXE += $(XML)
  SCF.STATIC += xml
  TO_INSTALL.STATIC_LIBS += $(XML)
endif

# XXX: Needs a test
LIBXML2.CFLAGS = -I/usr/include/libxml2
LIBXML2.LFLAGS = -L/usr/lib -lxml2

INC.XML = $(wildcard plugins/documentsystem/xml/*.h)
SRC.XML = $(wildcard plugins/documentsystem/xml/*.cpp)
OBJ.XML = $(addprefix $(OUT)/,$(notdir $(SRC.XML:.cpp=$O)))
DEP.XML = CSUTIL CSTOOL CSSYS CSUTIL CSGEOM CSTOOL CSGFX

MSVC.DSP += XML
DSP.XML.NAME = xml
DSP.XML.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xml xmlclean
xml: $(OUTDIRS) $(XML)

$(OUT)/%$O: plugins/documentsystem/xml/%.cpp
	$(DO.COMPILE.CPP) $(LIBXML2.CFLAGS)

$(XML): $(OBJ.XML) $(LIB.XML)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIBXML2.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: xmlclean
xmlclean:
	-$(RM) $(XML) $(OBJ.XML)

ifdef DO_DEPEND
dep: $(OUTOS)/xml.dep
$(OUTOS)/xml.dep: $(SRC.XML)
	$(DO.DEP1) $(LIBXML2.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/xml.dep
endif

endif # ifeq ($(MAKESECTION),targets)
