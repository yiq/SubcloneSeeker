/**
 * @file Subclone.cc
 * Implementation of class Subclone
 * 
 * @author Yi Qiao
 */

#include "Subclone.h"

using namespace SubcloneExplorer;

void Subclone::addEventCluster(EventCluster *cluster) {
	bool alreadyExist = false;

	for(size_t i=0; i<_eventClusters.size(); i++) {
		if(_eventClusters[i] == cluster) {
			alreadyExist = true;
			break;
		}
	}

	if(!alreadyExist) {
		_eventClusters.push_back(cluster);
	}
}
