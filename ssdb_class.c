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

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "Zend/zend_exceptions.h"

#include "ssdb_library.h"
#include "ssdb_class.h"
#include "php_ssdb.h"

int le_ssdb_sock;
zend_class_entry *ssdb_ce;
zend_class_entry *ssdb_exception_ce;
zend_class_entry *spl_ce_RuntimeException = NULL;

//异常
PHP_SSDB_API zend_class_entry *ssdb_get_exception_base(int root TSRMLS_DC) {
#if HAVE_SPL
    if (!root) {
        if (!spl_ce_RuntimeException) {
            zend_class_entry **pce;
            if (zend_hash_find(CG(class_table), "runtimeexception", sizeof("RuntimeException"), (void **) &pce) == SUCCESS) {
                spl_ce_RuntimeException = *pce;
                return *pce;
            }
        } else {
            return spl_ce_RuntimeException;
        }
    }
#endif
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 2)
    return zend_exception_get_default();
#else
    return zend_exception_get_default(TSRMLS_C);
#endif
}

//获取连接或重新连接
PHP_SSDB_API int ssdb_sock_get(zval *id, SSDBSock **ssdb_sock TSRMLS_DC, int no_throw) {
    zval **socket;
    int resource_type;

    if (Z_TYPE_P(id) != IS_OBJECT || zend_hash_find(Z_OBJPROP_P(id), "socket", sizeof("socket"), (void **) &socket) == FAILURE) {
        if (!no_throw) {
        	zend_throw_exception(ssdb_exception_ce, "SSDB server went away", 0 TSRMLS_CC);
        }

        return -1;
    }

    *ssdb_sock = (SSDBSock *) zend_list_find(Z_LVAL_PP(socket), &resource_type);

    if (!*ssdb_sock || resource_type != le_ssdb_sock) {
    	if (!no_throw) {
    		zend_throw_exception(ssdb_exception_ce, "SSDB server went away", 0 TSRMLS_CC);
    	}
		return -1;
    }

    if ((*ssdb_sock)->lazy_connect) {
        (*ssdb_sock)->lazy_connect = 0;
        if (ssdb_open_socket(*ssdb_sock, 1 TSRMLS_CC) < 0) {
            return -1;
        }
    }

    return Z_LVAL_PP(socket);
}

//连接
PHP_SSDB_API int ssdb_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent) {
	zval *object;
	zval **socket;
	int host_len;
	int id;
	char *host = NULL;
	long port = -1;
	double timeout = 0.0;
	SSDBSock *ssdb_sock  = NULL;
	char *persistent_id = NULL;
	int persistent_id_len = -1;
	long retry_interval = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|ldsl",
			&object, ssdb_ce,
			&host, &host_len,
			&port, &timeout,
			&persistent_id, &persistent_id_len,
			&retry_interval) == FAILURE) {
		return FAILURE;
	}

	if (timeout < 0L || timeout > INT_MAX) {
		zend_throw_exception(ssdb_exception_ce, "Invalid timeout", 0 TSRMLS_CC);
		return FAILURE;
	}

	if (port == -1) {
		port = 8888;
	}

	if (timeout == 0.0) {
		timeout = 30.0;
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 1) > 0) {
		if (zend_hash_find(Z_OBJPROP_P(object), "socket", sizeof("socket"), (void **)&socket) == SUCCESS) {
			zend_list_delete(Z_LVAL_PP(socket));
		}
	}

	ssdb_sock = ssdb_create_sock(host, host_len, port, timeout, persistent, persistent_id, retry_interval, 0);
	if (ssdb_open_socket(ssdb_sock, 1) < 0) {
		ssdb_free_socket(ssdb_sock);
		return FAILURE;
	}

#if PHP_VERSION_ID >= 50400
	id = zend_list_insert(ssdb_sock, le_ssdb_sock TSRMLS_CC);
#else
	id = zend_list_insert(ssdb_sock, le_ssdb_sock);
#endif
	add_property_resource(object, "socket", id);

	return SUCCESS;
}

//构造函数
PHP_METHOD(SSDB, __construct) {
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
        RETURN_FALSE;
    }
}

//选项设置
PHP_METHOD(SSDB, option) {
	SSDBSock *ssdb_sock;
	zval *object;
	long option, val_long;
	char *val_str;
	int val_len;
	struct timeval read_tv;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ols",
									 &object, ssdb_ce,
									 &option,
									 &val_str, &val_len) == FAILURE) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	switch(option) {
		case SSDB_OPT_PREFIX:
			if (ssdb_sock->prefix) {
				efree(ssdb_sock->prefix);
			}
			if (val_len == 0) {
				ssdb_sock->prefix = NULL;
				ssdb_sock->prefix_len = 0;
			} else {
				ssdb_sock->prefix_len = val_len;
				ssdb_sock->prefix = ecalloc(1 + val_len, 1);
				memcpy(ssdb_sock->prefix, val_str, val_len);
			}
			RETVAL_TRUE;
			break;
		case SSDB_OPT_READ_TIMEOUT:
			ssdb_sock->read_timeout = atof(val_str);
			if (ssdb_sock->stream) {
				read_tv.tv_sec  = (time_t)ssdb_sock->read_timeout;
				read_tv.tv_usec = (int)((ssdb_sock->read_timeout - read_tv.tv_sec) * 1000000);
				php_stream_set_option(ssdb_sock->stream, PHP_STREAM_OPTION_READ_TIMEOUT,0, &read_tv);
			}
			RETVAL_TRUE;
			break;
		case SSDB_OPT_SERIALIZER:
			val_long = atol(val_str);
			if(val_long == SSDB_SERIALIZER_NONE
#ifdef HAVE_SSDB_IGBINARY
				|| val_long == SSDB_SERIALIZER_IGBINARY
#endif
				|| val_long == SSDB_SERIALIZER_PHP) {
				ssdb_sock->serializer = val_long;
				RETVAL_TRUE;
			} else {
				RETVAL_FALSE;
			}
			break;
		default:
			RETVAL_FALSE;
	}
}

//连接
PHP_METHOD(SSDB, connect) {
	if (ssdb_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) == FAILURE) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}

PHP_METHOD(SSDB, auth) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *password = NULL, *cmd;
	int password_len = 0, cmd_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
			&object, ssdb_ce,
			&password, &password_len) == FAILURE
			|| 0 == password_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "auth", password, NULL);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, set) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *value = NULL, *expire = NULL, *cmd;
	int key_len = 0, value_len = 0, expire_len = 0, cmd_len;
	int key_free, value_free;
	zval *z_value;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osz|s",
			&object, ssdb_ce,
			&key, &key_len,
			&z_value,
			&expire, &expire_len) == FAILURE
			|| 0 == key_len
			|| 0 == Z_STRLEN_P(z_value)) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	value_free = ssdb_serialize(ssdb_sock, z_value, &value, &value_len);

	if (0 == expire_len) {
		cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "set", key, value, NULL);
	} else {
		cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "setx", key, value, expire, NULL);
	}

	if (key_free) efree(key);
	if (value_free) STR_FREE(value);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, getset) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *value = NULL, *cmd;
	int key_len = 0, value_len = 0, cmd_len;
	int key_free, value_free;
	zval *z_value;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osz",
			&object, ssdb_ce,
			&key, &key_len,
			&z_value) == FAILURE
			|| 0 == key_len
			|| 0 == Z_STRLEN_P(z_value)) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	value_free = ssdb_serialize(ssdb_sock, z_value, &value, &value_len);

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "getset", key, value, NULL);

	if (key_free) efree(key);
	if (value_free) STR_FREE(value);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_string_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, countbit) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;
	long offset_index;
	long offset_size;
	int num_args = ZEND_NUM_ARGS();

	if (zend_parse_method_parameters(num_args TSRMLS_CC, getThis(), "Os|ll",
			&object, ssdb_ce,
			&key, &key_len,
			&offset_index,
			&offset_size) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);

	if (1 == num_args) {
		cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "countbit", key, NULL);
	} else if (2 == num_args) {
		char *offset_index_str = NULL;
		spprintf(&offset_index_str, 0, "%ld", offset_index);
		cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "countbit", key, offset_index_str, NULL);
		efree(offset_index_str);
	} else {
		char *offset_index_str = NULL;
		char *offset_size_str = NULL;

		spprintf(&offset_index_str, 0, "%ld", offset_index);
		spprintf(&offset_size_str, 0, "%ld", offset_size);

		cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "countbit", key, offset_index_str, offset_size_str, NULL);

		efree(offset_index_str);
		efree(offset_size_str);
	}

	if (key_free) efree(key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, substr) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;
	long offset_index;
	long offset_size;
	int num_args = ZEND_NUM_ARGS();

	if (zend_parse_method_parameters(num_args TSRMLS_CC, getThis(), "Os|ll",
			&object, ssdb_ce,
			&key, &key_len,
			&offset_index,
			&offset_size) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);

	if (1 == num_args) {
		cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "substr", key, NULL);
	} else if (2 == num_args) {
		char *offset_index_str = NULL;
		spprintf(&offset_index_str, 0, "%ld", offset_index);
		cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "substr", key, offset_index_str, NULL);
		efree(offset_index_str);
	} else {
		char *offset_index_str = NULL;
		char *offset_size_str = NULL;

		spprintf(&offset_index_str, 0, "%ld", offset_index);
		spprintf(&offset_size_str, 0, "%ld", offset_size);

		cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "substr", key, offset_index_str, offset_size_str, NULL);

		efree(offset_index_str);
		efree(offset_size_str);
	}

	if (key_free) efree(key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_string_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, setbit) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;
	long offset_index;
	long offset_value;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osll",
			&object, ssdb_ce,
			&key, &key_len,
			&offset_index,
			&offset_value) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	char *offset_index_str = NULL;
	spprintf(&offset_index_str, 0, "%ld", offset_index);

	char *offset_value_str = NULL;
	spprintf(&offset_value_str, 0, "%ld", offset_value);

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "setbit", key, offset_index_str, offset_value_str, NULL);

	if (key_free) efree(key);
	efree(offset_index_str);
	efree(offset_value_str);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, setnx) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *value = NULL, *cmd;
	int key_len = 0, value_len = 0, cmd_len;
	int key_free, value_free;
	zval *z_value;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osz",
			&object, ssdb_ce,
			&key, &key_len,
			&z_value) == FAILURE
			|| 0 == key_len
			|| 0 == Z_STRLEN_P(z_value)) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	value_free = ssdb_serialize(ssdb_sock, z_value, &value, &value_len);

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "setnx", key, value, NULL);

	if (key_free) efree(key);
	if (value_free) STR_FREE(value);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, del) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
			&object, ssdb_ce,
			&key, &key_len) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "del", key, NULL);
	if (key_free) efree(key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, exists) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
			&object, ssdb_ce,
			&key, &key_len) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "exists", key, NULL);
	if (key_free) efree(key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, get) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
			&object, ssdb_ce,
			&key, &key_len) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "get", key, NULL);
	if (key_free) efree(key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_string_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, strlen) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
			&object, ssdb_ce,
			&key, &key_len) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "strlen", key, NULL);
	if (key_free) efree(key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, getbit) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;
	long offset = 0;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|l",
			&object, ssdb_ce,
			&key, &key_len,
			&offset) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	char *offset_str = NULL;
	spprintf(&offset_str, 0, "%ld", offset);

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "getbit", key, offset_str, NULL);
	if (key_free) efree(key);
	efree(offset_str);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, ttl) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
			&object, ssdb_ce,
			&key, &key_len) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "ttl", key, NULL);
	if (key_free) efree(key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, incr) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;
	long incr_number = 1;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|l",
			&object, ssdb_ce,
			&key, &key_len,
			&incr_number) == FAILURE
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	char *incr_number_str = NULL;
	spprintf(&incr_number_str, 0, "%ld", incr_number);

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "incr", key, incr_number_str, NULL);
	if (key_free) efree(key);
	efree(incr_number_str);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, expire) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key = NULL, *cmd;
	int key_len = 0, cmd_len;
	int key_free;
	long expire;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osl",
			&object, ssdb_ce,
			&key, &key_len,
			&expire) == FAILURE) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	char *expire_str = NULL;
	spprintf(&expire_str, 0, "%ld", expire);

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "expire", key, expire_str, NULL);

	if (key_free) efree(key);
	efree(expire_str);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, keys) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key_start = NULL, *key_stop = NULL, *cmd;
	int key_start_len = 0, key_stop_len = 0, cmd_len;
	long limit;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ossl",
			&object, ssdb_ce,
			&key_start, &key_start_len,
			&key_stop, &key_stop_len,
			&limit) == FAILURE
			|| limit <= 0) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	char *limit_str = NULL;
	spprintf(&limit_str, 0, "%ld", limit);

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "keys", key_start, key_stop, limit_str, NULL);
	efree(limit_str);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_list_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock, SSDB_FILTER_KEY_PREFIX, SSDB_UNSERIALIZE_NONE);
}

PHP_METHOD(SSDB, scan) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key_start = NULL, *key_stop = NULL, *cmd;
	int key_start_len = 0, key_stop_len = 0, cmd_len;
	long limit;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ossl",
			&object, ssdb_ce,
			&key_start, &key_start_len,
			&key_stop, &key_stop_len,
			&limit) == FAILURE
			|| limit <= 0) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	char *limit_str = NULL;
	spprintf(&limit_str, 0, "%ld", limit);

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "scan", key_start, key_stop, limit_str, NULL);
	efree(limit_str);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_map_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock, SSDB_FILTER_KEY_PREFIX, SSDB_UNSERIALIZE);
}

PHP_METHOD(SSDB, rscan) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *key_start = NULL, *key_stop = NULL, *cmd;
	int key_start_len = 0, key_stop_len = 0, cmd_len;
	long limit;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ossl",
			&object, ssdb_ce,
			&key_start, &key_start_len,
			&key_stop, &key_stop_len,
			&limit) == FAILURE
			|| limit <= 0) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	char *limit_str = NULL;
	spprintf(&limit_str, 0, "%ld", limit);

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "rscan", key_start, key_stop, limit_str, NULL);
	efree(limit_str);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_map_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock, SSDB_FILTER_KEY_PREFIX, SSDB_UNSERIALIZE);
}

PHP_METHOD(SSDB, multi_set) {
	zval *object, *z_args;
	SSDBSock *ssdb_sock;
	char *cmd;
	int cmd_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oz",
			&object, ssdb_ce,
			&z_args) == FAILURE
			|| Z_TYPE_P(z_args) != IS_ARRAY) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	cmd_len = ssdb_cmd_format_by_zval(ssdb_sock, &cmd, "multi_set", 9, z_args, 1);
	if (0 == cmd_len) {
		RETURN_NULL();
	}

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, multi_get) {
	zval *object, *z_args;
	SSDBSock *ssdb_sock;
	char *cmd;
	int cmd_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oz",
			&object, ssdb_ce,
			&z_args) == FAILURE
			|| Z_TYPE_P(z_args) != IS_ARRAY) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	cmd_len = ssdb_cmd_format_by_zval(ssdb_sock, &cmd, "multi_get", 9, z_args, 0);
	if (0 == cmd_len) {
		RETURN_NULL();
	}

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_map_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock, SSDB_FILTER_KEY_PREFIX, SSDB_UNSERIALIZE);
}

PHP_METHOD(SSDB, multi_del) {
	zval *object, *z_args;
	SSDBSock *ssdb_sock;
	char *cmd;
	int cmd_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oz",
			&object, ssdb_ce,
			&z_args) == FAILURE
			|| Z_TYPE_P(z_args) != IS_ARRAY) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	cmd_len = ssdb_cmd_format_by_zval(ssdb_sock, &cmd, "multi_del", 9, z_args, 0);
	if (0 == cmd_len) {
		RETURN_NULL();
	}

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, hset) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *hash_key = NULL, *key = NULL, *key_value = NULL, *cmd;
	int hash_key_len = 0, hash_key_free = 0, key_len = 0, key_value_len = 0, key_value_free = 0, cmd_len;
	zval *z_value;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ossz",
			&object, ssdb_ce,
			&hash_key, &hash_key_len,
			&key, &key_len,
			&z_value) == FAILURE
			|| 0 == hash_key_len
			|| 0 == key_len
			|| 0 == Z_STRLEN_P(z_value)) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	hash_key_free = ssdb_key_prefix(ssdb_sock, &hash_key, &hash_key_len);
	key_value_free = ssdb_serialize(ssdb_sock, z_value, &key_value, &key_value_len);

	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "hset", hash_key, key, key_value, NULL);

	if (hash_key_free) efree(hash_key);
	if (key_value_free) STR_FREE(key_value);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, hget) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *hash_key = NULL, *key = NULL, *cmd;
	int hash_key_len = 0, hash_key_free = 0, key_len = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
			&object, ssdb_ce,
			&hash_key, &hash_key_len,
			&key, &key_len) == FAILURE
			|| 0 == hash_key_len
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	hash_key_free = ssdb_key_prefix(ssdb_sock, &hash_key, &hash_key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "hget", hash_key, key, NULL);

	if (hash_key_free) efree(hash_key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_string_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, hdel) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *hash_key = NULL, *key = NULL, *cmd;
	int hash_key_len = 0, hash_key_free = 0, key_len = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
			&object, ssdb_ce,
			&hash_key, &hash_key_len,
			&key, &key_len) == FAILURE
			|| 0 == hash_key_len
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	hash_key_free = ssdb_key_prefix(ssdb_sock, &hash_key, &hash_key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "hdel", hash_key, key, NULL);

	if (hash_key_free) efree(hash_key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, hincr) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *hash_key = NULL, *key = NULL, *cmd;
	int hash_key_len = 0, hash_key_free = 0, key_len = 0, cmd_len;
	int key_free;
	long incr_number;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ossl",
			&object, ssdb_ce,
			&hash_key, &hash_key_len,
			&key, &key_len,
			&incr_number) == FAILURE
			|| 0 == hash_key_len
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	char *incr_number_str = NULL;
	spprintf(&incr_number_str, 0, "%ld", incr_number);

	hash_key_free = ssdb_key_prefix(ssdb_sock, &hash_key, &hash_key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "hincr", hash_key, key, incr_number_str, NULL);

	if (hash_key_free) efree(hash_key);
	efree(incr_number_str);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, hexists) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *hash_key = NULL, *key = NULL, *cmd;
	int hash_key_len = 0, hash_key_free = 0, key_len = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
			&object, ssdb_ce,
			&hash_key, &hash_key_len,
			&key, &key_len) == FAILURE
			|| 0 == hash_key_len
			|| 0 == key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	hash_key_free = ssdb_key_prefix(ssdb_sock, &hash_key, &hash_key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "hexists", hash_key, key, NULL);

	if (hash_key_free) efree(hash_key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_bool_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

PHP_METHOD(SSDB, hsize) {
	zval *object;
	SSDBSock *ssdb_sock;
	char *hash_key = NULL, *cmd;
	int hash_key_len = 0, hash_key_free = 0, cmd_len;
	int key_free;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
			&object, ssdb_ce,
			&hash_key, &hash_key_len) == FAILURE
			|| 0 == hash_key_len) {
		RETURN_NULL();
	}

	if (ssdb_sock_get(object, &ssdb_sock TSRMLS_CC, 0) < 0) {
		RETURN_NULL();
	}

	hash_key_free = ssdb_key_prefix(ssdb_sock, &hash_key, &hash_key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, "hsize", hash_key, NULL);

	if (hash_key_free) efree(hash_key);

	SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len);

	ssdb_number_response(INTERNAL_FUNCTION_PARAM_PASSTHRU, ssdb_sock);
}

static void ssdb_destructor_socket(zend_rsrc_list_entry * rsrc TSRMLS_DC) {
	SSDBSock *ssdb_sock = (SSDBSock *) rsrc->ptr;
	ssdb_disconnect_socket(ssdb_sock);
    ssdb_free_socket(ssdb_sock);
}

//方法列表
const zend_function_entry ssdb_class_methods[] = {
	PHP_ME(SSDB, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(SSDB, option,      NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, connect,     NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, auth,        NULL, ZEND_ACC_PUBLIC)
	//string
	PHP_ME(SSDB, countbit,    NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, del,         NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, incr,        NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, set,         NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, setbit,      NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, setnx,       NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, substr,      NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, strlen,      NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, get,         NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, getbit,      NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, getset,      NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, expire,      NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, exists,      NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, ttl,         NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, keys,        NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, scan,        NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, rscan,       NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, multi_set,   NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, multi_get,   NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, multi_del,   NULL, ZEND_ACC_PUBLIC)
	//hash
	PHP_ME(SSDB, hset,        NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, hget,        NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, hdel,        NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, hincr,       NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, hexists,     NULL, ZEND_ACC_PUBLIC)
	PHP_ME(SSDB, hsize,       NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//注册
void register_ssdb_class(int module_number TSRMLS_DC) {
	//异常类
	zend_class_entry ece;
	INIT_CLASS_ENTRY(ece, "SSDBException", NULL);
	ssdb_exception_ce = zend_register_internal_class_ex(
		&ece,
		ssdb_get_exception_base(0 TSRMLS_CC),
		NULL TSRMLS_CC
	);

	zend_declare_property_null(ssdb_exception_ce, ZEND_STRL("message"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(ssdb_exception_ce, ZEND_STRL("code"), 0,	ZEND_ACC_PROTECTED TSRMLS_CC);

	//类
	zend_class_entry cce;
	INIT_CLASS_ENTRY(cce, "SSDB", ssdb_class_methods);
	ssdb_ce = zend_register_internal_class(&cce TSRMLS_CC);
	le_ssdb_sock = zend_register_list_destructors_ex(ssdb_destructor_socket, NULL, "SSDB Socket Buffer", module_number);

	zend_declare_class_constant_long(ssdb_ce,    ZEND_STRL("OPT_PREFIX"),       SSDB_OPT_PREFIX TSRMLS_CC);
	zend_declare_class_constant_long(ssdb_ce,    ZEND_STRL("OPT_READ_TIMEOUT"), SSDB_OPT_READ_TIMEOUT TSRMLS_CC);
	zend_declare_class_constant_long(ssdb_ce,    ZEND_STRL("OPT_SERIALIZER"),   SSDB_OPT_SERIALIZER TSRMLS_CC);
	zend_declare_class_constant_stringl(ssdb_ce, ZEND_STRL("VERSION"),          ZEND_STRL(PHP_SSDB_VERSION) TSRMLS_CC);

	zend_declare_class_constant_long(ssdb_ce,    ZEND_STRL("SERIALIZER_NONE"),     SSDB_SERIALIZER_NONE TSRMLS_CC);
	zend_declare_class_constant_long(ssdb_ce,    ZEND_STRL("SERIALIZER_PHP"),      SSDB_SERIALIZER_PHP TSRMLS_CC);
	zend_declare_class_constant_long(ssdb_ce,    ZEND_STRL("SERIALIZER_IGBINARY"), SSDB_SERIALIZER_IGBINARY TSRMLS_CC);
}
