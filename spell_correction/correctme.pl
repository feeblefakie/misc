#!/usr/bin/env perl

use strict;
use warnings;
use Levenshtein;
use Text::Ngram qw(ngram_counts);
use YAML::XS;
use IO::Handle;
use utf8;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";

$| = 1;

MAIN:
{
    print "loading index ... ";
    my $inv = YAML::XS::LoadFile("inv.idx");
    print "done\n";

    my $lev = Levenshtein->new();
    while (1) {
        print "query: ";
        my $q = STDIN->getline;
        chomp $q;
        my $t1 = (times)[0];
        my $q_bigrams = ngram_counts($q, 2);
        my $counter = {};
        while (my ($k, $v) = each %$q_bigrams) {
            for my $gram (@{$inv->{$k}}) {
                if (defined $counter->{$gram}) {
                    ++$counter->{$gram};
                } else {
                    $counter->{$gram} = 1;
                }
            }
        }
        my @keys = sort {$counter->{$b} <=> $counter->{$a}} keys %$counter;
        my $i = 0;
        my $disth = {};
        foreach (@keys) {
            my $jaccard = $counter->{$_} / (length($q) + length($_) - 2 - $counter->{$_});
            #print "jaccard: " . $jaccard . "\n";
            next if $jaccard < 0.4;
            $disth->{$_} = $lev->distance($q, $_);
            #last if ++$i == 10;
        }
        my @suggests = sort {$disth->{$a} <=> $disth->{$b}} keys %$disth;
        my $t2 = (times)[0];
        print "------------------------------\n";
        print sprintf("%3f", $t2 - $t1) . " seconds.\n";
        $i = 0;
        foreach my $k (@suggests) {
            print "$k (" . $disth->{$k} . ")\n";
            last if ++$i == 10;
        }
        print "------------------------------\n\n";
    }
}

1;
