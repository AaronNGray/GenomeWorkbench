#################################
# $Id: Makefile.blast_macros.mk 565266 2018-06-08 17:19:08Z lavr $
# This file contains macro definitions for using libraries maintained by the
# BLAST TEAM
# Author:  Christiam Camacho (camacho@ncbi.nlm.nih.gov)
#################################


BLAST_FORMATTER_MINIMAL_LIBS = xblastformat align_format taxon1 blastdb_format \
    gene_info $(XFORMAT_LIBS) xalnmgr blastxml blastxml2 xcgi xhtml
# BLAST_FORMATTER_LIBS = $(BLAST_FORMATTER_MINIMAL_LIBS)
BLAST_FORMATTER_LIBS = $(BLAST_INPUT_LIBS)
BLAST_DB_DATA_LOADER_LIBS = ncbi_xloader_blastdb_rmt ncbi_xloader_blastdb
BLAST_INPUT_LIBS = blastinput \
    $(BLAST_DB_DATA_LOADER_LIBS) $(BLAST_FORMATTER_MINIMAL_LIBS)

# Libraries required to link against the internal BLAST SRA library
BLAST_SRA_LIBS=blast_sra $(SRAXF_LIBS) vxf $(SRA_LIBS)

# BLAST_FORMATTER_LIBS and BLAST_INPUT_LIBS need $BLAST_LIBS
BLAST_LDEP = xalgoblastdbindex composition_adjustment \
             xalgodustmask xalgowinmask seqmasks_io seqdb blast_services xalnmgr \
             xobjutil $(OBJREAD_LIBS) xnetblastcli xnetblast blastdb scoremat tables $(LMDB_LIB)
BLAST_LIBS = proteinkmer xblast $(BLAST_LDEP)

# BLAST additionally needs xconnect $(SOBJMGR_LIBS) or $(OBJMGR_LIBS)

BLAST_THIRD_PARTY_INCLUDE       = $(LMDB_INCLUDE)
BLAST_THIRD_PARTY_LIBS          = $(LMDB_LIBS)
BLAST_THIRD_PARTY_STATIC_LIBS   = $(LMDB_STATIC_LIBS)

