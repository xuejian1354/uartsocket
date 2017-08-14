ifeq ($(TOPDIR),)
TOPDIR ?= $(patsubst %/,%,$(dir $(shell pwd)))
endif

BIN_DIR :=$(addsuffix lib/,$(TOPDIR)/$(DIR))

include $(TOPDIR)/config.mk
include $(TOPDIR)/lib/library.mk

ifneq ("$(INCLUDE)","")
LIBS_INC :=$(INCLUDE)
endif

TARGET_LIBS:=$(addprefix $(BIN_DIR),$(patsubst %,%.a,$(patsubst %.a,%,$(LDLIBS))))

$(foreach libsrc,$(LDLIBS), $(eval LIBSRC_FILES += $($(patsubst lib%.a,%_libsrc,$(libsrc)))))
LIBSRC_TARGETS := $(addprefix $(BIN_DIR),$(patsubst %.c,%.o,$(LIBSRC_FILES)))

all:$(TARGET_LIBS)
.PHONY: all clean

LIB_HEARDS:=$(shell ls $(patsubst %,$(TOPDIR)/%,lib/*.h))

$(TARGET_LIBS):$(LIBSRC_TARGETS)
	$(call echocmd,AR, $(patsubst $(TOPDIR)/%,%,$@), \
	  $(TARGET_AR) crs $@ $(patsubst %.c,$(BIN_DIR)%.o,$($(patsubst lib%.a,%_libsrc,$(notdir $@)))))

$(LIBSRC_TARGETS):$(LIBSRC_FILES) $(LIB_HEARDS)
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CC, $(patsubst $(TOPDIR)/%,%,$@), \
	  $(TARGET_CC) $(DMACRO) $(LIBS_INC) -Werror -w -O2 -o $@ -c $(patsubst $(BIN_DIR)%.o,%.c,$@))

clean:
	if [ -d $(BIN_DIR) ]; then find -L $(BIN_DIR) -name "*.[oa]" | xargs $(RM); fi; 
	-if [ -d $(BIN_DIR) ]; then rmdir $(BIN_DIR); fi;
	-if [ -d $(TOPDIR)/$(DIR) ]; then rmdir $(TOPDIR)/$(DIR); fi;
