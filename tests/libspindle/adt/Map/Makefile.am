
check_PROGRAMS = test01.exe test02.exe test03.exe

test01_exe_SOURCES = test01.cc
test02_exe_SOURCES = test02.cc
test03_exe_SOURCES = test03.cc

EXTRA_DIST = $(check_PROGRAMS:.exe=.out) old_unit.sh

LIBS = ../../../../libspindle/libspindle.a
INCLUDES = -I$(top_srcdir)/include

