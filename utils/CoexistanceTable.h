#ifndef COEXISTANCETABLE_H
#define COEXISTANCETABLE_H

#include "EventCluster.h"
#include <map>

using SubcloneSeeker::EventCluster;

class CoexistanceTable {

	typedef std::map<EventCluster, size_t> clusterIndexMapT;

	protected:
		clusterIndexMapT clusterIndexMap;
		unsigned long **clusterCoexistanceMap;
		bool **clusterCoexistanceOnceMap;
		unsigned long caseCount;
		size_t clusterCount;

	public:
		CoexistanceTable(const std::vector<EventCluster>& clusters);
		~CoexistanceTable();

		void NewCase();
		void ObserveCoexistance(const EventCluster& cluster1, const EventCluster& cluster2);

		class ClusterNotFoundError {};
		class ClusterPairAlreadyObservedError {};
};

#endif
