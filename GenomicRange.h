#ifndef GENOMIC_RANGE_H
#define GENOMIC_RANGE_H

#include "GenomicLocation.h"

/**
 * @file GenomicRange.h
 * Interface description of the helper class GenomicRange
 *
 * @author Yi Qiao
 */

#include "GenomicLocation.h"

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
	class GenomicRange : public GenomicLocation {
		public:
			unsigned long length;	/**< The length of the segment */

			/**
			 * minimal constructor to reset all member variables
			 */
			GenomicRange() : GenomicLocation(), length(0) {}

			/**
			 * Check if two GenomicRange overlap with each other
			 *
			 * @param another The other GenomicRange to check with
			 * @return true if overlaps, or false
			 */
			virtual inline bool overlaps(GenomicRange& another) {
				//check chrom
				if(another.chrom != chrom)
					return false;

				//check overlaps
				if( another.position + another.length < position 
						|| another.position > position + length )
					return false;
				return true;
			}

			/**
			 * GenomicRange compare operator ==
			 *
			 * @param another The other GenomicRange to compare to
			 * @return true if they are equal, false if not
			 */
			inline bool operator==(const GenomicRange& another) const {
				return (chrom == another.chrom) && (position == another.position) && (length == another.length);
			}
	};
}

#endif
