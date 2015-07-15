#!/usr/bin/perl

use strict;
use warnings;

MAIN:
{
    while (<>) {
        chomp;
        my ($kana, $keyword) = split /\t/;
        print "$keyword\n";
    }
}

1;
