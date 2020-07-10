# $Id: Makefile.http_connector_hit.app 568792 2018-08-10 15:48:03Z lavr $

APP = http_connector_hit
SRC = http_connector_hit
LIB = connssl connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = http_connector_hit www.ncbi.nlm.nih.gov 443 /Service/bounce.cgi 'arg1+arg2+arg3' '-' 'CAF-CHECK: YES!' /CHECK_NAME=http_connector_hit

WATCHERS = lavr
