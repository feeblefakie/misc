#!/usr/bin/env perl

use strict;
use warnings;

MAIN:
{
    # threshold level
    my $tlevel =  shift;
    my @ios = (1);
    my @ios_improved = (1);
    my @plots = ();
    my @plots_improved = ();

    my $level = 1;
    while (1) {
        push @ios, get_succ_merges_io($level);
        push @ios_improved, get_succ_merges_io_improved($level);
        my $total = 0;
        foreach (@ios) { $total += $_; }
        push @plots, $total;
        $total = 0;
        foreach (@ios_improved) { $total += $_; }
        push @plots_improved, $total;

        # @ios = (a1,a2,a3,a4)
        # new @ios = (a1, a2, a3, a4, a1, a2, a3)
        push @ios, @ios;
        push @ios_improved, @ios_improved;
        pop @ios if scalar(@ios) > 0;
        pop @ios_improved if scalar(@ios_improved) > 0;
        if (++$level == $tlevel) {
            push @ios, get_succ_merges_io($level);
            push @ios_improved, get_succ_merges_io_improved($level);
            my $total = 0;
            foreach (@ios) { $total += $_; }
            push @plots, $total;
            $total = 0;
            foreach (@ios_improved) { $total += $_; }
            push @plots_improved, $total;
            last;
        }
    }
    my $i = 1;
    for (0 .. scalar(@plots)-1) {
        print 2 ** $i . " " . $plots[$_] . " " . $plots_improved[$_] . "\n";
        ++$i;
    }
}

sub get_succ_merges_io {
    my $level = shift;
    my $ios = 0;
    for (my $i = 1; $i < $level; ++$i) {
        $ios += (2 ** $i) * 3
    }
    return 1 + 2 ** $level + $ios;
}

sub get_succ_merges_io_improved {
    my $level = shift;
    my $ios = 0;
    for (my $i = 0; $i <= $level; ++$i) {
        $ios += 2 ** $i;
    }
    return $ios;
}

1;
