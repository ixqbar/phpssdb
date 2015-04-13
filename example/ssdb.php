<?php

$ssdb_handle = new SSDB();

$ssdb_handle->connect('127.0.0.1', 8888);

$ssdb_handle->option(SSDB::OPT_PREFIX, 'test_');
//使用压缩会导致类似substr命令返回出错
//$ssdb_handle->option(SSDB::OPT_SERIALIZER, SSDB::SERIALIZER_PHP);

$result = $ssdb_handle->auth("xingqiba");
var_dump($result);

$result = $ssdb_handle->set("name", "xingqiba");
var_dump($result);

$result = $ssdb_handle->get("name");
var_dump($result);
    
$result = $ssdb_handle->setnx("location", "shanghai");
var_dump($result);
    
$result = $ssdb_handle->get("location");
var_dump($result);
    
$result = $ssdb_handle->del("location");
var_dump($result);
    
$result = $ssdb_handle->get("location");
var_dump($result);

$result = $ssdb_handle->get("blog");
var_dump($result);
    
$result = $ssdb_handle->set("blog", "http://xingqiba.sinaapp.com/", 5);
var_dump($result);
    
sleep(3);

$result = $ssdb_handle->get("blog");
var_dump($result);

sleep(3);

$result = $ssdb_handle->get("blog");
var_dump($result);

$result = $ssdb_handle->set("blog", "http://xingqiba.sinaapp.com/index.php");
var_dump($result);

$result = $ssdb_handle->expire("blog", 5);
var_dump($result);
sleep(3);

$result = $ssdb_handle->get("blog");
var_dump($result);

sleep(3);

$result = $ssdb_handle->get("blog");
var_dump($result);

echo "getset" . PHP_EOL;
$result = $ssdb_handle->getset("blog", "http://xingqiba.sinaapp.com/index.php");
var_dump($result);
    
$result = $ssdb_handle->getset("blog", "http://xingqiba.sinaapp.com");
var_dump($result);
    
$result = $ssdb_handle->get("blog");
var_dump($result);

echo "hits" . PHP_EOL;
$result = $ssdb_handle->incr("hits");
var_dump($result);
    
echo "exists" . PHP_EOL;
$result = $ssdb_handle->exists("blog");
var_dump($result);
    
echo "exists" . PHP_EOL;
$result = $ssdb_handle->exists("weibo");
var_dump($result);

echo "setbit" . PHP_EOL;
$result = $ssdb_handle->setbit("friends", 0, 1);
var_dump($result);

echo "setbit" . PHP_EOL;
$result = $ssdb_handle->setbit("friends", 1, 0);
var_dump($result);

echo "setbit" . PHP_EOL;
$result = $ssdb_handle->setbit("friends", 2, 1);
var_dump($result);

echo "getbit" . PHP_EOL;
$result = $ssdb_handle->getbit("friends", 1);
var_dump($result);

echo "countbit" . PHP_EOL;
$result = $ssdb_handle->countbit("friends");
var_dump($result);

echo "countbit" . PHP_EOL;
$result = $ssdb_handle->countbit("friends", 0);
var_dump($result);

echo "setbit" . PHP_EOL;
$result = $ssdb_handle->setbit("friends", 2, 0);
var_dump($result);

echo "strlen" . PHP_EOL;
$result = $ssdb_handle->strlen("friends");
var_dump($result);

echo "strlen" . PHP_EOL;
$result = $ssdb_handle->strlen("name");
var_dump($result);

echo "keys" . PHP_EOL;
$result = $ssdb_handle->keys("", "", 100);
var_dump($result);

echo "scan" . PHP_EOL;
$result = $ssdb_handle->scan("", "", 100);
var_dump($result);

echo "multi_set" . PHP_EOL;
$result = $ssdb_handle->multi_set(array("extension" => "ssdb", "version" => "0.0.1"));
var_dump($result);

echo "multi_get" . PHP_EOL;
$result = $ssdb_handle->multi_get(array("extension", "version"));
var_dump($result);

echo "rscan" . PHP_EOL;
$result = $ssdb_handle->rscan("", "", 100);
var_dump($result);

echo "multi_del" . PHP_EOL;
$result = $ssdb_handle->multi_del(array("extension", "version"));
var_dump($result);

echo "hset" . PHP_EOL;
$result = $ssdb_handle->hset("blog_list", "php_ssdb_extension", "20150413");
var_dump($result);

echo "hget" . PHP_EOL;
$result = $ssdb_handle->hget("blog_list", "php_ssdb_extension");
var_dump($result);

echo "hdel" . PHP_EOL;
$result = $ssdb_handle->hdel("blog_list", "php_ssdb_extension");
var_dump($result);

echo "hexists" . PHP_EOL;
$result = $ssdb_handle->hexists("blog_list", "php_ssdb_extension");
var_dump($result);

echo "hsize" . PHP_EOL;
$result = $ssdb_handle->hsize("blog_list");
var_dump($result);

echo "hincr" . PHP_EOL;
$result = $ssdb_handle->hincr("blog_list", "total", 1);
var_dump($result);

echo "hsize" . PHP_EOL;
$result = $ssdb_handle->hsize("blog_list");
var_dump($result);