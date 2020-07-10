# $Id: Makefile.socket_io_bouncer.app 448737 2014-10-08 16:23:00Z lavr $

APP = socket_io_bouncer
SRC = socket_io_bouncer
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

WATCHERS = lavr
