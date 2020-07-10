#!/usr/bin/perl -w
#

# Batch processing of CHPB and analysis files
# Author: Melvin Quintos
# Date:   2008-03-27
# History: - 
#

use strict;
use warnings;
use Getopt::Std;
use File::Find;
use Cwd;

####################################################################
# Usage
####################################################################
sub usage()
{
    print STDERR << "EOF";
    
usage: $0 [-d directory] prefix_chpb prefix_analysis

This program goes through and creates chpa data for all analysis and chpb 
files in specified directory.

The chpb files must be in the format of <name>chr[1-22,x,y].chpb
The analysis files must be in the format of <name>chr[1-22,x,y].txt

Options:
-h      : this help message
-d      : sets the path to chpb and analysis files (must be in same folder)
          defaults to current directory 

example 1: $0 ILMN_Human-1_
example 2: $0 -d path_to_files ILMN_Human-1_

EOF
    exit;
}

####################################################################
# Global Variables
####################################################################
my %opts;
my %mapChrChpb;  # map of chromosome to chpb file
my $workdir;
my $initdir;
my $chpb_prefix;

####################################################################
# Functions
####################################################################

# Parse command-line options
sub init() 
{   
    getopts( 'hd:', \%opts ) or usage();
    usage() if $opts{h};
}

# setup initial program conditions
sub verify_args()
{	
    $workdir = ($opts{d}) ? $opts{d} : '.';
    $initdir = cwd;
    
    if (scalar(@ARGV)!=1) {
    	usage();
    }
    else {
        $chpb_prefix = $ARGV[0];	
    }
}

# given an array of files, return only an array of analysis files
sub filter_analysis
{
    my @files;
    foreach my $file (@_) {
    	push @files, $file if $file =~ /\.txt$/i;
    }    	
	@files;
}

# given an array of files, return only an array of chpb files
sub filter_chpb
{
	my @files;
	foreach my $file (@_) {
		# check extension
		if ($file =~ /\.chpb$/i) {
			# check prefix of filename)
			if ( substr($file, 0, length($chpb_prefix)) eq $chpb_prefix) {
				push @files, $file;
			}
		} 		
	}
	@files;
}

# Store the chpb file by the chromosome it represents
sub process_chpb
{
	my $fn = $_;
	if (-f $fn) {                        # if a file
		if ($fn =~ /(chr.+)\.chpb$/) {   # extract the chromsome name from file
			$mapChrChpb{$1} = $fn;       # store in map
		}		
	}
}

# Process each analysis file against the proper chromosome
sub process_analysis
{
	my $fn = $_;
	if (-f $fn) {                  # if a file
        if ($fn =~ /(chr.+)\./) {  # extract chomosome name
            # if found in map, then process the files           
        	my $chpb_file = $mapChrChpb{$1};
        	if (defined($chpb_file)) {
        		my $new_file = $fn;
        		$new_file =~ s/\.txt$/\.chpa/i;
        		print "Processing $new_file ... ";
        		my $exeCmd = qq{"$initdir/create_chpa.pl" -o $new_file $chpb_file $fn};
        		system($exeCmd);
        	}
        }                    	  
	}	
}

####################################################################
# Program flow
####################################################################

init();
verify_args();

# Process the names of the CHPB files
find({wanted=>\&process_chpb, preprocess=>\&filter_chpb}, $workdir);

# Process the names of analysis files
find({wanted=>\&process_analysis, preprocess=>\&filter_analysis}, $workdir);

print "Done!\n";
