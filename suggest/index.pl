#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use Encode;
use IO::File;
use MeCab;
use Text::Kakasi;
use Unicode::Japanese;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";

MAIN: {
    my $kakasi = Text::Kakasi->new('-Ha', '-Ka', '-Ja', '-Ea', '-ka');
    my $mecab = new MeCab::Tagger("-Oyomi");

    my $fh_en = IO::File->new;
    $fh_en->open("> pfx_en.idx") or die "can't open the file | $!";
    binmode $fh_en, ":utf8";
    my $fh_ja = IO::File->new;
    $fh_ja->open("> pfx_ja.idx") or die "can't open the file | $!";
    binmode $fh_ja, ":utf8";

    my $h = {'en' => {}, 'ja' => {}};
    my $dup_h = {};
    while (<>) {
        chomp;
        my ($key_org, $score) = split /\t/;
        my $str = Unicode::Japanese->new($key_org);
        my $str_norm = $str->h2zKana->z2hAlpha->z2hNum;
        my $key_norm = lc(decode_utf8($str_norm->get));

        # indexing romanji
        my $key_roman = $kakasi->get(encode("eucjp", $key_norm));
        $key_roman =~ s/(.)\^/$1$1/g;
        my $h_lang = $key_roman eq $key_norm ? $h->{en} : $h->{ja};
        index_prefixes($h_lang, $dup_h, $key_roman, {key => $key_norm, score => $score});
        next if $key_roman eq $key_norm;

        # indexing katakana
        my $key_kata = $str_norm->hira2kata->get;
        # covert the key into katakana only if it is not.
        my $key_kataed = $mecab->parse(encode("utf8", $key_norm)) if $key_kata ne $key_norm;
        index_prefixes($h->{ja}, $dup_h, decode("utf8", $key_kataed), 
                       {key => $key_norm, score => $score}) if length($key_kataed) > 0;

        # indexing original
        index_prefixes($h->{ja}, $dup_h, decode("utf8", $key_kata), 
                       {key => $key_norm, score => $score})
                             if $key_kataed ne $key_kata && length($key_kata) > 0;
    }

    dump_prefixes($h->{en}, $fh_en);
    dump_prefixes($h->{ja}, $fh_ja);

    $fh_en->close;
    $fh_ja->close;
}

sub dump_prefixes {
    my ($h, $fh) = @_;

    while (my ($k, $v) = each %$h) {
        my $out .= "$k";
        foreach (@$v) {
            $out .= $_->{key} . "" . $_->{score} . "";
        }
        $out =~ s/\n//g;
        chop $out;
        print $fh "$out\n";
    }
}

sub index_prefixes {
    my ($h, $dup_h, $str, $val) = @_;
    for my $off (1 .. length($str)) {
         my $key = substr($str, 0, $off);
         next if defined $dup_h->{$key}->{$val->{key}};
         if (!defined $h->{$key}) {
             push @{$h->{$key}}, $val;
             $dup_h->{$key}->{$val->{key}} = 1;
             next;
         }
         my $i = 0;
         my $inserted = 0;
         # TODO: linked list array causes inefficiency
         foreach (@{$h->{$key}}) {
             #$_->{key}
             # TODO: duplicate check by hash
             if ($_->{score} < $val->{score}) {
                 splice(@{$h->{$key}}, $i, 0, $val);
                 $dup_h->{$key}->{$val->{key}} = 1;
                 my $tmp_val = pop @{$h->{$key}} if scalar(@{$h->{$key}}) > 10;
                 delete $dup_h->{$key}->{$tmp_val->{key}};
                 $inserted = 1;
                 last;
             }
             ++$i;
         }
         if (scalar(@{$h->{$key}}) < 10 && $inserted != 1) {
             push @{$h->{$key}}, $val;
             $dup_h->{$key}->{$val->{key}} = 1;
         }
    }
}

1;
