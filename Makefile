all: libss utils

libss:
	make -C src

utils: libss
	make -C utils

doc: DOXYGEN-exists doc/mainpage.md
	doxygen doc/Doxyfile

DOXYGEN-exists: ; @which doxygen > /dev/null 2>&1

doc/mainpage.md: README.md doc/mainpage_header.md doc/mainpage_additional.md
	cat doc/mainpage_header.md $< doc/mainpage_additional.md > $@

check: libss utils
	make -C vendor/UnitTest++
	make -C test check
	make -C utils check

clean:
	make -C vendor/UnitTest++ clean
	make -C src clean
	make -C utils clean
	make -C test clean

.PHONY: all libss utils check clean doc
