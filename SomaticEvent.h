#ifndef SOMATIC_EVENT_H
#define SOMATIC_EVENT_H

/**
 * @file SomaticEvent.h
 * Interface description of data structure class SomaticEvent
 * 
 * @author Yi Qiao
 */

#include "Archivable.h"
#include <vector>

namespace SubcloneExplorer {

	/**
	 * @brief Abstract data structure class that represents any type of somatic mutation
	 *
	 * The most important aspect of the mutations represented by this class
	 * is that it has an cell frequency associated, which describes in what
	 * fraction of the cells does this specific event exist
	 */
	class SomaticEvent : public Archivable {
		protected:
			// Implements Archivable
			virtual std::string createTableStatementStr();

			sqlite3_int64 ofClusterID; /**< to which cluster in database does this event belongs */

		public:
			double frequency; /**< Cell frequency */

			/**
			 * minimal constructor to reset member variables
			 */
			SomaticEvent(): Archivable(), ofClusterID(0), frequency(0) {;}

			/**
			 * Set the cluster's database id to which this event belongs
			 *
			 * @param clusterID The cluster's database record id
			 */
			inline void setClusterID(sqlite3_int64 clusterID) {ofClusterID = clusterID;}

	};

	/**
	 * A vector of SomaticEvent Pointers
	 */
	typedef std::vector<SomaticEvent *> SomaticEventPtr_vec;
}

#endif
