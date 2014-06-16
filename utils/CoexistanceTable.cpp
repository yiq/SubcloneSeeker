#include "CoexistanceTable.h"
#include <cstring>
#include <cstdlib>

CoexistanceTable::CoexistanceTable(const std::vector<EventCluster>& clusters) 
	: clusterCoexistanceMap(NULL), clusterCoexistanceOnceMap(NULL), caseCount(0), clusterCount(0)
{
	clusterCount = clusters.size();

	clusterCoexistanceMap = (unsigned long**)malloc(sizeof(unsigned long *) * clusterCount);
	for(size_t i=0; i<clusterCount; i++) {
		clusterCoexistanceMap[i] = (unsigned long *)malloc(sizeof(unsigned long) * clusterCount);
		memset(clusterCoexistanceMap[i], 0, sizeof(unsigned long) * clusterCount);
	}

	clusterCoexistanceOnceMap = (bool**)malloc(sizeof(bool*) * clusterCount);
	for(size_t i=0; i<clusterCount; i++) {
		clusterCoexistanceOnceMap[i] = (bool*)malloc(sizeof(bool) * clusterCount);
		memset(clusterCoexistanceOnceMap[i], 0, sizeof(bool) * clusterCount);
	}


	clusterIndexMap.clear();
	std::vector<EventCluster>::const_iterator cit = clusters.cbegin();
	size_t idx = 0;
	for(; cit != clusters.cend(); cit++, idx++)
		clusterIndexMap[*cit] = idx;
}

CoexistanceTable::~CoexistanceTable()
{
	if(clusterCoexistanceMap) {
		for(size_t i=0; i<clusterIndexMap.size(); i++) {
			if(clusterCoexistanceMap[i]) {
				free(clusterCoexistanceMap[i]);
				clusterCoexistanceMap[i] = NULL;
			}
		}
		free(clusterCoexistanceMap);
		clusterCoexistanceMap = NULL;
	}
}

void CoexistanceTable::NewCase()
{
	caseCount++;
	for(size_t i=0; i<clusterCount; i++) {
		memset(clusterCoexistanceOnceMap[i], 0, sizeof(bool) * clusterCount);
	}
}

void CoexistanceTable::ObserveCoexistance(const EventCluster& cluster1, const EventCluster& cluster2)
{
	clusterIndexMapT::const_iterator it1 = clusterIndexMap.find(cluster1);
	clusterIndexMapT::const_iterator it2 = clusterIndexMap.find(cluster2);

	if(it1 == clusterIndexMap.cend() || it2 == clusterIndexMap.cend()) throw new ClusterNotFoundError;
	if(clusterCoexistanceOnceMap[it1->second][it2->second]) throw new ClusterPairAlreadyObservedError; 

	clusterCoexistanceMap[it1->second][it2->second]++;
	clusterCoexistanceMap[it2->second][it1->second]++;
	
	clusterCoexistanceOnceMap[it1->second][it2->second] = true;
	clusterCoexistanceOnceMap[it2->second][it1->second] = true;
}
