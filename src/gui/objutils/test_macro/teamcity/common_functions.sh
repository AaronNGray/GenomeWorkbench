#!/bin/bash

set -o errexit
set -o pipefail
set -x

tc_start()
{
    printf '%s%s%s\n' '#' '#' "teamcity[testStarted name='$1' captureStandardOutput='true']"
}

tc_start_suite()
{
    printf '%s%s%s\n' '#' '#' "teamcity[testSuiteStarted name='$1']"
}

tc_finish()
{
    printf '%s%s%s\n' '#' '#' "teamcity[testFinished name='$1']"
}

tc_finish_suite()
{
    printf '%s%s%s\n' '#' '#' "teamcity[testSuiteFinished name='$1']"
}

tc_failed()
{
    printf '%s%s%s\n' '#' '#' "teamcity[testFailed name='$1' message='$2']"
}

function compare_with_golden_file()
{
   local golden_file=$1
   local current_file=$2
   local tool_args=$4
   local test_name=$5
   local suite_name=$6
   local debug_opts=$7
   
   echo $1 $2 $4 $5 $6 $7

   create_output_file "$tool_args" $2 $3

   local gold_size=`stat -c %s $golden_file`
   local current_size=`stat -c %s $current_file`
   local suite_name="Significant difference"
   local failure=""

   if [ $(($gold_size)) -eq 0 ]; then
       failure="Original output is empty"
   elif [ $(($current_size)) -eq 0 ]; then
       failure="Current output is empty"
   elif [ $(($gold_size/$current_size)) -gt 5 ]; then
       failure="Sizes of current & original outputs significantly differ, skipping compare"
   elif [ $(($current_size/$gold_size)) -gt 5 ]; then
       failure="Sizes of current & original outputs significantly differ, skipping compare"
   else
      failure=""
      suite_name=$5
      diff -u $golden_file $current_file > $diff_folder/$test_name.diff || failure="diff"

      # lets check agains frozen differences
      if [ "$suite_name" == "unit_test_compare_to_psasnconvert" ]; then
        local saved_diff=$input_folder/$test_name.diff
        if [ "$savediff" == "yes" ]; then
          local saved_diff_size=`stat -c %s $diff_folder/$test_name.diff`
          if [ $((saved_diff_size)) -gt 0 ]; then
            cp $diff_folder/$test_name.diff $saved_diff
            failure=""
          fi
        else
          # once we have differences explained and/or reasonable lets freeze it and not report as a difference
          if [ -f $saved_diff ]; then
            diff $diff_folder/$test_name.diff $saved_diff > /dev/null || failure=""
          fi 
        fi
      fi

   fi
   
   tc_start_suite "$suite_name"
   tc_start "$test_name"
   echo "Invoking $TEST_TOOL_NAME with: $tool_args"
   
   echo "diff $golden_file($gold_size) $current_file($current_size)"
   if [ -n "$failure" ]; then
      if [ "$failure" = "diff" ]; then
         tc_failed "$test_name" "Differences seen, showing only 100 lines of $diff_folder/$test_name.diff"
         head -n 100 $diff_folder/$test_name.diff
      else
         tc_failed "$test_name" "$failure"
      fi
   else
      rm $diff_folder/$test_name.diff
   fi
   tc_finish "$test_name"
   tc_finish_suite "$suite_name"
}

function  unit_test_compare_to_previous()
{
   local input_folder=$1
   local input_file=$2

sed -e 's/\#.*//' -e '/^\s*$/d' < "$input_folder/$input_file" |
while read args
do
    local tool_args="$args"
    
	local fname=`make_test_name "$tool_args"`
    local test_name=$fname

    local golden_file=$fname.golden
    local current_file=$tmp_folder/$fname.new
    local cerr_file=$tmp_folder/$fname.cerr

    if [ -f test_cases/$golden_file ]
    then
       compare_with_golden_file test_cases/$golden_file $current_file $cerr_file "$tool_args" $test_name "unit_test_compare_to_previous"
    else
       create_output_file "$tool_args" $golden_file $cerr_file
    fi

done

}

