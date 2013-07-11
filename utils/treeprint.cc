/**
 * @file treeprint.cc
 * The source for util 'treeprint', which takes a database and a root id and
 * prints out the subclonal tree
 *
 * @author Yi Qiao
 */

#include "Subclone.h"
#include "EventCluster.h"
#include <sqlite3/sqlite3.h>
#include <iostream>

using namespace SubcloneExplorer;

// Traverser borrowed from SubcloneExplore.cc
class TreePrintTraverser: public TreeTraverseDelegate {
	public:
		virtual void preprocessNode(TreeNode *node) {
			if(!node->isLeaf())
				std::cerr<<"(";
		}

		virtual void processNode(TreeNode * node) {
			std::cerr<<((Subclone *)node)->fraction()<<",";
		}

		virtual void postprocessNode(TreeNode *node) {
			if(!node->isLeaf())
				std::cerr<<")";
		}
};

int main(int argc, char* argv[]) {

	if(argc<3) {
		std::cerr<<"Usage "<<argv[0]<<" <sqlite-db> <root-id>"<<std::endl;
	}

	sqlite3 *database;
	int rc;

	if(sqlite3_open_v2(argv[1], &database, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
		std::cerr<<"Unable to open database "<<argv[1]<<std::endl;
		return(1);
	}

	Subclone *root = new Subclone();

	root->unarchiveObjectFromDB(database, atoi(argv[2]));

	SubcloneLoadTreeTraverser loadTr(database);
	TreeNode::PreOrderTraverse(root, loadTr);

	TreePrintTraverser traverser;
	TreeNode::PreOrderTraverse(root, traverser);

	std::cout<<std::endl;

	return 0;
}
