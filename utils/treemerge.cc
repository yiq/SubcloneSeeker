/*
 * @file treemerge.cc
 * The source file for the utility 'treemerge', which compares
 * two tree sets with implied relationship that the treeset-1
 * gave raise to treeset-2 with extra mutations, and figures
 * out which (tree in set1, tree in set2) pairs are logically
 * correct
 *
 * @author Yi Qiao
 */

#include <iostream>
#include <sqlite3/sqlite3.h>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include "Archivable.h"
#include "SomaticEvent.h"
#include "SegmentalMutation.h"
#include "EventCluster.h"
#include "Subclone.h"
#include "TreeNode.h"
#include "treemerge_p.h"

using namespace SubcloneExplorer;

void usage(const char *prog_name) {
	std::cout<<"Usage: "<<prog_name<<" <tree-set 1 database file> <tree-set 2 database file>"<<std::endl;
	exit(0);
}

SubclonePtr_vec loadTreeFromTreesetDB(sqlite3 *treesetDB) {

	Subclone dummySubclone;
	
	DBObjectID_vec rootIDs = SubcloneLoadTreeTraverser::rootNodes(treesetDB);

	if(rootIDs.size() == 0) {
		std::cerr<<"No tree is found in tree-set 1 database"<<std::endl;
		exit(2);
	}

	SubclonePtr_vec trees;
	for(size_t i=0; i<rootIDs.size(); i++) {
		Subclone *newRoot = new Subclone();
		newRoot->unarchiveObjectFromDB(treesetDB, rootIDs[i]);
		SubcloneLoadTreeTraverser loadTraverser(treesetDB);
		TreeNode::PreOrderTraverse(newRoot, loadTraverser);
		trees.push_back(newRoot);
	}
	return(trees);
}

int main(int argc, char* argv[]) {
	sqlite3 *ts1_db, *ts2_db;
	int rc;

	if(argc < 3) {
		usage(argv[0]);
	}

	// ******** OPEN TREE-SET 1 DATABASE ********
	if(sqlite3_open_v2(argv[1], &ts1_db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
		std::cerr<<"Unable to open tree-set 1 database file "<<argv[1]<<std::endl;
		return(1);
	}
	SubclonePtr_vec ts1Roots = loadTreeFromTreesetDB(ts1_db);
	sqlite3_close(ts1_db);

	std::cerr<<ts1Roots.size()<<" trees load from primary"<<std::endl;

	// ******** OPEN TREE-SET 2 DATABASE ********
	if(sqlite3_open_v2(argv[2], &ts2_db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
		std::cerr<<"Unable to open tree-set 2 database file "<<argv[2]<<std::endl;
		return(1);
	}
	SubclonePtr_vec ts2Roots = loadTreeFromTreesetDB(ts2_db);
	sqlite3_close(ts2_db);

	std::cerr<<ts2Roots.size()<<" trees load from secondary"<<std::endl;

	for(size_t i=0; i<ts1Roots.size(); i++) {
		for(size_t j=0; j<ts2Roots.size(); j++) {

			// Duplicate primary tree for each comparison
			sqlite3 *mem_db;
			sqlite3_open(":memory:", &mem_db);
			SubcloneSaveTreeTraverser stt(mem_db);
			TreeNode::PreOrderTraverse(ts1Roots[i], stt);
			Subclone *newPRoot = new Subclone();
			newPRoot->unarchiveObjectFromDB(mem_db, ts1Roots[i]->getId());
			SubcloneLoadTreeTraverser loadTraverser(mem_db);
			TreeNode::PreOrderTraverse(newPRoot, loadTraverser);

			if(TreeMerge(newPRoot, ts2Roots[j])) {
				std::cout<<"Primary tree "<<ts1Roots[i]->getId()<<" is compatible with Secondary tree "<<ts2Roots[j]->getId()<<std::endl;
			}
		}
	}

	return 0;
}
