#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use Encode;
use Unicode::Japanese;
use File::Basename;
use Storable;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";

$| = 1;

# Complement NaiveBayes test

MAIN:
{
    my $usage = "$0 train_data_dir";
    my $train_data_dir = shift or die $usage;
    my @dirs = glob "./$train_data_dir/*";

    my $w = retrieve("index.nb");
    my $catmap = retrieve("catmap.nb");
    my $stat = retrieve("stat.nb");
    print "data loaded\n";

    my $testcnt = 0;
    my $correct = 0;
    my $top3 = 0;
    foreach my $dir (@dirs) {
        my @files = glob "$dir/*";
        my $cat_teacher = basename($dir);
        print "checking $cat_teacher\n";
        foreach my $file (@files) {
            open FIN, "< $file" or die $!;
            undef $/;
            my $doc = <FIN>;
            close(FIN);
            chomp $doc;
            my $keys = identify($w, $catmap, $stat, $doc);
            #print $keys->[-1] . " - " . $cat_teacher . "\n";
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

sub identify {
    my ($w, $catmap, $stat, $doc) = @_;
    my $norm_doc = normalize($doc);
    my $doc_tokens_ref = tokenize($norm_doc);
    normalize_tokens($doc_tokens_ref);
    my $token_h = {};
    foreach (@$doc_tokens_ref) {
        $token_h->{$_} = 0 unless defined $token_h->{$_};
        ++$token_h->{$_};
    }
    my $score = {};
    my $min_cat;
    my $min_score = 100000000000;
    while (my ($cat, $catid) = each %$catmap) {
        next if $cat eq "##NUMCAT##";
        $score->{$cat} = 0;
        while (my ($token, $freq) = each %$token_h) {
            $score->{$cat} += $token_h->{$token} * $w->{$catid}->{$token} if defined $w->{$catid}->{$token};
            #print $token_h->{$token} . "\n";
            #print $w->{$catid}->{$token} . "\n";
        }
        #$score->{$cat} = log($stat->{cat}->{$catid}/$stat->{catall}) - $score->{$cat};
        #$score->{$cat} = -$score->{$cat};
        #print $score->{$cat} . "\n";
#=comment
        if ($min_score > $score->{$cat}) {
            $min_score = $score->{$cat};
            $min_cat = $cat;
        }
#=cut
=comment
        if ($max_score < $score->{$cat}) {
            $max_score = $score->{$cat};
            $max_cat = $cat;
        }
=cut
    }
    my @score_keys = sort { $score->{$a} <=> $score->{$b} } keys %$score;
    #my @score_keys = sort { $score->{$b} <=> $score->{$a} } keys %$score;
    die "something wrong!" if $score_keys[0] ne $min_cat;
    #die "something wrong!" if $score_keys[0] ne $max_cat;

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
    # assume english text now.
    my @tokens = split / /, $str;
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
