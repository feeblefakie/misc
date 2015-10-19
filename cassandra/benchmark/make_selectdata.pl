#!/usr/bin/env perl

use strict;
use warnings;

MAIN:
{
    my $usage = "$0 num_records from to attr";
    my $num_records = shift or die $usage;
    my $from = shift or die $usage;
    my $to = shift or die $usage;
    my $attr = shift or die $usage;

    $from = 0 if $from eq "zero";
    my $diff = $to - $from;

    my $i = 0;
    while (1) {
        my $num = int(rand($diff)) + $from;
        print "$attr=$num\n";
        last if ++$i >= $num_records
    }
}
