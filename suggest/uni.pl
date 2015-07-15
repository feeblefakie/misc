#!/usr/bin/perl
# 入力はEUC-JPのTSVで住所とその読み仮名（カタカナ）。
# 出力はUTF-8のTSVで住所とその読み仮名（カタカナ＋ひらがな）。
use strict;
use warnings;
use Unicode::Japanese;

my $str = "検索";
print "org:$str\n";
my $yomi = Unicode::Japanese->new($str, 'utf8');
print $yomi->get . "\n";
print $yomi->kata2hira->get . "\n";
print $yomi->hira2kata->get . "\n";
