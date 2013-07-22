#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <UnitTest++/src/UnitTest++.h>
#include <sqlite3/sqlite3.h>

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
