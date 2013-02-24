/**
 * @file SNP.cc
 * Implementation of class SNP
 *
 * @author Yi Qiao
 */

#include "SNP.h"

using namespace SubcloneExplorer;

std::string SNP::createObjectStatementStr() {
	return "INSERT INTO Events (frequency, chrom, start, length) VALUES (?,?,?,NULL);";
}

std::string SNP::updateObjectStatementStr() {
	return "UPDATE Events SET frequency=?, chrom=?, start=?, length=NULL WHERE id=?;";
}

std::string SNP::selectObjectColumnListStr() {
	return "frequency, chrom, start";
}

void SNP::bindObjectToStatement(sqlite3_stmt *statement) {
	int bind_loc = 1;
	sqlite3_bind_double(statement, bind_loc++, frequency);
	sqlite3_bind_int(statement, bind_loc++, location.chrom);
	sqlite3_bind_int64(statement, bind_loc++, location.position);
}

void SNP::updateObjectFromStatement(sqlite3_stmt *statement) {
	int col_pos = 0;
	frequency = sqlite3_column_double(statement, col_pos++);
	location.chrom = sqlite3_column_int(statement, col_pos++);
	location.position = sqlite3_column_int64(statement, col_pos++);
}
