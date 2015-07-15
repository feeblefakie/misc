#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use Encode;
use Unicode::Japanese;
use Data::Dumper;
use File::Basename;
use YAML::XS;
use Storable;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";

use constant ALPHAI => 1;
$| = 1;

# Complement NaiveBayes test

MAIN:
{
    my $usage = "$0 train_data_dir c";
    my $train_data_dir = shift or die $usage;
    my $c = shift;
    my @dirs = glob "./$train_data_dir/*";

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
            my $doc_tokens_ref = tokenize($norm_doc);
            normalize_tokens($doc_tokens_ref);
            # do something for stop words like tokens ?

            if (defined $c) {
                dostat_doc($stat, $docid, $doc_tokens_ref, $catid);
            } else {
                dostat_doc_for_mnb($stat, $docid, $doc_tokens_ref, $catid);
            }

            # category map
            push @doccat, $catid; 
            ++$docid;
        }
    }

    my $theta = cnb($stat, $catmap) if defined $c;

    print "dumping index ...\n";
    if (defined $c) {
        store $theta, "index.nb";
    }
    store $stat, "stat.nb";
    store $catmap, "catmap.nb";
} 

# Complement NaiveBayes
sub cnb {
    my ($stat, $catmap) = @_;

    # document transformation (statistics transformation)
#=comment
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
#=cut

    # learn with complement documents
    my $theta = {};
    my $freq_except = {};
    my $alpha = {};
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

                    $theta->{$catid}->{$token} = 0 
                        unless defined $theta->{$catid}->{$token};
                    $theta->{$catid}->{$token} += $freq;

                    $freq_except->{$catid} = 0
                        unless defined $freq_except->{$catid};
                    $freq_except->{$catid} += $freq;

                    $alpha->{$catid} = 0
                        unless defined $alpha->{$catid};
                    ++$alpha->{$catid};
                }
            }
        }
        print "\n";
    }
    print "\n";

    # weight-normalized
    print "normalizing by weight ...\n";
    while (my ($cat, $catid) = each %$catmap) {
        next if $cat eq "##NUMCAT##";
        my $wc = 0;
        print "$cat\n";
        while (my ($token, $freq_total) = each %{$theta->{$catid}}) {
            #print "$token\n";
            #print "theta = (" . $theta->{$catid}->{$token} . "+ 1) / (" . $freq_except->{$catid} . " + " . $alpha->{$catid} . ")\n";
            $theta->{$catid}->{$token} = ($freq_total + 1) / 
                                            ($freq_except->{$catid} + $alpha->{$catid});
                                            #print $theta->{$catid}->{$token} . "\n";

            $theta->{$catid}->{$token} = log($theta->{$catid}->{$token});
            #print "semi: " . $theta->{$catid}->{$token} . "\n";
            $wc += abs($theta->{$catid}->{$token});
        }
        print "wc($catid): $wc\n";
        # weight normalization
#=comment
        while (my ($token, $freq_total) = each %{$theta->{$catid}}) {
            print "before final: " . $theta->{$catid}->{$token} . "\n";
            $theta->{$catid}->{$token} = $theta->{$catid}->{$token} / $wc;
            print "after final: " . $theta->{$catid}->{$token} . "\n";
        }
#=cut
        print "\n";
    }
    return $theta;
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
=comment
        if (!defined $stat->{df}->{$token}) {
            $stat->{df}->{$token} = 1;
            $df_counted = 1;
        } elsif (!$df_counted) {
            ++$stat->{df}->{$token};
            $df_counted = 1;
        }
=cut
        $stat->{N_ci}->{$catid}->{$token} = 0 unless defined $stat->{N_ci}->{$catid}->{$token};
        ++($stat->{N_ci}->{$catid}->{$token});
        $stat->{N_c}->{$catid} = 0 unless defined $stat->{N_c}->{$catid};
        ++($stat->{N_c}->{$catid});
    }
    #++$stat->{N};

    $stat->{cat}->{$catid} = 0 unless defined $stat->{cat}->{$catid};
    ++($stat->{cat}->{$catid});
    $stat->{catall} = 0 unless defined $stat->{catall};
    ++($stat->{catall});
}

1;
