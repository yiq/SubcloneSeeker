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

			/**
			 * minimal constructor to reset all member variables
			 */
			GenomicRange() : start(), length(0) {}

			/**
			 * Check if two GenomicRange overlap with each other
			 *
			 * @param another The other GenomicRange to check with
			 * @return true if overlaps, or false
			 */
			virtual inline bool overlaps(GenomicRange& another) {
				//check chrom
				if(another.start.chrom != start.chrom)
					return false;

				//check overlaps
				if( another.start.position + another.length < start.position 
						|| another.start.position > start.position + length )
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
				return (start.chrom == another.start.chrom) && (start.position == another.start.position) && (length == another.length);
			}

			/**
			 * GenomicRange compare operator <
			 *
			 * @param another The other GenomicRange to compare to
			 * @return true if the object takes place before the other object, false if not
			 */
			inline bool operator<(const GenomicRange& another) const {
				if(start.chrom < another.start.chrom) return true;
				if(start.position < another.start.position) return true;
				return false;
			}
			
			/**
			 * GenomicRange compare operator >
			 *
			 * @param another The other GenomicRange to compare to
			 * @return true if the object takes place after the other object, false if not
			 */
			inline bool operator>(const GenomicRange& another) const {
				if(start.chrom > another.start.chrom) return true;
				if(start.position > another.start.position) return true;
				return false;
			}
	};
}

#endif
