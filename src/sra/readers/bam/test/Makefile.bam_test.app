#################################
# $Id: Makefile.bam_test.app 590760 2019-08-04 21:12:21Z vakatov $
# Author:  Eugene Vasilchenko
#################################

# Build application "sra_test"
#################################

APP = bam_test
SRC = bam_test bam_test_common

LIB =   bamread $(BAM_LIBS) xobjreadex $(OBJREAD_LIBS) xobjutil xobjsimple \
        $(OBJMGR_LIBS)
LIBS =  $(GENBANK_THIRD_PARTY_LIBS) $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects

CPPFLAGS = $(ORIG_CPPFLAGS) $(SRA_INCLUDE)

CHECK_CMD = bam_test -file NA10851.chrom20.ILLUMINA.bwa.CEU.low_coverage.20111114.bam -no_index /CHECK_NAME=bam_test_no_index0
CHECK_CMD = bam_test -file NA10851.chrom20.ILLUMINA.bwa.CEU.low_coverage.20111114.bam /CHECK_NAME=bam_test_no_index1
CHECK_CMD = bam_test -file NA10851.chrom20.ILLUMINA.bwa.CEU.low_coverage.20111114.bam -refseq 20 /CHECK_NAME=bam_test
CHECK_CMD = bam_test -refseq GL000207.1 -refwindow 1 /CHECK_NAME=bam_test_none
CHECK_CMD = bam_test -file 1k.unaligned.bam /CHECK_NAME=bam_test_none
CHECK_CMD = bam_test -file header-only.bam /CHECK_NAME=bam_test_none
#CHECK_CMD = bam_test -file minimal.bam /CHECK_NAME=bam_test_none
CHECK_CMD = bam_test -file 1k.unaligned.bam -refseq GL000207.1 -refwindow 1 /CHECK_NAME=bam_test_q_none
CHECK_CMD = bam_test -file header-only.bam -refseq GL000207.1 -refwindow 1 /CHECK_NAME=bam_test_q_none

CHECK_REQUIRES = in-house-resources -MSWin -Solaris

WATCHERS = vasilche ucko
