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

# Multinomial NaiveBayes test

my $c;

MAIN:
{
    my $usage = "$0 test_data_dir|train_data_file";
    my $test_data = shift or die $usage;

    my $stat = retrieve("stat.nb");
    my $catmap = retrieve("catmap.nb");
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
    my $score = {};
    my $max_cat;
    my $max_score = 0;
    while (my ($cat, $catid) = each %$catmap) {
        next if $cat eq "##NUMCAT##";
        $score->{$cat} = 0;
        my $alpha = keys %$token_h;
        while (my ($token, $freq) = each %$token_h) {
            $score->{$cat} = 0 unless defined $score->{$cat};
            my $N_ci = defined $stat->{N_ci}->{$catid}->{$token} ? $stat->{N_ci}->{$catid}->{$token} : 0;
            $score->{$cat} += $freq * log(($N_ci + 1) / ($stat->{N_c}->{$catid} + $alpha));
        }
        $score->{$cat} += log($stat->{cat}->{$catid} / $stat->{catall});
        #print $score->{$cat} . "\n";
        if ($max_score < $score->{$cat}) {
            $max_score = $score->{$cat};
            $max_cat = $cat;
        }
    }
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
