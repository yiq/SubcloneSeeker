/**
 * @file treeprint.cc
 * The source for util 'treeprint', which takes a database and a root id and
 * prints out the subclonal tree
 *
 * @author Yi Qiao
 */

#include "Archivable.h"
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

class NodePrintTraverser: public TreeTraverseDelegate {
	public:
		virtual void processNode(TreeNode * node) {
			Subclone *clone = dynamic_cast<Subclone *>(node);
			if(clone == NULL)
				return;
			double fracP = clone->fraction() * 100;
			std::cout<<"\tn"<<clone->getId()<<" [label=\"n"<<clone->getId()<<": ";
			std::cout.precision(3);
			std::cout<<fracP<<"%\"];"<<std::endl;
		}
};

class EdgePrintTraverser: public TreeTraverseDelegate {
	public:
		virtual void processNode(TreeNode * node) {
			Subclone *clone = dynamic_cast<Subclone *>(node);
			if(clone == NULL)
				return;
			if(clone->getParent() == NULL)
				return;
			Subclone *pClone = dynamic_cast<Subclone *>(node->getParent());
			if(pClone == NULL)
				return;

			std::cout<<"\tn"<<pClone->getId()<<"->n"<<clone->getId()<<";"<<std::endl;
		}
};

int main(int argc, char* argv[]) {

	if(argc<3) {
		std::cerr<<"Usage "<<argv[0]<<" <sqlite-db> <root-id> [-g]"<<std::endl;
		std::cerr<<"The optional -g switch will produce dot file suitable to be visualized with GraphViz"<<std::endl;
		exit(0);
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

	if(argc<4) {
		TreePrintTraverser traverser;
		TreeNode::PreOrderTraverse(root, traverser);
	} else if(strcmp(argv[3], "-g") == 0) {
		std::cout<<"digraph {"<<std::endl;
		// Node list
		NodePrintTraverser npTrav;
		TreeNode::PreOrderTraverse(root, npTrav);
		
		// Edge list
		EdgePrintTraverser epTrav;
		TreeNode::PreOrderTraverse(root, epTrav);
		std::cout<<"}"<<std::endl;
	}

	std::cout<<std::endl;

	return 0;
}
