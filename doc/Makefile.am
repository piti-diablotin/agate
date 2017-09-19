if HAVE_DOXYGEN
directory = $(top_builddir)/doc

dist_man_MANS = $(directory)/man
$(directory)/man: doxyfile.stamp

doxyfile.stamp:
	$(DOXYGEN) Doxyfile > /dev/null
	touch doxyfile.stamp

CLEANFILES = doxyfile.stamp \
             pdf.stamp

all-local: doc pdf.stamp

#mkdir -p $(docdir)/
#install: doc pdf.stamp
#	for type in *; do \
#	test ! -d $$type && continue || mkdir -p $(docdir)/$$type; \
#	done

doc: doxyfile.stamp

pdf: pdf.stamp

pdf.stamp:doxyfile.stamp
if HAVE_PDFLATEX
	cd latex && $(MAKE)
	touch pdf.stamp
endif

clean-local:
	for type in *; do \
	test ! -d $$type  || rm -rf $$type; \
	done;

#uninstall:
#	for type in *; do \
#	test ! -d $$type  || rm -rf $(docdir)/$$type; \
#	done;
	
endif 
