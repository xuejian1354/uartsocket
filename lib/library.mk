LDLIBS:=libcjson.a libhttp-parser.a

cjson_libsrc:=cjson.c
http-parser_libsrc:=http_parser.c

LDPATH := -L$(TOPDIR)/$(DIR)lib
LDFLAGS += $(patsubst lib%.a,-l%,$(filter %.a, $(LDLIBS)))
LIBS_INC := $(patsubst %,-I%,$(shell find -type d))
