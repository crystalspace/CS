#
# installation makefile
#

INSTALL_LOG=$(INSTALL_DIR)/install.log

$(INSTALL_DIR) $(INSTALL_DIR)/data $(INSTALL_DIR)/bin $(INSTALL_DIR)/lib \
$(INSTALL_DIR)/include $(INSTALL_DIR)/data/config:
	$(MKDIR)

install_config: $(INSTALL_DIR)/data/config
	$(CP) $(TO_INSTALL.CONFIG) $(INSTALL_DIR)/data/config

install_data: $(INSTALL_DIR)/data
	$(CP) $(TO_INSTALL.DATA) $(INSTALL_DIR)/data

install_dynamiclibs: $(INSTALL_DIR)/lib
	$(CP) $(TO_INSTALL.DYNAMIC_LIBS) $(INSTALL_DIR)/lib

install_staticlibs: $(INSTALL_DIR)/lib
	$(CP) $(TO_INSTALL.STATIC_LIBS) $(INSTALL_DIR)/lib

install_exe: $(INSTALL_DIR)/bin
	$(CP) $(TO_INSTALL.EXE) $(INSTALL_DIR)/bin

INSTALL_INCLUDE.FILES = $(wildcard include/*h include/*/*h include/*/*/*h)
# take all .h files in include/, take their directory parts, sort those,
# and remove trailing /, then add the INSTALLDIR prefix
INSTALL_INCLUDE.DIR = $(addprefix $(INSTALL_DIR)$/,  \
  $(patsubst %/, %,$(sort $(dir $(INSTALL_INCLUDE.FILES)))))

$(INSTALL_INCLUDE.DIR):
	$(MKDIR)

install_include: $(INSTALL_DIR)/include $(INSTALL_INCLUDE.DIR)
	@echo $(INSTALL_INCLUDE.DIR)
	#$(CP)  $(INSTALL_DIR)/include

