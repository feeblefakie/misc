<?php

$str = "333山田ＡＡＡ１１１";
$str_norm = mb_convert_kana($str, "KVa");
print "NORM: $str_norm\n";
$str = mb_convert_encoding($str, "EUC-JP", "UTF-8");

kakasi_getopt_argv(5, array("-Ha", "-Ka", "-Ja", "-Ea", "-ka"));
$str_roman = kakasi_do($str);



?>

