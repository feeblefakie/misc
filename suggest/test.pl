#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

my $str = "gu^guru^";

$str =~ s/(.)\^/$1$1/g;

print "$str\n";
