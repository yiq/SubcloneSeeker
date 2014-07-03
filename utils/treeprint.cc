/**
 * @file treeprint.cc
 * The source for util 'treeprint', which takes a database and a root id and
 * prints out the subclonal tree
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

#include "Archivable.h"
#include "Subclone.h"
#include "EventCluster.h"
#include <sqlite3/sqlite3.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <unistd.h>

using namespace SubcloneSeeker;

enum {RUN_MODE_LIST, RUN_MODE_PRINT} runMode;
enum {OUT_FORMAT_TEXT, OUT_FORMAT_GVIZ} outputMode;
int isRootIDSpecified;
int32_t rootID;

// Traverser borrowed from SubcloneExplore.cc
/**
 * @brief A tree traverser that prints tree in a text format
 *
 * if B and C are children of A, the final output will look like A (B, C)
 */
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

/**
 * @brief A tree traverser that prints the nodes in Graphviz .dot format
 */
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

/**
 * @brief A tree traverser that prints the edges in Graphviz .dot format
 */
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

/**
 * @brief Print the usage information
 *
 * @param progName the string containing the name of the executable
 */
void usage(const char* progName) {
	std::cout<<"Usage: "<<progName<<" [Options] <sqlite-db-file>"<<std::endl;
	std::cout<<"Options:"<<std::endl;
	std::cout<<"\t-l\t\t\tList all root subclone IDs"<<std::endl;
	std::cout<<"\t-r <subclone-id>\tOnly output the subclone structure rooted with the given id"<<std::endl;
	std::cout<<"\t-g\t\t\tOutput in graphviz format"<<std::endl;
	std::cout<<"\t-h\t\t\tPrint this message"<<std::endl;
	exit(0);
}

/**
 * @brief List all root subclone IDs
 *
 * @param database An live sqlite3 database connection
 */
void listRootIDs(sqlite3* database) {

	DBObjectID_vec rootIDs = SubcloneLoadTreeTraverser::rootNodes(database);
	for(DBObjectID_vec::iterator it = rootIDs.begin(); it != rootIDs.end(); it++) {
		std::cout<<*it<<std::endl;
	}
}

/**
 * @brief Print details about a subclone structure
 * The structure contains the given root, and all its descendent nodes
 *
 * @param database An live sqlite3 database connection
 * @param rootID The id of the root node for which the structure is printed
 */
void printSubcloneWithID(sqlite3* database, int32_t rootID) {
	Subclone *root = new Subclone();

	root->unarchiveObjectFromDB(database, rootID);

	SubcloneLoadTreeTraverser loadTr(database);
	TreeNode::PreOrderTraverse(root, loadTr);

	if(outputMode == OUT_FORMAT_TEXT) {
		TreePrintTraverser traverser;
		TreeNode::PreOrderTraverse(root, traverser);
	} 
	else if(outputMode == OUT_FORMAT_GVIZ) {
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
}

/**
 * @brief Print all subclone structures
 *
 * @param database An live sqlite3 database connection
 */
void printAllSubclones(sqlite3* database) {

	DBObjectID_vec rootIDs = SubcloneLoadTreeTraverser::rootNodes(database);
	for(DBObjectID_vec::iterator it = rootIDs.begin(); it != rootIDs.end(); it++) {
		printSubcloneWithID(database, *it);
	}
}

/**
 * Main function of the treeprint utility
 */

int main(int argc, char* argv[]) {

	runMode = RUN_MODE_PRINT;
	outputMode = OUT_FORMAT_TEXT;

	isRootIDSpecified = 0;

	int c;
	while((c = getopt(argc, argv, "lgr:h")) != -1) {
		switch(c)
		{
			case 'l':
				runMode = RUN_MODE_LIST;
				break;
			case 'g':
				outputMode = OUT_FORMAT_GVIZ;
				break;
			case 'r':
				isRootIDSpecified = 1;
				rootID = atoi(optarg);
				break;
			case 'h':
				usage(argv[0]);
				break;
			default:
				std::cerr<<"Unknown option "<<(char)c<<std::endl;
				usage(argv[0]);
				break;
		}
	}

	if(optind == argc) {
		std::cerr<<"Missing subclone database file"<<std::endl;
		usage(argv[0]);
	}

	sqlite3 *database;
	int rc;

	if(sqlite3_open_v2(argv[optind], &database, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
		std::cerr<<"Unable to open database "<<argv[optind]<<std::endl;
		return(1);
	}

	switch(runMode)
	{
		case RUN_MODE_LIST:
			listRootIDs(database);
			break;
		case RUN_MODE_PRINT:
			if(isRootIDSpecified) {
				printSubcloneWithID(database, rootID);
			}
			else {
				printAllSubclones(database);
			}
	}

	sqlite3_close(database);
	
	return 0;
}
