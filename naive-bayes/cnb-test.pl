#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use Encode;
use Unicode::Japanese;
use File::Basename;
use Storable;
use MeCab;
binmode STDIN, ":utf8";
#binmode STDOUT, ":utf8";

$| = 1;

# Complement NaiveBayes test

my $c;

MAIN:
{
    my $usage = "$0 test_data_dir|train_data_file";
    my $test_data = shift or die $usage;

    my $catmap = retrieve("catmap.nb");
    my $stat = retrieve("stat.nb");
    print "data loaded\n";

    $c = new MeCab::Tagger (join " ", @ARGV);
    if (-d $test_data) {
        my @dirs = glob "./$test_data/*";
        check_all($stat, $catmap, \@dirs);
    } else {
        check_specific($stat, $catmap, $test_data);
    }
}

sub check_all {
    my ($stat, $catmap, $dirs) = @_;

    my $testcnt = 0;
    my $correct = 0;
    my $top3 = 0;
    foreach my $dir (@$dirs) {
        my @files = glob "$dir/*";
        my $cat_teacher = basename($dir);
        print "checking $cat_teacher\n";
        foreach my $file (@files) {
            open FIN, "< $file" or die $!;
            undef $/;
            my $doc = <FIN>;
            close(FIN);
            chomp $doc;
            my $keys = identify($stat, $catmap, $doc);
            if ($keys->[0] eq $cat_teacher) {
                ++$correct;
            } else {
                for (my $i = 1; $i < 3; ++$i) {
                    if ($keys->[$i] eq $cat_teacher) {
                        ++$top3;
                    }
                }
            }
            ++$testcnt;
            print ".";
        }
        print "\n";
    }
    print "total test: " . $testcnt . "\n";
    print "correct test: " . $correct . "\n";
    print "top3: " . $top3 . "\n";
    print "correct rate: " . $correct/$testcnt . "\n";
    print "top3 rate: " . ($correct+$top3)/$testcnt . "\n";
}

sub check_specific {
    my ($stat, $catmap, $file) = @_;

    open FIN, "< $file" or die $!;
    undef $/;
    my $doc = <FIN>;
    close(FIN);
    chomp $doc;
    my $keys = identify($stat, $catmap, $doc);
    print "estimated category: " . $keys->[0] . "\n";
}

sub identify {
    my ($stat, $catmap, $doc) = @_;
    my $norm_doc = normalize($doc);
    my $doc_tokens_ref = tokenize($norm_doc);
    normalize_tokens($doc_tokens_ref);
    my $token_h = {};
    foreach (@$doc_tokens_ref) {
        $token_h->{$_} = 0 unless defined $token_h->{$_};
        ++$token_h->{$_};
    }
    my $catmapin = $catmap;
    my $w = {};
    my $score = {};

    my $alpha = keys %$token_h;
    while (my ($token, $freq) = each %$token_h) {

        while (my ($cat, $catid) = each %$catmap) {
            next if $cat eq "##NUMCAT##";
            my $d = 0;
            $d = $stat->{theta}->{$catid}->{$token} 
                if defined $stat->{theta}->{$catid}->{$token};
                #print "($d + 1) / (" . $stat->{freq_except}->{$catid} . ") + $alpha)\n";
            $w->{$catid}->{$token} = ($d + 1) / ($stat->{freq_except}->{$catid} + $alpha);
            $w->{$catid}->{$token} = log($w->{$catid}->{$token});
            #print $w->{$catid}->{$token} . "\n";
            $score->{$cat} += $freq * $w->{$catid}->{$token};
        }
    }
=comment
    # weight normalization (it badly degrades precision not like the paper.)
    while (my ($cat, $catid) = each %$catmap) {
        next if $cat eq "##NUMCAT##";
        my $w_total = 0;
        while (my ($token, $freq) = each %$token_h) {
            $w_total += abs($w->{$catid}->{$token});
        }
        $score->{$cat} = 0;
        while (my ($token, $freq) = each %$token_h) {
            #print "before: $w->{$catid}->{$token}\n";
            $w->{$catid}->{$token} = $w->{$catid}->{$token} / $w_total;
            #print "after: $w->{$catid}->{$token}\n";
            $score->{$cat} += $freq * $w->{$catid}->{$token};
        }
    }
=cut
    my @score_keys = sort { $score->{$b} <=> $score->{$a} } keys %$score;

    return \@score_keys;
}

sub normalize {
    my $str = shift;
    #$str =~ s/From:.+\n//;
    #$str =~ s/Reply-To:.+\n//;
    #$str =~ s/In article.+\n//;
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
    #my @tokens = split / /, $str;
    my @tokens = ();
    for (my $m = $c->parseToNode($str); $m; $m = $m->{next}) {
        next if (split /,/, $m->{feature})[0] eq "BOS/EOS";
        push @tokens, $m->{surface};
    }
    return \@tokens;
}

sub normalize_tokens {
    my $tokens_ref = shift;
    foreach (@$tokens_ref) {
        s/^[\.,\?:;\('">\[<]+//;
        s/[\.,\?:;\)'"\]<>]+$//;
        s/^\s+//;
        s/\s+$//;
    }
}

1;
