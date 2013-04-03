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

#ifdef TREEMERGE_TEST
#define BOOST_TEST_MODULE treemerge
#include <boost/test/included/unit_test.hpp>
#endif
#include <iostream>
#include <sqlite3/sqlite3.h>

#include "Archivable.h"
#include "SomaticEvent.h"
#include "SegmentalMutation.h"
#include "EventCluster.h"
#include "Subclone.h"
#include "TreeNode.h"

#define BOUNDRY_RESOLUTION 20000000L

using namespace SubcloneExplorer;

void usage(const char *prog_name) {
	std::cout<<"Usage: "<<prog_name<<" <tree-set 1 database file> <tree-set 2 database file>"<<std::endl;
	exit(0);
}

SubclonePtr_vec loadTreeFromTreesetDB(sqlite3 *treesetDB);
bool TreeMerge(Subclone *p, Subclone *q);

class SubclonePrintTraverser : public TreeTraverseDelegate {
	public:
		virtual void processNode(TreeNode *node) {
			Subclone *clone = dynamic_cast<Subclone *>(node);
			std::cerr<<"clone id: "<<clone->getId()<<std::endl;
			for(size_t i=0; i<clone->vecEventCluster().size(); i++) {
				for(size_t j=0; j<clone->vecEventCluster()[i]->members().size(); j++) {
					CNV *cnv = dynamic_cast<CNV *>(clone->vecEventCluster()[i]->members()[j]);
					if(cnv != NULL) {
						std::cerr<<cnv->range.chrom<<":"<<cnv->range.position<<"--"<<cnv->range.position + cnv->range.length<<std::endl;
					}
				}
			}
		}
};

#ifndef TREEMERGE_TEST

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

	SubclonePrintTraverser printTraverser;

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
			TreeMerge(ts1Roots[i], ts2Roots[j]);
		}
	}

	return 0;
}

#endif



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

SomaticEventPtr_vec SomaticEventDifference(const SomaticEventPtr_vec& master, const SomaticEventPtr_vec& unwanted) {
	SomaticEventPtr_vec differenceSet;
	for(size_t i=0; i<master.size(); i++) {
		// if member i is not found in unwanted, append it to result set
		bool found = false;
		for(size_t j=0; j<unwanted.size(); j++) {
			if(master[i]->isEqualTo(unwanted[j], BOUNDRY_RESOLUTION)) {
				found = true;
				break;
			}
		}

		if(!found) {
			differenceSet.push_back(master[i]);
		}
	}
	return differenceSet;
}

bool resultSetComparator(const SomaticEventPtr_vec& v1, const SomaticEventPtr_vec &v2) {
	return v1.size() < v2.size();
}

bool eventSetContains(const SomaticEventPtr_vec& v_container, const SomaticEventPtr_vec& v_containee) {
	bool contains = true;

	for(size_t i=0; i<v_containee.size(); i++) {
		bool elementIsContained = false;
		for(size_t j=0; j<v_container.size(); j++) {
			if(v_container[j]->isEqualTo(v_containee[i], BOUNDRY_RESOLUTION)) {
				elementIsContained = true;
				break;
			}
		}

		if(!elementIsContained) {
			contains = false;
			break;
		}
	}

	return contains;
}

SomaticEventPtr_vec checkPlacement(Subclone *pnode, SomaticEventPtr_vec somaticEvents, bool * placeableOnSubtree) {
	SomaticEventPtr_vec pnodeEvents;
	for(size_t i=0; i<pnode->vecEventCluster().size(); i++) {
		for(size_t j=0; j<pnode->vecEventCluster()[i]->members().size(); j++) {
			pnodeEvents.push_back(pnode->vecEventCluster()[i]->members()[j]);
		}
	}

#ifdef TREEMERGE_TEST_VERBOSE
	if(pnode->vecEventCluster().size() > 0)
		std::cerr<<"trying to place under node "<<pnode->getId()<<" ";
		//std::cerr<<"trying to place under node "<<dynamic_cast<CNV*>(pnode->vecEventCluster()[0]->members()[0])->range.chrom<<" ";
	else
		std::cerr<<"trying to place under root ";
#endif

	// if pnode is not completely contained by somaticEvent, it cannot be placed under pnode
	bool didPassContainment = true;
	if(!eventSetContains(somaticEvents, pnodeEvents)) {
#ifdef TREEMERGE_TEST_VERBOSE
		std::cerr<<"failed (containment)"<<std::endl;
#endif
		didPassContainment = false;
	}

	SomaticEventPtr_vec eventDiff = SomaticEventDifference(somaticEvents, pnodeEvents);

	if(didPassContainment && pnode->isLeaf()) {
		// if passed containment test and this is a leaf, it's placeable
		*placeableOnSubtree = true;
		return(eventDiff);
	}
	else if(pnode->isLeaf()) {
		// or, if this is a leaf but not contained, it's unplacable
		*placeableOnSubtree = false;
		return(eventDiff);
	}

	// Leaf node won't it so far, so at least one children is present

	// check children placement
	int numChildrenPlaceable = 0;
	std::vector<SomaticEventPtr_vec> childEventDiffSet;

	for(size_t i=0; i<pnode->getVecChildren().size(); i++) {
		bool childPlacable = false;
		SomaticEventPtr_vec childEventDiff = checkPlacement(dynamic_cast<Subclone *>(pnode->getVecChildren()[i]), eventDiff, &childPlacable);

		if(childPlacable) {
			numChildrenPlaceable++;
		}

		childEventDiffSet.push_back(childEventDiff);

		//if(childPlacable == false && (not eventSetContains(childEventDiff, eventDiff)))
		//	didPassContainment = false;

	}

	// find the path that would leads to the most symbol consumption
	std::sort(childEventDiffSet.begin(), childEventDiffSet.end(), resultSetComparator);

	bool isCheckedOut = true;
	*placeableOnSubtree = false;

	switch(numChildrenPlaceable) {
		case 0:
			// if there is no child node that can contains eventDiff, the only chance for pnode to be able to contain
			// it is that none of the child node contains any event in eventDiff. This can be checked by accessing whether
			// the returned eventSet of all children after symbol consumption are all the same as eventDiff
			for(size_t i=0; i<childEventDiffSet.size(); i++) {
				if(childEventDiffSet[i].size() != eventDiff.size() || !eventSetContains(eventDiff, childEventDiffSet[i])) {
					isCheckedOut = false;
					break;
				}
			}

			if(isCheckedOut && didPassContainment) {
#ifdef TREEMERGE_TEST_VERBOSE
				std::cerr<<"success! (no-children)"<<std::endl;
#endif
				*placeableOnSubtree = true;
			}
			else {
#ifdef TREEMERGE_TEST_VERBOSE
				std::cerr<<"failed (no-children)"<<std::endl;
#endif
			}
			break;
		case 1:
			// if exactly one child node is found to be able to contain eventDiff, then the node is placeable on this tree.
			// only if the returned symbol list is completely contained by any other children's return list, otherwise some
			// symbols not found in other children's return list must exist on their branch
			// moreover, it must yield the most symbol consumption, because other subtrees will not be able to consume
			// the symbols specifically found in the placeable child node.
			for(size_t i=1; i<childEventDiffSet.size(); i++) {
				if(!eventSetContains(childEventDiffSet[i], childEventDiffSet[0])) {
					isCheckedOut = false;
					break;
				}
			}

			if(isCheckedOut && didPassContainment) {
#ifdef TREEMERGE_TEST_VERBOSE
				std::cerr<<"success! (one-child)"<<std::endl;
#endif
				*placeableOnSubtree = true;
			}
			break;
		default:
			// if more than one children nodes are found to be able to contain eventDiff, none of them actually can without
			// violating the tree structure. In this case, the status returned is false, and the minimal set is returned
			// so that the node cannot be placed as a new child either
#ifdef TREEMERGE_TEST_VERBOSE
			std::cerr<<"failed (>2 children)"<<std::endl;
#endif
			break;
	}

	return(childEventDiffSet[0]);
}

bool TreeMerge(Subclone *p, Subclone *q) {
	class TreeMergeTraverseSecondary : public TreeTraverseDelegate {
		protected:
			Subclone *_proot;

		public:
			bool isCompatible;

			TreeMergeTraverseSecondary(Subclone *proot): TreeTraverseDelegate(), _proot(proot), isCompatible(true) {;}

			void processNode(TreeNode *node) {
				bool placeable;
				SomaticEventPtr_vec subcloneEvents;

				Subclone *wp = dynamic_cast<Subclone *>(node);

				// ignores zero frequency subclones as their order cannot be reliably determined
				if(wp->fraction()< 1e-2)
					return;


				while(wp != NULL) {
					for(size_t i=0; i<wp->vecEventCluster().size(); i++) {
						for(size_t j=0; j<wp->vecEventCluster()[i]->members().size(); j++)
							subcloneEvents.push_back(wp->vecEventCluster()[i]->members()[j]);
					}
					wp = dynamic_cast<Subclone *>(wp->getParent());
				}

#ifdef TREEMERGE_TEST_VERBOSE
				std::cerr<<"Placing relapse subclone "<<dynamic_cast<Subclone *>(node)->getId()<<std::endl;
				std::cerr<<"its stacked symbol list is: ";
				for(size_t i=0; i<subcloneEvents.size(); i++)
					std::cerr<<(dynamic_cast<CNV *>(subcloneEvents[i])->range.chrom)<<",";
				std::cerr<<std::endl;
#endif

				SomaticEventPtr_vec diff = checkPlacement(_proot, subcloneEvents, &placeable);
				if(!placeable) {
					isCompatible = false;
					terminate();
				}
			}
	};

	TreeMergeTraverseSecondary secondaryTraverser(p);
	TreeNode::PreOrderTraverse(q, secondaryTraverser);
	if(secondaryTraverser.isCompatible) {
		std::cout<<"Primary SubcTree "<<p->getId()<<" is compatible with Secondary SubcTree "<<q->getId()<<std::endl;
		return true;
	}

	return false;
}

#ifdef TREEMERGE_TEST
// 0 == 0
BOOST_AUTO_TEST_CASE( test_tree_merge_0_0 ) {
	Subclone *clone1 = new Subclone();
	Subclone *clone2 = new Subclone();

	BOOST_CHECK(TreeMerge(clone1, clone2));
	delete clone1;
	delete clone2;
}

// 0 == A
BOOST_AUTO_TEST_CASE( test_tree_merge_0_a ) {
	Subclone *clone1 = new Subclone();
	Subclone *clone2 = new Subclone();

	CNV cnv;
	cnv.range.chrom = 1;
	cnv.range.position = 1000000L;
	cnv.range.length = 1000;
	cnv.frequency = 0.4;

	EventCluster cluster;
	cluster.addEvent(&cnv);

	clone2->addEventCluster(&cluster);

	BOOST_CHECK(TreeMerge(clone1, clone2));
	delete clone1;
	delete clone2;
}


// A == A
BOOST_AUTO_TEST_CASE( test_tree_merge_a_a ) {

	CNV *cnv = new CNV();
	cnv->range.chrom = 1;
	cnv->range.position = 100000L;
	cnv->range.length = 100;
	cnv->frequency = 0.4;

	EventCluster *cluster1 = new EventCluster();
	EventCluster *cluster2 = new EventCluster();

	BOOST_CHECK(cluster1->members().size() == 0);
	BOOST_CHECK(cluster2->members().size() == 0);

	cluster1->addEvent(cnv);
	cluster2->addEvent(cnv);

	BOOST_CHECK(cluster1->members().size() == 1);
	BOOST_CHECK(cluster2->members().size() == 1);

	Subclone *clone1 = new Subclone();
	Subclone *clone2 = new Subclone();
	Subclone *clone3 = new Subclone();
	Subclone *clone4 = new Subclone();

	BOOST_CHECK(clone1->vecEventCluster().size() == 0);
	BOOST_CHECK(clone2->vecEventCluster().size() == 0);
	BOOST_CHECK(clone3->vecEventCluster().size() == 0);
	BOOST_CHECK(clone4->vecEventCluster().size() == 0);

	clone3->addEventCluster(cluster1);
	clone4->addEventCluster(cluster2);

	BOOST_CHECK(clone3->vecEventCluster().size() == 1);
	BOOST_CHECK(clone4->vecEventCluster().size() == 1);

	clone1->addChild(clone3);
	clone2->addChild(clone4);

	BOOST_CHECK(TreeMerge(clone1, clone2));

	delete clone1;
	delete clone2;
}

// A ~= A'
BOOST_AUTO_TEST_CASE( test_tree_merge_a_a1 ) {

	CNV *cnv1 = new CNV();
	CNV *cnv2 = new CNV();
	cnv1->range.chrom = 1;
	cnv1->range.position = 100000L;
	cnv1->range.length = 1000;
	cnv1->frequency = 0.4;

	cnv2->range.chrom = 1;
	cnv2->range.position = 100123L;
	cnv2->range.length = 986;
	cnv2->frequency = 0.4;

	BOOST_CHECK(cnv1->isEqualTo(cnv2));

	EventCluster *cluster1 = new EventCluster();
	EventCluster *cluster2 = new EventCluster();

	cluster1->addEvent(cnv1);
	cluster2->addEvent(cnv2);

	Subclone *clone1 = new Subclone();
	Subclone *clone2 = new Subclone();
	Subclone *clone3 = new Subclone();
	Subclone *clone4 = new Subclone();

	clone3->addEventCluster(cluster1);
	clone4->addEventCluster(cluster2);

	clone1->addChild(clone3);
	clone2->addChild(clone4);

	BOOST_CHECK(TreeMerge(clone1, clone2));

	delete clone1;
	delete clone2;
	delete clone3;
	delete clone4;
}

// 0, (A, (B, (C) ) ) != 0, (A, (B, C) )
BOOST_AUTO_TEST_CASE( test_tree_merge_3_1 ) {
	CNV A, B, C;
	A.range.chrom = 1;
	B.range.chrom = 2;
	C.range.chrom = 3;

	BOOST_CHECK(not A.isEqualTo(&B));
	BOOST_CHECK(not A.isEqualTo(&C));
	BOOST_CHECK(not B.isEqualTo(&C));

	EventCluster cA, cB, cC;
	cA.addEvent(&A);
	cB.addEvent(&B);
	cC.addEvent(&C);

	Subclone *p0, *pA, *pB, *pC;
	Subclone *r0, *rA, *rB, *rC;

	p0 = new Subclone(); pA = new Subclone(); pB = new Subclone(); pC = new Subclone();
	r0 = new Subclone(); rA = new Subclone(); rB = new Subclone(); rC = new Subclone();

	p0->setId(10); pA->setId(11); pB->setId(12); pC->setId(13);
	r0->setId(20); rA->setId(21); rB->setId(22); rC->setId(23);



	pA->addEventCluster(&cA); rA->addEventCluster(&cA);
	pB->addEventCluster(&cB); rB->addEventCluster(&cB);
	pC->addEventCluster(&cC); rC->addEventCluster(&cC);

	p0->addChild(pA); pA->addChild(pB); pB->addChild(pC);
	r0->addChild(rA); rA->addChild(rB); rA->addChild(rC);

	BOOST_CHECK(not TreeMerge(p0, r0));

}

// 0, (A, (B, (C))) != 0, (ACD)
BOOST_AUTO_TEST_CASE( test_tree_merge_3_4 ) {
	CNV A, B, C, D;
	A.range.chrom = 1;
	B.range.chrom = 2;
	C.range.chrom = 3;
	D.range.chrom = 4;

	EventCluster cA, cB, cC, cD;
	cA.addEvent(&A);
	cB.addEvent(&B);
	cC.addEvent(&C);
	cD.addEvent(&D);

	Subclone *p0, *pA, *pB, *pC;
	Subclone *r0, *rACD;

	p0 = new Subclone(); pA = new Subclone(); pB = new Subclone(); pC = new Subclone();
	r0 = new Subclone(); rACD = new Subclone();

	p0->setId(10); pA->setId(11); pB->setId(12); pC->setId(13);
	r0->setId(20); rACD->setId(2134);

	pA->addEventCluster(&cA); 
	pB->addEventCluster(&cB); 
	pC->addEventCluster(&cC); 

	rACD->addEventCluster(&cA);
	rACD->addEventCluster(&cC);
	rACD->addEventCluster(&cD);

	p0->addChild(pA); pA->addChild(pB); pB->addChild(pC);
	r0->addChild(rACD);

	BOOST_CHECK(not TreeMerge(p0, r0));
}

// 0, (A, (B, (C) ) ) == 0, (A, (B, D) )
BOOST_AUTO_TEST_CASE( test_tree_merge_3_2 ) {
	CNV A, B, C, D;
	A.range.chrom = 1;
	B.range.chrom = 2;
	C.range.chrom = 3;
	D.range.chrom = 4;

	EventCluster cA, cB, cC, cD;
	cA.addEvent(&A);
	cB.addEvent(&B);
	cC.addEvent(&C);
	cD.addEvent(&D);

	Subclone *p0, *pA, *pB, *pC;
	Subclone *r0, *rA, *rB, *rD;

	p0 = new Subclone(); pA = new Subclone(); pB = new Subclone(); pC = new Subclone();
	r0 = new Subclone(); rA = new Subclone(); rB = new Subclone(); rD = new Subclone();

	p0->setId(10); pA->setId(11); pB->setId(12); pC->setId(13);
	r0->setId(20); rA->setId(21); rB->setId(22); rD->setId(24);



	pA->addEventCluster(&cA); rA->addEventCluster(&cA);
	pB->addEventCluster(&cB); rB->addEventCluster(&cB);
	pC->addEventCluster(&cC); rD->addEventCluster(&cD);

	p0->addChild(pA); pA->addChild(pB); pB->addChild(pC);
	r0->addChild(rA); rA->addChild(rB); rA->addChild(rD);

	BOOST_CHECK(TreeMerge(p0, r0));
}

// 0, (A, (B, (C) ) ) == 0, (AD, E) )
BOOST_AUTO_TEST_CASE( test_tree_merge_3_3 ) {
	CNV A, B, C, D, E;
	A.range.chrom = 1;
	B.range.chrom = 2;
	C.range.chrom = 3;
	D.range.chrom = 4;
	E.range.chrom = 5;

	EventCluster cA, cB, cC, cAD, cE;
	cA.addEvent(&A);
	cB.addEvent(&B);
	cC.addEvent(&C);
	cAD.addEvent(&A);
	cAD.addEvent(&D);
	cE.addEvent(&E);

	Subclone *p0, *pA, *pB, *pC;
	Subclone *r0, *rAD, *rE;

	p0 = new Subclone(); pA = new Subclone(); pB = new Subclone(); pC = new Subclone();
	r0 = new Subclone(); rAD = new Subclone(); rE = new Subclone();

	p0->setId(10); pA->setId(11); pB->setId(12); pC->setId(13);
	r0->setId(20); rAD->setId(214); rE->setId(25);



	pA->addEventCluster(&cA); rAD->addEventCluster(&cAD);
	pB->addEventCluster(&cB); rE->addEventCluster(&cE);
	pC->addEventCluster(&cC);

	p0->addChild(pA); pA->addChild(pB); pB->addChild(pC);
	r0->addChild(rAD); rAD->addChild(rE);

	BOOST_CHECK(TreeMerge(p0, r0));
}

// aml1
BOOST_AUTO_TEST_CASE( test_tree_merge_aml1_1 ) {
	CNV a, b, c, d, e;
	a.range.chrom = 1; b.range.chrom = 2; c.range.chrom = 3; d.range.chrom = 4; e.range.chrom = 5;

	EventCluster cA, cB, cC, cD, cE;
	cA.addEvent(&a); cB.addEvent(&b); cC.addEvent(&c); cD.addEvent(&d), cE.addEvent(&e);

	Subclone *p0, *p1, *p2, *p3, *p4;
	Subclone *r0, *r1, *r3, *r4, *r5;

	p0 = new Subclone(); r0 = new Subclone();
	p1 = new Subclone(); p1->addEventCluster(&cA); r1 = new Subclone(); r1->addEventCluster(&cA);
	p2 = new Subclone(); p2->addEventCluster(&cB);
	p3 = new Subclone(); p3->addEventCluster(&cC); r3 = new Subclone(); r3->addEventCluster(&cC);
	p4 = new Subclone(); p4->addEventCluster(&cD); r4 = new Subclone(); r4->addEventCluster(&cD);
	r5 = new Subclone(); r5->addEventCluster(&cE);

	p0->setId(10); r0->setId(20);
	p1->setId(11); r1->setId(21);
	p2->setId(12);
	p3->setId(13); r3->setId(23);
	p4->setId(14); r4->setId(24);
	r5->setId(25);

	p0->addChild(p1); p1->addChild(p2); p1->addChild(p3); p2->addChild(p4);
	r0->addChild(r1); r1->addChild(r3); r3->addChild(r4); r4->addChild(r5);

	BOOST_CHECK(not TreeMerge(p0, r0));
}

#endif

