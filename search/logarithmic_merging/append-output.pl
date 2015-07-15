#!/usr/bin/env perl

use strict;
use warnings;

my $usage = "$0 base_file append_file";
my $base_file = shift or die $usage;
my $append_file = shift or die $usage;

open FIN, "< $base_file" or die "can't open $base_file | $!";
open FIN2, "< $append_file" or die "can't open $append_file | $!";

while (<FIN>) {
    chomp;
    my $line = $_;
    my $appending_line = <FIN2>;
    chomp $appending_line;
    $line .= " $appending_line";
    print "$line\n";
}

close FIN;
close FIN2;
