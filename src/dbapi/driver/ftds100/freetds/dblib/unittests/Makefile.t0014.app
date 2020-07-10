# $Id: Makefile.t0014.app 554151 2017-12-28 18:34:57Z ucko $

APP = db100_t0014
SRC = t0014 common

CPPFLAGS = -DHAVE_CONFIG_H=1 -DNEED_FREETDS_SRCDIR $(FTDS100_INCLUDE) \
           $(ORIG_CPPFLAGS)
LIB      = sybdb_ftds100$(STATIC) tds_ftds100$(STATIC)
LIBS     = $(FTDS100_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

# Needs automatic textptr metadata, gone as of TDS 7.2.
CHECK_CMD  = test-db100 --ms-ver 7.1 --no-auto db100_t0014
CHECK_COPY = t0014.sql data.bin

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
