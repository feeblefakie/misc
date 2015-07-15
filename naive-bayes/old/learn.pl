#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use MeCab;
use Encode;
use YAML::XS;
use Unicode::Japanese;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";

use constant ND => "##NUMDOCS##";
use constant NW => "##NUMWORDS##";

# category[\t]sub-category[\t]keywords

MAIN: {
    my $c = new MeCab::Tagger (join " ", @ARGV);
    my $map = {};
    my $cnt = 0;
    my $prev_cat = "NOCAT";
    my $catid = 0;
    my $catmap = {};
    while (<>) {
        chomp;
        my $line = Unicode::Japanese->new($_);
        my $line_norm = lc(decode_utf8($line->h2zKana->z2hAlpha->z2hNum->get));
        my ($cat, $subcats_str, $keywords_str) = split /\t/, $line_norm;

        my @subcats = split /,/, $subcats_str;
        my @keywords = split /,/, $keywords_str;

        # assign category id
        if ($cat ne $prev_cat && $prev_cat ne "NOCAT") {
            $catmap->{++$catid} = $prev_cat;
            # dump the current category map
            print "dumping $prev_cat map ... ";
            YAML::XS::DumpFile("$catid.map", $map);
            print "[OK]\n";
            $map = {};
        }

        $map->{ND} = 0 unless defined $map->{ND};
        ++($map->{ND});
        $map->{NW} = 0 unless defined $map->{NW};
        foreach (@subcats, @keywords) {
            for (my $m = $c->parseToNode($_); $m; $m = $m->{next}) {
                next if (split /,/, $m->{feature})[0] eq "BOS/EOS";
                my $token = decode_utf8($m->{surface});
                if (!defined $map->{$token}) {
                    $map->{$token} = 1;
                } else {
                    ++($map->{$token});
                }
                ++$map->{NW};
            }
        }
        $prev_cat = $cat;
    }

    if (%$map) {
        $catmap->{++$catid} = $prev_cat;
        print "dumping $prev_cat map ... ";
        YAML::XS::DumpFile("$catid.map", $map);
        print "[OK]\n";
    }

    # dump catmap
    print "dumping catmap ... ";
    YAML::XS::DumpFile("catmap", $catmap);
    print "[OK]\n";
}

1;
