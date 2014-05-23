/**
 * @file SomaticEvent.cc
 *
 * Implementation of class SomaticEvent
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

#include "SomaticEvent.h"
#include <sqlite3/sqlite3.h>
#include <string>

using namespace SubcloneSeeker;

std::string SomaticEvent::createTableStatementStr() {
	return ", frequency REAL NOT NULL, chrom INTEGER NOT NULL, start INTEGER NOT NULL, length INTEGER NULL, ofClusterID INTEGER NULL REFERENCES Clusters(id)";
}

DBObjectID_vec SomaticEvent::allObjectsOfCluster(sqlite3 *database, sqlite3_int64 clusterID) {
	std::string queryStr = "SELECT id FROM " + getTableName() + " WHERE ofClusterID=?;";
	sqlite3_stmt *st;
	int rc;
	DBObjectID_vec res_vec;

	rc = sqlite3_prepare_v2(database, queryStr.c_str(), -1, &st, 0);
	if(rc != SQLITE_OK) return res_vec;

	rc = sqlite3_bind_int64(st, 1, clusterID);
	if(rc != SQLITE_OK) return res_vec;

	while(sqlite3_step(st) == SQLITE_ROW) {
		sqlite3_int64 newID;
		newID = sqlite3_column_int64(st, 0);
		res_vec.push_back(newID);
	}

	sqlite3_finalize(st);

	return res_vec;
}
