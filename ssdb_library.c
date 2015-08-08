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
#include "php_network.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_rand.h"
#include "Zend/zend_exceptions.h"

#include <stdlib.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "ssdb_library.h"

SSDBSock* ssdb_create_sock(
		char *host,
		int host_len,
		long port,
		long timeout,
		int persistent,
		char *persistent_id,
		long retry_interval,
		zend_bool lazy_connect) {
	SSDBSock *ssdb_sock;

	ssdb_sock = ecalloc(1, sizeof(SSDBSock));
	ssdb_sock->host = estrndup(host, host_len);
	ssdb_sock->stream = NULL;
	ssdb_sock->status = SSDB_SOCK_STATUS_DISCONNECTED;
	ssdb_sock->retry_interval = retry_interval * 1000;
	ssdb_sock->persistent = persistent;
	ssdb_sock->lazy_connect = lazy_connect;
	ssdb_sock->serializer = SSDB_SERIALIZER_NONE;

	if (persistent_id) {
		size_t persistent_id_len = strlen(persistent_id);
		ssdb_sock->persistent_id = ecalloc(persistent_id_len + 1, 1);
		memcpy(ssdb_sock->persistent_id, persistent_id, persistent_id_len);
	} else {
		ssdb_sock->persistent_id = NULL;
	}

    memcpy(ssdb_sock->host, host, host_len);
    ssdb_sock->host[host_len] = '\0';

    ssdb_sock->port = port;
    ssdb_sock->timeout = timeout;
    ssdb_sock->read_timeout = timeout;

    ssdb_sock->err = NULL;
    ssdb_sock->err_len = 0;

    return ssdb_sock;
}

void ssdb_free_socket(SSDBSock *ssdb_sock) {
    if (ssdb_sock->prefix) {
		efree(ssdb_sock->prefix);
	}
    if (ssdb_sock->err) {
		efree(ssdb_sock->err);
	}
	if (ssdb_sock->auth) {
		efree(ssdb_sock->auth);
	}
	if (ssdb_sock->persistent_id) {
		efree(ssdb_sock->persistent_id);
	}
    efree(ssdb_sock->host);
    efree(ssdb_sock);
}

void ssdb_stream_close(SSDBSock *ssdb_sock) {
	if (!ssdb_sock->persistent) {
		php_stream_close(ssdb_sock->stream);
	} else {
		php_stream_pclose(ssdb_sock->stream);
	}
}

int ssdb_open_socket(SSDBSock *ssdb_sock, int force_connect) {
    int result = -1;

    switch (ssdb_sock->status) {
        case SSDB_SOCK_STATUS_DISCONNECTED:
            return ssdb_connect_socket(ssdb_sock);
        case SSDB_SOCK_STATUS_CONNECTED:
        	result = 0;
        break;
        case SSDB_SOCK_STATUS_UNKNOWN:
            if (force_connect > 0 && ssdb_connect_socket(ssdb_sock) < 0) {
            	result = -1;
            } else {
            	result = 0;
                ssdb_sock->status = SSDB_SOCK_STATUS_CONNECTED;
            }
        break;
    }

    return result;
}

int ssdb_connect_socket(SSDBSock *ssdb_sock) {
    struct timeval tv, read_tv, *tv_ptr = NULL;
    char *host = NULL, *persistent_id = NULL, *errstr = NULL;
    int host_len, err = 0;
	php_netstream_data_t *sock;
	int tcp_flag = 1;

    if (ssdb_sock->stream != NULL) {
    	ssdb_disconnect_socket(ssdb_sock);
    }

    tv.tv_sec  = (time_t)ssdb_sock->timeout;
    tv.tv_usec = (int)((ssdb_sock->timeout - tv.tv_sec) * 1000000);
    if (tv.tv_sec != 0 || tv.tv_usec != 0) {
	    tv_ptr = &tv;
    }

    read_tv.tv_sec  = (time_t)ssdb_sock->read_timeout;
    read_tv.tv_usec = (int)((ssdb_sock->read_timeout - read_tv.tv_sec) * 1000000);

    if (ssdb_sock->port == 0) {
		ssdb_sock->port = 8888;
    }
	host_len = spprintf(&host, 0, "%s:%ld", ssdb_sock->host, ssdb_sock->port);

	if (ssdb_sock->persistent) {
		if (ssdb_sock->persistent_id) {
			spprintf(&persistent_id, 0, "phpssdb:%s:%s", host, ssdb_sock->persistent_id);
		} else {
			spprintf(&persistent_id, 0, "phpssdb:%s:%ld", host, ssdb_sock->timeout);
		}
	}

    ssdb_sock->stream = php_stream_xport_create(
    		host,
    		host_len,
			ENFORCE_SAFE_MODE,
			STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT,
			persistent_id,
			tv_ptr,
			NULL,
			&errstr,
			&err);

    if (persistent_id) {
    	efree(persistent_id);
    }

    efree(host);

    if (!ssdb_sock->stream) {
        efree(errstr);
        return -1;
    }

    /* set TCP_NODELAY */
	sock = (php_netstream_data_t*)ssdb_sock->stream->abstract;
    setsockopt(sock->socket, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_flag, sizeof(int));

    php_stream_auto_cleanup(ssdb_sock->stream);

    if (tv.tv_sec != 0 || tv.tv_usec != 0) {
        php_stream_set_option(ssdb_sock->stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &read_tv);
    }
    php_stream_set_option(ssdb_sock->stream, PHP_STREAM_OPTION_WRITE_BUFFER, PHP_STREAM_BUFFER_NONE, NULL);
    php_stream_set_option(ssdb_sock->stream, PHP_STREAM_OPTION_READ_BUFFER, PHP_STREAM_BUFFER_NONE, NULL);
    php_stream_set_option(ssdb_sock->stream, PHP_STREAM_OPTION_BLOCKING, 1, NULL);

    ssdb_sock->status = SSDB_SOCK_STATUS_CONNECTED;

    return 0;
}

int ssdb_disconnect_socket(SSDBSock *ssdb_sock) {
    if (ssdb_sock == NULL) {
	    return 0;
    }

    if (ssdb_sock->stream != NULL) {
    	ssdb_sock->status = SSDB_SOCK_STATUS_DISCONNECTED;
		if (ssdb_sock->stream && !ssdb_sock->persistent) {
			ssdb_stream_close(ssdb_sock);
		}
		ssdb_sock->stream = NULL;
    }

    return 1;
}

int ssdb_key_prefix(SSDBSock *ssdb_sock, char **key, int *key_len) {
	int ret_len;
	char *ret;

	if (ssdb_sock->prefix == NULL || ssdb_sock->prefix_len == 0) {
		return 0;
	}

	ret_len = ssdb_sock->prefix_len + *key_len;
	ret = ecalloc(1 + ret_len, 1);
	memcpy(ret, ssdb_sock->prefix, ssdb_sock->prefix_len);
	memcpy(ret + ssdb_sock->prefix_len, *key, *key_len);

	*key = ret;
	*key_len = ret_len;

	return 1;
}

int ssdb_cmd_format_by_str(SSDBSock *ssdb_sock, char **ret, void *params, ...) {
    smart_str buf = {0};
    char *var = (char*)params;
    int var_len = 0;
    int i = 0;

    va_list ap;
	va_start(ap, params);

	while (1) {
		if (0 == i % 2) {
			var_len = va_arg(ap, int);
			smart_str_append_long(&buf, var_len);
			smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
			smart_str_appendl(&buf, var, var_len);
			smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
		} else {
			var = va_arg(ap, char*);
			if (var == NULL) break;
		}
		i++;
	}

	smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
	smart_str_0(&buf);
	va_end(ap);

	*ret = buf.c;

	SSDB_DEBUG_LOG("%s|", buf.c);

	return buf.len;
}

int ssdb_cmd_format_by_zval(SSDBSock *ssdb_sock,
		char **ret,
		char *cmd, int cmd_len,
		char *key, int key_len,
		zval *params,
		int read_all,
		int fill_prefix,
		int serialize) {
	HashTable *hash = Z_ARRVAL_P(params);
	int element = zend_hash_num_elements(hash);
	if (0 == element) {
		return 0;
	}

	smart_str buf = {0};
	smart_str_append_long(&buf, cmd_len);
	smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
	smart_str_appendl(&buf, cmd, cmd_len);
	smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);

	if (key_len > 0) {
		smart_str_append_long(&buf, key_len);
		smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
		smart_str_appendl(&buf, key, key_len);
		smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
	}

	for (zend_hash_internal_pointer_reset(hash); zend_hash_has_more_elements(hash) == SUCCESS; zend_hash_move_forward(hash)) {
		zval **z_value_pp;
		char *key, *val, *key_str = NULL;
		unsigned int key_len;
		unsigned long idx;
		int val_len = 0, val_free = 0, key_free = 0;

		zend_hash_get_current_data(hash, (void**)&z_value_pp);
		if (read_all) {
			int type = zend_hash_get_current_key_ex(hash, &key, &key_len, &idx, 0, NULL);
			if (type != HASH_KEY_IS_STRING) {
				key_len = spprintf(&key_str, 0, "%lu", idx);
				key = (char*)key_str;
			} else if(key_len > 0) {
				key_len--;
			}

			if (fill_prefix) {
				key_free = ssdb_key_prefix(ssdb_sock, &key, (int*)&key_len);
			}

			if (serialize) {
				val_free = ssdb_serialize(ssdb_sock, *z_value_pp, &val, &val_len);
			} else {
				convert_to_string(*z_value_pp);
				val = Z_STRVAL_PP(z_value_pp);
				val_len = Z_STRLEN_PP(z_value_pp);
			}

			smart_str_append_long(&buf, (long)key_len);
			smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
			smart_str_appendl(&buf, key, (long)key_len);
			smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);

			smart_str_append_long(&buf, (long)val_len);
			smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
			smart_str_appendl(&buf, val, (long)val_len);
			smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
		} else {
			convert_to_string(*z_value_pp);
			key = Z_STRVAL_PP(z_value_pp);
			key_len = Z_STRLEN_PP(z_value_pp);
			if (key_len <= 0) {
				continue;
			}

			if (fill_prefix) {
				key_free = ssdb_key_prefix(ssdb_sock, &key, (int*)&key_len);
			}

			smart_str_append_long(&buf, (long)key_len);
			smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
			smart_str_appendl(&buf, key, (long)key_len);
			smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
		}

		if (key_str) efree(key_str);
		if (key_free) efree(key);
		if (val_free) STR_FREE(val);
	}

	smart_str_appendl(&buf, _NL, sizeof(_NL) - 1);
	smart_str_0(&buf);

	*ret = buf.c;

	SSDB_DEBUG_LOG("%s|", buf.c);

	return buf.len;
}

int ssdb_check_eof(SSDBSock *ssdb_sock) {
    int eof;
    int count = 0;

	if (!ssdb_sock->stream) {
		return -1;
	}

	eof = php_stream_eof(ssdb_sock->stream);
    for (; eof; count++) {
        if (count == 10) {
	        if (ssdb_sock->stream) {
                ssdb_stream_close(ssdb_sock);
                ssdb_sock->stream = NULL;
                ssdb_sock->status = SSDB_SOCK_STATUS_FAILED;
	        }

            zend_throw_exception(ssdb_exception_ce, "Connection lost", 0 TSRMLS_CC);
	        return -1;
        }

        /* Close existing stream before reconnecting */
        if (ssdb_sock->stream) {
            ssdb_stream_close(ssdb_sock);
            ssdb_sock->stream = NULL;
	    }

        /* Wait for a while before trying to reconnect */
        if (ssdb_sock->retry_interval) {
            // Random factor to avoid having several (or many) concurrent connections trying to reconnect at the same time
            long retry_interval = (count ? ssdb_sock->retry_interval : (php_rand(TSRMLS_C) % ssdb_sock->retry_interval));
            usleep(retry_interval);
        }

        ssdb_connect_socket(ssdb_sock); /* reconnect */
        if (ssdb_sock->stream) { /*  check for EOF again. */
            eof = php_stream_eof(ssdb_sock->stream);
        }
    }

    /* We've reconnected if we have a count */
    if (count) {
        /* If we're using a password, attempt a reauthorization */
        if (ssdb_sock->auth && resend_auth(ssdb_sock) != 0) {
            return -1;
        }
    }

    /* Success */
    return 0;
}

SSDBResponse *ssdb_response_create() {
	SSDBResponse *ssdb_response = emalloc (sizeof (SSDBResponse));
	if (ssdb_response == NULL) {
		zend_throw_exception(ssdb_exception_ce, "SSDBResponse memory malloc failed", 0 TSRMLS_CC);
		return NULL;
	}

	ssdb_response->status = SSDB_IS_DEFAULT;
	ssdb_response->block  = NULL;
	ssdb_response->tail   = NULL;
	ssdb_response->num    = 0;

	return ssdb_response;
}

void ssdb_response_free(SSDBResponse *ssdb_response) {
	if (ssdb_response) {
		SSDBResponseBlock *ssdb_response_block = ssdb_response->block;
		while (ssdb_response_block != NULL) {
			SSDBResponseBlock *ssdb_response_block_next = ssdb_response_block->next;
			if (ssdb_response_block->data != NULL) {
				efree(ssdb_response_block->data);
			}
			efree(ssdb_response_block);
			ssdb_response_block = ssdb_response_block_next;
		}
		efree(ssdb_response);
	}
}

void ssdb_response_add_block(SSDBResponse *ssdb_response, char *data, size_t len) {
	if (ssdb_response == NULL) {
		zend_throw_exception(ssdb_exception_ce, "SSDBResponse must be malloc", 0 TSRMLS_CC);
		return;
	}

	if (data == NULL) {
		zend_throw_exception(ssdb_exception_ce, "SSDBResponseData must be malloc", 0 TSRMLS_CC);
		return;
	}

	SSDBResponseBlock *ssdb_response_block = emalloc(sizeof (SSDBResponseBlock));
	if (ssdb_response_block == NULL) {
		zend_throw_exception(ssdb_exception_ce, "SSDBResponseBlock memory malloc failed", 0 TSRMLS_CC);
		return;
	}

	ssdb_response_block->data = estrndup(data, len);
	ssdb_response_block->len  = len;
	ssdb_response_block->next = NULL;

	if (ssdb_response->tail == NULL) {
		ssdb_response_block->prev = NULL;
		ssdb_response->block = ssdb_response_block;
		ssdb_response->tail = ssdb_response_block;
	} else {
		ssdb_response_block->prev = ssdb_response->tail;
		ssdb_response_block->prev->next = ssdb_response_block;
		ssdb_response->tail = ssdb_response_block;
	}

	ssdb_response->num += 1;
}

SSDBResponse *ssdb_sock_read(SSDBSock *ssdb_sock) {
    if (-1 == ssdb_check_eof(ssdb_sock)) {
        return NULL;
    }

    SSDBResponse *ssdb_response = ssdb_response_create();

    int read_step = 0; //default is read len mode

    size_t expect_read_num = 1;
    size_t actual_read_num = 0;

    int to_read_buf_max   = 1024;
    int to_read_buf_total = 0;

    char *to_read_buf = emalloc(to_read_buf_max + 1);
    if (to_read_buf == NULL) {
    	ssdb_response_free(ssdb_response);
    	return NULL;
    }

    memset(to_read_buf, '\0', to_read_buf_max + 1);

    while (1) {
    	if (0 == read_step || 3 == read_step) {
    		char buf[1] = {' '};
    		actual_read_num = php_stream_read(ssdb_sock->stream, buf, 1);
    		SSDB_DEBUG_LOG("read sock actual num %zu on step first\n", actual_read_num);
    		SSDB_DEBUG_LOG("read sock actual data %c on step first\n", buf[0]);
    		if (actual_read_num == 0) {
    			break;
    		}

    		if (to_read_buf_total + actual_read_num > to_read_buf_max) {
    			break;
    		}

    		if (3 == read_step) {
    			if (buf[0] == '\n') {
					SSDB_DEBUG_LOG("read sock end\n");
					break;
    			} else {
    				read_step = 0;
    			}
    		}

    		if (buf[0] == '\n') {
    			expect_read_num = atoi(to_read_buf) + 1;
    			if (expect_read_num == 0) {
    				break;
    			}

    			if (expect_read_num > to_read_buf_max) {
    				efree(to_read_buf);
    				to_read_buf_max = expect_read_num;
    				to_read_buf = emalloc(to_read_buf_max + 1);
    				memset(to_read_buf, '\0', to_read_buf_max + 1);
    			}

    			read_step = 1; //switch to read var mode
    			memset(to_read_buf, '\0', expect_read_num);
    			to_read_buf_total = 0;

    			SSDB_DEBUG_LOG("read sock num %zu before step second\n", expect_read_num);
    		} else {
    			memcpy(to_read_buf + to_read_buf_total, buf, actual_read_num);
    			to_read_buf_total += actual_read_num;
    			SSDB_DEBUG_LOG("read sock all data %s on step first\n", to_read_buf);
    		}
    	} else {
    		actual_read_num = php_stream_read(ssdb_sock->stream, to_read_buf + to_read_buf_total, expect_read_num);
    		SSDB_DEBUG_LOG("read sock actual num %zu on step second expect read num %zu\n", actual_read_num, expect_read_num);

    		if (actual_read_num == 0) {
				break;
			}

    		to_read_buf_total += actual_read_num;
			expect_read_num -= actual_read_num;
			if (expect_read_num != 0) {
				continue;
			}

			if (to_read_buf[to_read_buf_total - 1] != '\n') {
				break;
			}

			to_read_buf[to_read_buf_total - 1] = '\0';

			SSDB_DEBUG_LOG("read sock all data %s on step second\n", to_read_buf);

			if (ssdb_response->status == SSDB_IS_DEFAULT) {
				if (0 == strcmp(to_read_buf, "ok")) {
					ssdb_response->status = SSDB_IS_OK;
				} else if (0 == strcmp(to_read_buf, "not_found")) {
					ssdb_response->status = SSDB_IS_NOT_FOUND;
				} else if (0 == strcmp(to_read_buf, "error")) {
					ssdb_response->status = SSDB_IS_ERROR;
				} else if (0 == strcmp(to_read_buf, "fail")) {
					ssdb_response->status = SSDB_IS_FAIL;
				} else {
					ssdb_response->status = SSDB_IS_CLIENT_ERROR;
				}
			} else {
				ssdb_response_add_block(ssdb_response, to_read_buf, to_read_buf_total - 1);
			}

			expect_read_num = 1;
			read_step = 3; //switch to read len mode
			memset(to_read_buf, '\0', to_read_buf_max);
			to_read_buf_total = 0;
    	}
    }

    efree(to_read_buf);

    if (ssdb_response->status == SSDB_IS_DEFAULT) {
    	ssdb_response_free(ssdb_response);
    	return NULL;
    }

    return ssdb_response;
}

int ssdb_sock_write(SSDBSock *ssdb_sock, char *cmd, size_t sz) {
	if (ssdb_sock && ssdb_sock->status == SSDB_SOCK_STATUS_DISCONNECTED) {
		zend_throw_exception(ssdb_exception_ce, "Connection closed", 0 TSRMLS_CC);
		return -1;
	}

    if (-1 == ssdb_check_eof(ssdb_sock)) {
        return -1;
    }

    return php_stream_write(ssdb_sock->stream, cmd, sz);
}

int resend_auth(SSDBSock *ssdb_sock) {
    char *cmd;
    int cmd_len;

    cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, ZEND_STRL("auth"), ssdb_sock->auth, strlen(ssdb_sock->auth), NULL);
    if (ssdb_sock_write(ssdb_sock, cmd, cmd_len TSRMLS_CC) < 0) {
        efree(cmd);
        return -1;
    }

    efree(cmd);

    SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
    if (ssdb_response == NULL || ssdb_response->status != SSDB_IS_OK) {
		ssdb_response_free(ssdb_response);
		return -1;
	}

    if (0 != strcmp(ssdb_response->block->data, "ok")) {
    	ssdb_response_free(ssdb_response);
        return -1;
    }

    ssdb_response_free(ssdb_response);

    return 0;
}

int ssdb_serialize(SSDBSock *ssdb_sock, zval *z, char **val, int *val_len) {
#if ZEND_MODULE_API_NO >= 20100000
	php_serialize_data_t ht;
#else
	HashTable ht;
#endif
	smart_str sstr = {0};
	zval *z_copy;
#ifdef HAVE_SSDB_IGBINARY
	size_t sz;
	uint8_t *val8;
#endif

	switch(ssdb_sock->serializer) {
		case SSDB_SERIALIZER_NONE:
			switch(Z_TYPE_P(z)) {
				case IS_STRING:
					*val = Z_STRVAL_P(z);
					*val_len = Z_STRLEN_P(z);
					return 0;
				case IS_OBJECT:
					MAKE_STD_ZVAL(z_copy);
					ZVAL_STRINGL(z_copy, "Object", 6, 1);
					break;
				case IS_ARRAY:
					MAKE_STD_ZVAL(z_copy);
					ZVAL_STRINGL(z_copy, "Array", 5, 1);
					break;
				default: /* copy */
					MAKE_STD_ZVAL(z_copy);
					*z_copy = *z;
					zval_copy_ctor(z_copy);
					break;
			}
			/* return string */
			convert_to_string(z_copy);
			*val = Z_STRVAL_P(z_copy);
			*val_len = Z_STRLEN_P(z_copy);
			efree(z_copy);
			return 1;
			break;
		case SSDB_SERIALIZER_PHP:
#if ZEND_MODULE_API_NO >= 20100000
			PHP_VAR_SERIALIZE_INIT(ht);
#else
			zend_hash_init(&ht, 10, NULL, NULL, 0);
#endif
			php_var_serialize(&sstr, &z, &ht TSRMLS_CC);
			*val = sstr.c;
			*val_len = (int)sstr.len;
#if ZEND_MODULE_API_NO >= 20100000
			PHP_VAR_SERIALIZE_DESTROY(ht);
#else
			zend_hash_destroy(&ht);
#endif
			return 1;
			break;
		case SSDB_SERIALIZER_IGBINARY:
#ifdef HAVE_SSDB_IGBINARY
			if(igbinary_serialize(&val8, (size_t *)&sz, z TSRMLS_CC) == 0) { /* ok */
				*val = (char*)val8;
				*val_len = (int)sz;
				return 1;
			}
#endif
			return 0;
			break;
	}

	return 0;
}

int ssdb_unserialize(SSDBSock *ssdb_sock, const char *val, int val_len, zval **return_value) {
	php_unserialize_data_t var_hash;
	int ret, rv_free = 0;

	switch(ssdb_sock->serializer) {
		case SSDB_SERIALIZER_NONE:
			return 0;
		case SSDB_SERIALIZER_PHP:
			if (!*return_value) {
				MAKE_STD_ZVAL(*return_value);
				rv_free = 1;
			}
#if ZEND_MODULE_API_NO >= 20100000
			PHP_VAR_UNSERIALIZE_INIT(var_hash);
#else
			memset(&var_hash, 0, sizeof(var_hash));
#endif
			if (!php_var_unserialize(return_value, (const unsigned char**)&val, (const unsigned char*)val + val_len, &var_hash TSRMLS_CC)) {
				if (rv_free==1) efree(*return_value);
				ret = 0;
			} else {
				ret = 1;
			}
#if ZEND_MODULE_API_NO >= 20100000
			PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
#else
			var_destroy(&var_hash);
#endif
			return ret;
		case SSDB_SERIALIZER_IGBINARY:
#ifdef HAVE_SSDB_IGBINARY
			if (!*return_value) {
				MAKE_STD_ZVAL(*return_value);
				rv_free = 1;
			}
			if (igbinary_unserialize((const uint8_t *)val, (size_t)val_len, return_value TSRMLS_CC) == 0) {
				return 1;
			}
			if(rv_free==1) efree(*return_value);
#endif
			return 0;
			break;
	}

	return 0;
}

void ssdb_bool_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock) {
	SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
	if (ssdb_response == NULL
			|| ssdb_response->status != SSDB_IS_OK) {
		ssdb_response_free(ssdb_response);
		RETURN_FALSE;
	}

	//qset只返回2\nok\n\n
	if (ssdb_response->num > 0
			&& 0 == strcmp(ssdb_response->block->data, "0")) {
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}

	ssdb_response_free(ssdb_response);
}

void ssdb_string_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock) {
    SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
    if (ssdb_response == NULL
    		|| ssdb_response->status != SSDB_IS_OK) {
    	ssdb_response_free(ssdb_response);
        RETURN_NULL();
    }

    if (ssdb_unserialize(ssdb_sock, ssdb_response->block->data, ssdb_response->block->len, &return_value) == 0) {
    	RETVAL_STRINGL(ssdb_response->block->data, ssdb_response->block->len, 1);
	}

    ssdb_response_free(ssdb_response);
}

void ssdb_long_number_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock) {
	SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
	if (ssdb_response == NULL
			|| ssdb_response->status != SSDB_IS_OK) {
		ssdb_response_free(ssdb_response);
		RETURN_NULL();
	}

	RETVAL_LONG(atof(ssdb_response->block->data));

	ssdb_response_free(ssdb_response);
}

void ssdb_double_number_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock) {
	SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
	if (ssdb_response == NULL
			|| ssdb_response->status != SSDB_IS_OK) {
		ssdb_response_free(ssdb_response);
		RETURN_NULL();
	}

	RETVAL_DOUBLE(atof(ssdb_response->block->data));

	ssdb_response_free(ssdb_response);
}

void ssdb_list_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock, int filter_prefix, int unserialize) {
    SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
    if (ssdb_response == NULL
    		|| ssdb_response->status != SSDB_IS_OK) {
    	ssdb_response_free(ssdb_response);
        RETURN_NULL();
    }

    array_init_size(return_value, ssdb_response->num);
    SSDBResponseBlock *ssdb_response_block = ssdb_response->block;
    while (ssdb_response_block != NULL) {
    	zval *z = NULL;
    	if (filter_prefix == SSDB_FILTER_KEY_PREFIX
    			&& ssdb_sock->prefix
				&& 0 == strncmp(ssdb_response_block->data, ssdb_sock->prefix, ssdb_sock->prefix_len)) {
    		if (unserialize == SSDB_UNSERIALIZE_NONE) {
    			add_next_index_string(return_value, ssdb_response_block->data + ssdb_sock->prefix_len, 1);
    		} else {
    			if (ssdb_unserialize(ssdb_sock, ssdb_response_block->data + ssdb_sock->prefix_len, ssdb_response_block->len - ssdb_sock->prefix_len, &z)) {
    				add_next_index_zval(return_value, z);
    			} else {
    				add_next_index_string(return_value, ssdb_response_block->data + ssdb_sock->prefix_len, 1);
    			}
    		}
    	} else {
    		if (unserialize == SSDB_UNSERIALIZE_NONE) {
    			add_next_index_string(return_value, ssdb_response_block->data, 1);
    		} else {
    			if (ssdb_unserialize(ssdb_sock, ssdb_response_block->data, ssdb_response_block->len, &z)) {
    				add_next_index_zval(return_value, z);
    			} else {
    				add_next_index_string(return_value, ssdb_response_block->data, 1);
    			}
    		}
    	}
    	ssdb_response_block = ssdb_response_block->next;
    }

    ssdb_response_free(ssdb_response);
}

void ssdb_map_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock, int filter_prefix, int unserialize, int convert_type) {
    SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
    if (ssdb_response == NULL
    		|| ssdb_response->status != SSDB_IS_OK
			|| ssdb_response->num % 2 != 0) {
    	ssdb_response_free(ssdb_response);
    	php_printf("error\n");
        RETURN_NULL();
    }


    int i = 1;
    array_init(return_value);
    SSDBResponseBlock *ssdb_response_block = ssdb_response->block;
    while (ssdb_response_block != NULL) {
    	if (0 == i % 2) {
    		zval *z = NULL;
    		if (filter_prefix == SSDB_FILTER_KEY_PREFIX
					&& ssdb_sock->prefix
					&& 0 == strncmp(ssdb_response_block->prev->data, ssdb_sock->prefix, ssdb_sock->prefix_len)) {
    			if (unserialize == SSDB_UNSERIALIZE_NONE) {
    				switch (convert_type) {
    					case SSDB_CONVERT_TO_LONG:
    						add_assoc_long(return_value, ssdb_response_block->prev->data + ssdb_sock->prefix_len, atol(ssdb_response_block->data));
    						break;
    					case SSDB_CONVERT_TO_STRING:
    						add_assoc_string(return_value, ssdb_response_block->prev->data + ssdb_sock->prefix_len, ssdb_response_block->data, 1);
    						break;
    				}
    			} else {
    				if (ssdb_unserialize(ssdb_sock, ssdb_response_block->data, ssdb_response_block->len, &z)) {
    					add_assoc_zval(return_value, ssdb_response_block->prev->data + ssdb_sock->prefix_len, z);
    				} else {
    					add_assoc_string(return_value, ssdb_response_block->prev->data + ssdb_sock->prefix_len, ssdb_response_block->data, 1);
    				}
    			}
			} else {
				if (unserialize == SSDB_UNSERIALIZE_NONE) {
					switch (convert_type) {
						case SSDB_CONVERT_TO_LONG:
							add_assoc_long(return_value, ssdb_response_block->prev->data, atol(ssdb_response_block->data));
							break;
						case SSDB_CONVERT_TO_STRING:
							add_assoc_string(return_value, ssdb_response_block->prev->data, ssdb_response_block->data, 1);
							break;
					}
				} else {
					if (ssdb_unserialize(ssdb_sock, ssdb_response_block->data, ssdb_response_block->len, &z)) {
						add_assoc_zval(return_value, ssdb_response_block->prev->data, z);
					} else {
						add_assoc_string(return_value, ssdb_response_block->prev->data, ssdb_response_block->data, 1);
					}
				}
			}
    	}
    	ssdb_response_block = ssdb_response_block->next;
    	i++;
    }

    ssdb_response_free(ssdb_response);
}
