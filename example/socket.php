<?php

$socket_handle = new SSDB('192.168.1.53');
$socket_handle->option(SSDB::OPT_READ_TIMEOUT, 3);

$result = $socket_handle->write("7\nversion\n\n");
var_dump($result);

$result = $socket_handle->read(100);
var_dump($result);