#!/usr/bin/env perl

use strict;
use warnings;

MAIN:
{
    my @array = <>;
    my @sorted = sort @array;
    foreach (@sorted) {
        print "$_";
    }
}

1;
