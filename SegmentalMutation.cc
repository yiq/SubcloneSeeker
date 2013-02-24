
/**
 * @file SegmentalMutation.cc
 * Implementation of class SegmentalMutation
 *
 * @author Yi Qiao
 */

#include "SegmentalMutation.h"

using namespace SubcloneExplorer;

sqlite3_int64 SegmentalMutation::archiveObjectToDB(sqlite3 *database) {
	sqlite3_stmt *statement;
	int rc;
	int bind_loc;

	// First, determines if the record already exist
	rc = sqlite3_prepare_v2(database, "SELECT id FROM Events WHERE id=?;", -1, &statement, 0);
	if(rc != SQLITE_OK) {
		sqlite3_finalize(statement);
		return -1;
	}

	sqlite3_bind_int64(statement, 1, id);
	rc = sqlite3_step(statement);
	sqlite3_finalize(statement);

	if(rc == SQLITE_ROW) {
		// record exist, update mode
		rc = sqlite3_prepare_v2(database, "UPDATE Events SET frequency=?, chrom=?, start=?, length=? WHERE id=?;", -1, &statement, 0);
		if(rc != SQLITE_OK) {
			sqlite3_finalize(statement);
			return -1;
		}

		bind_loc = 1;
		sqlite3_bind_double(statement, bind_loc++, frequency);
		sqlite3_bind_int(statement, bind_loc++, range.start.chrom);
		sqlite3_bind_int64(statement, bind_loc++, range.start.position);
		sqlite3_bind_int64(statement, bind_loc++, range.length);

		rc = sqlite3_step(statement);
		sqlite3_finalize(statement);
		if(rc != SQLITE_DONE) {
			return -1;
		}

		return id;
	}
	else {
		// record does not exist, insert mode
		rc = sqlite3_prepare_v2(database, "INSERT INTO Events (frequency, chrom, start, length) VALUES (?,?,?,?);", -1, &statement, 0);
		if(rc != SQLITE_OK) {
			sqlite3_finalize(statement);
			return -1;
		}

		bind_loc = 1;
		sqlite3_bind_double(statement, bind_loc++, frequency);
		sqlite3_bind_int(statement, bind_loc++, range.start.chrom);
		sqlite3_bind_int64(statement, bind_loc++, range.start.position);
		sqlite3_bind_int64(statement, bind_loc++, range.length);

		rc = sqlite3_step(statement);
		sqlite3_finalize(statement);
		if(rc != SQLITE_DONE) {
			return -1;
		}

		id = sqlite3_last_insert_rowid(database);
		return id;
	}
}

bool SegmentalMutation::unarchiveObjectFromDB(sqlite3* database, sqlite3_int64 id) {
	sqlite3_stmt* statement;
	int rc;
	rc = sqlite3_prepare_v2(database, "SELECT frequency, chrom, start, length FROM Events WHERE id=?;", -1, &statement, 0);
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

	int col_pos = 0;
	frequency = sqlite3_column_double(statement, col_pos++);
	range.start.chrom = sqlite3_column_int(statement, col_pos++);
	range.start.position = sqlite3_column_int64(statement, col_pos++);
	range.length = sqlite3_column_int64(statement, col_pos++);

	sqlite3_finalize(statement);
	return true;
}
