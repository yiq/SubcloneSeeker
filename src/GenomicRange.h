#ifndef GENOMIC_RANGE_H
#define GENOMIC_RANGE_H

/**
 * @file GenomicRange.h
 * Interface description of the helper class GenomicRange
 *
 * @author Yi Qiao
 */

/*
The MIT License (MIT)

Copyright (c) 2013 Yi Qiao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "GenomicLocation.h"

namespace SubcloneSeeker {

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
