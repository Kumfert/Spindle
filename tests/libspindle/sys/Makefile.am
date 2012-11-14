
check_PROGRAMS = test01.exe test02.exe test04.exe test05.exe # test03

test01_exe_SOURCES = test01.cc	
test02_exe_SOURCES = test02.cc	
test04_exe_SOURCES = test04.cc	
test05_exe_SOURCES = test05.cc	
#test03_exe_SOURCES = test03.cc	

EXTRA_DIST = $(check_PROGRAMS:.exe=.out) test05.in old_unit.sh

LIBS = ../../../libspindle/libspindle.a
INCLUDES = -I$(top_srcdir)/include

