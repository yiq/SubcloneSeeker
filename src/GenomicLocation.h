#ifndef GENOMIC_LOCATION_H
#define GENOMIC_LOCATION_H

/**
 * @file GenomicLocation.h
 * Interface description of the helper class GenomicLocation
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

			/**
			 * minimal constructor to reset all member variables
			 */
			GenomicLocation() : chrom(0), position(0) {}

			/**
			 * GenomicLocation compare operator <
			 *
			 * @param another The other GenomicLocation to compare to
			 * @return true if the object takes place before the other object, false if not
			 */
			inline virtual bool operator<(const GenomicLocation& another) const {
				if(chrom < another.chrom) return true;
				if(chrom > another.chrom) return false;
				if(position < another.position) return true;
				return false;
			}
			
			/**
			 * GenomicLocation compare operator >
			 *
			 * @param another The other GenomicLocation to compare to
			 * @return true if the object takes place after the other object, false if not
			 */
			inline virtual bool operator>(const GenomicLocation& another) const {
				if(chrom > another.chrom) return true;
				if(chrom < another.chrom) return false;
				if(position > another.position) return true;
				return false;
			}
	};
}
#endif
