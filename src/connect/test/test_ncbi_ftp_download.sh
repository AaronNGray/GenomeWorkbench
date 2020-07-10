#! /bin/sh
# $Id: test_ncbi_ftp_download.sh 593199 2019-09-13 19:52:56Z lavr $

outlog()
{
  logfile="$1"
  if [ -s "$logfile" ]; then
    echo "=== $logfile ==="
    if [ "`head -501 $logfile 2>/dev/null | wc -l`" -gt "500" ]; then
      head -200 "$logfile"
      echo '...'
      tail -300 "$logfile"
    else
      cat "$logfile"
    fi
  fi
}

exit_code=0
log=test_ncbi_ftp_download.log
rm -f $log

trap 'echo "`date`."' 0 1 2 3 15

if [ "`echo $FEATURES | grep -c -E '(^| )connext( |$)'`" != "1" ]; then
  n=2
else
  n=3
fi

case "`expr '(' $$ / 10 ')' '%' $n`" in
  0)
    file=
    ;;
  1)
    file='ftp://ftp.freebsd.org/'
    ;;
  2)
    file='ftp://ftp-ext.ncbi.nlm.nih.gov/'
    ;;
esac

:    ${CONN_MAX_TRY:=1} ${CONN_DEBUG_PRINTOUT:=SOME}
export CONN_MAX_TRY       CONN_DEBUG_PRINTOUT

$CHECK_EXEC test_ncbi_ftp_download $file 2>&1
exit_code=$?

test "$exit_code" = "0"  ||  outlog "$log"

exit $exit_code
