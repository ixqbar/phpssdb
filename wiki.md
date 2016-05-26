
#More features
[DEV](https://github.com/jonnywang/phpssdb/tree/dev)

#目录

-----
1. [common]
   * [install](#install)
   * [usage] (#usage)
   * [connect/pconnect](#connect-pconnect)
   * [option](#option)
   * [auth](#auth)
   * [ping](#ping)
   * [version](#version)
   * [dbsize](#dbsize)
   * [request](#request)
   * [read/write](#read-write)
   * [info] (#info)
2. [string]
   * [set](#set)
   * [setx](#set)
   * [get](#get)
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
   * [scan](#scan-rscan)
   * [rscan](#scan-rscan)
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
	* [hlist](#hlist-hrlist)
	* [hrlist](#hlist-hrlist)
	* [hkeys](#hkeys)
	* [hsize](#hsize)
	* [hgetall](#hgetall)
	* [hscan](#hscan-hrscan)
	* [hrscan](#hscan-hrscan)
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
   * [zlist](#zlist-zrlist)
   * [zrlist](#zlist-zrlist)
   * [zkeys](#zkeys)
   * [zscan](#zscan-zrscan)
   * [zrscan](#zscan-zrscan)
   * [zrank](#zrank-zrrank)
   * [zrrank](#zrank-zrrank)
   * [zrange](#zrange-zrrange)
   * [zrrange](#zrange-zrrange)
   * [zclear](#zclear)
   * [zcount](#zcount)
   * [zsum](#zsum)
   * [zavg](#zavg)
   * [zremrangebyrank](#zremrangebyrank)
   * [zremrangebyscore](#zremrangebyscore)
   * [multi_zset](#multi_zset)
   * [multi_zget](#multi_zget)
   * [multi_zdel](#multi_zdel)
   * [zpop_front](#zpop_front)
   * [zpop_back](#zpop_back)
5. [list]  
   * [qsize](#qsize)
   * [qlist](#qlist-qrlist)
   * [qrlist](#qlist-qrlist)
   * [qclear](#qclear)
   * [qfront](#qfront)
   * [qback](#qback)
   * [qget](#qget)
   * [qset](#qset)
   * [qrange](#qrange)
   * [qslice](#qslice)
   * [qpush](#qpush-qpush_back)
   * [qpush_front](#qpush_front)
   * [qpush_back](#qpush_back)
   * [qpop](#qpop-qpop_back)
   * [qpop_front](#qpop_front)
   * [qpop_back](#qpop_back)
   * [qtrim_front](#qtrim_front)
   * [qtrim_back](#qtrim_back)
6. [geo]
   * [geo_set](#geo_set)
   * [geo_get](#geo_get)
   * [geo_neighbour] (#geo_neighbour)
   * [geo_del] (#geo_del)
   * [geo_clear] (#geo_clear)
   * [geo_distance] (#geo_distance)

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
//可省略connect方法使用$ssdb_handle = new SSDB('127.0.0.1', 8888); 
$ssdb_handle->connect('127.0.0.1', 8888);
$ssdb_handle->set('ssdb_version', '1.8.0');
$ssdb_handle->get('ssdb_version');
```
* 本扩展支持的所有命令如果返回为NULL，代表可能的错误为命令参数错误、连接中断、服务器返回失败、客户端发送失败等

#connect pconnect
#####params#####
*host* string 主机

*port* long 端口

*timeout* double 超时 单位秒

*persistent_id* 用于长连接

*retry_interval*  重连间隔 单位毫秒
#####return#####
bool
```
$ssdb_handle->connect("127.0.0.1", 8888);
```

#option
#####params#####
*option_name*   
* SSDB::OPT_PREFIX
* SSDB::OPT_READ_TIMEOUT 
* SSDB::OPT_SERIALIZER   

提供
SSDB::SERIALIZER_NONE 
SSDB::SERIALIZER_PHP 
SSDB::SERIALIZER_IGBINARY(需要编译开启)三种模式，默认无

*option_value* 
#####return#####
bool
```
$ssdb_handle->option(SSDB::OPT_READ_TIMEOUT, 15); //设置读取超时时间，单位秒
$ssdb_handle->option(SSDB::OPT_PREFIX, 'test_'); //设置key前缀
//设置value压缩模式 使用压缩会导致类似substr命令返回出错
$ssdb_handle->option(SSDB::OPT_SERIALIZER, SSDB::SERIALIZER_PHP);
```

#auth
#####params####
*password*
#####return####
bool
```
$ssdb_handle->auth('your_auth_password');
```

#ping
#####params####
*void*
#####return####
bool
```
$ssdb_handle->ping();
```

#version
#####params####
*void*
#####return####
string or NULL
```
$ssdb_handle->version(); //ssdb-server版本>=1.9.0
```

#dbsize
#####params####
*void*
#####return####
long
```
$ssdb_handle->dbsize();
```

#request
#####params####
*params*
#####return####
array
```
$ssdb_handle->request('multi_hset', 'info', 'name', 'xingqiba', 'version', '1.0.0'); //array(2)
$ssdb_handle->request('hgetall', 'info'); //array('name', 'xingqiba', 'version', '1.0.0')
```

#read write
```
$write_result = $socket_handle->write("7\nversion\n\n"); //发送指定字符串，返回bool or NULL
var_dump($write_result);

$read_buf = $socket_handle->read(100); //读取指定长度字符串，默认阻塞直到默认超时30， 超时设置请参考option
var_dump($read_buf);
```

#set
#####params#####
*key*

*key_value*

*expire*  选填参数 过期时间（单位秒） 填充时等价于setx
#####return#####
成功true,失败false
```
$ssdb_handle->set('name', 'xingqiba');
$ssdb_handle->set('blog', 'http://xingqiba.sinaapp.com/', 3600);
```

#info
####params####
void
####return####
array
```
$ssdb_handle->info();
```

#get
#####params#####
*key*
#####return#####
key存在返回对应value,否则返回NULL
```
$ssdb_handle->get('name'); //xingqiba
$ssdb_handle->get('blog'); //http://xingqiba.sinaapp.com/ or NULL
```

#getset
#####params#####
*key*
#####return#####
key存在返回对应之前value,否则返回NULL
```
$ssdb_handle->getset('name', 'i am xingqiba'); //xingqiba
$ssdb_handle->get('name'); //i am xingqiba
$ssdb_handle-> getset('none'); //NULL
```

#del
#####params#####
*key*
#####return#####
bool
```
$ssdb_handle->del('none'); //true
```

#incr
#####params#####
*key*

*incr_num* 可选 默认1
#####return#####
long
```
$ssdb_handle->incr('hits', 1); //1
$ssdb_handle->incr('hits', 1); //2
```

#exists
#####params#####
*key*
#####return#####
bool
```
$ssdb_handle-> exists('none'); //false
```

#setnx
#####params#####
*key*

*key_value*
#####return#####
bool
```
$ssdb_handle->setnx('name', 'xingqiba'); //false
```

#expire
#####params#####
*key*

*timeout*  单位 秒
#####return#####
bool
```
$ssdb_handle->expire('name', 60); //true
$ssdb_handle->expire('none', 60); //false
```

#ttl
#####params#####
*key*
#####return#####
long
```
$ssdb_handle->ttl('name'); //60
$ssdb_handle->ttl('none'); //0
```

#strlen
#####params#####
*key*
#####return#####
long
```
$ssdb_handle->set('versoion', '0.0.0');
$ssdb_handle->strlen('versoion'); //5
$ssdb_handle->strlen('none'); //0
```

#setbit
#####params#####
*key*

*offset*

*value* 0 or 1
#####return#####
bool
```
$ssdb_handle->setbit('onlinue_num', 0, 1);
```

#getbit
#####params#####
*key*

*offset*
#####return#####
0 or 1
```
$ssdb_handle->getbit('onlinue_num', 0);//1
```

#countbit
#####params#####
*key*

*start* 可选

*size* 可选
#####return#####
long
```
$ssdb_handle->countbit('onlinue_num', 0);
```

#substr
#####params#####
*key*

*start* 可选

*size* 可选
#####return#####
long
```
$ssdb_handle->substr('name');
```

#keys
#####params#####
*key_start_name* 可传递空 -inf

*key_end_name* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->keys('', '', 100);
```
* 返回(key_start_name, key_end_name]的key数组

#scan rscan
#####params#####
*key_start_name* 可传递空 -inf

*key_end_name* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->scan('', '', 100);
$ssdb_handle->rscan('', '', 100);
```
* 返回(key_start_name, key_end_name]的key-value数组

#multi_set
#####params#####
*key_var_arr*
#####return#####
long
```
$ssdb_handle->multi_set(array('name' => 'xingqiba', 'blog' => 'http://xingqiba.sinaapp.com/'));
```

#multi_get
#####params#####
*key_arr*
#####return#####
array
```
$ssdb_handle->multi_get(array('name', 'xingqiba', 'none')); // array('name' => 'xingqiba', 'blog' => 'http://xingqiba.sinaapp.com/')
```
* 如果某个key不存在, 则它不会出现在返回数组中

#multi_del
#####params#####
*key_arr*
#####return#####
long
```
$ssdb_handle->multi_del(array('name', 'none')); //1
```

#zdel
#####params#####
*key*

*member*
#####return#####
bool
```
$ssdb_handle->zdel('login', 'zhangsan');
```

#zset
#####params#####
*key*

*member*

*score* ssdb仅支持整数
#####return#####
bool
```
$ssdb_handle->zset('login', 'zhangsan', 1);
```

#zget
#####params#####
*key*

*member*
#####return#####
long
```
$ssdb_handle->zget('login', 'zhangsan'); //1
```

#zincr
#####params#####
*key*

*member*

*score*
#####return#####
long
```
$ssdb_handle->zincr('login', 'zhangsan', 1);//2
```
* 返回新的值

#zsize
#####params#####
*key*

*member*
#####return#####
long
```
$ssdb_handle->zget('login'); //1
```

#zlist zrlist
#####params#####
*key_start_name* 可传递空 -inf

*key_end_name* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->zlist('', '', 100);
$ssdb_handle->zrlist('', '', 100);
```
* 返回(key_start_name, key_end_name]的key数组

#zexists
#####params#####
*key*

*member*
#####return#####
bool
```
$ssdb_handle->zexists('login', 'zhangsan'); //true
```

#zkeys
#####params#####
*key*

*member_start_name* 可传递空

*member_start_score* 可传递空 -inf

*member_end_score* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->zkeys('login', '', 0, 100, 100);
```
* 列出zset中处于区间(member_start_name + member_start_score, member_end_score]的key列表. 
* 如果member_start_name为空, 那么对应权重值大于或者等于 member_start_score的key将被返回. 
* 如果member_start_name不为空, 那么对应权重值大于member_start_score的key, 或者大于member_start_score且对应权重值等于member_start_score的key将被返回.
* 更多可以参考ssdb-server官网[http://ssdb.io/docs/zh_cn/php/index.html中zcan](http://ssdb.io/docs/zh_cn/php/index.html)的解释

#zscan zrscan
#####params#####
*key*

*member_start_name* 可传递空

*member_start_score* 可传递空 -inf

*member_end_score* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->zscan('login', '', 0, 100, 100);
$ssdb_handle->zrscan('login', '', 0, 100, 100);
```
* 更多可以参考ssdb-server官网[http://ssdb.io/docs/zh_cn/php/index.html中zcan](http://ssdb.io/docs/zh_cn/php/index.html)的解释

#zrank zrrank
#####params#####
*key*

*member*
#####return#####
long
```
$ssdb_handle->zrank('login', 'zhangsan'); //0
$ssdb_handle->zrrank('login', 'zhangsan'); //0
$ssdb_handle->zrank('login', 'none');//NULL
```

#zrange zrrange
#####params#####
*key*

*offset*

*limit*
#####return#####
array
```
$ssdb_handle->zrange('login', 0, 100);
$ssdb_handle->zrrange('login', 0, 100);
```
* 返回[offset, offset+limit)之间key-value数组

#zlear
#####params#####
*key*
#####return#####
bool
```
$ssdb_handle->zclear('login');
```

#zcount
#####params#####
*key*

*score_start*

*score_end*
#####return#####
long
```
$ssdb_handle->zcount('login', 0, 100);
```
* 返回[score_start, score_end]之间数量

#zsum
#####params#####
*key*

*score_start*

*score_end*
#####return#####
long
```
$ssdb_handle->zsum('login', 0, 100);
```
* 返回[score_start, score_end]之间score总和

#zavg
#####params#####
*key*

*score_start*

*score_end*
#####return#####
double
```
$ssdb_handle->zavg('login', 0, 100);
```
* 返回[score_start, score_end]之间score平均值

#zremrangebyscore
#####params#####
*key*

*score_start*

*score_end*
#####return#####
long
```
$ssdb_handle->zremrangebyscore('login', 0, 100);
```
* 删除[score_start, score_end]之间member

#zremrangebyrank
#####params#####
*key*

*offset_start*

*offset_end*
#####return#####
long
```
$ssdb_handle->zremrangebyrank('login', 0, 100);
```
* 删除[offset_start, offset_end]之间member

#multi_zset
#####params#####
*key_var_arr*
#####return#####
long
```
$ssdb_handle->multi_zset('login', array('lisi' => 1, 'wangwu' => 2));
```

#multi_zget
#####params#####
*key_arr*
#####return#####
array
```
$ssdb_handle->multi_zget('login', array('lisi', 'wangwu', 'none')); // array('lisi' => 1, 'wangwu' => 2)
```
* 如果某个key不存在, 则它不会出现在返回数组中

#multi_zdel
#####params#####
*key_arr*
#####return#####
long
```
$ssdb_handle->multi_zdel('login', array('wangwu', 'none')); //1
```

#zpop_front
#####params#####
*key*

*size* 可选填 默认1
#####return#####
long
```
$ssdb_handle->zpop_front('login');
```

#zpop_back
#####params#####
*key*

*size* 可选填 默认1
#####return#####
long
```
$ssdb_handle->zpop_back('login');
```

#hset
#####params#####
*hash_key*

*key*

*value*
#####return#####
bool
```
$ssdb_handle->hset('news', 'version', '1.0.0');
```

#hget
#####params#####
*hash_key*

*key*
#####return#####
string or NULL
```
$ssdb_handle->hget('news', 'version');//1.0.0
```

#hdel
#####params#####
*hash_key*

*key*
#####return#####
bool
```
$ssdb_handle->hdel('news', 'version');
```

#hincr
#####params#####
*hash_key*

*key*

*incr_value* 可选 默认1
#####return#####
long
```
$ssdb_handle->hincr('news', 'hits');
$ssdb_handle->hincr('news', 'hits', 1);
```

#hexists
#####params#####
*hash_key*

*key*
#####return#####
bool
```
$ssdb_handle->hexists('news', 'hits'); //true
$ssdb_handle->hexists('news', 'top'); //false
```

#hsize
#####params#####
*hash_key*
#####return#####
long
```
$ssdb_handle->hsize('news');//1
```

#hlist hrlist
#####params#####
*key_start_name* 可传递空 -inf

*key_end_name* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->hlist('', '', 100);
$ssdb_handle->hrlist('', '', 100);
```
* 返回(key_start_name, key_end_name]的key数组

#hkeys
#####params#####
*hask_key*

*key_start_name* 可传递空 -inf

*key_end_name* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->hkeys('new', '', '', 100);
```
* 返回(key_start_name, key_end_name]的key数组

#hgetall
#####params#####
*hash_key*
#####return#####
array
```
$ssdb_handle->hgetall('news');
```

#hclear
#####params#####
*hash_key*
#####return#####
long
```
$ssdb_handle->hclear('news');
```

#hscan hrscan
#####params#####
*hash_key*

*key_start_name* 可传递空 -inf

*key_end_name* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->hscan('login', '', '', 100);
$ssdb_handle->hrscan('login', '', '', 100);
```
* 列出hash中key处于区间(key_start_name, key_end_name]的key-value数组

#multi_hset
#####params#####
*key_var_arr*
#####return#####
long
```
$ssdb_handle->multi_zset('news', array('version' => '1.0.0', 'author' => 'xingqiba'));
```

#multi_hget
#####params#####
*key_arr*
#####return#####
array
```
$ssdb_handle->multi_zget('news', array('version', 'none')); // array('version' => '1.0.0')
```
* 如果某个key不存在, 则它不会出现在返回数组中

#multi_hdel
#####params#####
*key_arr*
#####return#####
long
```
$ssdb_handle->multi_zdel('news', array('author', 'none')); //1
```

#qsize
#####params#####
*key*
#####return#####
long
```
$ssdb_handle->qsize('queue');//0
```

#qlist qrlist
#####params#####
*key*

*key_start_name* 可传递空 -inf

*key_end_name* 可传递空 +inf

*limit*
#####return#####
array
```
$ssdb_handle->qlist('new', '', '', 100);
$ssdb_handle->qrlist('new', '', '', 100);
```
* 返回(key_start_name, key_end_name]的key数组

#qclear
#####params#####
*key*
#####return#####
long
```
$ssdb_handle->qclear('queue');//0
```

#qpush qpush_back
#####params#####
*key*

*value*
#####return#####
long
```
$ssdb_handle->qpush('queue', 'zhangsan');
$ssdb_handle->qpush('queue', array('lisi', 'wangwu'));
$ssdb_handle->qpush_back('queue', 'zhangsan');
$ssdb_handle->qpush_back('queue', array('lisi', 'wangwu'));
```
* 往队列的尾部添加一个或者多个元素

#qpush_front
#####params#####
*key*

*value*
#####return#####
long
```
$ssdb_handle->qpush_front('queue', 'zhangsan');
$ssdb_handle->qpush_front('queue', array('lisi', 'wangwu'));
```
* 往队列的尾部添加一个或者多个元素

#qpop qpop_back
#####params#####
*key*

*size* 可选填 默认返回1个
#####return#####
string or array
```
$ssdb_handle->qpop('queue'); //返回string类型
$ssdb_handle->qpop('queue', 1); //返回数组类型
$ssdb_handle->qpop('queue', 2);
$ssdb_handle->qpop_back('queue');
$ssdb_handle->qpop_back('queue', 2);
```
* 从队列尾部弹出最后一个或者多个元素

#qpop_front
#####params#####
*key*

*size* 可选填 默认返回1个
#####return#####
string or array
```
$ssdb_handle->qpop_front('queue'); //返回string类型
$ssdb_handle->qpop_front('queue', 1); //返回数组类型
$ssdb_handle->qpop_front('queue', 2);
```
* 从队列头部删除多个元素

#qfront
#####params#####
*key*
#####return#####
string or NULL
```
$ssdb_handle->qfront('queue');
```
* 返回队列的第一个元素 

#qback
#####params#####
*key*
#####return#####
string or NULL
```
$ssdb_handle->qback('queue');
```
* 返回队列的最后一个元素

#qget
#####params#####
*key*

*offset* 可选填 默认0
#####return#####
string or NULL
```
$ssdb_handle->qget('queue');
$ssdb_handle->qget('queue', 1);
```
* 返回指定位置的元素

#qset
#####params#####
*key*

*offset*

*value*
#####return#####
bool
```
$ssdb_handle->qset('queue', 0, 'finish');
```

#qtrim_front
#####params#####
*key*

*size* 可选填 默认1
#####return#####
long
```
$ssdb_handle->qtrim_front('queue');
$ssdb_handle->qtrim_front('queue', 2);
```
* 从队列头部删除多个元素

#qtrim_back
#####params#####
*key*

*size* 可选填 默认1
#####return#####
long
```php
$ssdb_handle->qtrim_back('queue');
$ssdb_handle->qtrim_back('queue', 2);
```
* 从队列尾部删除多个元素

# geo_set
#####params#####
*key*

*member*

*latitude*

*longitude*
#####return#####
long
```php
$ssdb_handle->geo_set('geo_test', 'b', 31.196456, 121.515778);
```

# geo_get
#####params#####
*key*

*member*
#####return#####
array
```php
$ssdb_handle->geo_get('geo_test', 'b');
```

# geo_neighbour
#####params#####
*key*

*member*

*radius_meters*

*return_limit*
default all

*zscan_limit*
default 2000
#####return#####
array
```php
$ssdb_handle->geo_neighbour('geo_test', 'b', 1000);
$ssdb_handle->geo_neighbour('geo_test', 'b', 1000, 3);
```

# geo_del
#####params#####
*key*

*member*
#####return#####
boolean
```php
$ssdb_handle->geo_del('geo_test', 'b');
```

# geo_clear
#####params#####
*key*

#####return#####
boolean
```php
$ssdb_handle->geo_del('geo_test');
```

# geo_distance
#####params#####
*key*

*member*

*member*
#####return#####
double
```php
$ssdb_handle->geo_distance('geo_test', 'a', 'b');
```
