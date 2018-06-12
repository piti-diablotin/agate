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
    config.h \
    ar-lib \
    compile \
    config.guess \
    config.status \
    config.sub \
    depcomp \
    install-sh \
    missing \
    Win_x86/Makefile.am \
    Win_x86/Makefile.in \
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
    config.h.in \
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
AM_CPPFLAGS = -I\$(top_srcdir)/include @AM_CPPFLAGS@  -DDATADIR=\\\"\$(datadir)\\\"" >> Makefile.am
    cd -
  done
  cd ..

  # Generate Makefile.am in src/qtgui
  cd src/qtgui
  echo "if HAVE_QT
  include \$(top_srcdir)/m4/autotroll.mk" > Makefile.am
  echo "noinst_LIBRARIES = libqtgui.a
" >> Makefile.am 
  echo "libqtgui_a_SOURCES = \\" >> Makefile.am 
  for src in $(ls *.cpp | grep -v moc.cpp)
  do
    echo "  $src \\" >> Makefile.am
  done
  sed -i -e '$s/\\//' Makefile.am
  echo "lib${lib#./}_a_CXXFLAGS = -fPIC" >> Makefile.am


  echo "BUILT_SOURCES = \\" >> Makefile.am 
  for src in $(ls *.cpp | grep -v moc.cpp)
  do
    echo "  ${src%.cpp}.moc.cpp \\" >> Makefile.am
  done
  sed -i -e '$s/\\//' Makefile.am

  echo "nodist_libqtgui_a_SOURCES = \$(BUILT_SOURCES)" >> Makefile.am

  echo "CLEANFILES = \\" >> Makefile.am 
  for src in $(ls *.cpp | grep -v moc.cpp)
  do
    echo "  ${src%.cpp}.moc.cpp \\" >> Makefile.am
  done
  sed -i -e '$s/\\//' Makefile.am

  echo "
endif
AM_CXXFLAGS = @AM_CXXFLAGS@ \$(QT_CXXFLAGS)
AM_CPPFLAGS = -I\$(top_srcdir)/include @AM_CPPFLAGS@ \$(QT_CPPFLAGS)
" >> Makefile.am
  cd ../..

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
  echo "include \$(top_srcdir)/m4/autotroll.mk
LDADD = \\" > Makefile.am 
cat >> Makefile.am << EOF
\$(top_builddir)/src/libagate.la
EOF
#  for lib in $(find ../src -type d | sed -e '1d' -e '/qt/d')
#  do
#    echo "\$(top_builddir)/src/${lib#../src/}/lib${lib#../src/}.a \\" >> Makefile.am
#  done
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

  echo "if HAVE_QT
RESSOURCES = \\" >> Makefile.am 
  for qrc in $(ls ../*.qrc)
  do
    echo "  qrc_$(basename $qrc .qrc).cpp \\" >> Makefile.am
  done
  sed -i -e '$s/\\//' Makefile.am

  echo "
bin_PROGRAMS += qagate
qagate_SOURCES = qagate.cpp 
nodist_qagate_SOURCES = \$(RESSOURCES)
qagate_CPPFLAGS = \$(QT_CPPFLAGS) \$(AM_CPPFLAGS)
qagate_CXXFLAGS = \$(QT_CXXFLAGS) \$(AM_CXXFLAGS)
qagate_LDADD = \$(LDADD) \$(top_builddir)/src/qtgui/libqtgui.a \$(QT_LIBS)
qagate_LDFLAGS = @AM_LDFLAGS@ \$(QT_LDFLAGS)

endif
" >> Makefile.am
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

  # Generate Makefile.am in Win_x86
  cd Win_x86
  cat > Makefile.am << EOF
EXTRA_DIST = \\
  AbiOut.exe
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
  echo "  include src bin doc FINDSYM Win_x86" >> Makefile.am

  cat >> Makefile.am << EOF
EXTRA_DIST = autogen.sh configure1 configure2 icons icons.qrc spglib-1.9.9.tar.gz $(if test "$1" = "-t"; then echo "glfw-3.1.1.zip"; fi)
#EXTRA_DIST = autogen.sh glfw-3.1.2.zip spglib-1.8.3.tar.gz

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

  touch config.h.in
  libtoolize && \
  aclocal \
    && automake --add-missing \
    && autoconf \
    && autoheader
fi
