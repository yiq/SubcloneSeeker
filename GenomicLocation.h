#ifndef GENOMIC_LOCATION_H
#define GENOMIC_LOCATION_H

/**
 * @file GenomicLocation.h
 * Interface description of the helper class GenomicLocation
 *
 * @author Yi Qiao
 */

namespace SubcloneExplorer {

	/**
	 * @brief A point on a reference genome
	 *
	 * This class represents a point location on a (implied) reference genome.
	 * It consists of a chromosome id, and a position.
	 *
	 * The position is 0 based
	 */
	class GenomicLocation {
		public:
			int chrom; /**< The integer id of the chromosome */
			unsigned long position; /**< The 0-based position */
	};
}
#endif
