dnl $Id$
dnl config.m4 for extension ssdb

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(ssdb, for ssdb support,
dnl Make sure that the comment is aligned:
dnl [  --with-ssdb             Include ssdb support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(ssdb, whether to enable ssdb support,
Make sure that the comment is aligned:
[  --enable-ssdb           Enable ssdb support])

PHP_ARG_ENABLE(ssdb-igbinary, whether to enable igbinary serializer support,
[  --enable-ssdb-igbinary      Enable igbinary serializer support], no, no)

if test "$PHP_SSDB" != "no"; then
  if test "$PHP_SSDB_IGBINARY" != "no"; then
    AC_MSG_CHECKING([for igbinary includes])
    igbinary_inc_path=""

    if test -f "$abs_srcdir/include/php/ext/igbinary/igbinary.h"; then
      igbinary_inc_path="$abs_srcdir/include/php"
    elif test -f "$abs_srcdir/ext/igbinary/igbinary.h"; then
      igbinary_inc_path="$abs_srcdir"
    elif test -f "$phpincludedir/ext/igbinary/igbinary.h"; then
      igbinary_inc_path="$phpincludedir"
    else
      for i in php php4 php5 php6; do
        if test -f "$prefix/include/$i/ext/igbinary/igbinary.h"; then
          igbinary_inc_path="$prefix/include/$i"
        fi
      done
    fi

    if test "$igbinary_inc_path" = ""; then
      AC_MSG_ERROR([Cannot find igbinary.h])
    else
      AC_MSG_RESULT([$igbinary_inc_path])
    fi
  fi

  AC_MSG_CHECKING([for ssdb igbinary support])
  if test "$PHP_SSDB_IGBINARY" != "no"; then
    AC_MSG_RESULT([enabled])
    AC_DEFINE(HAVE_SSDB_IGBINARY,1,[Whether ssdb igbinary serializer is enabled])
    IGBINARY_INCLUDES="-I$igbinary_inc_path"
    IGBINARY_EXT_DIR="$igbinary_inc_path/ext"
    ifdef([PHP_ADD_EXTENSION_DEP],
    [
      PHP_ADD_EXTENSION_DEP(ssdb, igbinary)
    ])
    PHP_ADD_INCLUDE($IGBINARY_EXT_DIR)
  else
    IGBINARY_INCLUDES=""
    AC_MSG_RESULT([disabled])
  fi

  dnl # --with-ssdb -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/ssdb.h"  # you most likely want to change this
  dnl if test -r $PHP_SSDB/$SEARCH_FOR; then # path given as parameter
  dnl   SSDB_DIR=$PHP_SSDB
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for ssdb files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SSDB_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SSDB_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the ssdb distribution])
  dnl fi

  dnl # --with-ssdb -> add include path
  dnl PHP_ADD_INCLUDE($SSDB_DIR/include)

  dnl # --with-ssdb -> check for lib and symbol presence
  dnl LIBNAME=ssdb # you may want to change this
  dnl LIBSYMBOL=ssdb # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SSDB_DIR/lib, SSDB_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SSDBLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong ssdb lib version or lib not found])
  dnl ],[
  dnl   -L$SSDB_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SSDB_SHARED_LIBADD)

  PHP_NEW_EXTENSION(ssdb, ssdb_library.c ssdb_class.c ssdb.c, $ext_shared)
fi
