
check_PROGRAMS = test01.exe test02.exe test03.exe test04.exe test05.exe test06.exe test06b.exe test07.exe test08.exe
# test06b.exe

test01_exe_SOURCES = test01.cc
test02_exe_SOURCES = test02.cc
test03_exe_SOURCES = test03.cc
test04_exe_SOURCES = test04.cc
test05_exe_SOURCES = test05.cc
test06_exe_SOURCES = test06.cc
test06b_exe_SOURCES = test06b.cc
test07_exe_SOURCES = test07.cc
test08_exe_SOURCES = test08.cc

EXTRA_DIST = $(check_PROGRAMS:.exe=.out) old_unit.sh

LIBS = ../../../libspindle/libspindle.a
INCLUDES = -I$(top_srcdir)/include

dist-hook:
	d="$(distdir)/tempout"; \
	test -x $$d \
	|| mkdir -p $$d \
	|| exit 1; \
	d="$(distdir)/input"; \
	test -x $$d \
	|| mkdir -p $$d \
	|| exit 1; \
	cp -p $(srcdir)/input/*.* $$d/; 
	
