LDLIBS := libdatastruct.a

datastruct_libsrc := strings_t.c mincode.c tpool.c

LDPATH := -L$(TOPDIR)/$(DIR)lib
LDFLAGS += $(patsubst lib%.a,-l%,$(filter %.a, $(LDLIBS)))
