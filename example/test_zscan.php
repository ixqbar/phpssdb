<?php

$ssdb_handle = new SSDB('127.0.0.1', 8888);

$i = 1;
while ($i < 1000000) {
	$result = $ssdb_handle->geo_set('geo_test', 'b' . $i, 31.196456, 121.515778);
	//echo $result . PHP_EOL;
	$i++;
}

echo "finished" . PHP_EOL;