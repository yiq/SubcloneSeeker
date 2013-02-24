#include <iostream>
#include <sstream>
#include <assert.h>
#include "TreeNode.h"
#include "SubcloneExplore.h"
#include <sqlite3/sqlite3.h>

#define TEST_DB "sample.sqlite"

using namespace qiaoy;

void test_tree();
void test_traverse();
void test_write_to_db();
void test_read_from_db();

// simple tree node with int as it's satllite data
class SimpleTreeNode : public TreeNode {
protected:
	int data;
	
public:
	explicit SimpleTreeNode(): TreeNode() {;}
	explicit SimpleTreeNode(sqlite3 *database, sqlite3_int64 id): TreeNode(database, id) { load(database);}
	void setData(int data) {this->data = data;}
	int getData() {return data;}
	virtual void update(sqlite3 *database) {
		sqlite3_stmt *statement;
		int rc = sqlite3_prepare_v2(database, "UPDATE Node SET data=? WHERE id=?;", -1, &statement, 0);
		if(rc == SQLITE_OK) {
			sqlite3_bind_int(statement, 1, data);
			sqlite3_bind_int64(statement, 2, dbRecord.id);
			sqlite3_step(statement);
			sqlite3_finalize(statement);
		}
	}
	virtual void load(sqlite3 *database) {
		TreeNode::load(database);

		sqlite3_stmt *statement;
		int rc = sqlite3_prepare_v2(database, "SELECT data FROM Node WHERE id=?;", -1, &statement, 0);
		if(rc == SQLITE_OK) {
			sqlite3_bind_int64(statement, 1, dbRecord.id);
			while(sqlite3_step(statement) == SQLITE_ROW) {
				data = sqlite3_column_int(statement, 0);
			}
			sqlite3_finalize(statement);
		}
	}
	virtual std::string description() {
		std::stringstream ss;
		ss<<data;
		return ss.str();
	}

	static void initializeDatabase(sqlite3 *database) {
		sqlite3_stmt *stmt;
		int rc;
		rc = sqlite3_prepare_v2(database, "CREATE TABLE Node (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, parentNodeId INTEGER NULL REFERENCES Node(id), data INTEGER NULL);", -1, &stmt, 0);
		if(rc == SQLITE_OK) {
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}
	}
};


int main(int argc, char *argv[])
{
//	test_tree();
//	test_traverse();
//	SubcloneExplorerMain(argc, argv);
	test_write_to_db();
	test_read_from_db();
	return 0;
}



#pragma mark --Text Functions--
void test_tree() {
	SimpleTreeNode *root = new SimpleTreeNode();
	assert(root->getParent() == NULL);
	assert(root->getData() == 0);
	std::cout<<"Test passed"<<std::endl;
}

class PrintDataTraverser : public TreeTraverseDelegate {
public:
	virtual void processNode(TreeNode * node) {
		
		std::cout<<((SimpleTreeNode *)node)->getData()<<",";
	}
} pdTraverserObj;

class FaultingTraverser : public TreeTraverseDelegate {
	protected:
		sqlite3 *_database;
	public:
		FaultingTraverser(sqlite3 *database) : 
			TreeTraverseDelegate(), _database(database) {;}
		virtual void processNode(TreeNode *node) {
			// search for children nodes
			sqlite3_stmt *statement;
			int rc;
			rc = sqlite3_prepare_v2(_database, "SELECT id FROM Node WHERE parentNodeId=?;", -1, &statement, 0);
			if(rc == SQLITE_OK) {
				sqlite3_bind_int64(statement, 1, node->getDBId());
				while(true) {
					rc = sqlite3_step(statement);
					if(rc == SQLITE_ROW) {
						sqlite3_int64 cid = sqlite3_column_int64(statement, 0);
						SimpleTreeNode *child = new SimpleTreeNode(_database, cid);
						node->addChild(child);
					}
					else {
						break;
					}
				}
				sqlite3_finalize(statement);
			}
		}
};

void test_traverse() {
	// 1
	SimpleTreeNode * root = new SimpleTreeNode;
	root->setData(1);
	
	// 2
	SimpleTreeNode * node2 = new SimpleTreeNode;
	node2->setData(2);
	root->addChild(node2);
	
	// 3
	SimpleTreeNode * node3 = new SimpleTreeNode;
	node3->setData(3);
	node2->addChild(node3);
	
	// 4
	SimpleTreeNode * node4 = new SimpleTreeNode;
	node4->setData(4);
	node2->addChild(node4);
	
	// 5
	SimpleTreeNode * node5 = new SimpleTreeNode;
	node5->setData(5);
	root->addChild(node5);
	
	// 6
	SimpleTreeNode * node6 = new SimpleTreeNode;
	node6->setData(6);
	node5->addChild(node6);
	
	// 7
	SimpleTreeNode * node7 = new SimpleTreeNode;
	node7->setData(7);
	node5->addChild(node7);
	
	//Traverser
	std::cout<<"pre-order traverse: ";
	TreeNode::PreOrderTraverse(root, pdTraverserObj);
	std::cout<<std::endl;
	
	std::cout<<"post-order traverse: ";
	TreeNode::PostOrderTraverse(root, pdTraverserObj);
	std::cout<<std::endl;
	
	// removing the right tree
	std::cout<<"removing the right tree rooted with node 5"<<std::endl;
	root->removeChild(node5);
	
	std::cout<<"pre-order traverse: ";
	TreeNode::PreOrderTraverse(root, pdTraverserObj);
	std::cout<<std::endl;
	
	std::cout<<"post-order traverse: ";
	TreeNode::PostOrderTraverse(root, pdTraverserObj);
	std::cout<<std::endl;
}

void test_write_to_db() {
	// 1
	SimpleTreeNode * root = new SimpleTreeNode;
	root->setData(1);
	
	// 2
	SimpleTreeNode * node2 = new SimpleTreeNode;
	node2->setData(2);
	root->addChild(node2);
	
	// 3
	SimpleTreeNode * node3 = new SimpleTreeNode;
	node3->setData(3);
	node2->addChild(node3);
	
	// 4
	SimpleTreeNode * node4 = new SimpleTreeNode;
	node4->setData(4);
	node2->addChild(node4);
	
	// 5
	SimpleTreeNode * node5 = new SimpleTreeNode;
	node5->setData(5);
	root->addChild(node5);
	
	// 6
	SimpleTreeNode * node6 = new SimpleTreeNode;
	node6->setData(6);
	node5->addChild(node6);
	
	// 7
	SimpleTreeNode * node7 = new SimpleTreeNode;
	node7->setData(7);
	node5->addChild(node7);

	sqlite3 *database;
	sqlite3_open(TEST_DB, &database);

	// init db
	SimpleTreeNode::initializeDatabase(database);

	TreeNode::saveTree(database, root);
	sqlite3_close(database);
}



void test_read_from_db() {
	sqlite3 *database;
	sqlite3_open(TEST_DB, &database);

	std::vector<sqlite3_int64> rootIDs = TreeNode::loadRoots(database);

	for(int i=0; i<rootIDs.size(); i++)	{
		SimpleTreeNode *root = new SimpleTreeNode(database, rootIDs[0]);

		FaultingTraverser faultingTrav(database);
		TreeNode::PreOrderTraverse(root, faultingTrav);
		std::cout<<"\t\t";
		TreeNode::PreOrderTraverse(root, pdTraverserObj);
		std::cout<<std::endl;
	}

	sqlite3_close(database);

}
