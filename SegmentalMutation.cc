/**
 * @file SegmentalMutation.cc
 * Implementation of class SegmentalMutation
 *
 * @author Yi Qiao
 */

#include "SegmentalMutation.h"

using namespace SubcloneExplorer;

std::string SegmentalMutation::createObjectStatementStr() {
	return "INSERT INTO Events (frequency, chrom, start, length) VALUES (?,?,?,?);";
}

std::string SegmentalMutation::updateObjectStatementStr() {
	return "UPDATE Events SET frequency=?, chrom=?, start=?, length=? WHERE id=?;";
}

std::string SegmentalMutation::selectObjectColumnListStr() {
	return "frequency, chrom, start, length";
}

void SegmentalMutation::bindObjectToStatement(sqlite3_stmt *statement) {
	int bind_loc = 1;
	sqlite3_bind_double(statement, bind_loc++, frequency);
	sqlite3_bind_int(statement, bind_loc++, range.start.chrom);
	sqlite3_bind_int64(statement, bind_loc++, range.start.position);
	sqlite3_bind_int64(statement, bind_loc++, range.length);
}

void SegmentalMutation::updateObjectFromStatement(sqlite3_stmt *statement) {
	int col_pos = 0;
	frequency = sqlite3_column_double(statement, col_pos++);
	range.start.chrom = sqlite3_column_int(statement, col_pos++);
	range.start.position = sqlite3_column_int64(statement, col_pos++);
	range.length = sqlite3_column_int64(statement, col_pos++);
}
