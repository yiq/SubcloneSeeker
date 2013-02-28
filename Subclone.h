#ifndef SUBCLONE_H
#define SUBCLONE_H

/**
 * @file Subclone.h
 * Interface description of the data structure class Subclone
 *
 * @author Yi Qiao
 */

#include "TreeNode.h"
#include <vector>

namespace SubcloneExplorer {

	// forward declaration of EventCluster, so that pointers can be made
	class EventCluster;
	
	/**
	 * @brief The class that represents a subclone in a subclonal structure tree
	 *
	 * This class implements both the TreeNode and Archivable interfaces, so that it
	 * can be a node in a tree structure, and at the same time be archived into a 
	 * sqlite3 database. This is the fundamental building block of subclonal
	 * deconvoution solutions.
	 */
	class Subclone : public TreeNode {
		protected:
			double _fraction; /**< The percentage of this subclone */
			double _treeFraction; /**< The total fraction taken by the subtree rooted by this object */
			std::vector<EventCluster *> _eventClusters; /**< the event clusters this subclone contains */

		public:
			/**
			 * Minimal constructor to reset all member variables
			 */
			Subclone() : TreeNode(), _fraction(0), _treeFraction(0) {;}

			/**
			 * return the fraction
			 * @return percentage of this subclone
			 */
			inline double fraction() const {return _fraction;}

			/**
			 * update the fraction
			 * @param fraction The new fraction value
			 */
			inline void setFraction(double fraction) { /*if (fraction >=0 && fraction <=1)*/ _fraction = fraction; }

			/**
			 * return the tree fraction
			 * @return percentage of the subtree rooted by this subclone
			 */
			inline double treeFraction() const {return _treeFraction;}

			/**
			 * update the tree fraction
			 * @param fraction The new tree fraction value
			 */
			inline void setTreeFraction(double fraction) { /*if (fraction >=0 && fraction <=1)*/ _treeFraction = fraction; }

			/**
			 * retreve a vector of all EventClusters this subclone has
			 *
			 * @return member EventCluster vector
			 */
			inline std::vector<EventCluster *> &vecEventCluster() {return _eventClusters;}

			/**
			 * Add a given EventCluster into the subclone
			 *
			 * @param cluster The EventCluster to be added to the subclone
			 */
			void addEventCluster(EventCluster *cluster);
	};
}

#endif
