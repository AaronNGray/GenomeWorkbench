# $Id: Makefile.t0002.app 553128 2017-12-13 16:54:04Z satskyse $

APP = tds95_t0002
SRC = t0002 common

CPPFLAGS = -DHAVE_CONFIG_H=1 $(FTDS95_INCLUDE) $(ORIG_CPPFLAGS)
LIB      = tds_ftds95$(STATIC)
LIBS     = $(FTDS95_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-tds95 tds95_t0002

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
