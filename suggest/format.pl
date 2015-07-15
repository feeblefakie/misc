#!/usr/bin/env perl

use strict;
use warnings;

while (<>) {
    chomp;
    print $_ . "\t" . int(rand(1000)) . "\n";
}

1;
