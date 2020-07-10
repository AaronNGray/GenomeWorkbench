#! /bin/sh
# $Id: test_ncbi_download.sh 574070 2018-11-05 20:24:50Z lavr $

fw="`expr $$ '%' 1`"

if [ "$fw" = "1" ]; then
  CONN_FIREWALL=TRUE
  export CONN_FIREWALL
fi

$CHECK_EXEC test_ncbi_download downtest
