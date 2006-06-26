#!/usr/bin/perl
# free_radius_converter.pl - converts free radius dictionary into open diameter
# $Revision: 1.2 $

use strict;
use File::Basename;
use Getopt::Long;

my($progname);
($progname = $0) =~ s(.*/)();
my $revision = '$Revision: 1.2 $';
$revision =~ s/.*\s+([.\d]).*/$1/;
my $ERR = \*STDERR;

my($usage) = <<USAGE;

Usage: $progname [--root [dictionary]] [--help] 

Options:

     --root [dictionary] Free radius root dictionary file.
                         (usually /etc/raddb/dictionary.in)

     --help              Prints the usage

Notes:
   Converts the radius dictionary files into an
   open diameter xml dictionary. The script should
   be executed with the root dictionary passed in
   as argument. The script resolves the path of 
   other dictionary files relative to the location 
   of the root, i.e. dictionary.in should have 
   INCLUDE entries that contains paths to the
   include file. If not, local directory is assumed.

   User defined attributes and values in the root 
   dictionary will be appended into the <base> section 
   of the open diameter dictionary unless a VENDOR tag 
   is present before the attributes and values.

   The output of the script is a fully usable open
   diameter XML dictionary. The output is directed
   to STDOUT so to generate a dictionary:

      ./$progname --root dictionary.in > dictionary.xml

   Note that there are also convertion notifications 
   and error messages that are sent to STDERR.

USAGE

my @base_attrs;
my @base_values;
my @vendor_map;
my $opt_root_dict = "dictionary.in";
my $opt_help = 0;

GetOptions('root=s' => \$opt_root_dict,
           'help' => \$opt_help) || die($usage);

if (($opt_help) || (! $opt_root_dict)) {
   print("$usage");
   exit 0;
}

my ($root_base, $root_path, $root_type) = 
          fileparse($opt_root_dict);

&do_parse_file($root_path, $root_base);

&do_print_header();

&do_print_base();

&do_print_vendors();

&do_print_footer();

exit 0;

sub do_parse_file
{
    my($curpath, $file) = @_;
    my($vendor_flag) = 0;
    my(@vendor_attrs);
    my(@vendor_values);
    my(@entry);
    my($line);

    print $ERR "Processing: $curpath/$file ...\n";

    open(DAT, "$curpath/$file") || die("ERROR: Could not open file $curpath/$file !");
    my(@inc_data)=<DAT>;
    close(DAT);

    foreach $line (@inc_data) {
       chop($line);
       $line =~ s/\s+/\ /g;
       @entry = split(/\ /, $line);
       if ($entry[0] eq "\$INCLUDE") {
          my ($base, $path, $type) = fileparse($entry[1]);
          if ($path eq "./") {
	      do_parse_file($curpath, $base);
	  }
          else {
              $path =~ s/\/\Z//;
              do_parse_file($path, $base);
	  }
       }
       elsif ($entry[0] eq "BEGIN-VENDOR") {
          # Don't use this, use VENDOR
          # to be sure
       }
       elsif ($entry[0] eq "END-VENDOR") {
          # Don't use this, some dict file
          # abruptly set's this prior to VALUE's
       }
       elsif ($entry[0] eq "VENDOR") {
          $vendor_flag = 1;
          push(@vendor_map, "VENDOR:$entry[1]:$entry[2]");
       }
       elsif ($entry[0] eq "ATTRIBUTE") {
	  if ($vendor_flag) {
             push(@vendor_attrs, "ATTRIBUTE:$line");
	  }
          else {
             push(@base_attrs, $line);
	  }
       }
       elsif ($entry[0] eq "VALUE") {
	  if ($vendor_flag) {
             push(@vendor_values, "VALUE:$line");
	  }
          else {
             push(@base_values, $line);
	  }
       }
    }

    if ($vendor_flag) {
       push(@vendor_map, @vendor_attrs);
       push(@vendor_map, @vendor_values);
    }
}

sub do_print_attribute
{
    my(@entry) = @_;

    print "   <attribute>\n";
    print "      <name>$entry[1]</name>\n";
    if ($entry[2] =~ /(0x)([\d|A-F|a-f]+)/) {
       my($int) = hex("$2");
       my($dec) = sprintf("%d", $int);
       print "      <attr_type>$dec</attr_type>\n";
    }
    elsif ($entry[2] =~ /\d+/) {
       print "      <attr_type>$entry[2]</attr_type>\n";
    }
    else {
       die("ERROR: Unknown attribute type $entry[2] !\n");
    }
    print "      <data_type>$entry[3]</data_type>\n";
    if ($entry[4] ne "")
    {
       $entry[4] =~ s/\s+//g;
       my(@flags) = split(/=/, $entry[4]);
       if ($flags[0] eq "encrypt")
       {
           print "      <encrypt>";
           if ($flags[1] eq "1") {
              print "true";
	   }
           else {
              print "false";
	   }
           print "</encrypt>";
       }
    }
    print "   </attribute>\n"
}

sub do_print_value
{
    my(@entry) = @_;
    print "   <value>\n";
    print "      <name>$entry[2]</name>\n";
    print "      <attribute>$entry[1]</attribute>\n";
    if ($entry[3] =~ /(0x)([\d|A-F|a-f]+)/) {
       my($int) = hex("$2");
       my($dec) = sprintf("%d", $int);
       print "      <value>$dec</value>\n";
    }
    elsif ($entry[3] =~ /\d+/) {
       print "      <value>$entry[3]</value>\n";
    }
    else {
       die("ERROR: Unknown attribute type $entry[3] !\n");
   }
    print "   </value>\n"
}

sub do_print_header {

    print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    print "<radius_dictionary xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
    print "                   xsi:noNamespaceSchemaLocation=\'dictionary.xsd\'>\n";

    print "<!-- *********************** typename\'s ************************ -->\n";
    print "<typename>string</typename>\n";
    print "<typename>integer</typename>\n";
    print "<typename>ipaddr</typename>\n";
    print "<typename>date</typename>\n";
    print "<typename>ifid</typename>\n";
    print "<typename>ipv6addr</typename>\n";
    print "<typename>ipv6prefix</typename>\n";
    print "<typename>octet</typename>\n";
    print "<!-- *********************** End Typename ********************** -->\n";
}

sub do_print_base {
    my($base_attrs_item);
    my($base_values_item);

    print "<base>\n";

    foreach $base_attrs_item (@base_attrs) {
       my(@content) = split(/\ /, $base_attrs_item);
       do_print_attribute(@content);
    }

    foreach $base_values_item (@base_values) {
       my(@content) = split(/\ /, $base_values_item);
       do_print_value(@content);
    }

    print "</base>\n";
}

sub do_print_vendors {
    my($vendor_map_item);
    my($print_vendor_footer) = 0;

    foreach $vendor_map_item (@vendor_map) {       
       my(@entry) = split(/:/, $vendor_map_item);
       if ($entry[0] eq "VENDOR") {
           if ($print_vendor_footer) {
               print "</vendor>\n";
	   }
           print "<vendor name=\"$entry[1]\" id=\"$entry[2]\">\n";    
           $print_vendor_footer = 1;
       }
       elsif ($entry[0] eq "ATTRIBUTE") {
           my(@content) = split(/\ /, $entry[1]);
           do_print_attribute(@content);
       }
       elsif ($entry[0] eq "VALUE") {
           my(@content) = split(/\ /, $entry[1]);
           do_print_value(@content);
       }
    }
    if ($print_vendor_footer) {
       print "</vendor>\n";
    }
}

sub do_print_footer {
    print "</radius_dictionary>\n";
}


