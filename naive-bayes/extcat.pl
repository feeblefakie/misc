#!/usr/bin/env perl

use strict;
use warnings;

while (<>) {
    chomp;
    my @items = split /\t/;
    print $items[0] . "\n";
}

1;
