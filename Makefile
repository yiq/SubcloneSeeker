all: libss utils

libss:
	make -C src

utils: libss
	make -C utils

doc: DOXYGEN-exists doc/mainpage.md

DOXYGEN-exists: ; @which doxygen > /dev/null
	$(error doxygen is not found in your PATH, cannot compile documentation)

doc/mainpage.md: README.md
	cp $< $@

check: libss utils
	make -C vendor/UnitTest++
	make -C test check
	make -C utils check

clean:
	make -C vendor/UnitTest++ clean
	make -C src clean
	make -C utils clean
	make -C test clean

.PHONY: all libss utils check clean
