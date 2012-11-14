#!/bin/sh
#
# This handy little script searches the source directory and 
# creates links to all the header files except the ones
# in the RCS directories
#
# Gary Kumfert, Old Dominion University
#
# 8Apr97 - switched from symbolic to hard links so that
#          the links accurately reflect modification stamps.
#          This should have applications in makefile dependency 
#          lists.
#
#          Also separated include directories into include.c and 
#          include.cc.  May switch this back later.
#
#

mi_prev=
for mi_option
do
 # If the previous option needs an argument, assign it.
  if test -n "$mi_prev"; then
    eval "$mi_prev=\$mi_option"
    mi_prev=
    continue
  fi

  case "$mi_option" in
  -*=*) mi_optarg=`echo "$mi_option" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) mi_optarg= ;;
  esac

  case "$mi_option" in
  -srcdir | --srcdir | --srcdi | --srcd | --src | --sr)
    mi_prev=srcdir ;;
  -srcdir=* | --srcdir=* | --srcdi=* | --srcd=* | --src=* | --sr=*)
    srcdir="$mi_optarg" ;;
  esac
done

if test "x$srcdir" != "x"; then 
  cd $srcdir;
fi
cur=`pwd`
mkdir -p include/spindle;
foundone=0;
#ls "${cur}/libspindle"
find "${cur}/libspindle" -name RCS -prune -o -name old -prune -o -name '*.h' -print  | while read fullpath
do
   path=`dirname "$fullpath"`;
   file=`basename "$fullpath"`;
   file=`expr "$file" : '\(.*\)\.h'`;
   if [ ! -f "include/spindle/$file.h" ] 
   then
       foundone=1;
       echo link file \'$fullpath\' to include/spindle/ ;
       ln -s "$fullpath" include/spindle/$file.h
#      else
#         echo file \'$fullpath\' already included in ../include/include.cc;
   fi
done
exit $foundone

