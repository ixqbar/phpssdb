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
PHP_METHOD(SSDB, option);
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

void register_ssdb_class(int module_number TSRMLS_DC);

#endif /* EXT_SSDB_SSDB_CLASS_H_ */
