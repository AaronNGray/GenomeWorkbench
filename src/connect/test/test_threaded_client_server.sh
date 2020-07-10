#! /bin/sh
# $Id: test_threaded_client_server.sh 533905 2017-04-20 18:12:05Z lavr $

status=0
port="565`expr $$ % 100`"

test_threaded_server -port $port </dev/null &
server_pid=$!
trap 'kill $server_pid' 0 1 2 3 15

sleep 2
$CHECK_EXEC test_threaded_client -port $port -requests 34 || status=1

exit $status
