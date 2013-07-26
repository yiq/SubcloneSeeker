SUBDIRS = contrib src test utils
EXTRA_DIST = contrib \
			 src/*.h \
			 src/*.cc \
			 test/*.h \
			 test/*.cc \
			 utils/*.h \
			 utils/*.cc \
			 doc/Doxyfile.in \
			 doc/*.md

if HAVE_DOXYGEN

doc: doc/mainpage.md
	$(DOXYGEN) doc/Doxyfile

.PHONY: doc

doc/mainpage.md: README INSTALL doc/mainpage_header.md doc/mainpage_additional.md
	cat doc/mainpage_header.md > doc/mainpage.md
	cat README >> doc/mainpage.md
	cat INSTALL >> doc/mainpage.md
	cat doc/mainpage_additional.md >> doc/mainpage.md

endif
