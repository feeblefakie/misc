#!/usr/bin/env perl

use strict;
use warnings;

MAIN:
{
    # threshold level
    my $usage = "$0 level k";
    my $tlevel = shift or die $usage;
    my $k = shift or die $usage;
    my $max = 2 ** $tlevel;
    my @ios_improved2 = ();
    for (my $i = 0; $i < $k; ++$i) {
        push @ios_improved2, 1;
    }

    my $level = 1;
    my $num_flushes = 0;
    my $base = $k + 1;
    while (1) {
        if (++$num_flushes == $base) {
            $base *= 2;
            push @ios_improved2, get_succ_merges_io_improved_with_late_merging($level, $k);
            last if $num_flushes >= $max;
            # @ios = (a1,a2,a3,a4)
            # new @ios = (a1, a2, a3, a4, a1, a2, a3)
            push @ios_improved2, @ios_improved2;
            pop @ios_improved2 if scalar(@ios_improved2) > 0;
            ++$level;
        }
    }
    my $total_lmi2_io = 0;
    my $i = 1;
    foreach (0 .. scalar(@ios_improved2)-1) {
        $total_lmi2_io += $ios_improved2[$_];
        print "$i $total_lmi2_io\n";
        ++$i;
    }
}

sub get_succ_merges_io_improved_with_late_merging {
    my $level = shift;
    my $k = shift;
    my $ios = 0;
    my $prev;
    my $base = $k + 1;
    $ios += $k;
    for (my $i = 1; $i < $level; ++$i) {
        $ios += $base;
        $base *= 2;
    }
    $ios = $ios * 2 + 1;
    return $ios;
}

1;
