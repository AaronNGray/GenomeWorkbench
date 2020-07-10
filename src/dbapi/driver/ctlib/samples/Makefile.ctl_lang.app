# $Id: Makefile.ctl_lang.app 573535 2018-10-29 18:54:02Z ucko $

APP = ctl_lang
SRC = ctl_lang dbapi_driver_sample_base_ctl

LIB  = ncbi_xdbapi_ctlib$(STATIC) dbapi_driver$(STATIC) $(XCONNEXT) xconnect xncbi
LIBS = $(SYBASE_LIBS) $(SYBASE_DLLS) $(NETWORK_LIBS) $(ORIG_LIBS) $(DL_LIBS)

CPPFLAGS = -I$(includedir)/dbapi/driver/ctlib $(SYBASE_INCLUDE) $(ORIG_CPPFLAGS)

REQUIRES = Sybase

CHECK_REQUIRES = in-house-resources
CHECK_COPY = ctl_lang.ini
# CHECK_CMD = run_sybase_app.sh ctl_lang /CHECK_NAME=ctl_lang
CHECK_CMD = run_sybase_app.sh ctl_lang -S DBAPI_SYB160_TEST /CHECK_NAME=ctl_lang-SYB16-2K
CHECK_CMD = run_sybase_app.sh ctl_lang -S DBAPI_DEV16_16K /CHECK_NAME=ctl_lang-SYB16-16K

WATCHERS = ucko satskyse
