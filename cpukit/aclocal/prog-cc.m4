dnl
dnl $Id$
dnl 
dnl Check for target gcc
dnl

AC_DEFUN([RTEMS_PROG_CC],
[
AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_BEFORE([$0], [AC_PROG_CC])dnl
AC_BEFORE([$0], [RTEMS_CANONICALIZE_TOOLS])dnl

_RTEMS_FLAGS([CFLAGS],
  ["\$(CPU_CFLAGS) \$(RTEMS_CFLAGS_\$(VARIANT_V)_V) \$(CFLAGS_\$(VARIANT_V)_V) -g"])

RTEMS_CHECK_TOOL(CC,gcc)
test -z "$CC" && \
  AC_MSG_ERROR([no acceptable cc found in \$PATH])
AC_PROG_CC
AC_PROG_CPP

AM_CONDITIONAL(RTEMS_USE_GCC,test x"$GCC" = x"yes")
])

AC_DEFUN([RTEMS_PROG_CC_FOR_TARGET],
[
dnl check target cc
RTEMS_PROG_CC
dnl check if the compiler supports -isystem
RTEMS_GCC_ISYSTEM
dnl check if the target compiler may use --pipe
RTEMS_GCC_PIPE
test "$rtems_cv_gcc_pipe" = "yes" && CC="$CC --pipe"

if test "$GCC" = yes; then
RTEMS_CFLAGS="-Wall $RTEMS_CFLAGS"
m4_if([$1],,[],[RTEMS_CFLAGS="$RTEMS_CFLAGS $1"])
fi

AS_IF([test x"$rtems_cv_gcc_isystem" = x"yes"],[
  RTEMS_CPPFLAGS="-isystem \$(PROJECT_INCLUDE)"],[
  RTEMS_CPPFLAGS="-I\$(PROJECT_INCLUDE)"
])
AC_SUBST(RTEMS_CPPFLAGS)

])
