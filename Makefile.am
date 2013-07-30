# The MIT License (MIT)
# 
# Copyright (c) 2013 Yi Qiao
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.



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
