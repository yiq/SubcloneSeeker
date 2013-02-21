#include "TreeNode.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <sqlite3/sqlite3.h>

using namespace qiaoy;

TreeNode::TreeNode(sqlite3 *database, sqlite3_int64 id) :
	parent(NULL), dbRecord()
{
	sqlite3_stmt *statement;
	int rc;
	rc = sqlite3_prepare_v2(database, "SELECT * FROM Node WHERE id=? LIMIT 1;", -1, &statement, 0);
	if(rc == SQLITE_OK) {
		sqlite3_bind_int64(statement, 1, id);
		rc = sqlite3_step(statement);
		if(rc == SQLITE_ROW) {
			dbRecord.id = sqlite3_column_int64(statement, 0);
			dbRecord.parentId = sqlite3_column_int64(statement, 1);
			dbRecord.isStoredInDB = true;
		}

		sqlite3_finalize(statement);
	}
}

void TreeNode::addChild(TreeNode *child)
{
	if(child==NULL) return;
	
	TreeNodeVec_t::iterator child_it = std::find(children.begin(), children.end(), child);
	if(child_it != children.end()) return;
	
	children.push_back(child);
	child->parent = this;

	if(dbRecord.id != 0)
		child->dbRecord.parentId = dbRecord.id;
}

void TreeNode::removeChild(TreeNode *child)
{
	TreeNodeVec_t::iterator child_it = std::find(children.begin(), children.end(), child);
	if(child_it != children.end()) {
		// child found in the node's children list, removing it from the vector
		children.erase(child_it);
		child->dbRecord.parentId = 0;
		child->parent = NULL;
	}
}

void TreeNode::PreOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate)
{
	// check if premature-termination has happened
	if(traverseDelegate.isTerminated())
		return;
	
	// Check if the root node exists at all
	if (root == NULL) return;
	
	// Do the bidding of the traverse delegate
	traverseDelegate.processNode(root);
	
	// recursively traverse the children nodes
	for(TreeNodeVec_t::iterator it = root->children.begin(); it != root->children.end(); it++) {
		TreeNode::PreOrderTraverse(*it, traverseDelegate);
		// check if premature-termination has happened
		if(traverseDelegate.isTerminated())
			return;
	}
}

void TreeNode::PostOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate)
{
	// check if premature-termination has happened
	if(traverseDelegate.isTerminated())
		return;
	
	// Check if the root node exists at all
	if (root == NULL) return;
	
	// recursively traverse the children nodes
	for(TreeNodeVec_t::iterator it = root->children.begin(); it != root->children.end(); it++) {
		TreeNode::PostOrderTraverse(*it, traverseDelegate);
		// check if premature-termination has happened
		if(traverseDelegate.isTerminated())
			return;
	}
	
	// Do the bidding of the traverse delegate
	traverseDelegate.processNode(root);
}

void TreeNode::saveTree(sqlite3 *database, TreeNode *root) {

	class TreeSaveTraverser : public TreeTraverseDelegate {
		protected:
			sqlite3 *_database;
			
		public:
			TreeSaveTraverser(sqlite3 *database): TreeTraverseDelegate(), _database(database) {;}
			virtual void processNode(TreeNode * node)
			{
				if(node->dbRecord.isStoredInDB) {
					if(node->dbRecord.id == 0) {
						std::cerr<<"DATABASE RECORD INCONSISTENT. RECORD INDICATES OBJECT IN DB YET ID==0"<<std::endl;
						abort();
					}
				}
				else {
					// save the current node
					sqlite3_stmt *statement;
					int rc;

					// if the current node is root, insert into Node table with parentNodeID equals NULL
					if(node->parent == NULL) {
						rc = sqlite3_prepare_v2(_database, "insert into node (parentNodeId) values (NULL);", -1, &statement, 0);
					}
					else {
						rc = sqlite3_prepare_v2(_database, "insert into node (parentNodeId) values (?);", -1, &statement, 0);
					}

					if(rc == SQLITE_OK) {
						if(node->parent != NULL)
							sqlite3_bind_int64(statement, 1, node->dbRecord.parentId);
						sqlite3_step(statement);
						sqlite3_finalize(statement);
					}

					// fetch the row id
					node->dbRecord.id = sqlite3_last_insert_rowid(_database);
					node->dbRecord.isStoredInDB = true;

					node->update(_database);
				}

				// update children node about parent's id
				TreeNodeVec_t::iterator it = node->children.begin();
				for(; it != node->children.end(); it++) {
					if((*it)->dbRecord.parentId != 0 && (*it)->dbRecord.parentId != node->dbRecord.id) {
						std::cerr<<"DATABASE RECORD INCONSISTENT. RECORD INDICATES A DIFFERENT PARENT IN DB"<<std::endl;
						abort();
					}
					(*it)->dbRecord.parentId = node->dbRecord.id;
				}
			}
	};

	TreeSaveTraverser saveTraverser(database);
	PreOrderTraverse(root, saveTraverser);
}

std::vector<sqlite3_int64> TreeNode::loadRoots(sqlite3 *database)
{
	sqlite3_stmt *statement;
	int rc;
	std::vector<sqlite3_int64> rootVec;

	rc = sqlite3_prepare_v2(database, "SELECT id FROM Node WHERE parentNodeId is NULL;", -1, &statement, 0);
	if(rc == SQLITE_OK) {
		while(true) {
			rc = sqlite3_step(statement);
			if(rc == SQLITE_ROW) {
				sqlite3_int64 id = sqlite3_column_int64(statement, 0);
				rootVec.push_back(id);
			}
			else {
				sqlite3_finalize(statement);
				break;
			}
		}
	}
	return rootVec;
}
