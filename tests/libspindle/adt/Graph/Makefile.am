
check_PROGRAMS = test01.exe test02.exe test03.exe test10.exe test11.exe test12.exe

test01_exe_SOURCES = test01.cc
test02_exe_SOURCES = test02.cc
test03_exe_SOURCES = test03.cc
test10_exe_SOURCES = test10.cc
test11_exe_SOURCES = test11.cc
test12_exe_SOURCES = test12.cc

EXTRA_DIST = $(check_PROGRAMS:.exe=.out) old_unit.sh

LIBS = ../../../../libspindle/libspindle.a
INCLUDES = -I$(top_srcdir)/include

