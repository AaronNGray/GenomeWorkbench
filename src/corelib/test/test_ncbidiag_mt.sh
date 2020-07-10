#! /bin/sh
# $Id: test_ncbidiag_mt.sh 398947 2013-05-09 16:44:15Z grichenk $

DIAG_OLD_POST_FORMAT=
export DIAG_OLD_POST_FORMAT

$CHECK_EXEC test_ncbidiag_mt $@
exit $?
