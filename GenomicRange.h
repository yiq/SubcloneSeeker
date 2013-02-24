#ifndef GENOMIC_RANGE_H
#define GENOMIC_RANGE_H

#include "GenomicLocation.h"

/**
 * @file GenomicRange.h
 * Interface description of the helper class GenomicRange
 *
 * @author Yi Qiao
 */

namespace SubcloneExplorer {

	/**
	 * @brief A segment on a reference genome
	 *
	 * This class represents a segment on a (implied) reference genome.
	 * It contains a start position described by GenomicLocation, and 
	 * a greater than 1 length. In this way a segment will never cross
	 * a chromosome boundry.
	 *
	 * @see GenomicLocation
	 */
	class GenomicRange {
		public:
			GenomicLocation start;	/**< The starting position of the segment */
			unsigned long length;	/**< The length of the segment */
	};
}

#endif
