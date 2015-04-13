#文档待完善更新

#目录

-----
1. [common]
   * [install](#install)
   * [usage] (#usage)
   * [connect](#connect)
   * [option](#option)
   * [auth](#auth)
2. [string]
   * [set](#set)
   * [set](#setx)
   * [setnx](#setnx)
   * [expire](#expire)
   * [ttl](#ttl)
   * [get](#get)
   * [getset](#getset)
   * [del](#del)
   * [incr](#incr)
   * [exists](#exists)
   * [getbit](#getbit)
   * [setbit](#setbit)
   * [countbit](#countbit)
   * [substr](#substr)
   * [strlen](#strlen)
   * [keys](#keys)
   * [scan](#scan)
   * [rscan](#rscan)
   * [multi_set](#multi_set)
   * [multi_get](#multi_get)
   * [multi_del](#multi_del)
3. [hash]
	* [hset](#hset)
	* [hset](#hget)
	* [hdel](#hdel)
	* [hincr](#hincr)
	* [hexists](#hexists)
	* [hsize](#hsize)
	* [hlist](#hlist)
	* [hrlist](#hrlist)
	* [hkeys](#hkeys)
	* [hsize](#hsize)
	* [hgetall](#hgetall)
	* [hscan](#hscan)
	* [hrscan](#hrscan)
	* [hclear](#hclear)
	* [multi_hset](#multi_hset)
	* [multi_hget](#multi_hget)
	* [multi_hdel](#multi_hdel)
4. [set]
   * [zset](#zset)
   * [zget](#zget)
   * [zdel](#zdel)
   * [zincr](#zincr)
   * [zexists](#zexists)
   * [zsize](#zsize)
   * [zlist](#zlist)
   * [zrlist](#zrlist)
   * [zkeys](#zkeys)
   * [zscan](#zscan)
   * [zrscan](#rzset)
   * [zrank](#zrank)
   * [zrrank](#zrrank)
   * [zrange](#zrange)
   * [zrrange](#zrrange)
   * [zclear](#zclear)
   * [zcount](#zcount)
   * [zsum](#zsum)
   * [zavg](#zavg)
   * [zremrangebyrank](#zremrangebyrank)
   * [zremrangebyscore](#zremrangebyscore)
   * [multi_zset](#multi_zset)
	* [multi_zget](#multi_zget)
	* [multi_zdel](#multi_zdel)
5. [list]  
	* [qsize](#qsize)
	* [qlist](#qlist)
	* [qrlist](#qrlist)
	* [qclear](#qclear)
	* [qfront](#qfront)
	* [qback](#qback)
	* [qget](#qget)
	* [qset](#qset)
	* [qrange](#qrange)
	* [qslice](#qslice)
	* [qpush](#qpush)
	* [qpush_front](#qpush_front)
	* [qpush_back](#qpush_back)
	* [qpop](#qpop)
	* [qpop_front](#qpop_front)
	* [qpop_back](#qpop_back)
	* [qtrim_front](#qtrim_front)
	* [qtrim_back](#qtrim_back)
	
	-----

#install
```
phpize
./configure [--with-php-config=YOUR_PHP_CONFIG_PATH] [--enable-ssdb-igbinary] 
make
make install
```

#usage
```
$ssdb_handle = new SSDB();
$ssdb_handle->connect('127.0.0.1', 8888);
$ssdb_handle->set('ssdb_version', '1.8.0');
$ssdb_handle->get('ssdb_version');
```

#connect(host, port[,timeout,persistent_id,retry_interval])
##params##
*host* string 主机

*port* int 端口

*timeout* int 超时 单位秒

*persistent_id* 用于长连接

*retry_interval*  重连间隔 单位毫秒
##return##
成功true 失败false
```
$ssdb_handle->connect("127.0.0.1", 8888);
```

#option(option_name, option_value)
##return##
成功true,失败false
```
$ssdb_handle->option(SSDB::OPT_READ_TIMEOUT, 15); //设置读取超时时间，单位秒
$ssdb_handle->option(SSDB::OPT_PREFIX, 'test_'); //设置key前缀
$ssdb_handle->option(SSDB::OPT_SERIALIZER, SSDB::SERIALIZER_PHP);//设置value压缩模式 使用压缩会导致类似substr命令返回出错
```
