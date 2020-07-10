# $Id: Makefile.update_blastdb.app 597907 2019-12-02 14:54:44Z camacho $
##################################

ifeq (0,1)
	APP = update_blastdb.pl
endif

WATCHERS = madden camacho
CHECK_COPY = update_blastdb.pl
CHECK_CMD = perl -c update_blastdb.pl
CHECK_CMD = perl update_blastdb.pl --showall
CHECK_CMD = perl update_blastdb.pl --showall --source gcp
CHECK_CMD = perl update_blastdb.pl --showall --source ncbi
