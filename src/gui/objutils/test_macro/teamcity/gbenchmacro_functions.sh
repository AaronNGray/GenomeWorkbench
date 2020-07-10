#!/bin/bash

set -o errexit
set -o pipefail
set -x

function create_output_file()
{
   local args=$1
   local result=$2
   local cerr=$3
   local intermediate=$result.proc

   if [[ "$args" =~ "read_only" ]]; then
        (cd test_cases && $TEST_TOOL_PATH $args -print $result -logfile $cerr)
   else 
		(cd test_cases && $TEST_TOOL_PATH $args -o $intermediate -logfile $cerr && asn_cleanup -i $intermediate -K bu -o $result)
   fi
}

function make_test_name()
{
    echo "$1" | sed -r -e 's/^-i //' -e 's/[ &=\\.\\(\\),\:\/"]+/-/g' -e 's/--/-/g' -e 's/-/_/g'
}
