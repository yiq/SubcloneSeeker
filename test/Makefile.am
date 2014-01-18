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


AM_CPPFLAGS = -I$(srcdir)/../contrib -I$(srcdir)/../src
AM_LDFLAGS = -L$(srcdir)/../src -L$(srcdir)/../contrib/UnitTest++
LDADD = -lss -lUnitTest++

check_PROGRAMS = TestGenomicLocation \
				 TestGenomicRange \
				 TestEventCluster \
				 TestSomaticEvent \
				 TestTreeNode \
				 TestSubclone

TESTS = $(check_PROGRAMS)

TestGenomicLocation_SOURCES = TestGenomicLocation.cc
TestGenomicRange_SOURCES = TestGenomicRange.cc
TestEventCluster_SOURCES = TestEventCluster.cc
TestSomaticEvent_SOURCES = TestSomaticEvent.cc
TestTreeNode_SOURCES = TestTreeNode.cc
TestSubclone_SOURCES = TestSubclone.cc
