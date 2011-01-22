#/usr/bin/perl -w
use strict;
use warnings;

sub processDir;


my $spaces = "";
my $root = ".";
my $print = 0;


# Check command line arguments. -p or --print will print the directories and 
# files as they are traversed. Otherwise, any other argument specifies the 
# root directory where the traversal is started.
if (scalar @ARGV > 0)
{
    foreach my $arg (@ARGV)
    {
        if ($arg eq '-p' or $arg eq '--print')
        {
            $print = 1;
        }
        else
        {
            $root = $arg;
        }
    }
}


# Do the work. Gather all header names recursively through the directories.
my @headers = processDir($root, $print, $spaces);


# Build a single string with all the lines in it.
#foreach my $header (@headers) { print "$header\n"; }
my $headers_string;
foreach my $header (@headers) { $headers_string .= $header; }


# Output the files, for VC7, VC7.1, VC8 and VC9.
my $fh;

#--------
# VC7
#--------
my $reg_vc7 = 'Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\7.0\Languages\Extensionless Files\{B2F072B0-ABC1-11D0-9D62-00C04FD9DFD9}]' . $headers_string;
open $fh, ">highlight70.reg";
print $fh $reg_vc7;
close $fh;


#--------
# VC7.1
#--------
my $reg_vc71 = 'Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\7.1\Languages\Extensionless Files\{B2F072B0-ABC1-11D0-9D62-00C04FD9DFD9}]' . $headers_string;
open $fh, ">highlight71.reg";
print $fh $reg_vc71;
close $fh;


#--------
# VC8
#--------
my $reg_vc8 = 'Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\8.0\Languages\Extensionless Files\{B2F072B0-ABC1-11D0-9D62-00C04FD9DFD9}]' . $headers_string;
open $fh, ">highlight80.reg";
print $fh $reg_vc8;
close $fh;


#--------
# VC9
#--------
my $reg_vc9 = 'Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\9.0\Languages\Extensionless Files\{B2F072B0-ABC1-11D0-9D62-00C04FD9DFD9}]' . $headers_string;
open $fh, ">highlight90.reg";
print $fh $reg_vc9;
close $fh;


#----------------------------------------------------------------------------
# Utility functions.

sub listFiles($)
{
    my ($dir) = @_;
    return glob "$dir/*";
}

sub processFile($$$)
{
    my ($file, $print, $spaces) = @_;
    if ($print) { print $spaces . "File $file\n"; }

    my ($class) = $file =~ /(\w+)$/;
    my $line = '"' . $class . '" = ""';

    return "$line\n";
}

sub processDir($$$)
{
    my ($dir, $print, $spaces) = @_;
    if ($print) { print $spaces . "Directory $dir\n"; }

    my @lines;
    my @files = listFiles($dir);
    foreach my $file (@files) 
    {
        if (-d $file)
        {
            my @newlines = processDir($file, $print, $spaces . "  ");
            foreach my $line (sort @newlines) { push @lines, $line; }
            push @lines, "\n";
        }
        else
        {
            push @lines, processFile($file, $print, $spaces . "  ");
        }
    }

    return @lines;
}
