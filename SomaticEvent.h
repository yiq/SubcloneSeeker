#ifndef SOMATIC_EVENT_H
#define SOMATIC_EVENT_H

/**
 * @file SomaticEvent.h
 * Interface description of data structure class SomaticEvent
 * 
 * @author Yi Qiao
 */

namespace SubcloneExplorer {

	/**
	 * @brief Abstract data structure class that represents any type of somatic mutation
	 *
	 * The most important aspect of the mutations represented by this class
	 * is that it has an cell frequency associated, which describes in what
	 * fraction of the cells does this specific event exist
	 */
	class SomaticEvent {
		public:
			double frequency; /**< Cell frequency */
	};
}

#endif
