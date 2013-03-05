/**
 * @file Subclone.cc
 * Implementation of class Subclone
 * 
 * @author Yi Qiao
 */

#include <assert.h>

#include "Subclone.h"
#include "EventCluster.h"

using namespace SubcloneExplorer;

void Subclone::addEventCluster(EventCluster *cluster) {
	bool alreadyExist = false;

	for(size_t i=0; i<_eventClusters.size(); i++) {
		if(_eventClusters[i] == cluster) {
			alreadyExist = true;
			break;
		}
	}

	if(!alreadyExist) {
		_eventClusters.push_back(cluster);
	}
}

// Implements Archivable
std::string Subclone::getTableName() {
	return "Subclones";
}

std::string Subclone::createTableStatementStr() {
	return ", fraction REAL NOT NULL, treeFraction REAL NOT NULL, parentId INTEGER NULL REFERENCES Subclones(id)";
}

std::string Subclone::createObjectStatementStr() {
	return "INSERT INTO Subclones (fraction, treeFraction, parentId) VALUES (?, ?, ?);";
}

std::string Subclone::updateObjectStatementStr() {
	return "UPDATE Subclones SET fraction=?, treeFraction=?, parentId=? WHERE id=?;";
}

std::string Subclone::selectObjectColumnListStr() {
	return "fraction, treeFraction, parentId";
}

void Subclone::bindObjectToStatement(sqlite3_stmt* statement) {
	int bind_loc = 1;
	sqlite3_bind_double(statement, bind_loc++, _fraction);
	sqlite3_bind_double(statement, bind_loc++, _treeFraction);
	if(parentId > 0) {
		sqlite3_bind_int64(statement, bind_loc++, parentId);
	}
	else {
		sqlite3_bind_null(statement, bind_loc++);
	}
}

void Subclone::updateObjectFromStatement(sqlite3_stmt* statement) {
	int col_pos = 0;
	_fraction = sqlite3_column_double(statement, col_pos++);
	_treeFraction = sqlite3_column_double(statement, col_pos++);

	if(sqlite3_column_type(statement, col_pos) != SQLITE_NULL) {
		parentId = sqlite3_column_int64(statement, col_pos++);
	}
	else {
		parentId = 0;
	}
}


// SubcloneSaveTreeTraverser
void SubcloneSaveTreeTraverser::processNode(TreeNode *node) {
	Subclone *clone = dynamic_cast<Subclone *>(node);

	sqlite3_int64 id  = clone->archiveObjectToDB(_database);

	assert(clone->getId() == id);

	for(size_t i=0; i<clone->vecEventCluster().size(); i++)
		(clone->vecEventCluster()[i])->setSubcloneID(id);
}

void SubcloneSaveTreeTraverser::preprocessNode(TreeNode *node) {
	Subclone *clone = dynamic_cast<Subclone *>(node);

	if(clone->isLeaf())
		return;

	for(size_t i=0; i<clone->getVecChildren().size(); i++) {
		Subclone *children = dynamic_cast<Subclone *>(clone->getVecChildren()[i]);
		children->setParentId(clone->getId());
	}
}

// SubcloneLoadTreeTraverser
std::vector<sqlite3_int64> SubcloneLoadTreeTraverser::rootNodes(sqlite3 *database) {
	return nodesOfParentID(database, 0);
}

std::vector<sqlite3_int64> SubcloneLoadTreeTraverser::nodesOfParentID(sqlite3 *database, sqlite3_int64 parentId) {

	sqlite3_stmt *statement;
	int rc;
	std::vector<sqlite3_int64> res;

	if(parentId == 0) 
		rc = sqlite3_prepare_v2(database, "SELECT id FROM Subclones WHERE parentId is NULL;", -1, &statement, 0);
	else
		rc = sqlite3_prepare_v2(database, "SELECT id FROM Subclones WHERE parentId = ?;", -1, &statement, 0);

	if(rc != SQLITE_OK) {
		sqlite3_finalize(statement);
		return res;
	}

	if(parentId != 0) {
		rc = sqlite3_bind_int64(statement, 1, parentId);

		if(rc != SQLITE_OK) {
			sqlite3_finalize(statement);
			return res;
		}
	}

	while((rc = sqlite3_step(statement)) == SQLITE_ROW) {
		sqlite3_int64 rootId = sqlite3_column_int64(statement, 0);
		res.push_back(rootId);
	}

	sqlite3_finalize(statement);
	return res;
}

void SubcloneLoadTreeTraverser::processNode(TreeNode * node) {
	Subclone *clone = dynamic_cast<Subclone *>(node);
	std::vector<sqlite3_int64> childrenIDs = nodesOfParentID(_database, clone->getId());

	for(size_t i=0; i<childrenIDs.size(); i++) {
		Subclone *children = new Subclone();
		children->unarchiveObjectFromDB(_database, childrenIDs[i]);
		clone->addChild(children);
	}
}
