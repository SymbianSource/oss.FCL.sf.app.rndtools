#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:  
#

use strict;

use Getopt::Std;
use Digest::MD5 qw( md5 );
use File::Basename;
use File::Copy;
use File::Find;
use File::Path;

my @changed_files=();
my $dir_1_length;
my $dir_2_length;
my $directory1;
my $directory2;
my %list1=();
my %list2=();
my %opts=();
my $switch= 0;
my $result_dir;

###################################################
##  usage()                                      ##
##  Prints the usage                             ##
###################################################
sub usage
{
  print "\nUsage\n\tsnapshot_compare_dirs.pl -1 <original dir> -2 <updated dir>  -r <result dir>\n";
  print "\nWhere\n\t-1  Original directory\n\t-2  Updated directory\n\t-r  Result directory\n";
  exit;
}

###################################################
##  get_param()                                  ##
##  Gets command line parameters                 ##
###################################################
sub get_param
{
  getopts('h1:2:r:', \%opts);

  if ( $opts{'h'} ) {
    usage();
  }

  if ( $opts{'1'} ) {
    $directory1 = $opts{'1'};
    if ( ! -d $directory1 ) {
      print "\nError:\t Directory \"$directory1\" doesn't exist\n";
      exit;
    }
  } else {
    usage();
  }

  if ( $opts{'2'} ) {
    $directory2 = $opts{'2'};
    if ( ! -d $directory2 ) {
      print "\nError:\t Directory \"$directory2\" doesn't exist\n";
      exit;
    }
  } else {
    usage();
  }

  if ( $opts{'r'} ) {
    $result_dir = $opts{'r'};
    if ( ! -d $result_dir ) {
        mkdir $result_dir;
    }
  } else {
    usage();
  }

}

###################################################
##  calculate_hash()  Reads file content and     ##
##                    calculates MD5 hash        ##
###################################################
sub calculate_hash
{
  my ( $file_name ) = @_;

  open( FILE, $file_name ) or die "Error: Cannot open $file_name\n";
  my @file_stat = stat FILE;
  binmode( FILE );

  my ( $buffer, $hash_value );
  read( FILE, $buffer, $file_stat[7] );

  close FILE;

  $hash_value = md5( $buffer );

  return $hash_value;
}

###################################################
##  copy_file()                                  ##
##  Copies files with same directory structure   ##
###################################################
sub copy_file
{
    my ( $file1, $file2 ) = @_;
    my $dir_name = dirname( $file2 );

    mkpath( $dir_name, 0, 0777 );

    copy( $file1, $file2 );
}

###################################################
##  process_files()                              ##
##  Indexes files                                ##
###################################################
sub process_files
{
  my $file = $File::Find::name;

  if ( -f $file ) {
    if ($switch eq 0 ) {
      $file = substr( $file, $dir_1_length );
      $list1{$file} = 1;;
    } else {
      $file = substr( $file, $dir_2_length );
      $list2{$file} = 1;
    }
  }
}

###################################################
##  main()                                       ##
##  Main function                                ##
###################################################
sub main
{
  get_param();

  $dir_1_length = length( $directory1 );
  $dir_2_length = length( $directory2 );

  print "Info:\tProcessing files. This may take some time\n";

  $switch = 0;
  find( \&process_files, $directory1 );

  $switch = 1;
  find( \&process_files, $directory2 );

  print "Info:\tFiles only exist in \"$directory2\" :\n\n";
  my $counter = 0;
  my $file;
  my $key;
  my $hash1;
  my $hash2;

  foreach $key ( sort keys %list2 ){
    if ( $list1{$key} ne 1 ) {
       $key =~ s/\//\\/g;
       print "\t$key\n";
       copy_file( "$directory2$key", "$result_dir$key" );
       $counter++;
    } else {
       $hash1 = calculate_hash("$directory1$key");
       $hash2 = calculate_hash("$directory2$key");
       if ($hash1 ne $hash2) {
         push @changed_files, $key;
       }
    }
  }

  print "\nInfo:\tTotal $counter new file(s)\n";

  print "\nInfo:\tList of the changed file\n";

  $counter = 0;

  foreach $file ( @changed_files ){
      $counter++;
      print "\t$file\n";
  }

  print "\nInfo:\tTotal $counter file(s) changed\n";
  print "Info:\tNote: Only new files are copied to $result_dir\n";
}

main();
