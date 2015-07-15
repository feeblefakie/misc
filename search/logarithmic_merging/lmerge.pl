#!/usr/bin/env perl

use strict;
use warnings;
use Data::Dumper;

my @Z = ();
my @I = ();
my @indexes = ();

MAIN:
{

    my $tokenizer = Tokenizer->new;
    $tokenizer->tokenize("aa");

    my $i = 0;
    while ($tokenizer->has_next()) {
        my $token = $tokenizer->get_next();
        print "=========================== iteration $i ==============================\n";
        print "input:\n";
        print Dumper($token);
        LMergeAddToken($token);
        print "===== Z:\n";
        print Dumper(\@Z);
        print "===== I:\n";
        print Dumper(\@I);
        print "===== indexes:\n";
        print Dumper(\@indexes);
        $i++;
    }
}

sub LMergeAddToken {
    my $token = shift;

    push @{$Z[0]}, $token;
    print "### " . scalar(@{$Z[0]}) . " ###\n";
    return if scalar(@{$Z[0]}) != 2;

    my $i = 0;
    while (1) {
        if (defined $indexes[$i]) {
            push @{$Z[$i+1]}, $Z[$i];
            push @{$Z[$i+1]}, $I[$i];
            undef $indexes[$i];
        } else {
            push @{$I[$i]}, $Z[$i];
            $indexes[$i] = 1;
            last;
        }
        ++$i;
    }
    $Z[0] = undef;
}

1;

package Tokenizer;

sub new {
    my $class = shift;
    return bless {
        tokens => []
    }, $class;
}

sub tokenize {
    my $self = shift;
    my $doc = shift;

    # tokenize

    my $offset = 0;
    foreach (@{["sony", "vaio", "digital", "dell", "hp"]}) {
        push @{$self->{tokens}}, {
            term => $_,
            offset => $offset,
        };
        $offset += length($_);
    }
}

sub get_next {
    my $self = shift;
    return shift @{$self->{tokens}};
}

sub has_next {
    my $self = shift;
    return 1 if scalar(@{$self->{tokens}}) > 0;
    return 0;
}
