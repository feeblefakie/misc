#!/usr/bin/env perl

use strict;
use warnings;

use Digest::MD5  qw(md5 md5_hex md5_base64);

MAIN:
{
    my $usage = "$0 num_records from";
    my $num_records = shift or die $usage;
    my $from = shift;
    $from = defined($from) ? $from : 0;
    my $user_id = $from;
    my $fname = undef;
    my $lname = undef;
    my $num = undef;

    while (1) {
        $num = $user_id + 100;
        $fname = md5_hex($user_id);
        $lname = md5_hex($num);
        print "$user_id,$fname,$lname,$num\n";
        last if ++$user_id >= $num_records + $from;
    }
}
