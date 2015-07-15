#!/usr/bin/env perl

use strict;
use warnings;

my $prev = undef;
while (<>) {
    chomp;
    my $line = $_;
    if (length($line) == 0) {
        if (defined $prev) {
            print "$prev\n";
        } else {
            print "\n";
        }
    } else {
        print "$line\n";
        $prev = $line;
    }
}
