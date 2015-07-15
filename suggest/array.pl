#!/usr/bin/env perl

use strict;
use warnings;

my $val = 7;
my @arr = (10, 9, 5, 3);
my $i = 0;
foreach (@arr) {
    if ($_ < $val) {
        splice(@arr, $i, 0, $val);
        #$arr[$i] = $val;
        last;
    }
    #print "$_\n";
    ++$i;
}

foreach (@arr) {
    print "$_\n";
}

print "top 3\n";
splice(@arr, 0, 3);
foreach (@arr) {
    print "$_\n";
}

1;
