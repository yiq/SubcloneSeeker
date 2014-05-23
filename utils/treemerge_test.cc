/**
 * @file treemerge_test.cc
 * Test cases for treemerge logics
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

#include <UnitTest++/src/UnitTest++.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include "treemerge_p.h"

#include "SomaticEvent.h"
#include "SegmentalMutation.h"
#include "EventCluster.h"
#include "TreeNode.h"
#include "Subclone.h"

#define OUTPUT_SEV(sev)													\
	for(size_t i=0; i<(sev).size(); i++) {								\
		std::cout<<dynamic_cast<CNV*>((sev)[i])->range.chrom<<", ";		\
	}																	\
	std::cout<<std::endl;

using namespace SubcloneSeeker;

struct _SampleSubclonesFixture {

	Subclone * tumor1;
	CNV a, b, c, d, e;
	EventCluster cA, cB, cC, cD, cE;

	_SampleSubclonesFixture() {
		a.range.chrom = 1; b.range.chrom = 2; c.range.chrom = 3; d.range.chrom = 4; e.range.chrom = 5;
		cA.addEvent(&a); cB.addEvent(&b); cC.addEvent(&c); cD.addEvent(&d), cE.addEvent(&e);

		Subclone *p0, *p1, *p2, *p3;
		p0 = new Subclone(); p0->setFraction(0.1);
		p1 = new Subclone(); p1->setFraction(0.1); p1->addEventCluster(&cA);
		p2 = new Subclone(); p2->setFraction(0.1); p2->addEventCluster(&cB);
		p3 = new Subclone(); p3->setFraction(0.1); p3->addEventCluster(&cC);

		p0->setId(10); p1->setId(11); p2->setId(12); p3->setId(13);
		p0->addChild(p1); p1->addChild(p2); p1->addChild(p3);
		tumor1 = p0;
	}

	~_SampleSubclonesFixture() {
		delete tumor1;
	}
};

SUITE(TestNodeEventsList) {
	TEST_FIXTURE(_SampleSubclonesFixture, T_nodeEventsList) {
		Subclone *p = tumor1;
		SomaticEventPtr_vec events;
		
		events = nodeEventsList(p);
		CHECK(events.size() == 0);

		p = dynamic_cast<Subclone *>(p->getVecChildren()[0]);

		events = nodeEventsList(p);
		CHECK(events.size() == 1);

		p = dynamic_cast<Subclone *>(p->getVecChildren()[0]);

		events = nodeEventsList(p);
		CHECK(events.size() == 2);

		CHECK(p->getVecChildren().size() == 0);
	}
}

struct _SampleSomaticEventsFixture {
	SomaticEventPtr_vec v1;
	SomaticEventPtr_vec v2;
	SomaticEventPtr_vec v3;
	SomaticEventPtr_vec v4;

	CNV *a, *b, *c, *d, *e;

	_SampleSomaticEventsFixture() {
		a = new CNV(); b = new CNV(); c = new CNV(); d = new CNV(); e = new CNV();
		a->range.chrom = 1; b->range.chrom = 2; c->range.chrom = 3; d->range.chrom = 4; e->range.chrom = 5;

		v1.push_back(a); 
		v1.push_back(b);
		v1.push_back(c);

		v2.push_back(a);
		v2.push_back(d);
		v2.push_back(e);
		
		v3.push_back(d);
		v3.push_back(e);

		v4.push_back(b);
	}

	~_SampleSomaticEventsFixture() {
		delete a;
		delete b;
		delete c;
		delete d;
		delete e;
	}
};

TEST_FIXTURE(_SampleSomaticEventsFixture, T_SomaticEventDifference) {
	SomaticEventPtr_vec diff = SomaticEventDifference(v1, v2);
	CHECK(diff.size() == 2);

	bool found = true;
	found = found && std::find(diff.begin(), diff.end(), a) == diff.end();
	found = found && std::find(diff.begin(), diff.end(), b) != diff.end();
	found = found && std::find(diff.begin(), diff.end(), c) != diff.end();
	CHECK(found);

	diff = SomaticEventDifference(v1, v1);
	CHECK(diff.size() == 0);
}

TEST_FIXTURE(_SampleSomaticEventsFixture, T_eventSetContains) {

	CHECK(eventSetContains(v1, v1));

	CHECK(eventSetContains(v2, v3));
	CHECK(not eventSetContains(v1, v3));

	CHECK(not eventSetContains(v1, v2));
}

TEST_FIXTURE(_SampleSomaticEventsFixture, T_resultSetComparator) {
	std::vector<SomaticEventPtr_vec> testSets;
	testSets.push_back(v1);
	testSets.push_back(v3);
	testSets.push_back(v4);

	std::sort(testSets.begin(), testSets.end(), resultSetComparator);

	CHECK(testSets[0].size() <= testSets[1].size());
	CHECK(testSets[1].size() <= testSets[2].size());
}

struct _placementRtnVal {
	SomaticEventPtr_vec events;
	SomaticEventPtr_vec ppEvents;
	SomaticEventPtr_vec eventsDiff;
	SomaticEventPtr_vec diff;
	bool placable;
	int cp;
};

_placementRtnVal checkPlacementWithNodes(Subclone *pnode, Subclone *rnode) {
	_placementRtnVal rtn;

	rtn.events = nodeEventsList(rnode);
	rtn.ppEvents = nodeEventsList(dynamic_cast<Subclone *>(pnode->getParent()));
	rtn.eventsDiff = SomaticEventDifference(rtn.events, rtn.ppEvents);

	rtn.diff = checkPlacement(pnode, rtn.eventsDiff, &rtn.placable, &rtn.cp);

	return rtn;
}

struct _TestPlacementFixture {
	CNV A, B, C, D, E, F, G, H, I;
	EventCluster cA, cB, cC, cD, cE, cF, cG, cH, cI;
	Subclone p0, p1, p2, p3, p4, p5, p6, p7;
	Subclone r0, r1, r2, r3, r4, r5, r6, r7;

	Subclone pp0, pp1, pp2;
	Subclone rr0, rr1, rr2, rr3;

	struct UPN933124 {
		Subclone p0, p1, p2, p3, p4;
		Subclone r0, r1, r2, r3;
	};

	UPN933124 * upn933124_1;
	UPN933124 * upn933124_2;
	UPN933124 * upn768168_2;

	bool placable;
	int cp;
	Subclone * m_pnode;

	SomaticEventPtr_vec events;
	SomaticEventPtr_vec ppEvents;
	SomaticEventPtr_vec eventsDiff;
	SomaticEventPtr_vec diff;


	_TestPlacementFixture() {
		A.range.chrom = 1; B.range.chrom = 2; C.range.chrom = 3; D.range.chrom = 4;
		E.range.chrom = 5; F.range.chrom = 6; G.range.chrom = 7; H.range.chrom = 8;
		I.range.chrom = 9;

		cA.addEvent(&A); cB.addEvent(&B); cC.addEvent(&C); cD.addEvent(&D);
		cE.addEvent(&E); cF.addEvent(&F); cG.addEvent(&G); cH.addEvent(&H);
		cI.addEvent(&I);

		p1.addEventCluster(&cA); p2.addEventCluster(&cB); p3.addEventCluster(&cG);
		p4.addEventCluster(&cC); p5.addEventCluster(&cD); p6.addEventCluster(&cH);
		p7.addEventCluster(&cI);

		r1.addEventCluster(&cA); r2.addEventCluster(&cB); r2.addEventCluster(&cD);
		r3.addEventCluster(&cE); r4.addEventCluster(&cF); r5.addEventCluster(&cH);
		r6.addEventCluster(&cI); r7.addEventCluster(&cC); r7.addEventCluster(&cD);

		p0.addChild(&p1); p1.addChild(&p2); p1.addChild(&p3);
		p2.addChild(&p4); p2.addChild(&p5); p4.addChild(&p6);
		p3.addChild(&p7);

		r0.addChild(&r1); r1.addChild(&r2); r1.addChild(&r3);
		r2.addChild(&r4); r2.addChild(&r5);
		r3.addChild(&r6); r3.addChild(&r7);


		pp1.addEventCluster(&cA); pp1.addEventCluster(&cB); pp1.addEventCluster(&cC);
		pp2.addEventCluster(&cD);

		rr1.addEventCluster(&cA); rr2.addEventCluster(&cE); rr3.addEventCluster(&cB);

		pp0.addChild(&pp1); pp1.addChild(&pp2);
		rr0.addChild(&rr1); rr1.addChild(&rr2); rr2.addChild(&rr3);

		pp0.setId(10); pp1.setId(11); pp2.setId(12);
		rr0.setId(20); rr1.setId(21); rr2.setId(22); rr3.setId(23);

		upn933124_1 = new UPN933124();
		upn933124_1->p1.addEventCluster(&cA); upn933124_1->p2.addEventCluster(&cB); upn933124_1->p3.addEventCluster(&cC); upn933124_1->p4.addEventCluster(&cD);
		upn933124_1->r1.addEventCluster(&cA); upn933124_1->r2.addEventCluster(&cC); upn933124_1->r2.addEventCluster(&cE); upn933124_1->r3.addEventCluster(&cD);

		upn933124_1->p0.addChild(&upn933124_1->p1); upn933124_1->p1.addChild(&upn933124_1->p2); upn933124_1->p1.addChild(&upn933124_1->p3); upn933124_1->p1.addChild(&upn933124_1->p4);
		upn933124_1->r0.addChild(&upn933124_1->r1); upn933124_1->r1.addChild(&upn933124_1->r2); upn933124_1->r2.addChild(&upn933124_1->r3);

		upn933124_1->p0.setFraction(0); upn933124_1->p1.setFraction(0.08); upn933124_1->p2.setFraction(0.53); upn933124_1->p3.setFraction(0.34); upn933124_1->p4.setFraction(0.05);
		upn933124_1->r0.setFraction(0); upn933124_1->r1.setFraction(0.05); upn933124_1->r2.setFraction(0.03); upn933124_1->r3.setFraction(0.91);

		upn933124_2 = new UPN933124();
		upn933124_2->p1.addEventCluster(&cA); upn933124_2->p2.addEventCluster(&cB); upn933124_2->p3.addEventCluster(&cC); upn933124_2->p4.addEventCluster(&cD);
		upn933124_2->r1.addEventCluster(&cA); upn933124_2->r2.addEventCluster(&cC); upn933124_2->r2.addEventCluster(&cE); upn933124_2->r3.addEventCluster(&cD);

		upn933124_2->p0.addChild(&upn933124_2->p1); upn933124_2->p1.addChild(&upn933124_2->p2); upn933124_2->p1.addChild(&upn933124_2->p3); upn933124_2->p3.addChild(&upn933124_2->p4);
		upn933124_2->r0.addChild(&upn933124_2->r1); upn933124_2->r1.addChild(&upn933124_2->r2); upn933124_2->r2.addChild(&upn933124_2->r3);

		upn933124_2->p0.setFraction(0); upn933124_2->p1.setFraction(0.08); upn933124_2->p2.setFraction(0.53); upn933124_2->p3.setFraction(0.34); upn933124_2->p4.setFraction(0.05);
		upn933124_2->r0.setFraction(0); upn933124_2->r1.setFraction(0.05); upn933124_2->r2.setFraction(0.03); upn933124_2->r3.setFraction(0.91);

		upn768168_2 = new UPN933124();
		upn768168_2->p1.addEventCluster(&cA); upn768168_2->p2.addEventCluster(&cB);
		upn768168_2->r1.addEventCluster(&cA); upn768168_2->r2.addEventCluster(&cB); upn768168_2->r3.addEventCluster(&cC);

		upn768168_2->p0.addChild(&upn768168_2->p1); upn768168_2->p1.addChild(&upn768168_2->p2);
		upn768168_2->r0.addChild(&upn768168_2->r1); upn768168_2->r1.addChild(&upn768168_2->r2); upn768168_2->r2.addChild(&upn768168_2->r3); 

		upn768168_2->p0.setFraction(0); upn768168_2->p1.setFraction(0.08); upn768168_2->p2.setFraction(0.92);
		upn768168_2->r0.setFraction(0); upn768168_2->r1.setFraction(0.42); upn768168_2->r2.setFraction(0.2); upn768168_2->r3.setFraction(0.38);



	}
	~_TestPlacementFixture() {
		delete upn933124_1;
		delete upn933124_2;
	}

	void PrepareTestcase(Subclone * pnode, Subclone * rnode) {
		m_pnode = pnode;
		events = nodeEventsList(rnode);
		ppEvents = nodeEventsList(dynamic_cast<Subclone *>(pnode->getParent()));
		eventsDiff = SomaticEventDifference(events, ppEvents);
	}

	void PerformTestcase() {
		diff = checkPlacement(m_pnode, eventsDiff, &placable, &cp);
	}
};

SUITE(TestPlacement) {
	TEST_FIXTURE(_TestPlacementFixture, T_Leaf) {
		// Placeable by containment, leaf
		PrepareTestcase(&p5, &r4);
		CHECK(events.size() == 4);
		CHECK(ppEvents.size() == 2);
		CHECK(eventsDiff.size() == 2);

		PerformTestcase();

		CHECK(diff.size() == 1);
		CHECK(cp == -1);
		CHECK(placable);

		// Not placable by containment, leaf
		PrepareTestcase(&p6, &r4);
		CHECK(events.size() == 4);
		CHECK(ppEvents.size() == 3);
		CHECK(eventsDiff.size() == 2);

		PerformTestcase();
		CHECK(diff.size() == 2);
		CHECK(cp == -1);
		CHECK(not placable);
	}

	TEST_FIXTURE(_TestPlacementFixture, T_0Child) {
		// Zero child, placable
		PrepareTestcase(&p1, &r3);
		CHECK(events.size() == 2);
		CHECK(ppEvents.size() == 0);
		CHECK(eventsDiff.size() == 2);

		PerformTestcase();

		CHECK(diff.size() == 1);
		CHECK(cp == 0);
		CHECK(placable);

		// Zero child, not placable
		PrepareTestcase(&p1, &r6);
		CHECK(events.size() == 3);
		CHECK(ppEvents.size() == 0);
		CHECK(eventsDiff.size() == 3);

		PerformTestcase();

		CHECK(diff.size() == 1);
		CHECK(cp == 1);
		CHECK(not placable);	
	}

	TEST_FIXTURE(_TestPlacementFixture, T_1Child) {
		// One child, placable
		PrepareTestcase(&p2, &r4);
		CHECK(events.size() == 4);
		CHECK(ppEvents.size() == 1);
		CHECK(eventsDiff.size() == 3);

		PerformTestcase();

		CHECK(diff.size() == 1);
		CHECK(cp == 1);
		CHECK(placable);

		// Zero child, not placable
		PrepareTestcase(&p2, &r5);
		CHECK(events.size() == 4);
		CHECK(ppEvents.size() == 1);
		CHECK(eventsDiff.size() == 3);

		PerformTestcase();

		CHECK(diff.size() == 1);
		CHECK(cp == 1);
		CHECK(not placable);	
	}

	TEST_FIXTURE(_TestPlacementFixture, T_2Child) {
		PrepareTestcase(&p2, &r7);
		CHECK(events.size() == 4);
		CHECK(ppEvents.size() == 1);
		CHECK(eventsDiff.size() == 3);

		PerformTestcase();

		CHECK(diff.size() == 2);
		CHECK(cp > 1);
		CHECK(not placable);
	}

	TEST_FIXTURE(_TestPlacementFixture, T_EX1) {
		PrepareTestcase(&pp0, &rr1);
		PerformTestcase();
		CHECK(cp == 0);
		CHECK(placable);
		
		PrepareTestcase(&pp0, &rr2);
		PerformTestcase();
		CHECK(placable);
		
		PrepareTestcase(&pp0, &rr3);
		PerformTestcase();
		CHECK(not placable);
	}
}


#pragma mark --Sample Tumors--

SUITE(SampleTumors) {
	// 0 == 0
	TEST(test_tree_merge_0_0) {
		Subclone *clone1 = new Subclone();
		Subclone *clone2 = new Subclone();

		clone1->setFraction(1);
		clone2->setFraction(1);

		CHECK(TreeMerge(clone1, clone2));

		delete clone1;
		delete clone2;
	}

	// 0 == A
	TEST(test_tree_merge_0_a) {
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

		CHECK(TreeMerge(clone1, clone2));
		delete clone1;
		delete clone2;
	}

	// A == A
	TEST(test_tree_merge_a_a) {
		CNV *cnv = new CNV();
		cnv->range.chrom = 1;
		cnv->range.position = 100000L;
		cnv->range.length = 100;
		cnv->frequency = 0.4;

		EventCluster *cluster1 = new EventCluster();
		EventCluster *cluster2 = new EventCluster();

		cluster1->addEvent(cnv);
		cluster2->addEvent(cnv);

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

		CHECK(TreeMerge(clone1, clone2));

		delete clone1;
		delete clone2;

	}

	// A ~= A'
	TEST(test_tree_merge_a_a1) {
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

		CHECK(cnv1->isEqualTo(cnv2, BOUNDRY_RESOLUTION));

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


		CHECK(TreeMerge(clone1, clone2));

		delete clone1;
		delete clone2;
		delete clone3;
		delete clone4;

	}

	// 0, (A, (B, (C))) != 0, (A, (B, C))
	TEST(test_tree_merge_3_1) {
		CNV A, B, C;
		A.range.chrom = 1;
		B.range.chrom = 2;
		C.range.chrom = 3;

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


		_placementRtnVal rtn;
		rtn = checkPlacementWithNodes(p0, rA);
		CHECK(rtn.placable);

		rtn = checkPlacementWithNodes(p0, rB);
		CHECK(rtn.placable);

		rtn = checkPlacementWithNodes(p0, rC);
		CHECK(not rtn.placable);



		CHECK(not TreeMerge(p0, r0));
	}

	// 0, (A, (B, (C))) == 0, (A, (B, D))
	TEST(test_tree_merge_3_2) {
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

		CHECK(TreeMerge(p0, r0));

	}

	// 0, (A, (B, (C))) == 0, (AD, E)
	TEST(test_tree_merge_3_3) {
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

		CHECK(TreeMerge(p0, r0));
	}

	// 0, (A, (B, (C))) != 0, (ACD)
	TEST(test_tree_merge_3_4) {
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

		CHECK(not TreeMerge(p0, r0));

	}

	// aml1
	TEST( test_tree_merge_aml1_1 ) {
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


		CHECK(not TreeMerge(p0, r0));
	}

	// UPN426980-rel-1
	TEST( test_tree_merge_upn426 ) {
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

		CHECK(TreeMerge(p0, r0));

	}

	// UPN426980-rel-2
	TEST( test_tree_merge_upn426_2 ) {
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

		CHECK(not TreeMerge(p0, r0));
	}

	TEST_FIXTURE(_TestPlacementFixture, test_upn933124_1) {
		PrepareTestcase(&upn933124_1->p1, &upn933124_1->r1);
		PerformTestcase();
		CHECK(cp == 0);
		CHECK(placable);

		PrepareTestcase(&upn933124_1->p1, &upn933124_1->r2);
		PerformTestcase();
		CHECK(cp == 1);
		CHECK(placable);

		PrepareTestcase(&upn933124_1->p1, &upn933124_1->r3);
		PerformTestcase();
		CHECK(not placable);

		CHECK(not TreeMerge(&upn933124_1->p0, &upn933124_1->r0));
	}

	TEST_FIXTURE(_TestPlacementFixture, test_upn933124_2) {
		PrepareTestcase(&upn933124_2->p1, &upn933124_2->r1);
		PerformTestcase();
		CHECK(cp == 0);
		CHECK(placable);

		PrepareTestcase(&upn933124_2->p3, &upn933124_2->r2);
		PerformTestcase();
		CHECK(cp == 0);
		CHECK(placable);

		PrepareTestcase(&upn933124_2->p3, &upn933124_2->r3);
		PerformTestcase();
		std::cout<<cp<<std::endl;
		CHECK(not placable);
	}

	TEST_FIXTURE(_TestPlacementFixture, test_upn933124_3) {
		PrepareTestcase(&upn933124_2->p1, &upn933124_2->r1);
		PerformTestcase();
		CHECK(cp == 0);
		CHECK(placable);

		PrepareTestcase(&upn933124_2->p3, &upn933124_2->r3);
		PerformTestcase();
		std::cout<<cp<<std::endl;
		CHECK(cp==1);
		CHECK(placable);
	}

	TEST_FIXTURE(_TestPlacementFixture, test_upn758168_2) {
		CHECK(TreeMerge(&upn768168_2->p0, &upn768168_2->r0));
	}
}


int main() {
	return UnitTest::RunAllTests();
}
