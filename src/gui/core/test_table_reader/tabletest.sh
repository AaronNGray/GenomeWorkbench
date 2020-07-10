#!/bin/bash

# This function reads in all the files (presumably all tables) in a directory
# and then reads in the transformations specification(s) from a subdirectory
# and tries to import them according to that specification.  It then compares
# the output of the new import with the correpsondingly named output in
# the subdirectory.  The naming convention is:
#
# tabletest.sh output  // call script with name of directory of comparson files
# sample2.txt  // a table file in main directory. No name restrictions
# // Files in the subdirectory (created using UI tool):
# output/sample2.txt.locs.asn
# output/sample2.txt.feats.asn
# output/sample2.txt.locs.desc.asn
# output/sample2.txt.feats.desc.asn

#Need a directory where tables that have already been created are saved
comparison_dir=$1

if [ -d "$comparison_dir" ]
then
	comparison_dir=`echo "${comparison_dir}" | sed -e "s/\/*$//" `
    echo "Comparing results to peviously loaded tables in: " $comparison_dir
else
    echo "test_table_reader table-comparison-directory"
    exit 1
fi

for file in *
do	
    if [ -f $file ] 
    then
        # get file name suffix make sure it's not an asn file or shell script
        ftype=`echo $file | grep -oE "[^.]+$"`
        
        if [ "$ftype" != "asn" ] && [ "$file" != `basename $0` ]
        then
            echo Loading: $file	
            seqloc_spec=$comparison_dir"/"$file".locs.desc.asn"
            if [ -f $seqloc_spec ]; then
                #echo $seqloc_spec
                #Load it and compare it to existing transformed files, if any
                test_table_reader -logfile $comparison_dir/log.txt -table-format $seqloc_spec $file
                if [ -f "$file".locs.asn"" ]; then
					diff $file".locs.asn" $comparison_dir"/"$file".locs.asn"
					#rm $file".locs.asn"
                else 
                    echo "Could not produce locations file from: " $file
                fi    
            else
                echo "No locations file: " $seqloc_spec " for: " $file	                       
            fi	
            
            features_spec=$comparison_dir"/"$file".feats.desc.asn"
            if [ -f $features_spec ]; then
                #echo $features_spec
                #Load it and compare it to existing transformed files, if any
                test_table_reader -logfile $comparison_dir/log.txt -table-format $features_spec $file
                if [ -f "$file".feats.asn"" ]; then
					diff $file".feats.asn" $comparison_dir"/"$file".feats.asn"
					#rm $file".feats.asn"
                else 
                    echo "Could not produce features file from: " $file
                fi
            else
                echo "No features file: " $features_spec " for: " $file	                 
            fi	            
        fi
    fi
done
	

	
	