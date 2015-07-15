#!/usr/bin/env perl

use strict;
use warnings;

MAIN:
{
    my $usage = "$0 l3 l3_column_no(1-base) l1 l1_column_no(1-base)";
    my $l3 = shift or die $usage;
    my $l3_cno = shift or die $usage;
    my $l1 = shift or die $usage;
    my $l1_cno = shift or die $usage;
    my $hash = {};

    open FIN, "< $l3" or die "can't open $l3 | $!";
    my $cnt = 0;
    while (<FIN>) {
        chomp;
        my $o_orderkey = (split / /)[$l3_cno-1];
        $hash->{$o_orderkey} = $cnt++;
    }
    close FIN;

    open FIN, "< $l1" or die "can't open $l1 | $!";
    $cnt = 0;
    my @arr = ();
    while (<FIN>) {
        chomp;
        my $l_orderkey = (split / /)[$l1_cno-1];
        print $hash->{$l_orderkey} . " " . $cnt . "\n";
        ++$cnt;
    }
    close FIN;
}

1;
