# $Id: Makefile.numeric.app 554154 2017-12-28 18:36:40Z ucko $

APP = db100_numeric
SRC = numeric common

CPPFLAGS = -DHAVE_CONFIG_H=1 -DNEED_FREETDS_SRCDIR $(FTDS100_INCLUDE) \
           $(ORIG_CPPFLAGS)
LIB      = sybdb_ftds100$(STATIC) tds_ftds100$(STATIC)
LIBS     = $(FTDS100_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-db100 --no-auto db100_numeric
CHECK_COPY = numeric.sql numeric_2.sql

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
