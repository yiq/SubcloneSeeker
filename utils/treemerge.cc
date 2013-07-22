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

			TreeMerge(newPRoot, ts2Roots[j]);
		}
	}

	return 0;
}

#ifdef TREEMERGE_TEST
// 0 == 0
BOOST_AUTO_TEST_CASE( test_tree_merge_0_0 ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_0_0"<<std::endl;

	Subclone *clone1 = new Subclone();
	Subclone *clone2 = new Subclone();

	clone1->setFraction(1);
	clone2->setFraction(1);

	BOOST_CHECK(TreeMerge(clone1, clone2));
	delete clone1;
	delete clone2;
}

// 0 == A
BOOST_AUTO_TEST_CASE( test_tree_merge_0_a ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_0_a"<<std::endl;

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

	clone1->setFraction(1);
	clone2->setFraction(1);

	BOOST_CHECK(TreeMerge(clone1, clone2));
	delete clone1;
	delete clone2;
}


// A == A
BOOST_AUTO_TEST_CASE( test_tree_merge_a_a ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_a_a"<<std::endl;

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

	clone1->setFraction(0.6); clone2->setFraction(0.6);
	clone3->setFraction(0.4); clone4->setFraction(0.4);

	BOOST_CHECK(TreeMerge(clone1, clone2));

	delete clone1;
	delete clone2;
}

// A ~= A'
BOOST_AUTO_TEST_CASE( test_tree_merge_a_a1 ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_a_a1"<<std::endl;
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

	clone1->setFraction(0.6); clone2->setFraction(0.6);
	clone3->setFraction(0.4); clone4->setFraction(0.4);


	BOOST_CHECK(TreeMerge(clone1, clone2));

	delete clone1;
	delete clone2;
	delete clone3;
	delete clone4;
}

// 0, (A, (B, (C) ) ) != 0, (A, (B, C) )
BOOST_AUTO_TEST_CASE( test_tree_merge_3_1 ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_3_1"<<std::endl;
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

	p0->setFraction(0.1); pA->setFraction(0.1); pB->setFraction(0.1); pC->setFraction(0.1);
	r0->setFraction(0.1); rA->setFraction(0.1); rB->setFraction(0.1); rC->setFraction(0.1);

	BOOST_CHECK(not TreeMerge(p0, r0));

}

// 0, (A, (B, (C))) != 0, (ACD)
BOOST_AUTO_TEST_CASE( test_tree_merge_3_4 ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_3_4"<<std::endl;
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

	p0->setFraction(0.1); pA->setFraction(0.1); pB->setFraction(0.1); pC->setFraction(0.1);
	r0->setFraction(0.1); rACD->setFraction(0.1);

	BOOST_CHECK(not TreeMerge(p0, r0));
}

// 0, (A, (B, (C) ) ) == 0, (A, (B, D) )
BOOST_AUTO_TEST_CASE( test_tree_merge_3_2 ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_3_2"<<std::endl;
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

	p0->setFraction(0.1); pA->setFraction(0.1); pB->setFraction(0.1); pC->setFraction(0.1);
	r0->setFraction(0.1); rA->setFraction(0.1); rB->setFraction(0.1); rD->setFraction(0.1);

	BOOST_CHECK(TreeMerge(p0, r0));
}

// 0, (A, (B, (C) ) ) == 0, (AD, E) )
BOOST_AUTO_TEST_CASE( test_tree_merge_3_3 ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_3_3"<<std::endl;

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

	p0->setFraction(0.1); pA->setFraction(0.1); pB->setFraction(0.1); pC->setFraction(0.1);
	r0->setFraction(0.1); rAD->setFraction(0.1); rE->setFraction(0.1);

	BOOST_CHECK(TreeMerge(p0, r0));
}

// aml1
BOOST_AUTO_TEST_CASE( test_tree_merge_aml1_1 ) {
	std::cerr<<std::endl<<"TEST CASE: test_tree_merge_aml1"<<std::endl;

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

	p0->setFraction(0.1); r0->setFraction(0.1);
	p1->setFraction(0.1); r1->setFraction(0.1);
	p2->setFraction(0.1);
	p3->setFraction(0.1); r3->setFraction(0.1);
	p4->setFraction(0.1); r4->setFraction(0.1);
	r5->setFraction(0.1);


	BOOST_CHECK(not TreeMerge(p0, r0));
}

// UPN426980-rel-1
BOOST_AUTO_TEST_CASE( test_tree_merge_upn426 ) {
	std::cerr<<std::endl<<"STARTING TEST CASE upn426"<<std::endl;
	CNV a, b, c, d, e;
	a.range.chrom = 1; b.range.chrom = 2; c.range.chrom = 3; d.range.chrom = 4; e.range.chrom = 5;

	EventCluster cA, cB, cC, cD, cE;
	cA.addEvent(&a); cB.addEvent(&b); cC.addEvent(&c); cD.addEvent(&d), cE.addEvent(&e);

	Subclone *p0, *p1, *p2;
	Subclone *r0, *r1, *r2, *r3;

	p0 = new Subclone(); r0 = new Subclone();
	p1 = new Subclone(); p1->addEventCluster(&cA); p1->addEventCluster(&cB); p1->addEventCluster(&cC);
	p2 = new Subclone(); p2->addEventCluster(&cD);

	r1 = new Subclone(); r1->addEventCluster(&cA);
	r2 = new Subclone(); r2->addEventCluster(&cB);
	r3 = new Subclone(); r3->addEventCluster(&cE);

	p0->setId(10); r0->setId(20);
	p1->setId(1123); r1->setId(21);
	p2->setId(14); r2->setId(22);
	r3->setId(25);

	p0->addChild(p1); p1->addChild(p2);
	r0->addChild(r1); r1->addChild(r2); r1->addChild(r3);

	p0->setFraction(0); p1->setFraction(0.6); p2->setFraction(0.4);
	r0->setFraction(0); r1->setFraction(0); r2->setFraction(0.3); r3->setFraction(0.7);

	BOOST_CHECK(TreeMerge(p0, r0));

}

// UPN426980-rel-2
BOOST_AUTO_TEST_CASE( test_tree_merge_upn426_2 ) {
	std::cerr.flush(); std::cout.flush();
	std::cerr<<std::endl<<"STARTING TEST CASE upn427_2 <<<<<< "<<std::endl;
	CNV a, b, c, d, e;
	a.range.chrom = 1; b.range.chrom = 2; c.range.chrom = 3; d.range.chrom = 4; e.range.chrom = 5;

	EventCluster cA, cB, cC, cD, cE;
	cA.addEvent(&a); cB.addEvent(&b); cC.addEvent(&c); cD.addEvent(&d), cE.addEvent(&e);

	Subclone *p0, *p1, *p2;
	Subclone *r0, *r1, *r2, *r3;

	p0 = new Subclone(); r0 = new Subclone();
	p1 = new Subclone(); p1->addEventCluster(&cA); p1->addEventCluster(&cB); p1->addEventCluster(&cC);
	p2 = new Subclone(); p2->addEventCluster(&cD);

	r1 = new Subclone(); r1->addEventCluster(&cA);
	r2 = new Subclone(); r2->addEventCluster(&cE);
	r3 = new Subclone(); r3->addEventCluster(&cB);

	p0->setId(10); r0->setId(20);
	p1->setId(1123); r1->setId(21);
	p2->setId(14); r2->setId(25);
	r3->setId(22);

	p0->addChild(p1); p1->addChild(p2);
	r0->addChild(r1); r1->addChild(r2); r2->addChild(r3);

	p0->setFraction(0); p1->setFraction(0.6); p2->setFraction(0.4);
	r0->setFraction(0); r1->setFraction(0.3); r2->setFraction(0.45); r3->setFraction(0.25);

	BOOST_CHECK(not TreeMerge(p0, r0));

}
#endif
