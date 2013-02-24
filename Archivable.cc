/**
 * @file Archivable.cc
 * Implementations of the class Archivable
 *
 * @author Yi Qiao
 */

#include "Archivable.h"
#include <sqlite3/sqlite3.h>
#include <string>

using namespace SubcloneExplorer;

bool Archivable::createTableInDB(sqlite3 *database) {
	sqlite3_stmt *stmt;

	std::string id_str = "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT";
	std::string stmt_str = "CREATE TABLE " + getTableName() + " ( " + id_str + createTableStatementStr() + ");";

	int rc = sqlite3_prepare_v2(database, stmt_str.c_str(), -1, &stmt, 0);
	if(rc != SQLITE_OK) {
		sqlite3_finalize(stmt);
		return false;
	}

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	if(rc != SQLITE_DONE) {
		return false;
	}

	return true;
}

sqlite3_int64 Archivable::archiveObjectToDB(sqlite3 *database) {
	sqlite3_stmt *statement;
	int rc;
	int bind_loc;

	// First, determines if the record already exist
	std::string select_str = "SELECT id FROM " + getTableName() + " WHERE id=?;";
	rc = sqlite3_prepare_v2(database, select_str.c_str(), -1, &statement, 0);
	if(rc != SQLITE_OK) {
		sqlite3_finalize(statement);
		return -1;
	}

	sqlite3_bind_int64(statement, 1, id);
	rc = sqlite3_step(statement);
	sqlite3_finalize(statement);

	if(rc == SQLITE_ROW) {
		// record exist, update mode
		rc = sqlite3_prepare_v2(database, updateObjectStatementStr().c_str(), -1, &statement, 0);
		if(rc != SQLITE_OK) {
			sqlite3_finalize(statement);
			return -1;
		}

		bindObjectToStatement(statement);

		rc = sqlite3_step(statement);
		sqlite3_finalize(statement);
		if(rc != SQLITE_DONE) {
			return -1;
		}

		return id;
	}
	else {
		// record does not exist, insert mode
		rc = sqlite3_prepare_v2(database, createObjectStatementStr().c_str(), -1, &statement, 0);
		if(rc != SQLITE_OK) {
			sqlite3_finalize(statement);
			return -1;
		}
		
		bindObjectToStatement(statement);

		rc = sqlite3_step(statement);
		sqlite3_finalize(statement);
		if(rc != SQLITE_DONE) {
			return -1;
		}

		id = sqlite3_last_insert_rowid(database);
		return id;
	}
}

bool Archivable::unarchiveObjectFromDB(sqlite3 *database, sqlite3_int64 id) {
	sqlite3_stmt* statement;
	int rc;
	std::string select_str = "SELECT " + selectObjectColumnListStr() + " FROM " + getTableName() + " WHERE id=?;";
	rc = sqlite3_prepare_v2(database, select_str.c_str(), -1, &statement, 0);
	if(rc != SQLITE_OK) {
		sqlite3_finalize(statement);
		return false;
	}

	sqlite3_bind_int64(statement, 1, id);
	rc = sqlite3_step(statement);
	if(rc != SQLITE_ROW) {
		sqlite3_finalize(statement);
		return false;
	}

	this->id = id;

	updateObjectFromStatement(statement);

	sqlite3_finalize(statement);
	return true;
}
