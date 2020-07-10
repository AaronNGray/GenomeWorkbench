# $Id: Makefile.blob1.app 553128 2017-12-13 16:54:04Z satskyse $

APP = odbc95_blob1
SRC = blob1 common

CPPFLAGS = -DHAVE_CONFIG_H=1 $(FTDS95_INCLUDE) $(ODBC_INCLUDE) $(ORIG_CPPFLAGS)
LIB      = odbc_ftds95$(STATIC) tds_ftds95$(STATIC) odbc_ftds95$(STATIC)
LIBS     = $(FTDS95_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

# Some systems would normally send data too fast(!) to MSSQL servers,
# and proceed to get throttled altogether; the dumping code adds
# enough of a slowdown to keep them out of trouble.
CHECK_CMD  = test-odbc95 --set-env TDSDUMP=/dev/null odbc95_blob1
CHECK_COPY = odbc.ini

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
