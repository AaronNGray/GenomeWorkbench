# $Id: Makefile.gbench_monitor.app 23037 2011-01-25 17:24:27Z katargir $

APP = gbench_monitor
SRC = gbench_monitor

LIB  = ncbi_xcache_netcache xconnect xutil xncbi

LIBS = $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

REQUIRES = gbench WinMain
