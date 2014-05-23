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

using namespace SubcloneSeeker;

void usage(const char *prog_name) {
	std::cout<<"Usage: "<<prog_name<<" <tree-set 1 database file> <tree-set 2 database file>"<<std::endl;
	exit(0);
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
	DBObjectID_vec ts1RootIDs = SubcloneLoadTreeTraverser::rootNodes(ts1_db);
	std::cerr<<ts1RootIDs.size()<<" primary trees found!"<<std::endl;

	// ******** OPEN TREE-SET 2 DATABASE ********
	if(sqlite3_open_v2(argv[2], &ts2_db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
		std::cerr<<"Unable to open tree-set 2 database file "<<argv[2]<<std::endl;
		return(1);
	}
	DBObjectID_vec ts2RootIDs = SubcloneLoadTreeTraverser::rootNodes(ts2_db);
	std::cerr<<ts2RootIDs.size()<<" secondary trees found!"<<std::endl;

	SubcloneLoadTreeTraverser pLoadTraverser(ts1_db);
	SubcloneLoadTreeTraverser sLoadTraverser(ts2_db);

	for(size_t i=0; i<ts1RootIDs.size(); i++) {
		for(size_t j=0; j<ts2RootIDs.size(); j++) {
			Subclone *pRoot = new Subclone();
			pRoot->unarchiveObjectFromDB(ts1_db, ts1RootIDs[i]);
			TreeNode::PreOrderTraverse(pRoot, pLoadTraverser);

			Subclone *sRoot = new Subclone();
			sRoot->unarchiveObjectFromDB(ts2_db, ts2RootIDs[j]);
			TreeNode::PreOrderTraverse(sRoot, sLoadTraverser);
		
			if(TreeMerge(pRoot, sRoot)) {
				std::cout<<"Primary tree "<<pRoot->getId()<<" is compatible with Secondary tree "<<sRoot->getId()<<std::endl;
			}
		}
	}

	return 0;
}
