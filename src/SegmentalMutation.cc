/**
 * @file SegmentalMutation.cc
 * Implementation of class SegmentalMutation
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

#include "SegmentalMutation.h"
#include <iostream>

using namespace SubcloneExplorer;

std::string SegmentalMutation::createObjectStatementStr() {
	return "INSERT INTO " + getTableName() + " (frequency, chrom, start, length, ofClusterID) VALUES (?,?,?,?,?);";
}

std::string SegmentalMutation::updateObjectStatementStr() {
	return "UPDATE " + getTableName() + " SET frequency=?, chrom=?, start=?, length=?, ofClusterID=? WHERE id=?;";
}

std::string SegmentalMutation::selectObjectColumnListStr() {
	return "frequency, chrom, start, length, ofClusterID";
}

int SegmentalMutation::bindObjectToStatement(sqlite3_stmt *statement) {
	int bind_loc = 1;
	sqlite3_bind_double(statement, bind_loc++, frequency);
	sqlite3_bind_int(statement, bind_loc++, range.chrom);
	sqlite3_bind_int64(statement, bind_loc++, range.position);
	sqlite3_bind_int64(statement, bind_loc++, range.length);
	if(ofClusterID > 0) {
		sqlite3_bind_int64(statement, bind_loc++, ofClusterID);
	}
	else {
		sqlite3_bind_null(statement, bind_loc++);
	}
	return bind_loc;
}

void SegmentalMutation::updateObjectFromStatement(sqlite3_stmt *statement) {
	int col_pos = 0;
	frequency = sqlite3_column_double(statement, col_pos++);
	range.chrom = sqlite3_column_int(statement, col_pos++);
	range.position = sqlite3_column_int64(statement, col_pos++);
	range.length = sqlite3_column_int64(statement, col_pos++);

	if(sqlite3_column_type(statement, col_pos) != SQLITE_NULL) {
		ofClusterID = sqlite3_column_int64(statement, col_pos++);
	}
	else {
		ofClusterID = 0;
	}
}

bool CNV::isEqualTo(SomaticEvent * anotherEvent, unsigned long resolution) {
	CNV *cnvEvent = dynamic_cast<CNV*>(anotherEvent);

	if(cnvEvent == NULL) {
		return false; 
	}

	if(range.chrom != cnvEvent->range.chrom) {
		return false; 
	}

	unsigned long startDiff = range.position > cnvEvent->range.position?
		range.position - cnvEvent->range.position:
		cnvEvent->range.position - range.position;

	unsigned long thisEnd = range.position + range.length;
	unsigned long anotherEnd = cnvEvent->range.position + cnvEvent->range.length;
	unsigned long endDiff = thisEnd > anotherEnd ? 
		thisEnd - anotherEnd :
		anotherEnd - thisEnd;

	if(startDiff < resolution && endDiff < resolution)
		return true;

	return false;
}

std::string CNV::getTableName() {
	return "Events_CNV";
}

std::string LOH::getTableName() {
	return "Events_LOH";
}
