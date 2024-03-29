AC_INIT([libcpuid CPU Identification library], [0.2.0], [libcpuid-devel@lists.sourceforge.net], [libcpuid])
AC_CONFIG_SRCDIR([libcpuid/libcpuid.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall dist-bzip2 dist-zip foreign])

dnl Versioning scheme shamelessly copied from libexif.
dnl Short walkthrough. C means CURRENT, A mean AGE, R means REVISION
dnl 1) When you make a change that breaks binary compatibility, increment CURRENT, reset REVISION to 0 and:
dnl   1.1) If the change doesn't remove interfaces, increment AGE, otherwise reset to 0.
dnl 2) When you make just a source change that doesn't break binary compatibility, increment REVISION.
dnl When you make a change that adds features, increment AGE, and set REVISION to 0
dnl C:A:R
dnl 10:0:0   Version 0.1.0
dnl 11:0:0   Version 0.1.1: four more fields to cpu_raw_data_t
dnl 11:0:1   Version 0.1.2: added cpu_msr* functions
dnl 11:0:2   Version 0.1.3: added cpu_clock_by_ic() function
dnl 11:0:3   Version 0.1.4: just an identification change, to reflect the detection table upgrades
dnl 12:1:0   Version 0.2.0: two more fields to cpu_id_t
LIBCPUID_CURRENT=12
LIBCPUID_AGE=1
LIBCPUID_REVISION=0
AC_SUBST([LIBCPUID_AGE])
AC_SUBST([LIBCPUID_REVISION])
AC_SUBST([LIBCPUID_CURRENT])
AC_SUBST([LIBCPUID_CURRENT_MIN],[`expr $LIBCPUID_CURRENT - $LIBCPUID_AGE`])
LIBCPUID_VERSION_INFO="$LIBCPUID_CURRENT:$LIBCPUID_REVISION:$LIBCPUID_AGE"
AC_SUBST([LIBCPUID_VERSION_INFO])


AC_PROG_CC
AC_C_CONST
AM_PROG_LIBTOOL
AM_CPPFLAGS="$CPPFLAGS"

AC_CHECK_HEADERS([stdint.h])

AM_LDFLAGS="$LDFLAGS"
if test "x$GCC" = "xyes"; then
    AM_CFLAGS="$AM_CFLAGS -ansi -pedantic-error"
    AM_CXXFLAGS="$AM_CXXFLAGS -ansi -pedantic-error"
    AM_CPPFLAGS="$AM_CPPFLAGS -g -Wall -Wmissing-declarations -Wmissing-prototypes"
    AM_LDFLAGS="$AM_LDFLAGS -g -Wall"
fi


AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)


AC_CONFIG_FILES([
  Makefile
  libcpuid.pc
  libcpuid/Makefile
  cpuid_tool/Makefile
  tests/Makefile
])
AC_OUTPUT

