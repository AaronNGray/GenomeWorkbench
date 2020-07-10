# $Id: Makefile.ctl_sp_databases_ftds.app 553128 2017-12-13 16:54:04Z satskyse $

APP = ctl_sp_databases_ftds
SRC = ctl_sp_databases_ftds dbapi_driver_sample_base_driver

LIB  = dbapi$(STATIC) ncbi_xdbapi_ftds$(STATIC) $(FTDS_LIB) \
       dbapi_driver$(STATIC) $(XCONNEXT) xconnect xncbi
LIBS = $(FTDS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS) $(DL_LIBS)

CPPFLAGS = -DFTDS_IN_USE $(FTDS_INCLUDE) $(ORIG_CPPFLAGS)

CHECK_REQUIRES = in-house-resources
# CHECK_CMD = run_sybase_app.sh ctl_sp_databases_ftds
CHECK_CMD = run_sybase_app.sh ctl_sp_databases_ftds -S MSDEV1 /CHECK_NAME=ctl_sp_databases_ftds-MS
# CHECK_CMD = run_sybase_app.sh ctl_sp_databases_ftds -S MS2008DEV1 /CHECK_NAME=ctl_sp_databases_ftds-MS2008

REQUIRES = FreeTDS

WATCHERS = ucko satskyse
