/**
 * @file treemerge_p.cc
 * The implementation file for the implementation part of 'treemerge'
 *
 * @author Yi Qiao
 */

#include "treemerge_p.h"
#include <assert.h>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "SomaticEvent.h"
#include "SegmentalMutation.h"
#include "EventCluster.h"
#include "Subclone.h"

static SubclonePtr_vec extrudeNodeList;
static int extSubId = 500;

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

// Check if a somatic event vector contains all the events found in another vector
bool eventSetContains(const SomaticEventPtr_vec& v_container, const SomaticEventPtr_vec& v_containee) {
	
	// If the container vector is smaller than the containee vector,
	// there is no way for the check to be true.
	if(v_container.size() < v_containee.size())
		return false;

	// Loop through all the events in the containee vector, and check to see if it's contained
	for(size_t i=0; i<v_containee.size(); i++) {
		bool elementIsContained = false;
		for(size_t j=0; j<v_container.size(); j++) {
			// If the current element is contained, checks the next element
			if(v_container[j]->isEqualTo(v_containee[i], BOUNDRY_RESOLUTION)) {
				elementIsContained = true;
				break;
			}
		}

		// If the current element is not contained return false
		if(!elementIsContained) {
			return false;
		}
	}

	return true;
}

// Compare SomaticEvent vectors by size
bool resultSetComparator(const SomaticEventPtr_vec& v1, const SomaticEventPtr_vec &v2) {
	return v1.size() < v2.size();
}

// Check if a node with certain events can be placed on a subtree
SomaticEventPtr_vec checkPlacement(Subclone *pnode, SomaticEventPtr_vec somaticEvents, bool * placeableOnSubtree) {
	SomaticEventPtr_vec pnodeEvents;
	bool didPassContainment = true;

	// All the events in pnode
	for(size_t i=0; i<pnode->vecEventCluster().size(); i++) {
		for(size_t j=0; j<pnode->vecEventCluster()[i]->members().size(); j++) {
			pnodeEvents.push_back(pnode->vecEventCluster()[i]->members()[j]);
		}
	}

	// if pnode is not completely contained by somaticEvent, it cannot be placed under pnode
	if(!eventSetContains(somaticEvents, pnodeEvents)) {
		didPassContainment = false;
	}

	SomaticEventPtr_vec eventDiff = SomaticEventDifference(somaticEvents, pnodeEvents);

	if(pnode->isLeaf()) {
		// if passed containment test and this is a leaf, it's placeable
		if(didPassContainment) *placeableOnSubtree = true;
		// or, if this is a leaf but not contained, it's unplacable
		else *placeableOnSubtree = false;

		return(eventDiff);
	}
	
	// Leaf node won't make it so far, so at least one children is present
	assert(pnode->getVecChildren().size() > 0);

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

			// If no children contained any events in somaticEvents
			if(isCheckedOut && didPassContainment) {
				*placeableOnSubtree = true;

				/*
				// check if the relapse is being placed on a extruded node
				if(std::find(extrudeNodeList.begin(), extrudeNodeList.end(), pnode) != extrudeNodeList.end()) {

					// if yes, create a dummy subclone that represents the change in topology
					Subclone * relExtNode = new Subclone();
					relExtNode->setId(extSubId++);
					EventCluster * relExtCluster = new EventCluster();
					SomaticEventPtr pnodeEventsSoFar = nodeEventsList(pnode);
					SomaticEventPtr eventsForRelapse = SomaticEventDifference(somaticEvents, pnodeEventsSoFar)
					for(size_t i=0; i<eventsForRelapse.size(); i++) {
						relExtCluster->addEvent(eventsForRelapse[i]);
					}
					relExtNode->addEventCluster(relExtCluster);
					relExtNode->setFraction(0.1);
					if(somaticEvents.size() > 0)
						pnode->addChild(relExtNode);
				}
				*/
			}
			else if (didPassContainment) {
				// But before quitting, a attempt to find a hidden node should be carried out. This is done by finding all children
				// nodes that:
				//   1. contains a subset of events in the float node that are also shared by other children nodes
				//   2. does not contain any events not in the "shared" event set
				//
				// Right now only nodes with one child are considered, as this is a relatively simple case
				if(pnode->getVecChildren().size() > 0) {
					// if the symbols not contained by the child is also not found anywhere down the tree, those events shared
					// by the child can be extruded.

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
					
					SubclonePtr_vec extruableChildren;
					SomaticEventPtr_vec extrudeEvents;
					bool extruable = true;
					for(size_t p=0; p<pnode->getVecChildren().size(); p++) {
						Subclone *pExtNode = dynamic_cast<Subclone *>(pnode->getVecChildren()[p]);
						SomaticEventPtr_vec childEvents = nodeEventsList(dynamic_cast<Subclone *>(pExtNode));
						SomaticEventPtr_vec thisExtrudeEvents = SomaticEventDifference(childEvents, SomaticEventDifference(childEvents, eventDiff));
						SomaticEventPtr_vec thisUniqueEvents = SomaticEventDifference(eventDiff, thisExtrudeEvents);

						if(thisExtrudeEvents.size() > 0) {
							if(thisUniqueEvents.size() == childEventDiffSet[p].size() && eventSetContains(thisUniqueEvents, childEventDiffSet[p])) {
								if(extrudeEvents.size() == 0) {
									extrudeEvents = thisExtrudeEvents;
									extruableChildren.push_back(pExtNode);
								}
								else {

									if(extrudeEvents.size() == thisExtrudeEvents.size() && eventSetContains(extrudeEvents, thisExtrudeEvents) ) {
										extruableChildren.push_back(pExtNode);
									}
									else {
										extruable = false;
										break;
									}
								}
							} else {
								extruable = false;
								break;
							}
						}
					}

					if(extruableChildren.size() == 0)
						extruable = false;

					SomaticEventPtr_vec uniqueEvents = SomaticEventDifference(eventDiff, extrudeEvents);

					if(extruable) {
						*placeableOnSubtree = true;

						// Create the extruded subclone
						Subclone * extrudedSubclone = new Subclone();
						// Aggregate the extruded events into one cluster, and put it into the new subclone
						EventCluster *extrudedCluster = new EventCluster();
						for(size_t i=0; i<extrudeEvents.size(); i++) {
							extrudedCluster->addEvent(extrudeEvents[i], false);
						}
						// Set the fraction of the extruded subclone to 0
						extrudedCluster->setCellFraction(0);
						extrudedSubclone->addEventCluster(extrudedCluster);
						extrudedSubclone->setFraction(0);
						// Remove the extruded events from the current node
						// Note: a simple implementation is to remove any cluster
						// which contains any events found in the extruded event list
						// This is ok if an entire cluster will always be extruded away
						for(int p=0; p<extruableChildren.size(); p++) {
							Subclone * pExtNode = dynamic_cast<Subclone *>(extruableChildren[p]);
							for(int i=0; i<pExtNode->vecEventCluster().size(); i++) {
								bool found = false;
								for(size_t j=0; j<extrudeEvents.size(); j++) {
									// check if the j-th event is present in the i-th cluster
									for(size_t k=0; k<pExtNode->vecEventCluster()[i]->members().size(); k++) {
										if(pExtNode->vecEventCluster()[i]->members()[k]->isEqualTo(extrudeEvents[j])) {
											found = true;
											break;
										}
									}
									if(found)
										break;
								}

								if(found) {
									pExtNode->vecEventCluster().erase(pExtNode->vecEventCluster().begin() + i);
									i--;
								}
							}
							pnode->removeChild(pExtNode);
							extrudedSubclone->addChild(pExtNode);
						}
						// Change the tree structure
						pnode->addChild(extrudedSubclone);
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
				*placeableOnSubtree = true;
			}
			break;
		default:
			// if more than one children nodes are found to be able to contain eventDiff, none of them actually can without
			// violating the tree structure. In this case, the status returned is false, and the minimal set is returned
			// so that the node cannot be placed as a new child either
			break;
	}

	return(childEventDiffSet[0]);
}

// Traverse the secondary tree, and try to place every node it encounters
// onto the primary tree, which was given as a constructor parameter
class TreeMergeTraverseSecondary : public TreeTraverseDelegate {
	protected:
		Subclone *_proot;

	public:
		bool isCompatible;

		TreeMergeTraverseSecondary(Subclone *proot): TreeTraverseDelegate(), _proot(proot), isCompatible(true) {;}

		void processNode(TreeNode *node) {
			bool placeable;
			Subclone *wp = dynamic_cast<Subclone *>(node);

			// If the given node is NULL, or is not a Subclone node, skip it.
			if(wp == NULL) return;

			// If the node is relapse's root, skip it.
			if(node->isRoot()) return;

			// If the subclone is very small in fraction, skip it.
			if(fabs(wp->fraction()) < 1e-2) return;

			SomaticEventPtr_vec subcloneEvents = nodeEventsList(wp);

			SomaticEventPtr_vec diff = checkPlacement(_proot, subcloneEvents, &placeable);
			if(!placeable) {
				isCompatible = false;
				terminate();
			}
		}
};


// Check if two trees are compatible
bool TreeMerge(Subclone *p, Subclone *q) {
		TreeMergeTraverseSecondary secondaryTraverser(p);
	TreeNode::PreOrderTraverse(q, secondaryTraverser);
	if(secondaryTraverser.isCompatible) {
		std::cout<<"Primary SubcTree "<<p->getId()<<" is compatible with Secondary SubcTree "<<q->getId()<<std::endl;
		return true;
	}

	return false;
}
