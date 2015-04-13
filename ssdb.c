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

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ssdb.h"

#include "ssdb_class.h"

/* If you declare any globals in php_ssdb.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(ssdb)
*/

/* {{{ ssdb_functions[]
 *
 * Every user visible function must have an entry in ssdb_functions[].
 */
const zend_function_entry ssdb_functions[] = {
	PHP_FE(ssdb_version, NULL)
	PHP_FE_END	/* Must be the last line in ssdb_functions[] */
};
/* }}} */

/* {{{ ssdb_module_entry
 */
zend_module_entry ssdb_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"ssdb",
	ssdb_functions,
	PHP_MINIT(ssdb),
	PHP_MSHUTDOWN(ssdb),
	PHP_RINIT(ssdb),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(ssdb),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(ssdb),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_SSDB_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SSDB
ZEND_GET_MODULE(ssdb)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("ssdb.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_ssdb_globals, ssdb_globals)
    STD_PHP_INI_ENTRY("ssdb.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_ssdb_globals, ssdb_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_ssdb_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_ssdb_init_globals(zend_ssdb_globals *ssdb_globals)
{
	ssdb_globals->global_value = 0;
	ssdb_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ssdb)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	register_ssdb_class(module_number TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ssdb)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ssdb)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ssdb)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ssdb)
{
	//info
	php_info_print_table_start();
	php_info_print_table_header(2, "ssdb support", "enabled");
	php_info_print_table_row(2, "version", PHP_SSDB_VERSION);
	php_info_print_table_row(2, "author",  "xingqiba");
	php_info_print_table_row(2, "website", "http://xingqiba.sinaapp.com");
	php_info_print_table_row(2, "contact", "ixqbar@gmail.com or qq174171262");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ ssdb_version
 */
PHP_FUNCTION(ssdb_version)
{
	RETURN_STRING(PHP_SSDB_VERSION, 1);
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
