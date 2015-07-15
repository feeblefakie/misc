#!/usr/bin/env perl

use strict;
use warnings;

MAIN:
{
    while (<>) {
        chomp;
        my ($key1, $key2) = split /\t/;
        print "$key1\n";
        #print "$key2\n";
    }
}

1;
