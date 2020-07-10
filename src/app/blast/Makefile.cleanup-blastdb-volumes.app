# $Id: Makefile.cleanup-blastdb-volumes.app 590894 2019-08-07 14:59:53Z camacho $
##################################

ifeq (0,1)
	APP = cleanup-blastdb-volumes.py
endif

WATCHERS = madden camacho
CHECK_COPY = cleanup-blastdb-volumes.py
CHECK_CMD = python3 -m unittest cleanup-blastdb-volumes
