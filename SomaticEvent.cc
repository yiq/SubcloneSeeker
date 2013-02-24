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

std::string SomaticEvent::getTableName() {
	return "Events";
}

bool SomaticEvent::createTableInDB(sqlite3 *database) {
	sqlite3_stmt *stmt;                                                                                                                                                                                        
	int rc;
	rc = sqlite3_prepare_v2(database, "CREATE TABLE Events (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, frequency REAL NOT NULL, chrom INTEGER NOT NULL, start INTEGER NOT NULL, length INTEGER NULL);", -1, &stmt, 0);
	if(rc == SQLITE_OK) {
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		return true;
	}
	return false;
}
