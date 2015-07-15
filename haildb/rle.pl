#!/usr/bin/env perl

use strict;
use warnings;

my $pos = 0;
my $cnt = 0;
# numbers never shown
my $pval = 111111111111; 
while (<>) {
    chomp;
    my $val = $_;
    if ($val != $pval && $cnt != 0) {
        # output
        my $spos = $pos - $cnt;
        print "$pval $spos $cnt\n";
        $cnt = 0;
    }
    ++$pos;
    ++$cnt;
    $pval = $val;
}
if ($cnt != 0) {
    # output
    my $spos = $pos - $cnt + 1;
    print "$pval $spos $cnt\n";
}

1;
