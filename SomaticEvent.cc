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

std::string SomaticEvent::createTableStatementStr() {
	return ", frequency REAL NOT NULL, chrom INTEGER NOT NULL, start INTEGER NOT NULL, length INTEGER NULL";
}
