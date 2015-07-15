#!/usr/bin/env perl

use MeCab;

my $str = "私は山田浩之です";
my $c = new MeCab::Tagger (join " ", @ARGV);

for (my $m = $c->parseToNode($str); $m; $m = $m->{next}) {
    next if (split /,/, $m->{feature})[0] eq "BOS/EOS";
    printf("%s\n", $m->{surface});
}

1;
