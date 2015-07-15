#!/usr/bin/perl

use strict;
use Unicode::Japanese;
use Encode;
use utf8;
binmode STDOUT, ":utf8";

my $str = Unicode::Japanese->new('ABCｱｲｳ１２３ＧＯＯＧＬＥ');
my $utf8 = decode_utf8($str->h2zKana->z2hAlpha->z2hNum->get);
print $utf8 . "\n";
