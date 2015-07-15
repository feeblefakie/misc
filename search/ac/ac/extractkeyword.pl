#!/usr/bin/env perl

#  Copyright (C) 2005 Taku Kudo <taku@chasen.org>
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.

$str = join "", <>;
$str =~ s/\\s/ /g;
$str =~s/\\//g;
$str =~s/&amp;/&/g;
$str =~s/&gt;/>/g;
$str =~s/&lt;/</g;

my @ary = sort (split '\|', $str);
for (@ary) {
    next if /^\s*$/;
    print "$_\n";
}
