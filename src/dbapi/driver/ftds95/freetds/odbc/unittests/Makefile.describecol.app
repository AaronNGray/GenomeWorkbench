# $Id: Makefile.describecol.app 553128 2017-12-13 16:54:04Z satskyse $

APP = odbc95_describecol
SRC = describecol common parser

CPPFLAGS = -DHAVE_CONFIG_H=1 -DNEED_FREETDS_SRCDIR $(FTDS95_INCLUDE) \
           $(ODBC_INCLUDE) $(ORIG_CPPFLAGS)
LIB      = odbc_ftds95$(STATIC) tds_ftds95$(STATIC) odbc_ftds95$(STATIC)
LIBS     = $(FTDS95_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-odbc95 odbc95_describecol
CHECK_COPY = odbc.ini describecol.in

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
