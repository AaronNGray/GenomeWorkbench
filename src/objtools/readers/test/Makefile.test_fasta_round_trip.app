#################################
# $Id: Makefile.test_fasta_round_trip.app 526134 2017-01-31 19:04:47Z foleyjp $
#################################

APP = test_fasta_round_trip
SRC = test_fasta_round_trip

LIB = $(OBJREAD_LIBS) xobjutil $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = ucko gotvyans foleyjp

CHECK_CMD = test_fasta_round_trip.sh
CHECK_COPY = test_fasta_round_trip.sh test_fasta_round_trip_data
