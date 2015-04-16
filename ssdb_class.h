/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: xingqiba ixqbar@gmail.com                                                             |
  +----------------------------------------------------------------------+
*/

#ifndef EXT_SSDB_SSDB_CLASS_H_
#define EXT_SSDB_SSDB_CLASS_H_

#define SSDB_OPT_PREFIX		  1
#define SSDB_OPT_READ_TIMEOUT 2
#define SSDB_OPT_SERIALIZER   3

PHP_METHOD(SSDB, __construct);
PHP_METHOD(SSDB, connect);
PHP_METHOD(SSDB, auth);
PHP_METHOD(SSDB, ping);
PHP_METHOD(SSDB, option);
PHP_METHOD(SSDB, version);
PHP_METHOD(SSDB, dbsize);
//command
PHP_METHOD(SSDB, request);
//string
PHP_METHOD(SSDB, countbit);
PHP_METHOD(SSDB, del);
PHP_METHOD(SSDB, expire);
PHP_METHOD(SSDB, exists);
PHP_METHOD(SSDB, incr);
PHP_METHOD(SSDB, get);
PHP_METHOD(SSDB, getbit);
PHP_METHOD(SSDB, getset);
PHP_METHOD(SSDB, set);
PHP_METHOD(SSDB, setbit);
PHP_METHOD(SSDB, setnx);
PHP_METHOD(SSDB, strlen);
PHP_METHOD(SSDB, substr);
PHP_METHOD(SSDB, ttl);
PHP_METHOD(SSDB, keys);
PHP_METHOD(SSDB, scan);
PHP_METHOD(SSDB, rscan);
PHP_METHOD(SSDB, multi_set);
PHP_METHOD(SSDB, multi_get);
PHP_METHOD(SSDB, multi_del);
//hash
PHP_METHOD(SSDB, hset);
PHP_METHOD(SSDB, hget);
PHP_METHOD(SSDB, hdel);
PHP_METHOD(SSDB, hincr);
PHP_METHOD(SSDB, hexists);
PHP_METHOD(SSDB, hsize);
PHP_METHOD(SSDB, hlist);
PHP_METHOD(SSDB, hrlist);
PHP_METHOD(SSDB, hkeys);
PHP_METHOD(SSDB, hgetall);
PHP_METHOD(SSDB, hscan);
PHP_METHOD(SSDB, hrscan);
PHP_METHOD(SSDB, hclear);
PHP_METHOD(SSDB, multi_hset);
PHP_METHOD(SSDB, multi_hget);
PHP_METHOD(SSDB, multi_hdel);
//set
PHP_METHOD(SSDB, zset);
PHP_METHOD(SSDB, zget);
PHP_METHOD(SSDB, zdel);
PHP_METHOD(SSDB, zincr);
PHP_METHOD(SSDB, zsize);
PHP_METHOD(SSDB, zlist);
PHP_METHOD(SSDB, zrlist);
PHP_METHOD(SSDB, zexists);
PHP_METHOD(SSDB, zkeys);
PHP_METHOD(SSDB, zscan);
PHP_METHOD(SSDB, zrscan);
PHP_METHOD(SSDB, zrank);
PHP_METHOD(SSDB, zrrank);
PHP_METHOD(SSDB, zrange);
PHP_METHOD(SSDB, zrrange);
PHP_METHOD(SSDB, zclear);
PHP_METHOD(SSDB, zcount);
PHP_METHOD(SSDB, zsum);
PHP_METHOD(SSDB, zavg);
PHP_METHOD(SSDB, zremrangebyrank);
PHP_METHOD(SSDB, zremrangebyscore);
PHP_METHOD(SSDB, multi_zset);
PHP_METHOD(SSDB, multi_zget);
PHP_METHOD(SSDB, multi_zdel);
PHP_METHOD(SSDB, zpop_front);
PHP_METHOD(SSDB, zpop_back);
//list
PHP_METHOD(SSDB, qsize);
PHP_METHOD(SSDB, qlist);
PHP_METHOD(SSDB, qrlist);
PHP_METHOD(SSDB, qclear);
PHP_METHOD(SSDB, qfront);
PHP_METHOD(SSDB, qback);
PHP_METHOD(SSDB, qget);
PHP_METHOD(SSDB, qset);
PHP_METHOD(SSDB, qrange);
PHP_METHOD(SSDB, qslice);
PHP_METHOD(SSDB, qpush);
PHP_METHOD(SSDB, qpush_front);
PHP_METHOD(SSDB, qpush_back);
PHP_METHOD(SSDB, qpop);
PHP_METHOD(SSDB, qpop_front);
PHP_METHOD(SSDB, qpop_back);
PHP_METHOD(SSDB, qtrim_front);
PHP_METHOD(SSDB, qtrim_back);

void register_ssdb_class(int module_number TSRMLS_DC);

#endif /* EXT_SSDB_SSDB_CLASS_H_ */
