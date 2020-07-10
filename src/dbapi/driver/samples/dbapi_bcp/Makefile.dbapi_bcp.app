# $Id: Makefile.dbapi_bcp.app 583697 2019-04-01 15:00:36Z ucko $

APP = dbapi_bcp
SRC = dbapi_bcp

LIB  = dbapi_sample_base$(STATIC) $(DBAPI_CTLIB) $(DBAPI_ODBC) \
       ncbi_xdbapi_ftds ncbi_xdbapi_ftds95 $(FTDS95_LIB) \
       ncbi_xdbapi_ftds100 $(FTDS100_LIB) dbapi_driver$(STATIC) \
       $(XCONNEXT) xconnect xutil xncbi

LIBS = $(SYBASE_LIBS) $(SYBASE_DLLS) $(ODBC_LIBS) $(FTDS95_LIBS) \
       $(FTDS100_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_COPY = dbapi_bcp.ini

WATCHERS = ucko satskyse
