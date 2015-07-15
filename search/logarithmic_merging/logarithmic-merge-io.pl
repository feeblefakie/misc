#!/usr/bin/env perl

use strict;
use warnings;

MAIN:
{
    # threshold level
    my $tlevel =  shift;
    my @ios = (1);
    my @ios_improved = (1);

    my $level = 1;
    while (1) {
        push @ios, get_succ_merges_io($level);
        push @ios_improved, get_succ_merges_io_improved($level);
        # @ios = (a1,a2,a3,a4)
        # new @ios = (a1, a2, a3, a4, a1, a2, a3)
        push @ios, @ios;
        push @ios_improved, @ios_improved;
        pop @ios if scalar(@ios) > 0;
        pop @ios_improved if scalar(@ios_improved) > 0;
        if (++$level == $tlevel) {
            push @ios, get_succ_merges_io($level);
            push @ios_improved, get_succ_merges_io_improved($level);
            last;
        }
    }
    my $total_lm_io = 0;
    my $total_lmi_io = 0;
    my $i = 1;
    foreach (0 .. scalar(@ios)-1) {
        $total_lm_io += $ios[$_];
        $total_lmi_io += $ios_improved[$_];
        my $total_re_io = $i ** 2;
        print "$i $total_re_io $total_lm_io $total_lmi_io\n";
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
