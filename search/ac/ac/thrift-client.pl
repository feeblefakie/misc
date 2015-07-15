#!/usr/bin/env perl

use lib './gen-perl';

# Thrift インストール時にインストールされているライブラリ群
use Thrift;
use Thrift::BinaryProtocol;
use Thrift::Socket;
use Thrift::BufferedTransport;

## IDL から gen-perl/ 以下に生成されたライブラリ
use TTYDIC;

my $transport = Thrift::Socket->new('localhost', 9090);
my $client = TTYDICClient->new( Thrift::BinaryProtocol->new($transport) );
#my $client = TTYDICClient->new( Thrift::BinaryProtocol->new(Thrift::BufferedTransport->new($transport)) );
$transport->open();

my $text = "福本伸行さんが９６年から「ヤングマガジン」（講談社）で連載し、単行本が累計で１３００万部を突破したギャンブルマンガを、映画「デスノート」の藤原竜也さん主演で映画化。天海祐希さん、香川照之さんら演技派が脇を固め、手に汗握るサスペンスが展開する。【関連写真特集】一発逆転を懸けたゲームに挑む…映画の一場面 自堕落な生活を送る伊藤カイジ（藤原さん）は、連帯保証人になったことで友人の借金を突然背負わされ、返済のために悪徳金融会社が主催する船上の「ギャンブルクルーズ」に参加させられる。カイジは、ほかの“負け犬”たちとともに一発逆転を懸けたゲームに挑むが、予想以上の試練に苦しめられる……というストーリー。";

my $results = $client->search($text);

print "extracted words:\n";
foreach my $word (@$results) {
    print "# $word #\n";
}

$transport->close();

1;
