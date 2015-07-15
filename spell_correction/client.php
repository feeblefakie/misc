<?php

$GLOBALS['THRIFT_ROOT'] = './php';
include_once $GLOBALS['THRIFT_ROOT'].'/Thrift.php';
require_once $GLOBALS['THRIFT_ROOT'].'/protocol/TBinaryProtocol.php';
require_once $GLOBALS['THRIFT_ROOT'].'/transport/TSocketPool.php';
require_once $GLOBALS['THRIFT_ROOT'].'/transport/TBufferedTransport.php';
require_once $GLOBALS['THRIFT_ROOT'].'/packages/spell_correction/SpellCorrection.php';

$host = 'localhost';
$port = 9090;

$socket = new TSocket($host, $port);
$bufferedSocket = new TBufferedTransport($socket, 1024, 1024);
$transport = $bufferedSocket;
$protocol = new TBinaryProtocol($transport);
$spell = new SpellCorrectionClient($protocol);

$transport->open();

$queries = array("googla");
$res = $spell->correctme($queries);
print_r($res);

$transport->close();

?>
