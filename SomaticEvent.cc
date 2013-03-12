/**
 * @file SomaticEvent.cc
 *
 * Implementation of class SomaticEvent
 *
 * @author Yi Qiao
 */

#include "SomaticEvent.h"
#include <sqlite3/sqlite3.h>
#include <string>

using namespace SubcloneExplorer;

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
