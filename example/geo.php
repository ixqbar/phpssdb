<?php

$ssdb_handle = new SSDB('127.0.0.1', 8888);

//临沂四村
$result = $ssdb_handle->geo_set('geo_test', 'a', 31.197452, 121.515095);
echo $result . PHP_EOL;

    
$result = $ssdb_handle->geo_get('geo_test', 'a');
print_r($result);
    
//临沂五村
$result = $ssdb_handle->geo_set('geo_test', 'b', 31.196456, 121.515778);
echo $result . PHP_EOL;

$result = $ssdb_handle->geo_get('geo_test', 'b');
print_r($result);

//临沂六村
$result = $ssdb_handle->geo_set('geo_test', 'c', 31.197159, 121.518015);
echo $result . PHP_EOL;

$result = $ssdb_handle->geo_get('geo_test', 'c');
print_r($result);

//临沂路
$result = $ssdb_handle->geo_set('geo_test', 'd', 31.196282, 121.51563);
echo $result . PHP_EOL;

$result = $ssdb_handle->geo_get('geo_test', 'd');
print_r($result);

//临沂一村
$result = $ssdb_handle->geo_set('geo_test', 'e', 31.203159, 121.518082);
echo $result . PHP_EOL;

$result = $ssdb_handle->geo_get('geo_test', 'e');
print_r($result);

//company
$result = $ssdb_handle->geo_set('geo_test', 'f', 31.211745, 121.485553);
echo $result . PHP_EOL;

$result = $ssdb_handle->geo_neighbour('geo_test', 'b', 4000);
print_r($result);

$result = $ssdb_handle->geo_neighbour('geo_test', 'b', 4000, 3);
print_r($result);

$result = $ssdb_handle->geo_distance('geo_test', 'b', 'e');
echo $result . PHP_EOL;

echo ssdb_geo_distance(31.196456, 121.515778, 31.203159, 121.518082) . PHP_EOL;