<?php

$ssdb = new SSDB();
$ssdb->connect('127.0.0.1', 8888, 1);
var_dump($ssdb->info());
