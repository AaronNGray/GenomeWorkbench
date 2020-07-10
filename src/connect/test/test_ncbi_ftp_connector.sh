#! /bin/sh
# $Id: test_ncbi_ftp_connector.sh 533905 2017-04-20 18:12:05Z lavr $

: ${CONN_DEBUG_PRINTOUT:=SOME};  export CONN_DEBUG_PRINTOUT

$CHECK_EXEC test_ncbi_ftp_connector
