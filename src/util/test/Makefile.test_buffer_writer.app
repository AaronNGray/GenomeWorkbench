# $Id: Makefile.test_buffer_writer.app 192795 2010-05-27 14:41:33Z satskyse $
# Author:  Sergey Satskiy (satskyse@ncbi.nlm.nih.gov)

# Build buffer writer test application "test_buffer_writer"
#################################

APP = test_buffer_writer
SRC = test_buffer_writer
LIB = xutil test_boost xncbi
REQUIRES = Boost.Test.Included

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

CHECK_CMD =

WATCHERS = satskyse
