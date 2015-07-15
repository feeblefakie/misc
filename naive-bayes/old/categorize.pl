#!/usr/bin/env perl

use strict;
use warnings;
use MeCab;
use Encode;
use YAML::XS;
use Unicode::Japanese;
#binmode STDIN, ":utf8";
#binmode STDOUT, ":utf8";

use constant NUM_OF_DOCS => 2021;

MAIN: {
    #my $usage = "$0 keywords";
    #my $keywords = shift or die $usage;
    #my $keywords = "メンズ,ファッション,スーツ,ネクタイ,ジャケット,デニム,Tシャツ,通販,インターネット,ショッピング,楽天市場";
    my $keywords = "yodobashi,ヨドバシ,ヨドバシドットコム,カメラ,デジカメ,家電";
    my @keys = split /,/, $keywords;

    my $c = new MeCab::Tagger (join " ", @ARGV);
    my @mapfiles = glob "*.map";
    my $probs = {};
    foreach my $file (@mapfiles) {
        my $prob = 1;
        my $map = YAML::XS::LoadFile($file);
        foreach (@keys) {
            for (my $m = $c->parseToNode($_); $m; $m = $m->{next}) {
                next if (split /,/, $m->{feature})[0] eq "BOS/EOS";
                my $token = decode_utf8($m->{surface});
                my $num = (defined $map->{$token}) ? log($map->{$token}) : 0;
                #print "$num\n" if $num > 0;
                $prob *= ($num + 1) / (log($map->{NW}) + 1);
            }
        }
        $prob *= $map->{ND} / NUM_OF_DOCS;
        $probs->{$file} = $prob;
    }

    my @k = sort {
        $probs->{$b} <=> $probs->{$a}
    }  keys %$probs;
    foreach (@k) {
        print "$_ - " . $probs->{$_} . "\n";
    }
}

1;
