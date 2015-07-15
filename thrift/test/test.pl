#!/usr/bin/env perl

use lib './gen-perl';

# Thrift インストール時にインストールされているライブラリ群
use Thrift;
use Thrift::BinaryProtocol;
use Thrift::Socket;
use Thrift::BufferedTransport;


## IDL から gen-perl/ 以下に生成されたライブラリ
use Something;

my $transport = Thrift::Socket->new('localhost', 9090);
my $client = SomethingClient->new( Thrift::BinaryProtocol->new($transport) );

#my $client = SomethingClient->new( Thrift::BinaryProtocol->new(Thrift::BufferedTransport->new($transport)) );


$transport->open();

printf "%s\n", $client->ping();

$transport->close();

1;
