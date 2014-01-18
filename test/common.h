#ifndef TEST_COMMON_H
#define TEST_COMMON_H

/*
 * @file test/common.h
 * Some macros commonly used by the test cases
 *
 * @author Yi Qiao
 */

/*
The MIT License (MIT)

Copyright (c) 2013 Yi Qiao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <UnitTest++/src/UnitTest++.h>
#include <sqlite3/sqlite3.h>
#include <cstdio>

/* Fixtures */

/* Fixture that provides a sqlite3 database environment */
struct DBFixture {
	sqlite3 *database;
	DBFixture() {
		sqlite3_open("test.sqlite", &database);
	}

	~DBFixture() {
		sqlite3_close(database);
		remove("test.sqlite");
	}
};


/* Macro that expands to the standard main function to run all tests */
#define TEST_MAIN int main() {return UnitTest::RunAllTests();}

#endif
