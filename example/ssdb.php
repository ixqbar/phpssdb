<?php

$ssdb_handle = new SSDB();

$ssdb_handle->connect('127.0.0.1', 8888);

$ssdb_handle->option(SSDB::OPT_PREFIX, 'test_');
//设置value压缩模式 使用压缩会导致类似substr命令返回出错
//$ssdb_handle->option(SSDB::OPT_SERIALIZER, SSDB::SERIALIZER_PHP);

$result = $ssdb_handle->auth("xingqiba");
var_dump($result);

$result = $ssdb_handle->set("name", "xingqiba");
var_dump($result);

echo "substr" . PHP_EOL;
$result = $ssdb_handle->substr("name");
var_dump($result);

echo "substr" . PHP_EOL;
$result = $ssdb_handle->substr("name", 1, 3);
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

$result = $ssdb_handle->getset("blog", "http://xingqiba.sinaapp.com/");
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

echo "hset" . PHP_EOL;
$result = $ssdb_handle->hset("blog_list", "php_ssdb_url", "http://xingqiba.sinaapp.com/");
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

echo "hlist" . PHP_EOL;
$result = $ssdb_handle->hlist("", "", 10);
var_dump($result);

echo "hrlist" . PHP_EOL;
$result = $ssdb_handle->hrlist("", "", 10);
var_dump($result);

echo "hkeys" . PHP_EOL;
$result = $ssdb_handle->hkeys("blog_list", "", "", 10);
var_dump($result);

echo "hgetall" . PHP_EOL;
$result = $ssdb_handle->hgetall("blog_list");
var_dump($result);

echo "hscan" . PHP_EOL;
$result = $ssdb_handle->hscan("blog_list", "", "", 100);
var_dump($result);

echo "hrscan" . PHP_EOL;
$result = $ssdb_handle->hrscan("blog_list", "", "", 100);
var_dump($result);

echo "hclear" . PHP_EOL;
$result = $ssdb_handle->hclear("blog_list");
var_dump($result);

echo "hsize" . PHP_EOL;
$result = $ssdb_handle->hsize("blog_list");
var_dump($result);

echo "hlist" . PHP_EOL;
$result = $ssdb_handle->hlist("", "", 10);
var_dump($result);

echo "multi_hset" . PHP_EOL;
$result = $ssdb_handle->multi_hset("blog_list", array(
    "name" => "xingqiba",
    "location" => "shanghai"
));
var_dump($result);

echo "hgetall" . PHP_EOL;
$result = $ssdb_handle->hgetall("blog_list");
var_dump($result);

echo "multi_hget" . PHP_EOL;
$result = $ssdb_handle->multi_hget("blog_list", array("name", "location"));
var_dump($result);

echo "multi_hdel" . PHP_EOL;
$result = $ssdb_handle->multi_hdel("blog_list", array("name", "position"));
var_dump($result);

echo "multi_hget" . PHP_EOL;
$result = $ssdb_handle->multi_hget("blog_list", array("name", "location"));
var_dump($result);

//set
echo "zclear" . PHP_EOL;
$result = $ssdb_handle->zclear("blog_online");
var_dump($result);

echo "zset" . PHP_EOL;
$result = $ssdb_handle->zset("blog_online", "zhangsan", 1);
var_dump($result);

echo "zset" . PHP_EOL;
$result = $ssdb_handle->zset("blog_online", "lisi", 2);
var_dump($result);

echo "zset" . PHP_EOL;
$result = $ssdb_handle->zset("blog_online", "wangwu", 3);
var_dump($result);

//ssdb的set不支持double
echo "multi_zset" . PHP_EOL;
$result = $ssdb_handle->multi_zset("blog_online", array("guanyu" => 4.4, "liubei" => 5.12));
var_dump($result);

echo "zget" . PHP_EOL;
$result = $ssdb_handle->zget("blog_online", "wangwu");
var_dump($result);

echo "zget" . PHP_EOL;
$result = $ssdb_handle->zget("blog_online", "none");
var_dump($result);

echo "zexists" . PHP_EOL;
$result = $ssdb_handle->zexists("blog_online", "wangwu");
var_dump($result);

echo "zdel" . PHP_EOL;
$result = $ssdb_handle->zdel("blog_online", "none");
var_dump($result);

echo "zdel" . PHP_EOL;
$result = $ssdb_handle->zdel("blog_online", "wangwu");
var_dump($result);

echo "zsize" . PHP_EOL;
$result = $ssdb_handle->zsize("blog_online");
var_dump($result);

echo "zincr" . PHP_EOL;
$result = $ssdb_handle->zincr("blog_online", "wangwu", 10);
var_dump($result);

echo "zget" . PHP_EOL;
$result = $ssdb_handle->zget("blog_online", "wangwu");
var_dump($result);

echo "zsize" . PHP_EOL;
$result = $ssdb_handle->zsize("blog_online");
var_dump($result);

echo "zlist" . PHP_EOL;
$result = $ssdb_handle->zlist("", "", 100);
var_dump($result);

echo "zrlist" . PHP_EOL;
$result = $ssdb_handle->zrlist("", "", 100);
var_dump($result);

echo "zkeys" . PHP_EOL;
$result = $ssdb_handle->zkeys("blog_online", "", 0, 100, 100);
var_dump($result);

echo "zscan" . PHP_EOL;
$result = $ssdb_handle->zscan("blog_online", "",0, 100, 100);
var_dump($result);

echo "zrscan" . PHP_EOL;
$result = $ssdb_handle->zrscan("blog_online", "",0, 100, 100);
var_dump($result);

echo "zrank" . PHP_EOL;
$result = $ssdb_handle->zrank("blog_online", "wangwu");
var_dump($result);

echo "zrrank" . PHP_EOL;
$result = $ssdb_handle->zrrank("blog_online", "wangwu");
var_dump($result);

echo "zrange" . PHP_EOL;
$result = $ssdb_handle->zrange("blog_online", 0, 10);
var_dump($result);

echo "zrrange" . PHP_EOL;
$result = $ssdb_handle->zrrange("blog_online", 0, 10);
var_dump($result);

echo "zcount" . PHP_EOL;
$result = $ssdb_handle->zcount("blog_online", 0, 100);
var_dump($result);

echo "zsum" . PHP_EOL;
$result = $ssdb_handle->zsum("blog_online", 0, 100);
var_dump($result);

echo "zavg" . PHP_EOL;
$result = $ssdb_handle->zavg("blog_online", 0, 100);
var_dump($result);

echo "multi_zget" . PHP_EOL;
$result = $ssdb_handle->multi_zget("blog_online", array("guanyu", "liubei"));
var_dump($result);

echo "multi_zdel" . PHP_EOL;
$result = $ssdb_handle->multi_zdel("blog_online", array("guanyu"));
var_dump($result);

echo "multi_zget" . PHP_EOL;
$result = $ssdb_handle->multi_zget("blog_online", array("guanyu", "liubei"));
var_dump($result);

//list
echo "qsize" . PHP_EOL;
$result = $ssdb_handle->qsize("queue");
var_dump($result);

echo "qset" . PHP_EOL;
$result = $ssdb_handle->qset("queue", 0, "first");
var_dump($result);

echo "qpush_front" . PHP_EOL;
$result = $ssdb_handle->qpush_front("queue", "first_first");
var_dump($result);

echo "qpush_back" . PHP_EOL;
$result = $ssdb_handle->qpush_back("queue", "second");
var_dump($result);

echo "qsize" . PHP_EOL;
$result = $ssdb_handle->qsize("queue");
var_dump($result);

echo "qlist" . PHP_EOL;
$result = $ssdb_handle->qlist("", "", 100);
var_dump($result);