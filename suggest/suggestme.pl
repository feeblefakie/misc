#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use Encode;
use MeCab;
use IO::File;
use Text::Kakasi;
use Unicode::Japanese;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";

$| = 1;

MAIN:
{
    print "loading index ";
    my $inv = {'en' => {}, 'ja' => {}};
    read_index("pfx_en.idx", $inv->{en});
    read_index("pfx_ja.idx", $inv->{ja});
    print " [done]\n";

    #my $mecab = new MeCab::Tagger("-Oyomi");
    my $kakasi = Text::Kakasi->new('-Ha', '-Ka', '-Ja', '-Ea', '-ka');
    while (1) {
        print "query: ";
        my $q = STDIN->getline;
        chomp $q;
        my $t1 = (times)[0];
        $q = decode_utf8($q) unless utf8::is_utf8($q);
        my $str = Unicode::Japanese->new($q);
        my $q_norm = lc(decode_utf8($str->h2zKana->z2hAlpha->z2hNum->hira2kata->get));
        my $q_roman = $kakasi->get(encode("eucjp", $q_norm));
        $q_roman =~ s/(.)\^/$1$1/g;

        print "\n================\n";
        if ($q_norm eq $q_roman) { # no japanese character included
            foreach (@{$inv->{en}->{$q_norm}}) {
                my ($key, $score) = split //;
                print "$key [$score]\n"
            }
            foreach (@{$inv->{ja}->{$q_norm}}) {
                my ($key, $score) = split //;
                print "$key [$score]\n"
            }
        } else {
            if ($q_norm !~ /\p{Han}/) {
                foreach (@{$inv->{ja}->{$q_roman}}) {
                    my ($key, $score) = split //;
                    print "$key [$score]\n"
                }
            }
            foreach (@{$inv->{ja}->{$q_norm}}) {
                my ($key, $score) = split //;
                print "$key [$score]\n"
            }
        }
        print "================\n\n";
        my $t2 = (times)[0];
        print sprintf("%3f", $t2 - $t1) . " seconds.\n";
    }
}

sub read_index
{
    my ($file, $inv) = @_;
    my $fh = IO::File->new;
    $fh->open("< $file") or die "can't open the $file | $!";
    binmode $fh, ":utf8";
    my $i = 0;
    while (<$fh>) {
        chomp;
        my ($key, $list) = split //;
        my @suggests = split //, $list;
        $inv->{$key} = \@suggests;
        print "." if ++$i % 100000 == 0;
    }
    $fh->close;
}

1;
