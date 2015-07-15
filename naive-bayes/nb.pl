#!/usr/bin/env perl

# Multinomial/Complement NaiveBayes learner (prototype)

use strict;
use warnings;
use utf8;
use Encode;
use Unicode::Japanese;
use Data::Dumper;
use File::Basename;
use YAML::XS;
use Storable;
use MeCab;
binmode STDIN, ":utf8";
#binmode STDOUT, ":utf8";

use constant ALPHAI => 1;
$| = 1;

MAIN:
{
    my $usage = "$0 train_data_dir m|c";
    my $train_data_dir = shift or die $usage;
    my $mode = shift or die $usage;
    my @dirs = glob "./$train_data_dir/*";
    my $c = new MeCab::Tagger (join " ", @ARGV);

    my $stat = {};
    my @doccat = ();
    my $catmap = {};
    $catmap->{"##NUMCAT##"} = 0;
    my $docid = 0;

    # make statistics
    print "making statistcs ...\n";
    foreach my $dir (@dirs) {
        my @files = glob "$dir/*";
        foreach my $file (@files) {
            open FIN, "< $file" or die $!;
            undef $/;
            my $doc = <FIN>;
            close(FIN);
            chomp $doc;
            my $cat = basename($dir);
            my $catid = get_category($cat, $catmap);
            my $norm_doc = normalize($doc);
            my $doc_tokens_ref = tokenize($c, $norm_doc);
            normalize_tokens($doc_tokens_ref);
            # do something for stop words like tokens ?

            if ($mode eq "c") {
                dostat_doc($stat, $docid, $doc_tokens_ref, $catid);
            } else {
                dostat_doc_for_mnb($stat, $docid, $doc_tokens_ref, $catid);
            }

            # category map
            push @doccat, $catid; 
            ++$docid;
        }
    }
    # mnb learning is done with this statistics

    cnb_learn($stat, $catmap) if $mode eq "c";

    print "dumping index ...\n";
    store $stat, "stat.nb";
    store $catmap, "catmap.nb";
}

sub cnb_learn {
    my ($stat, $catmap) = @_;

    # document transformation (statistics transformation)
    print "transforming documents ...\n"; 
    while (my ($catid, $doc_token_ref) = each (%{$stat->{tf}})) {
        while (my ($docid, $token_freq_ref) = each %$doc_token_ref) {
            while (my ($token, $freq) = each (%$token_freq_ref)) {
                my $tf = $token_freq_ref->{$token};
                # TF transform
                $tf = log($freq + 1);
                # IDF transform
                $tf *= log($stat->{N} / $stat->{df}->{$token});
                # length normalization
                $tf /= $stat->{tn}->{$docid};
                #$stat->{tf}->{$catid}->{$docid}->{$token} = $tf;
                $token_freq_ref->{$token} = $tf;
            }
        }
    }

    print "learning with complement documents ...\n";
    my %catmapin = %$catmap;
    while (my ($cat, $catid) = each %$catmap) {
        next if $cat eq "##NUMCAT##";
        print "$cat\n";
        while (my ($docid, $token_freq_ref) = each %{$stat->{tf}->{$catid}}) {
            print ".";
            while (my ($token, $freq) = each %$token_freq_ref) {

                # adding to the complement categories
                while (my ($catin, $catidin) = each %catmapin) {
                    next if $catin eq "##NUMCAT##";
                    next if $catidin == $catid;

                    $stat->{theta}->{$catid}->{$token} = 0 
                        unless defined $stat->{theta}->{$catid}->{$token};
                    $stat->{theta}->{$catid}->{$token} += $freq;

                    $stat->{freq_except}->{$catid} = 0
                        unless defined $stat->{freq_except}->{$catid};
                    $stat->{freq_except}->{$catid} += $freq;
                }
            }
        }
        print "\n";
    }
    print "\n";
}

sub get_category {
    my ($cat, $catmap) = @_;
    if (defined $catmap->{$cat}) {
        return $catmap->{$cat};
    } else {
        ++$catmap->{"##NUMCAT##"};
        $catmap->{$cat} = $catmap->{"##NUMCAT##"};
        return $catmap->{$cat};
    }
}

sub normalize {
    my $str = shift;
    #$str =~ s/From:.+\n//;
    #$str =~ s/Reply-To:.+\n//;
    #$str =~ s/In article.+\n//;
    #print "[$str]\n";
    $str =~ s/\n/ /g;
    $str =~ s/\s+/ /g;
    $str =~ s/^\s//;
    $str =~ s/\s$//;
    #my $ustr = Unicode::Japanese->new($str);
    #return lc(decode_utf8($ustr->h2zKana->z2hAlpha->z2hNum->get));
    return lc($str);
}

sub tokenize {
    my ($c, $str) = @_;
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

sub dostat_doc {
    my ($stat, $docid, $tokens_ref, $catid) = @_;

    my $df_counted = 0;
    foreach my $token (@$tokens_ref) {
        if (!defined $stat->{tf}->{$catid}->{$docid}->{$token}) {
            $stat->{tf}->{$catid}->{$docid}->{$token} = 1;
        } else {
            ++($stat->{tf}->{$catid}->{$docid}->{$token});
        }
        if (!defined $stat->{df}->{$token}) {
            $stat->{df}->{$token} = 1;
            $df_counted = 1;
        } elsif (!$df_counted) {
            ++($stat->{df}->{$token});
            $df_counted = 1;
        }
    }
    ++($stat->{N});

    # token length normalization factor
    while (my ($token, $freq) = each (%{$stat->{tf}->{$catid}->{$docid}})) {
        $stat->{tn}->{$docid} = 0 unless defined $stat->{tn}->{$docid};
        $stat->{tn}->{$docid} += $freq * $freq;
    }
    $stat->{tn}->{$docid} = sqrt($stat->{tn}->{$docid});

    $stat->{cat}->{$catid} = 0 unless defined $stat->{cat}->{$catid};
    ++($stat->{cat}->{$catid});
    $stat->{catall} = 0 unless defined $stat->{catall};
    ++($stat->{catall});
}

sub dostat_doc_for_mnb {
    my ($stat, $docid, $tokens_ref, $catid) = @_;

    my $df_counted = 0;
    foreach my $token (@$tokens_ref) {
        if (!defined $stat->{tf}->{$catid}->{$docid}->{$token}) {
            $stat->{tf}->{$catid}->{$docid}->{$token} = 1;
        } else {
            ++($stat->{tf}->{$catid}->{$docid}->{$token});
        }

        $stat->{N_ci}->{$catid}->{$token} = 0 unless defined $stat->{N_ci}->{$catid}->{$token};
        ++($stat->{N_ci}->{$catid}->{$token});
        $stat->{N_c}->{$catid} = 0 unless defined $stat->{N_c}->{$catid};
        ++($stat->{N_c}->{$catid});
    }

    $stat->{cat}->{$catid} = 0 unless defined $stat->{cat}->{$catid};
    ++($stat->{cat}->{$catid});
    $stat->{catall} = 0 unless defined $stat->{catall};
    ++($stat->{catall});
}

1;
