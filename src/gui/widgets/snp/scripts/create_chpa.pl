#!/usr/bin/perl -w

# Program to combine GWAS chip dumps with analysis files
# Author: Melvin Quintos
# Date:   2008-03-27
# History: - 
#

use strict;
use warnings;
use Getopt::Std;

####################################################################
# Usage
####################################################################
sub usage()
{
	print STDERR << "EOF";
	
usage: $0 -o output_file [-r column] [-p column] chpb_file analysis_file

This program takes a GWAS chip batch file and an analysis file and creates a
combined GWAS analysis file (chpa file).

Options:
-h      : this help message
-o      : sets the name of the output file (recommend extension of .chpa)
-r      : rsid column number in analysis file (1-based)
-p      : pvalue column number in analysis file (1-based)

example: $0 -o AREDS_1.chr1.chpa  ILMN_Human-1_chr1.chpb pha000001.v1.p1.chr1.txt

EOF
    exit;
}

####################################################################
# Global Variables
####################################################################
my %opts;
my %mapPvals;
my $col_rsid, my $col_pvalue;
my $outputfn;
my $ifn_chpb, my $ifn_analysis;
my @header;

####################################################################
# Functions
####################################################################

# Parse command-line options
sub init() 
{	
	getopts( 'ho:v:r:p:', \%opts ) or usage();
	usage() if $opts{h};
}

sub verify_args()
{
	$col_rsid       = ($opts{r}) ? $opts{r} : 1;
	$col_pvalue     = ($opts{p}) ? $opts{p} : 3;
    $outputfn       = $opts{o};
    
    # validate variables
    if (         (scalar(@ARGV) != 2)             # correct number of args 
          or    !(defined($outputfn))             # name of output file is given
          or    !($col_rsid   =~ /^[1-9]\d*$/)    # col is a positive number
          or    !($col_pvalue =~ /^[1-9]\d*$/)    # col is a positive number
       ) 
    { 
        usage();
    }    
    else {
	    $ifn_chpb       = $ARGV[0];
	    $ifn_analysis   = $ARGV[1];
    }
}

# use 1-based column numbering (because it's easier to count that way)
sub get_cols # params: $line, $col_array
{
	my ($line, $col_list) = @_;
	my @result;
	
	my @items = split(/\t/, $line);
	foreach my $col (@{$col_list})
	{
		push @result, $items[$col-1];  # 1-based columns
	}
	
	return @result;
}

sub read_chpb_file_version1()
{
	open IFILE, "$ifn_chpb";
	my @cols_of_interest = (2);  # rsid is the 2nd column
	
	my $reading_header = 1;
	
	my $line_num = 0;
	foreach my $line (<IFILE>) {
		$line_num++;
	
	    if ($reading_header) {	    	
	    	if ($line =~ /^ss#/) { 
	    		# Header information stops at row with 'ss#'
	    		$reading_header = 0;
	    	}
	    	else {
	    		push @header, $line;
	    	}
	    }	
	    
		# skip all lines that don't have a proper 'ss' number at start of row
		next unless $line =~ /^ss\d+/;		
		chomp $line;
		
		# grab the rsid column
        my ($rsid) = get_cols( $line, \@cols_of_interest );      
        
        if (!defined($rsid)) {        	
            die "Unknown value in chpb file at line: $line_num\n";
        }
          
        # trim string, then place in map with a default value of -1
        $rsid =~ s/^\s+|\s+$//g;
        $mapPvals{$rsid} = -1;
	}
		
	close IFILE;
}

sub read_analysis_file()
{	
    open IFILE, "$ifn_analysis";
    my @cols_of_interest = ($col_rsid, $col_pvalue);
    
    my $reading_header = 1;
    my $line_num = 0;
    foreach my $line (<IFILE>) {
        $line_num++;
                
        if ($reading_header) {          
            if ($line =~ /^# Marker accession/) { 
                # Header information stops at row with '# Marker accesssion'
                $reading_header = 0;
            }
            else {
                push @header, $line;
            }
        }
           
        # skip all lines that are comments or blank lines
        next if $line =~ /^\s*#|^\s*$/;      
        chomp $line;
        
        # for every valid line (e.g. not a comment or blank space)
        my ($rsid, $pval) = get_cols( $line, \@cols_of_interest );      
        
        if (!defined($rsid)) {          
            die "Unknown value in file at line: $line_num\n";
        }
          
        # trim string, then place in map with a default value of -1
        $rsid =~ s/^\s+|\s+$//g;
        $mapPvals{$rsid} = $pval;
    }
        
    close IFILE;	
}

# TODO: this is specific to version 1 of chpb file format
sub write_chpa_file()
{
	# open up new file
	open OFILE, ">$outputfn";
	my @cols_of_interest = (2);  # rsid is 2nd column in VERSION=1
	
	# write header information
	print OFILE @header;
	print OFILE "ss#\trs#\tloc_snp_id\tchrom\tchrom_pos\trsToss_orient\tssTochrom_orient\tweight\tbitfield\tpvalue\n";
	
	# read the chpb file again
	open IFILE, "$ifn_chpb";
	foreach my $line (<IFILE>) {
		chomp $line;
		
		# if the line is a data line, then do a pvalue lookup.
		# otherwise ignore
		if ($line =~ /^ss\d+/) {
			my ($rsid) = get_cols($line, \@cols_of_interest);
			my $pval = $mapPvals{$rsid};
			if (defined($pval) and $pval != -1) {
                $line = "$line\t$pval";	
                print OFILE $line, "\n";			
			}			
		}
	}
	close IFILE;
	
	close OFILE;
	
}

####################################################################
# Program flow
####################################################################

init();
verify_args();
# TODO: determine which chpb file version we're using
read_chpb_file_version1();
read_analysis_file();
write_chpa_file();

print "Done!\n";


