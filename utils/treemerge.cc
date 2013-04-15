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

#define BOUNDRY_RESOLUTION 20000000L

using namespace SubcloneExplorer;

SubclonePtr_vec extrudeNodeList;
static int extSubId = 500;

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


//helper functions
SomaticEventPtr_vec nodeEventsList(Subclone * node) {
	SomaticEventPtr_vec subcloneEvents;
	Subclone *wp = dynamic_cast<Subclone *>(node);
	while(wp != NULL) {
		for(size_t i=0; i<wp->vecEventCluster().size(); i++) {
			for(size_t j=0; j<wp->vecEventCluster()[i]->members().size(); j++)
				subcloneEvents.push_back(wp->vecEventCluster()[i]->members()[j]);
		}
		wp = dynamic_cast<Subclone *>(wp->getParent());
	}
	return subcloneEvents;
}
std::string nodeEvents(Subclone * node) {
	std::stringstream buffer;
	SomaticEventPtr_vec subcloneEvents;
	Subclone *wp = dynamic_cast<Subclone *>(node);
	while(wp != NULL) {
		for(size_t i=0; i<wp->vecEventCluster().size(); i++) {
			for(size_t j=0; j<wp->vecEventCluster()[i]->members().size(); j++)
				buffer<<dynamic_cast<CNV*>(wp->vecEventCluster()[i]->members()[j])->range.chrom<<",";
		}
		wp = dynamic_cast<Subclone *>(wp->getParent());
	}
	return buffer.str();
}

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

	if(v_container.size() < v_containee.size())
		return false;

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
	std::cerr<<"SomaticEvents:";
	for(size_t i=0; i<somaticEvents.size(); i++)
		std::cerr<<dynamic_cast<CNV*>(somaticEvents[i])->range.chrom<<",";
	std::cerr<<std::endl;
	
	if(pnode->vecEventCluster().size() > 0)
		std::cerr<<"trying to place under node "<<pnode->getId()<<" ";
	else
		std::cerr<<"trying to place under root ";
	std::cerr<<":"<<nodeEvents(pnode)<<std::endl;
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

	// Leaf node won't make it so far, so at least one children is present

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
			//
			for(size_t i=0; i<childEventDiffSet.size(); i++) {
				if(childEventDiffSet[i].size() != eventDiff.size() || !eventSetContains(eventDiff, childEventDiffSet[i])) {
					isCheckedOut = false;
					break;
				}
			}

			
			if(isCheckedOut && didPassContainment) {
#ifdef TREEMERGE_TEST_VERBOSE
				std::cerr<<"success! (no-children) "<<pnode->getId()<<std::endl;
				std::cerr<<"pnode addr: "<<pnode<<std::endl;
#endif
				*placeableOnSubtree = true;

				// check if the relapse is being placed on a extruded node
				if(std::find(extrudeNodeList.begin(), extrudeNodeList.end(), pnode) != extrudeNodeList.end()) {
#ifdef TREEMERGE_TEST_VERBOSE
					std::cerr<<"Relapse node being placed under an extruded node!"<<std::endl;
#endif
					Subclone * relExtNode = new Subclone();
					relExtNode->setId(extSubId++);
					EventCluster * relExtCluster = new EventCluster();
#ifdef TREEMERGE_TEST_VERBOSE
					std::cerr<<"Fixating relapse node onto primary with events: ";
#endif
					for(size_t i=0; i<somaticEvents.size(); i++) {
						relExtCluster->addEvent(somaticEvents[i]);
#ifdef TREEMERGE_TEST_VERBOSE
						std::cerr<<dynamic_cast<CNV*>(somaticEvents[i])->range.chrom<<",";
#endif
					}
#ifdef TREEMERGE_TEST_VERBOSE
					std::cerr<<std::endl;
#endif
					relExtNode->addEventCluster(relExtCluster);
					relExtNode->setFraction(0.1);
					if(somaticEvents.size() > 0)
						pnode->addChild(relExtNode);
				}
			}
			else if (didPassContainment) {
#ifdef TREEMERGE_TEST_VERBOSE
				std::cerr<<"failed (no-children)"<<std::endl;
#endif

				// But before quitting, a attempt to find a hidden node should be carried out. This is done by finding all children
				// nodes that:
				//   1. contains a subset of events in the float node that are also shared by other children nodes
				//   2. does not contain any events not in the "shared" event set
				//
				// Right now only node with one childare considered, as this is a relatively simple case
				if(pnode->getVecChildren().size() == 1) {
					// if the symbols not contained by the child is also not found anywhere down the tree, those events shared
					// by the child can be extruded.
					Subclone * pExtNode = dynamic_cast<Subclone *>(pnode->getVecChildren()[0]);
#ifdef TREEMERGE_TEST_VERBOSE
					std::cerr<<"Attempting to extrude on pExtNode "<<pExtNode->getId()<<" : "<<nodeEvents(pExtNode)<<std::endl;
#endif

					// To test extrudability, three set of symbols are needed
					// 1. Floating relapse node - current primary, which is the [eventDiff] set
					// 2. Events contained in the child. [eventChild]
					// 3. The shortest unconsumed event list, which is [childEventDiffSet[0]]
					//
					// A hidden node is in between the current node and its child node only if
					//   [eventDiff] - [eventChild] is not found anywhere on any subtrees
					// This is the same as testing whether [eventDiff] - [eventChild] == childEventDiffSet[0]
					//
					// The hidden node should contain those events that are shared by the floating relapse
					// and the children, which is
					// [eventChild] - ([eventChild] - eventDiff)

					SomaticEventPtr_vec childEvents = nodeEventsList(dynamic_cast<Subclone *>(pExtNode));
					SomaticEventPtr_vec uniqueEvents = SomaticEventDifference(eventDiff, childEvents);
					SomaticEventPtr_vec extrudeEvents = SomaticEventDifference(childEvents, SomaticEventDifference(childEvents, eventDiff));


#ifdef TREEMERGE_TEST_VERBOSE
					std::cerr<<"before extrudable test"<<std::endl;
					std::cerr<<"eventDiff: ";
					for(size_t i=0; i<eventDiff.size(); i++)
						std::cerr<<dynamic_cast<CNV*>(eventDiff[i])->range.chrom<<",";
					std::cerr<<std::endl;

					std::cerr<<"childEvent: ";
					for(size_t i=0; i<childEvents.size(); i++)
						std::cerr<<dynamic_cast<CNV*>(childEvents[i])->range.chrom<<",";
					std::cerr<<std::endl;


					std::cerr<<"Child EventDiff [0]: ";
					for(size_t i=0; i<childEventDiffSet[0].size(); i++)
						std::cerr<<dynamic_cast<CNV*>(childEventDiffSet[0][i])->range.chrom<<",";
					std::cerr<<std::endl;

					std::cerr<<"uniqueEvents: ";
					for(size_t i=0; i<uniqueEvents.size(); i++)
						std::cerr<<dynamic_cast<CNV*>(uniqueEvents[i])->range.chrom<<",";
					std::cerr<<std::endl;

					std::cerr<<"extrudeEvents: ";
					for(size_t i=0; i<extrudeEvents.size(); i++)
						std::cerr<<dynamic_cast<CNV*>(extrudeEvents[i])->range.chrom<<",";
					std::cerr<<std::endl;


#endif

					if(uniqueEvents.size() == childEventDiffSet[0].size() && eventSetContains(uniqueEvents, childEventDiffSet[0])) {
#ifdef TREEMERGE_TEST_VERBOSE
						std::cerr<<"the child can be extruded"<<std::endl;
#endif
						*placeableOnSubtree = true;

						// Create the extruded subclone
						Subclone * extrudedSubclone = new Subclone();
						// Aggregate the extruded events into one cluster, and put it into the new subclone
						EventCluster *extrudedCluster = new EventCluster();
#ifdef TREEMERGE_TEST_VERBOSE
						std::cerr<<"EXTRUDE ITEMS:";
#endif
						for(size_t i=0; i<extrudeEvents.size(); i++) {
							extrudedCluster->addEvent(extrudeEvents[i], false);
#ifdef TREEMERGE_TEST_VERBOSE
							std::cerr<<dynamic_cast<CNV*>(extrudeEvents[i])->range.chrom<<", ";
#endif
						}
#ifdef TREEMERGE_TEST_VERBOSE
						std::cerr<<std::endl;
#endif
						// Set the fraction of the extruded subclone to 0
						extrudedCluster->setCellFraction(0);
						extrudedSubclone->addEventCluster(extrudedCluster);
						extrudedSubclone->setFraction(0);
						// Remove the extruded events from the current node
						// Note: a simple implementation is to remove any cluster
						// which contains any events found in the extruded event list
						// This is ok if an entire cluster will always be extruded away
						for(int i=0; i<pExtNode->vecEventCluster().size(); i++) {
							bool found = false;
							for(size_t j=0; j<extrudeEvents.size(); j++) {
								// check if the j-th event is present in the i-th cluster
								for(size_t k=0; k<pExtNode->vecEventCluster()[i]->members().size(); k++) {
									if(pExtNode->vecEventCluster()[i]->members()[k]->isEqualTo(extrudeEvents[j])) {
#ifdef TREEMERGE_TEST_VERBOSE
										std::cerr<<"EXT ITEM "<<dynamic_cast<CNV*>(pExtNode->vecEventCluster()[i]->members()[k])->range.chrom<<" FOUND"<<std::endl;
#endif
										found = true;
										break;
									}
								}
								if(found)
									break;
							}

							if(found) {
#ifdef TREEMERGE_TEST_VERBOSE
								std::cerr<<"ITEM "<<dynamic_cast<CNV*>(pExtNode->vecEventCluster()[i]->members()[0])->range.chrom<<" REMOVED on pExtNode "<<pExtNode->getId()<<std::endl;
#endif
								pExtNode->vecEventCluster().erase(pExtNode->vecEventCluster().begin() + i);
								i--;
							}
						}
						// Change the tree structure
						pnode->addChild(extrudedSubclone);
						pnode->removeChild(pExtNode);
						extrudedSubclone->addChild(pExtNode);
						extrudeNodeList.push_back(extrudedSubclone);

						// Also the merged relapse tree needs to be recorded to prevent future incorrect extrusion
						Subclone * relExtNode = new Subclone();
						relExtNode->setId(extSubId++);
						EventCluster * relExtCluster = new EventCluster();
						for(size_t i=0; i<uniqueEvents.size(); i++) {
							relExtCluster->addEvent(uniqueEvents[i]);
						}
						relExtNode->addEventCluster(relExtCluster);
						relExtNode->setFraction(0.1);
						if(uniqueEvents.size() > 0)
							extrudedSubclone->addChild(relExtNode);



#ifdef TREEMERGE_TEST_VERBOSE
						std::cerr<<"Extruded node "<<extrudedSubclone->getId()<<":"<<nodeEvents(extrudedSubclone)<<std::endl;
#endif
					}
					else {
#ifdef TREEMERGE_TEST_VERBOSE
						std::cerr<<"pExtNode "<<pExtNode->getId()<<" cannot be extruded to contain relapse node"<<std::endl;
#endif
					}
				}
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

				if(node->isRoot()) {
#ifdef TREEMERGE_TEST_VERBOSE
					std::cerr<<"relapse root, skipping"<<std::endl;
#endif
					return;				
				}

				Subclone *wp = dynamic_cast<Subclone *>(node);
				
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
				wp = dynamic_cast<Subclone *>(node);
				if(fabs(wp->fraction()) < 1e-2) {
#ifdef TREEMERGE_TEST_VERBOSE
					std::cerr<<"0 fraction, skipping"<<std::endl;
#endif
					return;
				}

				
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

