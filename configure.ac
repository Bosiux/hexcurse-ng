dnl Process this file with autoconf to produce a configure script.
AC_INIT([hexcurse],[1.70.0])
AC_CONFIG_SRCDIR(src/file.c)
AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_HEADERS([config.h])
dnl AC_PROG_MAKE_SET

AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL

dnl Checks for libraries.
dnl Replace `main' with a function in -lncurses:
AC_CHECK_LIB(ncurses, initscr,,[AC_CHECK_LIB(curses, initscr,,[AC_MSG_ERROR([ncurses library not found])])])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)

dnl Checks for string libraries
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(strings.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INTMAX_T
AC_TYPE_OFF_T
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

dnl Checks for library functions.
AC_DIAGNOSE([obsolete],[your code may safely assume C89 semantics that RETSIGTYPE is void.
Remove this warning and the `AC_CACHE_CHECK' when you adjust the code.])dnl
AC_CACHE_CHECK([return type of signal handlers],[ac_cv_type_signal],[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <signal.h>
],
		 [return *(signal (0, 0)) (0) == 1;])],
		   [ac_cv_type_signal=int],
		   [ac_cv_type_signal=void])])
AC_DEFINE_UNQUOTED([RETSIGTYPE],[$ac_cv_type_signal],[Define as the return type of signal handlers
		    (`int' or `void').])


dnl Checks for BIG ENDIAN
dnl AC_C_BIGENDIAN 

AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])
AC_OUTPUT
