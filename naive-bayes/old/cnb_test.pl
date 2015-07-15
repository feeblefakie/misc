#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use Encode;
use Unicode::Japanese;
use YAML::XS;
use Storable;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";

# Complement NaiveBayes test

MAIN:
{
    my $usage = "$0 file";
    my $file = shift or die $usage;

    #my $w = YAML::XS::LoadFile("index.nb");
    my $w = retrieve("index.nb");
    #my $catmap = YAML::XS::LoadFile("catmap.nb");
    my $catmap = retrieve("catmap.nb");
    print "data loaded\n";

    open FIN, "< $file" or die $!;
    undef $/;
    my $doc = <FIN>;
    my $norm_doc = normalize($doc);
    my $doc_tokens_ref = tokenize($norm_doc);
    my $token_h = {};
    foreach (@$doc_tokens_ref) {
        $token_h->{$_} = 0 unless defined $token_h->{$_};
        ++$token_h->{$_};
    }
    my $score = {};
    while (my ($cat, $catid) = each %$catmap) {
        $score->{$cat} = 0;
        while (my ($token, $freq) = each %$token_h) {
            #print '$token_h->{$token} = ' . $token_h->{$token} . "\n";
            #print '$w->{$catid}->{$token} = ' . $w->{$catid}->{$token} . "\n";
            $score->{$cat} += $token_h->{$token} * $w->{$catid}->{$token} if defined $w->{$catid}->{$token};
        }
    }
    print YAML::XS::Dump($score);
}

sub normalize {
    my $str = shift;
    $str =~ s/\n/ /g;
    $str =~ s/\s+/ /g;
    $str =~ s/^\s//;
    $str =~ s/\s$//;
    #my $ustr = Unicode::Japanese->new($str);
    #return lc(decode_utf8($ustr->h2zKana->z2hAlpha->z2hNum->get));
    return lc($str);
}

sub tokenize {
    my $str = shift;
    # assume english text now.
    my @tokens = split / /, $str;
    return \@tokens;
}

1;
