#!/usr/bin/env perl

use strict;
use warnings;

# xlogx - xlog(k+1) + 2x + (k-1)

MAIN: {
    my $usage = "$0 k";
    my $k = shift or die $usage;

    my $x = $k+1;
    for (1 .. 10) {
        print func($x, $k) . "\n";
        $x *= 2;
    }
}

sub log2 {
    my $val = shift;
    return log($val)/log(2);
}

sub func {
    my $x = shift;
    my $k = shift;
    #return $x * log2($x) - $x * log2($k+1) + 2*$x + $x * (($k-1)/($k+1));
    return $x * log2($x) - $x * log2($k+1) + $x*((3*$k+1)/($k+1));
}
