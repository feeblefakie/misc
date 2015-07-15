#!/usr/bin/env perl

use strict;
use warnings;

my $keywords = {};
while (<>) {
    chomp;
    my @items = split /\t/;
    if (defined $keywords->{$items[1]}) {
        ++$keywords->{$items[1]};
    } else {
        $keywords->{$items[1]} = 1;
    }
}

my @keys = sort {$keywords->{$b} <=> $keywords->{$a}} keys %$keywords;
foreach (@keys) {
    print "$_\t" . $keywords->{$_} . "\n";
}

1;
