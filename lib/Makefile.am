EXTRA_DIST =
CLEANFILES =
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLIBEXECDIR=\""$(libexecdir)"\"

AM_CFLAGS = \
        -Wall -Wextra  \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections

AM_LDFLAGS = \
	-Wl,--gc-sections \
	-Wl,--as-needed

# The rules of thumb, when dealing with these values are:
#  Always increase the revision value.
#  Increase the current value whenever an interface has been added, removed or changed.
#  Increase the age value only if the changes made to the ABI are backward compatible.
LIBZC_CURRENT=1
LIBZC_REVISION=0
LIBZC_AGE=0

lib_LTLIBRARIES = libzc.la
libzc_la_SOURCES =\
	libzc_private.h \
	libzc.c \
	zc_file.c \
	zip.c \
	zc_crk.c \
	zc_pwdict.c \
	zc_crk_ptext.c \
	key_table.c \
	key2_reduce.c \
	crc32.h \
	key2_reduce.h \
	key_table.h \
	libzc.h \
	qsort.h \
	zip.h
include_HEADERS = libzc.h

EXTRA_DIST += libzc.sym

libzc_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info $(LIBZC_CURRENT):$(LIBZC_REVISION):$(LIBZC_AGE) \
	-Wl,--version-script=libzc.sym
libzc_la_DEPENDENCIES = libzc.sym

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libzc.pc
CLEANFILES += libzc.pc

# automated tests
if BUILD_TESTS
TESTS = test_libzc test_keytable test_key2reduce
check_PROGRAMS = test_libzc test_keytable test_key2reduce

# test_libzc
test_libzc_SOURCES = \
        test_libzc.c \
	test_file.c \
	test_crk.c \
	test_pwdict.c \
	test_crk_ptext.c \
	libzc.h
test_libzc_CFLAGS = @CHECK_CFLAGS@
test_libzc_LDADD = libzc.la @CHECK_LIBS@

# test_keytable
test_keytable_SOURCES = test_key_table.c key_table.c key_table.h
test_keytable_CFLAGS = @CHECK_CFLAGS@
test_keytable_LDADD = @CHECK_LIBS@

# test_key2reduce
test_key2reduce_SOURCES = test_key2_reduce.c key2_reduce.c key2_reduce.h key_table.c key_table.h
test_key2reduce_CFLAGS = @CHECK_CFLAGS@
test_key2reduce_LDADD = @CHECK_LIBS@
endif