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

#ifndef EXT_SSDB_SSDB_LIBRARY_H_
#define EXT_SSDB_SSDB_LIBRARY_H_

#define SSDB_SOCK_STATUS_FAILED 0
#define SSDB_SOCK_STATUS_DISCONNECTED 1
#define SSDB_SOCK_STATUS_UNKNOWN 2
#define SSDB_SOCK_STATUS_CONNECTED 3

#define SSDB_SERIALIZER_NONE 0
#define SSDB_SERIALIZER_PHP 1
#define SSDB_SERIALIZER_IGBINARY 2

#define SSDB_FILTER_KEY_PREFIX_NONE 0
#define SSDB_FILTER_KEY_PREFIX 1

#define SSDB_UNSERIALIZE_NONE 0
#define SSDB_UNSERIALIZE 1

#define _NL "\n"

typedef enum {SSDB_IS_DEFAULT,SSDB_IS_OK,SSDB_IS_NOT_FOUND,SSDB_IS_ERROR,SSDB_IS_FAIL,SSDB_IS_CLIENT_ERROR} ssdb_response_status;

//#define SSDB_DEBUG_LOG(fmt, args...) php_printf(fmt, ##args);
#define SSDB_DEBUG_LOG(fmt, args...)
#define SSDB_SOCKET_WRITE_COMMAND(ssdb_sock, cmd, cmd_len) if(ssdb_sock_write(ssdb_sock, cmd, cmd_len) < 0) { \
	efree(cmd); \
    RETURN_NULL(); \
} \
	efree(cmd);

typedef struct {
	php_stream *stream;
	char *host;
	short port;
	double timeout;
	double read_timeout;
	char *auth;
	char *prefix;
	int prefix_len;
	int status;
	zend_bool lazy_connect;
	char *err;
	int err_len;
	long retry_interval;
	int persistent;
	char *persistent_id;
	int serializer;
} SSDBSock;

typedef struct _SSDBResponseBlock {
	struct _SSDBResponseBlock *prev;
	struct _SSDBResponseBlock *next;
	size_t len;
	char *data;
} SSDBResponseBlock;

typedef struct {
	ssdb_response_status status;
	SSDBResponseBlock *block;
	SSDBResponseBlock *tail;
	int num;
} SSDBResponse;

extern zend_class_entry *ssdb_exception_ce;

SSDBSock* ssdb_create_sock(
		char *host,
		int host_len,
		unsigned short port,
		double timeout,
		int persistent,
		char *persistent_id,
        long retry_interval,
		zend_bool lazy_connect);
void ssdb_free_socket(SSDBSock *ssdb_sock);
void ssdb_stream_close(SSDBSock *ssdb_sock);

int ssdb_open_socket(SSDBSock *ssdb_sock, int force_connect);
int ssdb_connect_socket(SSDBSock *ssdb_sock);
int ssdb_disconnect_socket(SSDBSock *ssdb_sock);

int ssdb_key_prefix(SSDBSock *ssdb_sock, char **key, int *key_len);
int ssdb_cmd_format_by_str(SSDBSock *ssdb_sock, char **ret, char *params, ...);
int ssdb_cmd_format_by_zval(SSDBSock *ssdb_sock, char **ret, char *cmd, int cmd_len, zval *params, int read_all);

int ssdb_check_eof(SSDBSock *ssdb_sock);

SSDBResponse *ssdb_response_create();
void ssdb_response_free(SSDBResponse *ssdb_response);
void ssdb_response_add_block(SSDBResponse *ssdb_response, char *data, size_t len);

SSDBResponse *ssdb_sock_read(SSDBSock *ssdb_sock);
int ssdb_sock_write(SSDBSock *ssdb_sock, char *cmd, size_t sz);

int resend_auth(SSDBSock *ssdb_sock);

int ssdb_serialize(SSDBSock *ssdb_sock, zval *z, char **val, int *val_len);
int ssdb_unserialize(SSDBSock *ssdb_sock, const char *val, int val_len, zval **return_value);

void ssdb_number_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock);
void ssdb_bool_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock);
void ssdb_string_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock);
void ssdb_list_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock, int filter_prefix, int unserialize);
void ssdb_map_response(INTERNAL_FUNCTION_PARAMETERS, SSDBSock *ssdb_sock, int filter_prefix, int unserialize);

#endif /* EXT_SSDB_SSDB_LIBRARY_H_ */
