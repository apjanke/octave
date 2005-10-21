dnl @synopsis ACX_CHECK_HEADER_IN_DIRS (HEADER, DIR-LIST, [
dnl			ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND[, MESSAGE]]])
dnl
dnl This macro looks for a header file in the include directories
dnl and in the sub-directories specified by DIR-LIST.
dnl
dnl This macro requires autoconf 2.50 or later.
dnl
dnl @version $Id: acx_include_dirs.m4,v 1.3 2005-10-21 12:30:29 jwe Exp $
dnl @author David Bateman <dbateman@free.fr>
dnl
AC_DEFUN([ACX_CHECK_HEADER_IN_DIRS], [
AC_PREREQ(2.50)
acx_include_ok=no
acx_include_dir=

# First check the header in the base directory
AC_CHECK_HEADER($1, [acx_include_ok=yes])

# Now check the other directories
if test x"$acx_include_ok" = xno; then
  for dir in $2; do
    AC_CHECK_HEADER(${dir}/$1, [acx_include_ok=yes; acx_include_dir=${dir}; break])
  done
fi

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_include_ok" = xyes; then
  acx_header=HEADER_`echo $1 | sed -e 's/[[^a-zA-Z0-9_]]/_/g' \
    -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/'`
  acx_header_name_for_variable=`echo $1 | sed -e 's/[[^a-zA-Z0-9_]]/_/g'`
  eval acx_${acx_header_name_for_variable}_include_dir=$acx_include_dir
  if test -n "$acx_include_dir"; then
    eval acx_${acx_header_name_for_variable}_include_file=$acx_include_dir/$1
  else
    eval acx_${acx_header_name_for_variable}_include_file=$1
  fi
  ifelse([$3],,AC_DEFINE(${acx_header},
    [`eval echo '${'acx_${acx_header_name_for_variable}_include_file'}'`/$1],
    [$5]),[$3])
ifelse([$4],,,[
else
  $4
])
fi
])dnl ACX_CHECK_HEADER_IN_DIRS
