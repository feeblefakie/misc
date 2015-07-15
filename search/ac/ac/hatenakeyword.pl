#!/usr/bin/env perl
use strict;

open(F, "keywordlist") || die;
my $keywordlist = join "", <F>;
close(F);

while (my $text = <>) {
    chomp $text;
$text =~ s!
    ($keywordlist)
!
    my $enword = my $word = $1;
$enword =~ s/(\W)/sprintf("%%%x",ord($1))/ge;
qq|<a href="http://d.hatena.ne.jp/keyword/$enword">$word</a>|;
!egiox;

print "$text\n";
    }
