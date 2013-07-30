/**
 * @file Unit tests for Subclone
 *
 * @see Subclone
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
#include <cstdio>

#include "Subclone.h"
#include "EventCluster.h"

#include "common.h"

SUITE(TestSubclone) {
	TEST(ObjectCreation) {
		SubcloneExplorer::Subclone subclone;

		// default vault check
		CHECK_CLOSE(subclone.fraction(), 0, 1e-3);
		CHECK_CLOSE(subclone.treeFraction(), 0, 1e-3);
		CHECK(subclone.vecEventCluster().size() == 0);

		SubcloneExplorer::EventCluster cluster;

		subclone.addEventCluster(&cluster);

		CHECK(subclone.vecEventCluster().size() == 1);
		CHECK(subclone.vecEventCluster()[0] == &cluster);
	}

	TEST_FIXTURE(DBFixture, SubcloneToDB) {
		SubcloneExplorer::Subclone root, child1, child2, child11;

		root.setFraction(0.7);
		child1.setFraction(0.5);
		child2.setFraction(0.2);
		child11.setFraction(0.1);

		root.addChild(&child1);
		root.addChild(&child2);
		child1.addChild(&child11);

		// write
		SubcloneExplorer::SubcloneSaveTreeTraverser stt(database);
		SubcloneExplorer::TreeNode::PreOrderTraverse(&root, stt);

		// read
		SubcloneExplorer::EventCluster cluster2;

		std::vector<sqlite3_int64> rootNodes = SubcloneExplorer::SubcloneLoadTreeTraverser::rootNodes(database);
		
		CHECK(rootNodes.size() == 1);

		SubcloneExplorer::Subclone *newRoot = new SubcloneExplorer::Subclone();
		newRoot->unarchiveObjectFromDB(database, rootNodes[0]);

		SubcloneExplorer::SubcloneLoadTreeTraverser ltt(database);
		SubcloneExplorer::TreeNode::PreOrderTraverse(newRoot, ltt);


		CHECK_CLOSE(newRoot->fraction(), 0.7, 1e-3);
		CHECK(newRoot->getVecChildren().size() == 2);
		CHECK(newRoot->isRoot());
		CHECK(!newRoot->isLeaf());

		SubcloneExplorer::Subclone *newChild1 = dynamic_cast<SubcloneExplorer::Subclone *>( newRoot->getVecChildren()[0] );
		SubcloneExplorer::Subclone *newChild2 = dynamic_cast<SubcloneExplorer::Subclone *>( newRoot->getVecChildren()[1] );

		CHECK(newChild1->fraction() > 0.1 && newChild1->fraction() < 0.6);
		if(newChild1->fraction() < 0.3) {
			SubcloneExplorer::Subclone *tmp;
			tmp = newChild1;
			newChild1 = newChild2;
			newChild2 = tmp;
		}

		// now newChild1 is child1, newChild2 is child2
		CHECK_CLOSE(newChild1->fraction(), 0.5, 1e-3);
		CHECK_CLOSE(newChild2->fraction(), 0.2, 1e-3);
		CHECK(newChild1->getVecChildren().size() == 1);
		CHECK(newChild2->isLeaf());

		SubcloneExplorer::Subclone *newChild11 = dynamic_cast<SubcloneExplorer::Subclone *>(newChild1->getVecChildren()[0]);

		CHECK_CLOSE(newChild11->fraction(), 0.1, 1e-3);
		CHECK(newChild11->isLeaf());
	}
}

TEST_MAIN
