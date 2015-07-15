#!/usr/bin/perl

use strict;
use warnings;
use Text::Ngram qw(ngram_counts);
#use YAML::XS;
use IO::File;
use Encode;
use Unicode::Japanese;
use utf8;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";

use constant FILE => "inv.idx";

MAIN:
{
    my $inv = {};
    my $cnt = 0;
    while (<>) {
        chomp;
        my $keyword = $_;
        my $str = Unicode::Japanese->new($keyword);
        $keyword = decode_utf8($str->h2zKana->z2hAlpha->z2hNum->get);
        $keyword = lc($keyword);
        my $bigrams = ngram_counts($keyword, 2);
        while (my ($k, $v) = each %$bigrams) {
            push @{$inv->{$k}}, $keyword;
        }
        print STDERR $cnt . " done.\n" if ++$cnt % 1000 == 0;
    }
    #YAML::XS::DumpFile("./inv.idx", $inv);

    my $fh = new IO::File "> " . FILE;
    die unless defined $fh;
    binmode($fh, ":encoding(utf8)");

    while (my ($gram, $keyword) = each (%$inv)) {
        next if scalar(@$keyword) == 0;
        print $fh "$gram\t"; 
        my $keywords = "";
        foreach (@$keyword) {
            $keywords .= "$_";
        }
        chop($keywords);
        print $fh "$keywords\n";
    }
    $fh->close;
}

1;
