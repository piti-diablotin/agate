#!/bin/sh
# Clean
if test "$1" = "-c"
then
  rm -rf \
    Makefile.in \
    aclocal.m4 \
    autom4te.cache/ \
    configure \
    config.log \
    agate.h \
    ar-lib \
    compile \
    config.guess \
    config.status \
    config.sub \
    depcomp \
    install-sh \
    missing \
    configure.ac \
    m4/ar-lib \
    m4/compile \
    m4/config.guess \
    m4/config.sub \
    m4/depcomp \
    m4/install-sh \
    m4/missing \
    src/Makefile \
    src/Makefile.am \
    src/Makefile.in \
    src/*/Makefile \
    src/*/Makefile.in \
    src/*/Makefile.am \
    doc/Makefile.in \
    include/Makefile \
    include/Makefile.in \
    include/Makefile.am \
    include/*/Makefile \
    include/*/Makefile.in \
    include/*/Makefile.am \
    FINDSYM/Makefile \
    FINDSYM/Makefile.in \
    FINDSYM/Makefile.am \
    bin/Makefile \
    bin/Makefile.in \
    bin/Makefile.am \
    agate.h.in \
    doc/Makefile \
    Makefile \
    Makefile.in \
    Makefile.am \
    src/*/*.o \
    src/*/*.a \
    bin/*.o 
  find . -name "*~" -exec rm {} \;
  find . -name ".deps" -exec rm -rf {} \;
  for i in $(ls bin/*.cpp)
  do
    rm -rf ${i%%.cpp};
  done
else
  # Generate Makefile.am in include
  cd include
  echo "" > Makefile.am
  for lib in $(find . -type d | sed '1d')
  do
    echo "${lib##*/}dir=\$(includedir)/$lib" >> Makefile.am
    echo "${lib##*/}_HEADERS = \\" >> Makefile.am 
    for header in $(ls $lib/*.h*)
    do
      echo "  $header \\" >> Makefile.am
    done
    sed -i -e '$s/\\//' Makefile.am
    echo "" >> Makefile.am
  done
  cd ..

  # Generate Makefile.am in src
  cd src
  for lib in $(find . -type d | sed '1d')
  do
    cd $lib
    echo "noinst_LTLIBRARIES = lib${lib#./}.la
" > Makefile.am 
    echo "lib${lib#./}_la_SOURCES = \\" >> Makefile.am 
    for src in $(ls *.cpp)
    do
      echo "  $src \\" >> Makefile.am
    done
    sed -i -e '$s/\\//' Makefile.am
    echo "AM_CXXFLAGS = @AM_CXXFLAGS""@
AM_CPPFLAGS = -I\$(top_srcdir)/include @AM_CPPFLAGS@  -DAGATE_DATADIR=\\\"\$(datadir)\\\"" >> Makefile.am
    cd -
  done
  cd ..

  # Generate libagate.so
  cd src
  echo "lib_LTLIBRARIES = libagate.la

libagate_la_SOURCES = 
libagate_la_LIBADD = \\" > Makefile.am
  for lib in $(find . -type d | sed '1d')
  do
    [ $lib = "./qtgui" ] && continue
    echo "  $lib/lib${lib#./}.la \\" >> Makefile.am 
  done
  sed -i -e '$s/\\//' Makefile.am
  cd ..


  # Generate Makefile.am in bin
  cd bin
  echo "LDADD = \\" > Makefile.am 
cat >> Makefile.am << EOF
\$(top_builddir)/src/libagate.la @SPGLIB_LDFLAGS@
EOF
  sed -i -e '$s/\\//' Makefile.am
  echo "AM_LDFLAGS = @AM_LDFLAGS@
" >> Makefile.am 
  echo "bin_PROGRAMS = \\" >> Makefile.am 
  for bin in $(ls *.cpp | grep -v -e qagate -e "qrc*")
  do
    # name don't start with test or if -t provided
    if test "${bin#test}" = "${bin}" -o  "$1" = "-t" ; then
      echo "  ${bin%%.cpp} \\" >> Makefile.am
    fi
  done
  sed -i -e '$s/\\//' Makefile.am
  echo "
AM_CXXFLAGS = -I\$(top_srcdir)/include @AM_CXXFLAGS@ \$(QT_CXXFLAGS)" >> Makefile.am
  for bin in $(ls *.cpp | grep -v -e qagate -e "qrc*")
  do
    if test "${bin#test}" = "${bin}" -o  "$1" = "-t" ; then
      echo "
${bin%%.cpp}_SOURCES = $bin " >> Makefile.am
    fi
  done
  sed -i -e '$s/\\//' Makefile.am
  cd ..

  # Generate Makefile.am in FINDSYM
  cd FINDSYM
  cat > Makefile.am << EOF
EXTRA_DIST = \\
	const.dat \\
	data_images.txt \\
	findsym \\
	data_diperiodic.txt \\
	data_magnetic.txt \\
	data_space.txt \\
	data_wyckoff.txt \\
	symmetry.dat

fsymdatadir = \$(datadir)/@PACKAGE_NAME@/FINDSYM/

fsymdata_DATA = \\
	const.dat \\
	data_images.txt \\
	findsym \\
	data_diperiodic.txt \\
	data_magnetic.txt \\
	data_space.txt \\
	data_wyckoff.txt \\
	symmetry.dat

install-data-hook:
	chmod 755 \$(DESTDIR)/\$(fsymdatadir)/findsym
EOF
  cd ..


  # Generate Makefile.am in test
  cd test
  cat > Makefile.am << EOF
LDADD = \
\$(top_builddir)/src/libagate.la @SPGLIB_LDFLAGS@
AM_LDFLAGS = @AM_LDFLAGS@
AM_CXXFLAGS = -I\$(top_srcdir)/include -I\$(top_srcdir)/test/files @AM_CXXFLAGS@ 

EXTRA_DIST = \\
EOF
  for f in `ls files/`
  do
    echo "  files/$f \\" >> Makefile.am
  done
  sed -i -e '$s/\\//' Makefile.am

  bin=''
  for f in `ls *.h`;
  do
    bin="$bin$IFS${f%%_*}"
  done;
  bin=$(echo $bin | xargs -n1 | sort -u)
  echo "check_PROGRAMS = \\" >> Makefile.am
  for b in $bin
  do
    echo "  $b \\" >> Makefile.am
  done
  sed -i -e '$s/\\//' Makefile.am
  for b in $bin
  do
    echo "${b}_SOURCES = ${b}.cpp" >> Makefile.am
    echo "EXTRA_${b}_SOURCES = \\" >> Makefile.am
    for s in `ls ${b}_*.h`
    do
      echo "  $s \\" >> Makefile.am
    done
    sed -i -e '$s/\\//' Makefile.am
  done
  echo "TESTS = \$(check_PROGRAMS)" >> Makefile.am
  for b in $bin
  do
    echo "${b}.cpp:\$(EXTRA_${b}_SOURCES)" >> Makefile.am
    echo "	cxxtestgen --error-printer -o \$@ \$^" >> Makefile.am
  done
  cat >> Makefile.am << EOF
clean-local: clean-test-files
.PHONY: clean-test-files
clean-test-files:
	-rm -rf *.dat *.nc *.in ref* *DDB
EOF
  cd ..


  # Generate Makefile.am ./
  
  echo "ACLOCAL_AMFLAGS=-I m4
SUBDIRS = @AM_SPGLIB@ \\" > Makefile.am
  #for lib in $(find include -type d | sed '1d')
  #do
  #  echo "  ${lib} \\" >> Makefile.am
  #done
  for lib in $(find src -type d | sed '1d')
  do
    echo "  ${lib} \\" >> Makefile.am
  done
  echo "  include src bin doc FINDSYM test" >> Makefile.am

  cat >> Makefile.am << EOF
EXTRA_DIST = autogen.sh configure1 configure2 spglib-1.15.0.tar.gz $(if test "$1" = "-t"; then echo "glfw-3.2.1.zip"; fi)

nodist_include_HEADERS = agate.h

clean-local:
	find . -name "*~" -exec rm {} \; || echo "Not cleaning"
	find . -name "*swp" -exec rm {} \; || echo "Not cleaning"

doc:doc/doxyfile.stamp

doc/doxyfile.stamp:
	cd doc && \$(MAKE) doc

doc:doc/doxyfile.stamp

pdf:
	cd doc && \$(MAKE) $@
EOF

  cp configure1 configure.tmp
  for m in $(find include src -name Makefile.am)
  do
    echo "AC_CONFIG_FILES([${m%%.am}])" >> configure.tmp
  done
  cat configure.tmp configure2 > configure.ac
  rm configure.tmp

  LIBTOOLIZE=$(which glibtoolize  libtoolize)
  #if test "$OSTYPE" = "darwin*" 
  #then
  #  LIBTOOLIZE=glibtoolize 
  #else
  #  LIBTOOLIZE=libtoolize 
  #fi

  touch agate.h.in
  $LIBTOOLIZE && \
  aclocal \
    && automake --add-missing \
    && autoconf \
    && autoheader
fi
